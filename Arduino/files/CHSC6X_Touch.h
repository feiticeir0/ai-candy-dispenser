/*
 * Nome do ficheiro: CHSC6X_Touch.h
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

#pragma once
#include <Wire.h>

#define CHSC6X_I2C_ID 0x2e
#define CHSC6X_READ_POINT_LEN 5
#define CHSC6X_INT_PIN D7
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240

class CHSC6X_Touch {
public:
  void begin() {
    pinMode(CHSC6X_INT_PIN, INPUT_PULLUP);
    Wire.begin();
  }

  bool touched() {
    if (digitalRead(CHSC6X_INT_PIN) != LOW) {
      delay(1);
      if (digitalRead(CHSC6X_INT_PIN) != LOW)
        return false;
    }
    return true;
  }

  bool getTouch(uint16_t &x, uint16_t &y, uint8_t rotation = 0) {
    uint8_t data[CHSC6X_READ_POINT_LEN] = {0};
    if (Wire.requestFrom(CHSC6X_I2C_ID, CHSC6X_READ_POINT_LEN) != CHSC6X_READ_POINT_LEN) {
      return false;
    }

    Wire.readBytes(data, CHSC6X_READ_POINT_LEN);
    if (data[0] != 0x01) return false;

    uint8_t tx = data[2], ty = data[4];
    convertXY(tx, ty, rotation);
    x = tx;
    y = ty;
    return true;
  }

private:
  void convertXY(uint8_t &x, uint8_t &y, uint8_t rot) {
    uint8_t x_tmp = x, y_tmp = y, end = 0;
    for (int i = 1; i <= rot; i++) {
      x_tmp = x;
      y_tmp = y;
      end = (i % 2) ? SCREEN_WIDTH : SCREEN_HEIGHT;
      x = y_tmp;
      y = end - x_tmp;
    }
  }
};
