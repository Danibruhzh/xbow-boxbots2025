const SEND_DELAY = 100;
const dataSection = document.getElementById('data');
const logDiv = document.getElementById('log');
const demoButton = document.getElementById("start_demo");
logDiv.style.display = 'block';
const hiddenData = document.querySelectorAll(".data");

let isRunning = false;
    let successCount = 0;
    let failCount = 0;
    let lastSendTime = 0;

    function log(message, type = 'info') {
        const time = new Date().toLocaleTimeString();
        const color = type === 'error' ? '#f44336' : type === 'success' ? '#4CAF50' : '#888';
        logDiv.innerHTML = `<div style="color: ${color}">[${time}] ${message}</div>` + logDiv.innerHTML;
        console.log(message);
    }

    function updateStatus(message, className) {
        const statusDiv = document.getElementById('status');
        statusDiv.textContent = message;
        statusDiv.className = 'status ' + className;
    }

    function incrementEventCount() {
        const counter = document.getElementById("num-observed-events");
        counter.innerHTML = parseInt(counter.innerHTML) + 1;
    }

    function updateFieldIfNotNull(fieldId, value, precision = 2) {
        if (value != null) {
            document.getElementById(fieldId).innerHTML = value.toFixed(precision);
        }
    }

    function handleOrientation(event) {
        const alpha = event.alpha || 0;
        const beta = event.beta || 0;
        
        updateFieldIfNotNull('Orientation_a', alpha);
        updateFieldIfNotNull('Orientation_b', beta);
        incrementEventCount();

        // Rate limiting: only send if enough time has passed
        const now = Date.now();
        if (now - lastSendTime < SEND_DELAY) {
            return; // Skip this event
        }
        lastSendTime = now;

        // Send orientation to Flask server (which forwards to ESP32)
        fetch('/gyro', {  // Relative URL - automatically uses your ngrok domain
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ alpha, beta })
        })
        .then(response => {
            if (response.ok) {
                successCount++;
                document.getElementById('num-success').textContent = successCount;
                updateStatus('📡 Tracking & sending data...', 'connected');
            } else {
                throw new Error(`Server returned ${response.status}`);
            }
        })
        .catch(err => {
            failCount++;
            document.getElementById('num-failed').textContent = failCount;
            log('POST error: ' + err.message, 'error');
            updateStatus('⚠️ Tracking but connection error', 'error');
        });
    }

    // iOS permission request
    async function requestPermission() {
        if (typeof DeviceOrientationEvent.requestPermission === 'function') {
            try {
                const perm = await DeviceOrientationEvent.requestPermission();
                log('Orientation permission: ' + perm, perm === 'granted' ? 'success' : 'error');
                return perm === 'granted';
            } catch (err) {
                log('Permission denied: ' + err, 'error');
                alert("Permission denied: " + err);
                return false;
            }
        }
        return true; // Android doesn't need permission
    }

    demoButton.onclick = async function (e) {
        e.preventDefault();

        if (isRunning) {
            window.removeEventListener('deviceorientation', handleOrientation);
            demoButton.innerText = "Disconnected";
            demoButton.classList.remove('stop');
            isRunning = false;
            updateStatus('Stopped', '');
            log('Demo stopped');
        } else {
            const granted = await requestPermission();
            if (!granted) {
                updateStatus('Permission denied', 'error');
                return;
            }

            window.addEventListener('deviceorientation', handleOrientation);
            demoButton.innerText = "Targetting";
            demoButton.classList.add('stop');
            isRunning = true;
            updateStatus('🎯 Starting...', 'tracking');
            log('Demo started', 'success');
        }
    }

    // Test connection on load
    fetch('/gyro', { method: 'OPTIONS' })
        .then(() => log('✅ Connected to server', 'success'))
        .catch(() => log('❌ Cannot reach server', 'error'));

    // Trigger permission request on first click anywhere (iOS requirement)
    document.body.addEventListener('click', requestPermission, { once: true });