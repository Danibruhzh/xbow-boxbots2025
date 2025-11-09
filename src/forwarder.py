from flask import Flask, request, send_file
import requests
from flask_cors import CORS
import time
from waitress import serve

app = Flask(__name__)
CORS(app)

# Replace with your ESP32 IP on your local WiFi
ESP32_IP = "http://172.20.10.2"

def forward_to_esp32(data, retries=3, delay=1):
    esp_url = f"{ESP32_IP}/gyro"
    for attempt in range(1, retries + 1):
        try:
            r = requests.post(esp_url, json=data, timeout=5)
            if r.status_code == 200:
                print(f"Forwarded to ESP32 successfully on attempt {attempt}")
                return True
            else:
                print(f"ESP32 returned status {r.status_code} on attempt {attempt}")
        except requests.exceptions.RequestException as e:
            print(f"Attempt {attempt} failed: {e}")
        time.sleep(delay)
    print("Failed to forward data after retries")
    return False

@app.route('/')
def index():
    return send_file('index.html')  # serve your page at /

@app.route("/servo", methods=["POST"])
def move_servo():
    try:
        # Forward the command to ESP32
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
    # Handle preflight OPTIONS request
    if request.method == "OPTIONS":
        return "", 200
    
    try:
        data = request.get_json()
        if not data:
            return "No JSON received", 400

        #print("Received data:", data)
        
        # Forward the data to ESP32
        esp_url = f"{ESP32_IP}/gyro"
        session = requests.Session()
        r = session.post(esp_url, json=data, timeout=5)
        
        #print(f"Forwarded to ESP32, status: {r.status_code}")
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
    print("Starting Flask server with Waitress on port 5000...")
    # Run production server for persistent, reliable operation
    serve(app, host="0.0.0.0", port=5000)
