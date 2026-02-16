# Usamos una imagen base ligera de Python
FROM python:3.9-slim

# Directorio de trabajo dentro del contenedor
WORKDIR /app

# Copiamos primero los requerimientos (para aprovechar caché de Docker)
COPY requirements.txt .

# Instalamos dependencias
RUN pip install --no-cache-dir -r requirements.txt

# Copiamos el resto del código
COPY . .

# Exponemos el puerto 5000
EXPOSE "5000:5000"

# Usamos Gunicorn para producción en lugar del servidor de desarrollo de Flask
CMD ["gunicorn", "--bind", "0.0.0.0:5000", "app:app"]
