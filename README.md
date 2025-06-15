# 🍬 Candy Machine with Voice Recognition
This project is a tiny, AI-powered candy dispenser built with the Seeed Studio ESP32S3 Sense and the Xiao Round Display for XIAO.

Say the right magic words, and you’ll be rewarded with a cookie. 😄
If not, the screen politely tells you to try again.

---

## 📷 How It Works

1. The user presses a button on the screen.
2. The machine records a phrase for **5 seconds**.
3. The phrase is sent to a **remote server** that:
   - Automatically transcribes the audio using **Whisper** (OpenAI)
   - Classifies the phrase as **positive** or **negative** about chocolate or candy
4. If the phrase is **positive**:
   - A candy is automatically **dispensed**
5. Otherwise, a message is shown asking the user to try again.


---

🛠️  Project Structure

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

## 📷 How to Use
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


## ⚙️ Testing GPU (CUDA) Support

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

## 📦 Models Used
These are the models used. You can change them in `main.py`.

- [`openai/whisper-small`](https://github.com/openai/whisper): for audio transcription
- [`facebook/bart-large-mnli`](https://huggingface.co/facebook/bart-large-mnli): for zero-shot classification

---

## 🛠️ Customization

- You can change the classification **labels** (`positive`, `negative`) in `main.py`
- You can switch Whisper models to `tiny`, `base`, or `large` depending on your hardware
- You can switch the language used by model for the transcription (`pt`, `en`, `es`, etc.)


## Client (the candy dispenser)
### 3D Files
Go to the [3D](./3D/readme.md) folder for more information.

### Arduino Code
Go to the [Arduino](./Arduino/readme.md) folder for more information.

### Schematics
Go to the [Schematics](./Schematics/readme.md) folder for more information.

