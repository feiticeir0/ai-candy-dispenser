## 🍬 Jetson Deployment Guide
Overview

This document explains how to deploy the Candy Dispenser API on a Jetson Orin NX (or any Jetson device running JetPack 6+).
It covers creating a Python virtual environment, installing dependencies, configuring systemd for automatic startup, and solving common issues with ffmpeg and Whisper.

## 🧩 Requirements

- Ubuntu 20.04 / 22.04 (JetPack 6.x)
- Python 3.10+
- ffmpeg installed system-wide
```bash
sudo apt update
sudo apt install ffmpeg
```
- Internet connection for initial dependency installation

## 🐍 1. Create the virtual environment
```bash
cd ~
python3 -m venv candy-api
source candy-api/bin/activate
```
Upgrade pip and install dependencies:

```bash
pip install --upgrade pip
pip install fastapi uvicorn[standard] openai-whisper pydub
```
### Jetson Specific Pytorch packages
To install a supported Pytorch version, head up to Nvidia forums and follow the instructions according to your JetPack version.

https://forums.developer.nvidia.com/t/pytorch-for-jetson/72048

After installing PyTorch, install Whisper
```bash
 pip install git+https://github.com/openai/whisper.git \
 ```

## ⚙️ 2. Verify ffmpeg
Check that ffmpeg is accessible:
```bash
which ffmpeg
# Expected output: /usr/bin/ffmpeg
```
If this returns nothing, install it again or adjust your PATH.

## 🧠 3. Fix ffmpeg visibility for systemd
When running as a system service, systemd uses a limited PATH.
To make sure Whisper can find ffmpeg, add this snippet at the very top of your main.py:
```bash
# Fix for ffmpeg visibility under systemd
import os
os.environ["PATH"] += os.pathsep + "/usr/bin:/usr/local/bin"
```
## 🔧 4. Create the systemd service
Create a new file:
```bash
sudo nano /etc/systemd/system/candy-api.service
```
Paste the following configuration:
```bash
[Unit]
Description=Candy Dispenser FastAPI Service
After=network.target

[Service]
User=<your_user>
WorkingDirectory=/home/<your_user>/candy-api
ExecStart=/home/<your_user>/candy-api/bin/uvicorn main:app --host=0.0.0.0 --port=8000
Environment=PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/home/<your_user>/candy-api/bin
Restart=always
RestartSec=5
StandardOutput=append:/var/log/candy-api.log
StandardError=append:/var/log/candy-api.log

[Install]
WantedBy=multi-user.target
```
Save and enable the service:
```bash
sudo systemctl daemon-reexec
sudo systemctl daemon-reload
sudo systemctl enable candy-api
sudo systemctl start candy-api
```

## 📋 5. Check status and logs
To confirm that the service is running:
```bash
systemctl status candy-api
```
To view logs in real time:
```bash
journalctl -u candy-api -f
```
Logs are also written to /var/log/candy-api.log.

## 🧼 6. Maintenance commands
```bash
sudo systemctl restart candy-api
```
Stop the service:
```bash
sudo systemctl stop candy-api
```
Update dependencies:
```bash
source ~/Documents/candy-api/venv/bin/activate
pip install -U <package>
sudo systemctl restart candy-api
```

## 🧠 Notes
- If you later migrate to a Docker deployment, remove the os.environ["PATH"] fix and install ffmpeg inside the image (apt-get install -y ffmpeg).
- Always test Whisper locally before enabling the service:
```bash
source venv/bin/activate
python -m whisper test.wav
```
## ✅ Result
After following this guide, the API will:
- start automatically at boot,
- have full access to ffmpeg,
- run inside an isolated virtual environment,
- and log output to /var/log/candy-api.log.