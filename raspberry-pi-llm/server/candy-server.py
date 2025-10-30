from fastapi import FastAPI, UploadFile
from fastapi.responses import JSONResponse
import uvicorn
import subprocess
import tempfile
import os
import requests
from dotenv import load_dotenv

load_dotenv()

OLLAMA_URL = os.getenv("OLLAMA_URL", "http://localhost:11434")
WHISPER_MODEL = os.getenv("WHISPER_MODEL", "./whisper.cpp/models/ggml-base.en.bin")

app = FastAPI()

@app.post("/audio")
async def receive_audio(file: UploadFile):
    # Guarda o áudio recebido temporariamente
    with tempfile.NamedTemporaryFile(delete=False, suffix=".wav") as tmp:
        tmp.write(await file.read())
        wav_path = tmp.name

    # Executa whisper.cpp
    result = subprocess.run([
        "./whisper.cpp/main",
        "-m", WHISPER_MODEL,
        "-f", wav_path,
        "-otxt"
    ])

    txt_path = wav_path.replace(".wav", ".txt")
    if not os.path.exists(txt_path):
        return JSONResponse(status_code=500, content={"error": "Erro na transcrição"})

    with open(txt_path, 'r') as f:
        transcription = f.read().strip()

    os.remove(wav_path)
    os.remove(txt_path)

    # Prepara prompt
    prompt = f'A criança disse: "{transcription}". Esta frase está relacionada com doces? Responde apenas com SIM ou NÃO.'

    try:
        response = requests.post(f"{OLLAMA_URL}/api/generate", json={
            "model": "phi3:mini",
            "prompt": prompt,
            "stream": False
        })

        resposta = response.json()["response"].strip().upper()
        is_positive = "SIM" in resposta

    except Exception as e:
        return JSONResponse(status_code=500, content={"error": f"Erro no modelo: {str(e)}"})

    return JSONResponse(content={
        "positivo": is_positive,
        "text": transcription
    })

if __name__ == "__main__":
    uvicorn.run("server:app", host="0.0.0.0", port=5000)
