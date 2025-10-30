FROM python:3.11-slim

# Instala dependências do sistema
RUN apt update && apt install -y \
    build-essential cmake git curl libsndfile1 && \
    rm -rf /var/lib/apt/lists/*

# Cria diretório de trabalho
WORKDIR /app

# Copia código
COPY . /app

# Instala dependências Python
RUN pip install --no-cache-dir fastapi uvicorn requests python-dotenv

# Clona e compila whisper.cpp dentro do container
RUN git clone https://github.com/ggerganov/whisper.cpp && \
    cd whisper.cpp && make -j && \
    ./models/download-ggml-model.sh small

# Expõe porta
EXPOSE 5000

# Comando para arrancar o servidor
CMD ["uvicorn", "candy-server:app", "--host", "0.0.0.0", "--port", "5000"]
