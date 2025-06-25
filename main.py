from fastapi import FastAPI, UploadFile, File, Request
from fastapi.responses import JSONResponse
import whisper
import tempfile
import shutil
import os
from transformers import pipeline

app = FastAPI()

# Load the model
model = whisper.load_model("small")
classifier = pipeline("zero-shot-classification", model="facebook/bart-large-mnli")



@app.post("/analyze-audio")
async def analyze_raw_audio(request: Request):
    data = await request.body()

    # Guardar ficheiro temporariamente
    with tempfile.NamedTemporaryFile(suffix=".wav", delete=False) as f:
        f.write(data)
        temp_path = f.name

    # Transcrição
    result = model.transcribe(temp_path, language="pt")
    transcription = result["text"]
    #print (transcription)
    #print (temp_path)
    os.remove(temp_path)

    # Análise simples: contém "chocolate"?
    #is_valid = "chocolate" in transcription.lower()
    
    # Classificar a transcricao
    labels = ["positivo", "negativo"]
    classification = classifier(transcription, candidate_labels=labels)
    is_positive = classification["labels"][0] == "positivo"
    
    print (f"positivo: {is_positive},text: {transcription}")
    
    return JSONResponse(content={
        "positivo": is_positive,
        "text": transcription
    })
