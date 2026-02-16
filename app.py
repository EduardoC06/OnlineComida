from flask import Flask, render_template, jsonify, request
from datetime import datetime

app = Flask(__name__)

# Estado del sistema (En memoria, se reinicia si el contenedor se apaga)
# En producción real, esto iría a Redis o SQLite.
state = {
    "feed_command": False,
    "last_fed": "Nunca",
    "status": "Esperando orden"
}

@app.route('/')
def index():
    return render_template('index.html', last_fed=state["last_fed"])

# API: Endpoint para tu botón en la web
@app.route('/api/trigger', methods=['POST'])
def trigger():
    state["feed_command"] = True
    state["status"] = "Orden pendiente de recoger por Arduino"
    return jsonify({"success": True, "message": "¡Orden enviada!"})

# API: Endpoint para el Arduino (Polling)
@app.route('/api/arduino-check', methods=['GET'])
def check():
    if state["feed_command"]:
        state["feed_command"] = False # Resetear bandera
        state["last_fed"] = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        state["status"] = "Alimentado exitosamente"
        return "YES" # Respuesta corta para el ESP8266
    return "NO"

# API: Para que la web actualice el estado sin recargar
@app.route('/api/status', methods=['GET'])
def get_status():
    return jsonify(state)

if __name__ == '__main__':
    # Nota: En el Dockerfile usaremos Gunicorn, esto es solo para local
    app.run(host='0.0.0.0', port=5000, debug=True)