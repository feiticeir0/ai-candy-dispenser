# 🍬 Candy Machine with Voice Recognition

This project turns a simple touchscreen with a microphone into a **magical candy dispenser**. The machine listens to your voice and decides — with the help of artificial intelligence — if you deserve a chocolate! 😄

---

## 🧠 How It Works

1. The user presses a button on the screen.
2. The machine records a phrase for **5 seconds**.
3. The phrase is sent to a **remote server** that:
   - Automatically transcribes the audio using **Whisper** (OpenAI)
   - Classifies the phrase as **positive** or **negative** about chocolate or candy
4. If the phrase is **positive**:
   - A candy is automatically **dispensed**
5. Otherwise, a message is shown asking the user to try again.

This setup creates fun, educational, or promotional interactions with children and adults in events, schools, or public spaces.

---

## 📂 Project Structure

```
.
├── 3D/                       # Directory for the 3D models
├── Arduino/                  # Directory for the Arduino code
├── main.py                   # API code
├── Dockerfile                # Dockerfile for CPU
├── Dockerfile.gpu            # Dockerfile with CUDA support (GPU)
├── docker-compose.cpu.yml    # Compose file for CPU use
├── docker-compose.gpu.yml    # Compose file for GPU (NVIDIA)
```

---

## 🚀 How to Use
## Server
### Deployment with Docker Compose
### ⚙️ Requirements

- Docker 20.10+
- `docker compose` (modern CLI)
- For GPU use:
  - NVIDIA drivers installed
  - `nvidia-container-toolkit`: [official guide](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/install-guide.html)

---

### 🧠 Using with GPU (CUDA)

Uses GPU to accelerate transcription and classification.

#### ✅ Requirements:

- NVIDIA GPU compatible with CUDA 11.8+
- Drivers installed (`nvidia-smi` should work)
- Docker with NVIDIA runtime
- Install the toolkit:
```bash
sudo apt-get install -y nvidia-container-toolkit
sudo systemctl restart docker
```

#### ▶️ Command:

```bash
docker compose -f docker-compose.gpu.yml up --build
```

---

### 🖥️ Using with CPU only

Simpler and portable; good for testing and light deployments.

#### ▶️ Command:

```bash
docker compose -f docker-compose.cpu.yml up --build
```

---

## 🎧 Test the API

Assuming you have a `sample.wav` file:

```bash
curl -X POST --data-binary "@sample.wav" http://localhost:8000/analyze-audio
```

Expected response:

```json
{
  "positivo": true,
  "text": "Adoro chocolate com amêndoas."
}
```

---

## 📦 Models Used

- [`openai/whisper-small`](https://github.com/openai/whisper): for audio transcription
- [`facebook/bart-large-mnli`](https://huggingface.co/facebook/bart-large-mnli): for zero-shot classification

---

## 🛠️ Customization

- You can change the classification **labels** (`positive`, `negative`) in `main.py`
- You can switch Whisper models to `tiny`, `base`, or `large` depending on your hardware
- You can switch the language used by model

---

## 🧪 Testing GPU (CUDA) Support

### ✅ 1. Check NVIDIA drivers
```bash
nvidia-smi
```

### ✅ 2. Install `nvidia-container-toolkit`
```bash
sudo apt install nvidia-container-toolkit
sudo systemctl restart docker
```

### ✅ 3. Verify GPU access via Docker
```bash
docker run --rm --gpus all nvidia/cuda:11.8.0-base nvidia-smi
```

### ▶️ 4. Start with GPU
```bash
docker compose -f docker-compose.gpu.yml up --build
```

### 🛠️ 5. Check if PyTorch uses CUDA
```bash
docker exec -it candy-api-gpu python3 -c "import torch; print(torch.cuda.is_available())"
```

---

## 🔄 Lifecycle with Docker Compose

```bash
docker compose start         # Start stopped container
docker compose stop          # Stop container
docker compose restart       # Restart service
docker compose down          # Remove container and network
docker compose up --build    # Build and launch
docker compose up --build --force-recreate  # Rebuild from scratch
docker builder prune -af     # Clean build cache
```

---

## 🔌 Uploading Code to the ESP32-S3

To run the Candy Machine on a microcontroller with microphone and LCD (like the ESP32-S3 with 1.47" display), configure your development environment properly.

### ✅ Requirements

- Arduino IDE installed
- ESP32-S3 board added via **Boards Manager** (Espressif)
- Official `ESP32` library (v2.0.7 or higher recommended)

---

### ⚙️ Required Settings in Arduino IDE

Set these under **Tools**:

- **Board**: ESP32S3 Dev Module *(or your specific model)*
- **USB CDC On Boot**: ✔️ Enabled
- **PSRAM**: ✔️ Enabled ✅ required for audio/transcription
- **Flash Size**: 16MB
- **Partition Scheme**: "Default 4MB with spiffs" or larger
- **Upload Mode**: UART or CDC
- **Baud Rate**: 115200 or 921600

---

### 📤 Uploading the Code

1. Connect ESP32-S3 via USB
2. Select the correct port in **Tools → Port**
3. Click on **Upload**

If needed, press the **BOOT** button on the board during upload.

---

### 📁 Recommended Libraries

For microphone, touch, and LCD display projects:

- `TFT_eSPI` or `LovyanGFX` (LCD)
- `ESPAsyncWebServer` (optional)
- `Audio` or `I2S` (audio capture)
- `SPIFFS` or `LittleFS` (file storage)

---
