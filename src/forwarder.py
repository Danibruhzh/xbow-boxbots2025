from flask import Flask, request, send_file
import requests
from flask_cors import CORS
from waitress import serve

app = Flask(__name__)
CORS(app)

ESP32_IP = "http://172.20.10.2"

@app.route('/')
def index():
    return send_file('index.html')  # serve page at /

@app.route("/ping")
def ping():
    print("PING HIT")
    return send_file('index.html')

@app.route("/servo", methods=["POST"])
def move_servo():
    try:
        # Forward the command to the ESP32
        esp_url = f"{ESP32_IP}/servo"
        r = requests.post(esp_url, timeout=5)

        if r.status_code == 200:
            return "OK", 200
        else:
            return f"ESP32 returned {r.status_code}", 500

    except Exception as e:
        print(f"Error forwarding servo command: {e}")
        return f"Error: {e}", 500


@app.route("/gyro", methods=["POST", "OPTIONS"])
def forward_gyro():
    if request.method == "OPTIONS":
        return "", 200
    
    try:
        data = request.get_json()
        if not data:
            return "No JSON received", 400

        print("Received data:", data)
        
        # Forward data to the ESP32
        esp_url = f"{ESP32_IP}/gyro"
        session = requests.Session()
        r = session.post(esp_url, json=data, timeout=5)
        
        print(f"Forwarded to ESP32, status: {r.status_code}")
        return f"OK", 200

    except requests.exceptions.Timeout:
        print("ESP32 request timed out")
        return "ESP32 timeout", 500
    except requests.exceptions.ConnectionError:
        print("Cannot connect to ESP32")
        return "ESP32 connection error", 500
    except Exception as e:
        print(f"Error: {e}")
        return f"Error: {e}", 500

if __name__ == "__main__":
    print("Starting Flask server with Waitress on port 5500...")
    serve(app, host="0.0.0.0", port=5500)
