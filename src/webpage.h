const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">

<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>TempTron 607 A-C - Dashboard</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); min-height: 100vh; padding: 20px; }
    .container { max-width: 1200px; margin: 0 auto; }
    .header { background: white; padding: 20px 30px; border-radius: 15px; box-shadow: 0 10px 30px rgba(0, 0, 0, 0.2); margin-bottom: 20px; display: flex; justify-content: space-between; align-items: center; }
    .header h1 { color: #667eea; font-size: 2em; margin-bottom: 5px; }
    .header p { color: #666; font-size: 0.9em; }
    .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 20px; margin-bottom: 20px; }
    .card { background: white; border-radius: 15px; padding: 25px; box-shadow: 0 10px 30px rgba(0, 0, 0, 0.2); }
    .card h2 { color: #333; font-size: 1.3em; margin-bottom: 15px; border-bottom: 2px solid #667eea; padding-bottom: 10px; }
    .metric { display: flex; justify-content: space-between; align-items: center; margin: 15px 0; padding: 15px; background: #f8f9fa; border-radius: 10px; }
    .metric-label { color: #666; font-weight: 600; }
    .metric-value { font-size: 1.8em; font-weight: bold; color: #667eea; }
    .metric-value.temp { color: #ff6b6b; }
    .metric-value.humidity { color: #4ecdc4; }
    .status-grid { display: grid; grid-template-columns: repeat(2, 1fr); gap: 10px; margin-top: 15px; }
    .status-item { display: flex; align-items: center; gap: 10px; padding: 10px; background: #f8f9fa; border-radius: 8px; }
    .led { width: 20px; height: 20px; border-radius: 50%; background: #ccc; box-shadow: inset 0 2px 4px rgba(0, 0, 0, 0.3); transition: all 0.3s; }
    .led.on { box-shadow: 0 0 15px currentColor, inset 0 1px 2px rgba(255, 255, 255, 0.3); }
    .led.fan { background: #4ecdc4; } .led.fan.on { background: #4ecdc4; }
    .led.heater { background: #ff6b6b; } .led.heater.on { background: #ff6b6b; }
    .led.alarm { background: #ffd93d; } .led.alarm.on { background: #ffd93d; animation: blink 1s infinite; }
    .control-form { margin-top: 20px; }
    .form-group { margin-bottom: 15px; }
    .form-group label { display: block; color: #666; font-weight: 600; margin-bottom: 5px; }
    .form-group input { width: 100%; padding: 12px; border: 2px solid #e0e0e0; border-radius: 8px; font-size: 1em; transition: border-color 0.3s; }
    .form-group input:focus { outline: none; border-color: #667eea; }
    .btn { background: #667eea; color: white; border: none; padding: 12px 30px; border-radius: 8px; font-size: 1em; font-weight: 600; cursor: pointer; transition: all 0.3s; width: 100%; }
    .btn:hover { background: #5568d3; transform: translateY(-2px); box-shadow: 0 5px 15px rgba(102, 126, 234, 0.4); }
    .btn:active { transform: translateY(0); }
    .btn-relay { background: #667eea; color: white; border: none; padding: 15px 20px; border-radius: 8px; font-size: 0.95em; font-weight: 600; cursor: pointer; transition: all 0.3s; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.15); }
    .btn-relay:hover { transform: translateY(-2px); box-shadow: 0 6px 12px rgba(0, 0, 0, 0.25); }
    .btn-relay:active { transform: translateY(0); }
    .btn-relay.active { box-shadow: 0 0 20px currentColor, inset 0 2px 4px rgba(0, 0, 0, 0.2); filter: brightness(1.1); }
    .alert { padding: 15px; border-radius: 8px; margin-bottom: 15px; font-weight: 600; }
    .alert.success { background: #d4edda; color: #155724; border: 1px solid #c3e6cb; }
    .alert.error { background: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; }
    .timestamp { color: #999; font-size: 0.85em; text-align: center; margin-top: 10px; }
    @keyframes blink { 0%, 50% { opacity: 1; } 51%, 100% { opacity: 0.3; } }
    @media (max-width: 768px) { .grid { grid-template-columns: 1fr; } .status-grid { grid-template-columns: 1fr; } }
  </style>
</head>

<body>
  <div class="container">
    <div class="header">
      <div>
        <h1>🌡️ TempTron 607 A-C Dashboard</h1>
        <p>Temperature Controller - Real-time Monitoring & Control</p>
      </div>
      <!-- Removed External Link -->
    </div>

    <div class="grid">
      <!-- Current Status Card -->
      <div class="card">
        <h2>📊 Current Status</h2>
        <div class="metric">
          <span class="metric-label">Temperature</span>
          <span class="metric-value temp" id="currentTemp">--</span>
        </div>
        <div class="metric">
          <span class="metric-label">Humidity</span>
          <span class="metric-value humidity" id="currentHumidity">--</span>
        </div>
        <div class="metric">
          <span class="metric-label">Setpoint</span>
          <span class="metric-value" id="currentSetpoint">--</span>
        </div>
        <div class="timestamp" id="lastUpdate">Last update: --</div>
      </div>

      <!-- Control Panel Card -->
      <div class="card">
        <h2>🎛️ Control Panel</h2>
        <form class="control-form" id="setpointForm">
          <div class="form-group">
            <label for="newSetpoint">Set Temperature (20-100°C)</label>
            <input type="number" id="newSetpoint" name="setpoint" min="20" max="100" step="0.1"
              placeholder="Enter setpoint">
          </div>
          <button type="submit" class="btn">Update Setpoint</button>
        </form>
        <div id="formMessage"></div>
      </div>

      <!-- Cooling Status (Kipas 1-6) -->
      <div class="card">
        <h2>❄️ Cooling Status (Fans)</h2>
        <div class="status-grid">
          <div class="status-item"> <div class="led fan" id="fan1"></div> <span>Fan 1 (+1°C)</span> </div>
          <div class="status-item"> <div class="led fan" id="fan2"></div> <span>Fan 2 (+2°C)</span> </div>
          <div class="status-item"> <div class="led fan" id="fan3"></div> <span>Fan 3 (+3°C)</span> </div>
          <div class="status-item"> <div class="led fan" id="fan4"></div> <span>Fan 4 (+4°C)</span> </div>
          <div class="status-item"> <div class="led fan" id="fan5"></div> <span>Fan 5 (+5°C)</span> </div>
          <div class="status-item"> <div class="led fan" id="fan6"></div> <span>Fan 6 (+6°C)</span> </div>
        </div>
      </div>

      <!-- Heating Status (Heater 7-8) -->
      <div class="card">
        <h2>🔥 Heating Status</h2>
        <div class="status-grid">
          <div class="status-item">
            <div class="led heater" id="heater7"></div>
            <span>Heater 7 (-2°C)</span>
          </div>
          <div class="status-item">
            <div class="led fan" id="cooling"></div>
            <span>Cooling (-4°C)</span>
          </div>
        </div>
      </div>

      <!-- Alarm Status -->
      <div class="card">
        <h2>🚨 Alarm Status</h2>
        <div class="status-item">
          <div class="led alarm" id="alarm"></div>
          <span id="alarmText">Normal</span>
        </div>
        <div class="metric" style="margin-top: 15px;">
          <span class="metric-label">Alarm Range</span>
          <span class="metric-value" style="font-size: 1.2em;" id="alarmRange">--</span>
        </div>
      </div>

      <!-- System Info -->
      <div class="card">
        <h2>ℹ️ System Info</h2>
        <div class="status-item">
          <div class="led" id="systemStatus" style="background: #95e1d3;"></div>
          <span id="systemText">Checking...</span>
        </div>
        <div class="metric" style="margin-top: 15px;">
          <span class="metric-label">Device ID</span>
          <span style="font-size: 0.9em; color: #666;" id="deviceId">--</span>
        </div>
      </div>

      <!-- Manual Control Panel -->
      <div class="card" style="grid-column: 1 / -1;">
        <h2>🔧 Manual Control (Testing/Maintenance)</h2>
        <p style="color: #666; margin-bottom: 10px; font-size: 0.9em;">⚠️ Gunakan hanya untuk testing. Mode manual akan override kontrol otomatis.</p>

        <div class="status-item" style="margin-bottom: 15px;">
          <div class="led" id="manualModeLed" style="background:#ccc;"></div>
          <span id="manualModeText">Mode: AUTO</span>
        </div>

        <div style="display: grid; grid-template-columns: repeat(auto-fit, minmax(150px, 1fr)); gap: 10px; margin-bottom: 20px;">
          <button class="btn-relay" data-relay="fan1" style="background: #4ecdc4;">Fan 1</button>
          <button class="btn-relay" data-relay="fan2" style="background: #4ecdc4;">Fan 2</button>
          <button class="btn-relay" data-relay="fan3" style="background: #4ecdc4;">Fan 3</button>
          <button class="btn-relay" data-relay="fan4" style="background: #4ecdc4;">Fan 4</button>
          <button class="btn-relay" data-relay="fan5" style="background: #4ecdc4;">Fan 5</button>
          <button class="btn-relay" data-relay="fan6" style="background: #4ecdc4;">Fan 6</button>
          <button class="btn-relay" data-relay="heater7" style="background: #ff6b6b;">Heater 7</button>
          <button class="btn-relay" data-relay="cooling" style="background: #3498db;">Cooling</button>
        </div>

        <button id="btnStopAll" class="btn" style="background: #e74c3c; margin-bottom: 15px;">⏹️ STOP ALL</button>
        <div id="manualMessage"></div>
      </div>
    </div>
  </div>

  <script>
    // Fetch telemetry data
    async function fetchData() {
      try {
        const response = await fetch('/status', { cache: 'no-store' }); // CHANGED URL
        if (!response.ok) throw new Error('Network error');

        const data = await response.json();
        updateDashboard(data);
      } catch (error) {
        console.error('Error fetching data:', error);
        document.getElementById('systemText').textContent = 'Connection Error';
      }
    }

    // Store manual state globally
    let manualState = null;

    function updateDashboard(data) {
      const telemetry = data.telemetry || {};
      const config = data.config || {};
      
      // Update local logic: if state is present, use it
      if (data.state) {
        manualState = data.state;
      }

      // Update temperature & humidity
      if (telemetry.temp !== undefined) {
        document.getElementById('currentTemp').textContent = telemetry.temp.toFixed(1) + '°C';
      }
      if (telemetry.humidity !== undefined) {
        document.getElementById('currentHumidity').textContent = telemetry.humidity.toFixed(1) + '%';
      }
      if (telemetry.setpoint !== undefined) {
        document.getElementById('currentSetpoint').textContent = telemetry.setpoint.toFixed(1) + '°C';
      } else if (config.setpoint !== undefined) {
        document.getElementById('currentSetpoint').textContent = config.setpoint.toFixed(1) + '°C';
      }

      // Update timestamp
      document.getElementById('lastUpdate').textContent = 'Last update: ' + new Date().toLocaleTimeString();

      // Check if manual mode is active
      const isManualMode = manualState && (manualState.manual === 1 || manualState.manual === '1' || manualState.manual === true);
      const manualModeLed = document.getElementById('manualModeLed');
      const manualModeText = document.getElementById('manualModeText');
      
      manualModeLed.classList.toggle('on', isManualMode);
      manualModeText.textContent = isManualMode ? 'Mode: MANUAL' : 'Mode: AUTO';

      // Update Buttons State if in manual mode
      const relayButtons = document.querySelectorAll('.btn-relay');
      relayButtons.forEach(btn => {
        const key = btn.dataset.relay;
        if (manualState && manualState[key] !== undefined) {
           const val = manualState[key];
           const active = val === 1 || val === '1' || val === true;
           btn.classList.toggle('active', active);
        }
      });

      // Update LEDs
      // In this version, we trust the 'state' from server for actual ON/OFF status
      // whether it is auto or manual
      const s = manualState || {};
      const isOn = (val) => val === 1 || val === '1' || val === true;

      document.getElementById('fan1').classList.toggle('on', isOn(s.fan1));
      document.getElementById('fan2').classList.toggle('on', isOn(s.fan2));
      document.getElementById('fan3').classList.toggle('on', isOn(s.fan3));
      document.getElementById('fan4').classList.toggle('on', isOn(s.fan4));
      document.getElementById('fan5').classList.toggle('on', isOn(s.fan5));
      document.getElementById('fan6').classList.toggle('on', isOn(s.fan6));
      document.getElementById('heater7').classList.toggle('on', isOn(s.heater7));
      document.getElementById('cooling').classList.toggle('on', isOn(s.cooling));
    
      // Update Alarm
      const temp = telemetry.temp || 0;
      const lowerLimit = config.lowerLimit || 25;
      const upperLimit = config.upperLimit || 35;
      const alarmActive = temp < lowerLimit || temp > upperLimit;

      document.getElementById('alarm').classList.toggle('on', alarmActive);
      document.getElementById('alarmText').textContent = alarmActive ? 'ALARM ACTIVE!' : 'Normal';
      document.getElementById('alarmRange').textContent = lowerLimit + '°C - ' + upperLimit + '°C';

      // System status
      document.getElementById('systemStatus').classList.add('on');
      document.getElementById('systemText').textContent = 'System Active';

      // Device ID
      if (telemetry.device_id) {
        document.getElementById('deviceId').textContent = telemetry.device_id;
      }
    }

    // Handle setpoint form submission
    document.getElementById('setpointForm').addEventListener('submit', async (e) => {
      e.preventDefault();

      const setpoint = parseFloat(document.getElementById('newSetpoint').value);
      const messageDiv = document.getElementById('formMessage');

      if (setpoint < 20 || setpoint > 100) {
        messageDiv.innerHTML = '<div class="alert error">Setpoint must be between 20-100°C</div>';
        return;
      }

      try {
        // Use JSON body for advanced parsing, or simplified logic in wiring.ino
        const response = await fetch('/config', { // CHANGED URL
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ setpoint: setpoint })
        });

        if (response.ok) {
          messageDiv.innerHTML = '<div class="alert success">Setpoint updated successfully!</div>';
          document.getElementById('newSetpoint').value = '';
          setTimeout(() => { messageDiv.innerHTML = ''; }, 3000);
          fetchData(); // Refresh data
        } else {
          throw new Error('Update failed');
        }
      } catch (error) {
        messageDiv.innerHTML = '<div class="alert error">Failed to update setpoint</div>';
      }
    });

    // ========================================
    // Manual Control (Testing/Maintenance)
    // ========================================

    const relayButtons = document.querySelectorAll('.btn-relay');
    const stopAllButton = document.getElementById('btnStopAll');
    const manualMessageDiv = document.getElementById('manualMessage');

    async function sendManualUpdate(payload) {
      try {
        const res = await fetch('/manual', { // CHANGED URL
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify(payload)
        });
        if (!res.ok) throw new Error('Network error');
        
        // Optimistic update or success message
        manualMessageDiv.innerHTML = '<div class="alert success">Command sent</div>';
        setTimeout(() => { manualMessageDiv.innerHTML = ''; }, 1000);
        
        // Refresh status immediately
        fetchData();
      } catch (err) {
        console.error('Manual update error', err);
        manualMessageDiv.innerHTML = '<div class="alert error">Failed to send command</div>';
      }
    }

    relayButtons.forEach(btn => {
      btn.addEventListener('click', () => {
        const relayKey = btn.dataset.relay;
        const isActive = btn.classList.contains('active');
        const newState = !isActive;

        const payload = { manual: 1 };
        payload[relayKey] = newState ? 1 : 0;
        sendManualUpdate(payload);
      });
    });

    if (stopAllButton) {
      stopAllButton.addEventListener('click', () => {
        const payload = { manual: 0 };
        sendManualUpdate(payload);
      });
    }

    // Auto-refresh every 2 seconds
    setInterval(fetchData, 2000);
    fetchData();
  </script>
</body>
</html>
)rawliteral";
