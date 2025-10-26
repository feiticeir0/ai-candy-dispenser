# Nome do ficheiro: main.py
# Autor: Bruno Ricardo Sanntos - feiticeir0@whatgeek.com.pt (c) 2025
# Licença: GNU General Public License v3.0 (GPLv3)
#
# Este programa é software livre: pode redistribuí-lo e/ou modificá-lo
# sob os termos da Licença Pública Geral GNU conforme publicada pela Free Software Foundation,
# quer a versão 3 da Licença, ou (a teu critério) qualquer versão posterior.
#
# Este programa é distribuído na expectativa de que seja útil,
# mas SEM QUALQUER GARANTIA; sem mesmo a garantia implícita
# de COMERCIALIZAÇÃO ou ADEQUAÇÃO A UM DETERMINADO FIM.
# Veja a Licença Pública Geral GNU para mais detalhes.
#
# Deverás ter recebido uma cópia da Licença Pública Geral GNU
# juntamente com este programa. Se não, veja <https://www.gnu.org/licenses/>.

from fastapi import FastAPI, UploadFile, File, Request
from fastapi.responses import JSONResponse
import whisper
import tempfile
import shutil
import os
from transformers import pipeline

# Instanciar a aplicação FastAPI
app = FastAPI()

# Carregar o modelo Whisper para transcrição de áudio
# check here for the differences  between the models
# https://github.com/openai/whisper
model = whisper.load_model("small")

# Carregar o modelo de classificação zero-shot da Hugging Face
classifier = pipeline("zero-shot-classification", model="facebook/bart-large-mnli")



@app.post("/analyze-audio")
async def analyze_raw_audio(request: Request):
    """
    Endpoint para receber um ficheiro de áudio via POST,
    transcrever o conteúdo usando Whisper, e classificar o texto
    como 'positivo' ou 'negativo' com base em zero-shot classification.
    """
     
    # Ler o corpo da requisição (espera-se um ficheiro de áudio bruto)
    data = await request.body()

    # Guardar temporariamente o áudio num ficheiro WAV
    with tempfile.NamedTemporaryFile(suffix=".wav", delete=False) as f:
        f.write(data)
        temp_path = f.name

    # Transcrever o áudio para texto (Português)
    # If you want o change the language, it's here. Change 'language=' and replace pt with your country code
    # check if it's suppoprted by whisper - https://platform.openai.com/docs/guides/speech-to-text/supported-languages/#supported-languages
    result = model.transcribe(temp_path, language="pt")
    transcription = result["text"]

    # Remover o ficheiro temporário
    os.remove(temp_path)

    
    # Classificar a transcrição como 'positivo' ou 'negativo'
    labels = ["positivo", "negativo"]
    classification = classifier(transcription, candidate_labels=labels)
    is_positive = classification["labels"][0] == "positivo"
    
    # Imprimir o resultado no terminal (para debug)
    print (f"positivo: {is_positive},text: {transcription}")
    
    return JSONResponse(content={
        "positivo": is_positive,
        "text": transcription
    })
