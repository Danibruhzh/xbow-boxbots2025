from flask import Flask, request, send_file
import requests
from flask_cors import CORS

app = Flask(__name__)
CORS(app)

# Replace with your ESP32 IP on your local WiFi
ESP32_IP = "http://172.20.10.2"

@app.route('/')
def index():
    return send_file('index.html')  # serve your page at /


@app.route("/gyro", methods=["POST", "OPTIONS"])
def forward_gyro():
    # Handle preflight OPTIONS request
    if request.method == "OPTIONS":
        return "", 200
    
    try:
        data = request.get_json()
        if not data:
            return "No JSON received", 400

        print("Received data:", data)
        
        # Forward the data to ESP32
        esp_url = f"{ESP32_IP}/gyro"
        r = requests.post(esp_url, json=data, timeout=5)
        
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
    app.run(host="0.0.0.0", port=5000, debug=True)
