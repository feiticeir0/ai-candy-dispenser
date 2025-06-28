/*
 * Nome do ficheiro: candy_dispenser.ino
 * Autor: Bruno Ricardo Santos - feiticeir0@whatgeek.com.pt (c) 2025
 * Licença: GNU General Public License v3.0 (GPLv3)
 *
 * Este programa é software livre: pode redistribuí-lo e/ou modificá-lo
 * sob os termos da Licença Pública Geral GNU conforme publicada pela Free Software Foundation,
 * quer a versão 3 da Licença, ou (a teu critério) qualquer versão posterior.
 *
 * Este programa é distribuído na expectativa de que seja útil,
 * mas SEM QUALQUER GARANTIA; sem mesmo a garantia implícita
 * de COMERCIALIZAÇÃO ou ADEQUAÇÃO A UM DETERMINADO FIM.
 * Veja a Licença Pública Geral GNU para mais detalhes.
 *
 * Deverás ter recebido uma cópia da Licença Pública Geral GNU
 * juntamente com este programa. Se não, veja <https://www.gnu.org/licenses/>.
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <TFT_eSPI.h>
#include <math.h>
#include <ArduinoJson.h>
#include "CHSC6X_Touch.h"
#include "ESP_I2S.h"
#include <PNGdec.h>
#include "cookie.h"
#include "no_cookies.h"

// Motor PIN
#define DISPENSER_PIN_2 D0

// Time in seconds to record
#define RECORD_TIME 5

#define MAX_WAV_SIZE (16000 * 2 * RECORD_TIME) // 5s a 16kHz mono 16-bit

#define MAX_IMAGE_WIDTH 240
#define DEG_TO_RAD 0.0174532925


// Change here your wifi password and network
const char* ssid = "<wifi_network_name>";
const char* password = "<wifi_password>";

// change here the IP of the server - change change the IP address, nothing else
const char* apiUrl = "http://<server_ip_address>:8000/analyze-audio";

TFT_eSPI tft;
TFT_eSprite sprite = TFT_eSprite(&tft);
CHSC6X_Touch touch;
PNG png;

uint8_t wav_buffer[MAX_WAV_SIZE];
size_t wav_size;

SemaphoreHandle_t semState = NULL;
TaskHandle_t taskHandles[6]; // standby, gravacao, send, cookies, noCookies, recordingVisual

int frame = 0;
int rippleCooldown = 0;

volatile bool recordingVisualShouldStop = false;

struct Ripple {
  float radius;
  float alpha;
};
std::vector<Ripple> ripples;

enum AppState {
  STANDBY,
  RECORDING,
  SENDING,
  COOKIE,
  NO_COOKIE
} currentState = STANDBY;

// turn motor - it doesn't matter to which side it rotates
// as long it rotates
void giveCandy () {
  digitalWrite(DISPENSER_PIN_2, HIGH);
}

// turn motor - it doesn't matter to which side it rotates
// as long as it rotates
void stopCandy () {
  digitalWrite(DISPENSER_PIN_2, LOW);
}

/**
Updates the current state (currentState) and activates the appropriate Task.
Uses a Semaphore (semState) for synchronization 
*/
void changeState(AppState newState) {
  currentState = newState;

  xSemaphoreGive(semState);
  switch (newState) {
    case STANDBY:    vTaskResume(taskHandles[0]); break;
    case RECORDING:
      vTaskResume(taskHandles[1]);
      vTaskResume(taskHandles[5]);
      break;
    case SENDING:    vTaskResume(taskHandles[2]); break;
    case COOKIE:     vTaskResume(taskHandles[3]); break;
    case NO_COOKIE:  vTaskResume(taskHandles[4]); break;
  }
}

/* connect to WiFi */
void connectToWiFi() {
  int myAttempts = 0;
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); //if connected before
  delay (100);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.useStaticBuffers(true);
  WiFi.begin(ssid, password);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  
  while (WiFi.status() != WL_CONNECTED && myAttempts < 31) {
    Serial.print(".");
    delay(500);
    myAttempts++;
    if (myAttempts > 30) {
      Serial.println();
      Serial.print ("Not connected... Restarting.");
      ESP.restart();
    }
  }
  Serial.println();
  Serial.print("ESP32S3-Sense IP Address: ");
  Serial.println(WiFi.localIP());
}

/**
* Calculates a darker version of a color, for simulated transparency (TFT does not has a real alpha).
*/
uint16_t dimColor(uint16_t color, float factor) {
  uint8_t r = ((color >> 11) & 0x1F) * factor;
  uint8_t g = ((color >> 5) & 0x3F) * factor;
  uint8_t b = (color & 0x1F) * factor;
  return tft.color565(r << 3, g << 2, b << 3);
}

/**
* Shows a "breathing" animation, with a central pulsating circle.
* awaits a screen touch.
*/
void standbyTask(void* param) {
  uint16_t baseColor = tft.color565(0, 180, 255);
  int glowLayers = 15;
  sprite.fillSprite(TFT_BLACK);
  while (true) {
    if (currentState != STANDBY) vTaskSuspend(NULL);
    int radius = (int)(sin(frame * 0.1) * 10 + 30);
    for (int i = glowLayers; i > 0; i--) {
      float scale = 1.0 + i * 0.1;
      float alpha = 1.0 - (float)i / (glowLayers + 1);
      uint16_t glowColor = dimColor(baseColor, alpha);
      sprite.fillCircle(120, 120, radius * scale, glowColor);
    }
    sprite.fillCircle(120, 120, radius, baseColor);
    sprite.pushSprite(0, 0);
    frame++;
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

/**
* Shows a ripple effect during recording
* Uses std::vector<Ripple> to manage several growing waves with fade.
*/

void recordingVisualTask(void* param) {
  int radius = 20;
  uint16_t recordingColor = tft.color565(255, 0, 40);
  while (true) {
    if (currentState != RECORDING || recordingVisualShouldStop) {
      recordingVisualShouldStop = false;  // clears the flag
      vTaskSuspend(NULL);
    }
    sprite.fillSprite(TFT_BLACK);
    if (rippleCooldown <= 0) {
      ripples.push_back({(float)radius, 1.0f});
      rippleCooldown = 10;
    } else {
      rippleCooldown--;
    }
    for (int i = ripples.size() - 1; i >= 0; i--) {
      Ripple& r = ripples[i];
      r.alpha -= 0.02;
      r.radius += 2.0;
      if (r.alpha <= 0.0f) {
        ripples.erase(ripples.begin() + i);
        continue;
      }
      uint16_t rippleColor = dimColor(recordingColor, r.alpha);
      sprite.drawCircle(120, 120, (int)r.radius, rippleColor);
    }
    sprite.fillCircle(120, 120, radius, recordingColor);
    sprite.pushSprite(0, 0);
    frame++;
    vTaskDelay(pdMS_TO_TICKS(20));
  }
} 

/**
* Records 5 seconds of audio with I2S (PDM RX).
* keeps the recoring in a buffer 'wav_buffer', hence the PSRAM support
* sets the state to SENDING.
*/
void gravacaoTask(void* param) {
  while (true) {
    if (currentState != RECORDING) vTaskSuspend(NULL);
    I2SClass i2s;
    i2s.setPinsPdmRx(42, 41);
    i2s.begin(I2S_MODE_PDM_RX, 16000, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO);
    uint8_t* buf = i2s.recordWAV(RECORD_TIME, &wav_size);
    memcpy(wav_buffer, buf, wav_size);
    free(buf);
    recordingVisualShouldStop = true;
    changeState(SENDING);
  }
}

/**
* Sends the audio for the local server via HTTP POST.
* Shows a "waiting" animation on the screen.
* Analyses the JSON response: if "positivo": true, sets state to COOKIE; else, NO_COOKIE.
*/
void sendTask(void* param) {
  while (true) {
    if (currentState != SENDING) vTaskSuspend(NULL);
    uint16_t baseColor = tft.color565(0, 150, 255);
    int numBars = 12;
    float angleStep = 360.0 / numBars;
    float radius = 50;
    int barLength = 12;
    int barWidth = 4;

    //sending the recording
    WiFiClient client;
    HTTPClient http;
    http.begin(client, apiUrl);
    http.addHeader("Content-Type", "audio/wav");
    int code = http.POST(wav_buffer, wav_size);

    // animation
    for (int f = 0; f < 150; f++) {
      sprite.fillSprite(TFT_BLACK);
      for (int i = 0; i < numBars; i++) {
        float angle = (frame * 4 + i * angleStep) * DEG_TO_RAD;
        float x = 120 + cos(angle) * radius;
        float y = 120 + sin(angle) * radius;
        float glowFactor = (sin((frame * 0.15) + i) + 1.0) / 2.0;
        float coreAlpha = 0.4 + 0.6 * glowFactor;
        float outerAlpha = 0.2 * glowFactor;
        uint16_t coreColor = dimColor(baseColor, coreAlpha);
        uint16_t glowColor = dimColor(baseColor, outerAlpha);
        sprite.fillRoundRect(x - barWidth / 2, y - barLength / 2, barWidth, barLength, 2, glowColor);
        sprite.fillRoundRect(x - barWidth / 2, y - barLength / 2, barWidth, barLength, 2, coreColor);
      }
      sprite.pushSprite(0, 0);
      frame++;
      vTaskDelay(pdMS_TO_TICKS(20));
    }

    if (code > 0) {
      String payload = http.getString();
      StaticJsonDocument<256> doc;
      if (!deserializeJson(doc, payload)) {
        bool positivo = doc["positivo"];
        changeState(positivo ? COOKIE : NO_COOKIE);
      } else changeState(NO_COOKIE);
    } else changeState(NO_COOKIE);

    http.end();
  }
}

int xpos = 0;
int ypos = 0;

/**
* Draw a cookie animation
*/

void drawCookie(PNGDRAW* pDraw) {
  uint16_t lineBuffer[MAX_IMAGE_WIDTH];
  uint8_t maskBuffer[1 + MAX_IMAGE_WIDTH / 8];
  png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
  if (png.getAlphaMask(pDraw, maskBuffer, 255)) {
    tft.pushMaskedImage(xpos, ypos + pDraw->y, pDraw->iWidth, 1, lineBuffer, maskBuffer);
  }
}

/**
* Shows a cookie image - PNG - at random positions
* Ativates the motor giveCandy() for 1 second
* After that, it calls stopCandy() and returns to standBy.
*/
void cookieTask(void* param) {
  while (true) {
    if (currentState != COOKIE) vTaskSuspend(NULL);
    tft.fillScreen(TFT_BLACK);
    unsigned long start = millis();
    const unsigned long duration = 1000; // how long the motor turns

    Serial.println ("Dispensing a candy...");
    // Turn the motor on for a candy
    giveCandy();

    while (millis() - start < duration) {
      xpos = random(0, tft.width() - 50);
      ypos = random(0, tft.height() - 50);

      int16_t rc = png.openFLASH((uint8_t *)cookie, sizeof(cookie), drawCookie);
      tft.startWrite();
      png.decode(NULL, 0);
      tft.endWrite();

      vTaskDelay(pdMS_TO_TICKS(200));
    }
    // Turn the motor off
    stopCandy();
    Serial.println ("Stoped dispensing a candy...");
    changeState(STANDBY);
  }
}

/**
* no_cookie drawing in the screen
*/
void drawNoCookie(PNGDRAW* pDraw) {
  uint16_t lineBuffer[MAX_IMAGE_WIDTH];
  png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
  tft.pushImage(0, pDraw->y, pDraw->iWidth, 1, lineBuffer);
}

/**
* Shows the no_cookie image for 5 seconds.
* returns to standBy.
*/
void noCookieTask(void* param) {
  while (true) {
    if (currentState != NO_COOKIE) vTaskSuspend(NULL);
    tft.fillScreen(TFT_BLACK);
    int16_t rc = png.openFLASH((uint8_t *)no_cookies, sizeof(no_cookies), [](PNGDRAW* pDraw) {
      uint16_t lineBuffer[MAX_IMAGE_WIDTH];
      png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
      tft.pushImage(0, pDraw->y, pDraw->iWidth, 1, lineBuffer);
    });
    png.decode(NULL, 0);
    delay(5000);
    changeState(STANDBY);
  }
}

/**
* Initialize the hardware: serie, screen, motor, Wi-Fi, touch, etc.
* Creates parallel tasks with xTaskCreatePinnedToCore for the several states: standby, gravação, envio, resposta.
*/
void setup() {
 
  Serial.begin(115200);

  delay (1000); //Waiting for it to settle

  currentState = STANDBY; //initial state

  pinMode(DISPENSER_PIN_2, OUTPUT);
  digitalWrite(DISPENSER_PIN_2, LOW);
  
  connectToWiFi();

  tft.init();
  tft.setRotation(0);
  sprite.setColorDepth(16);
  sprite.createSprite(240, 240);
  touch.begin();
  
  semState = xSemaphoreCreateBinary();
  xTaskCreatePinnedToCore(standbyTask, "standby", 4096, NULL, 1, &taskHandles[0], 0);
  xTaskCreatePinnedToCore(gravacaoTask, "gravacao", 4096, NULL, 1, &taskHandles[1], 0);
  xTaskCreatePinnedToCore(sendTask, "send", 4096, NULL, 1, &taskHandles[2], 0);
  xTaskCreatePinnedToCore(cookieTask, "cookie", 2048, NULL, 1, &taskHandles[3], 0);
  xTaskCreatePinnedToCore(noCookieTask, "nocookie", 2048, NULL, 1, &taskHandles[4], 0);
  xTaskCreatePinnedToCore(recordingVisualTask, "recordVis", 2048, NULL, 1, &taskHandles[5], 0);
}

/**
* main function
* Verifies if the screen has been touched while on standby.
* if yes, starts the recording task by switching the state
*/
void loop() {
  static bool touchedLastLoop = false;
  bool isTouched = touch.touched();
  if (isTouched && !touchedLastLoop && currentState == STANDBY) {
    Serial.println("Ecrã tocado. Iniciar gravação.");
    digitalWrite(DISPENSER_PIN_2, LOW);
    changeState(RECORDING);
  }
  touchedLastLoop = isTouched;
  vTaskDelay(pdMS_TO_TICKS(50));
}

