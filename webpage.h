const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">

<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>TempTron 607 A-C Controller</title>
  <style>
    * {
      margin: 0;
      padding: 0;
      box-sizing: border-box;
    }

    body {
      font-family: Arial, sans-serif;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      min-height: 100vh;
      display: flex;
      justify-content: center;
      align-items: center;
      padding: 20px;
    }

    #app {
      width: 100%;
      max-width: 800px;
    }

    .controller {
      background: linear-gradient(145deg, #f4e04d, #dcc943);
      border-radius: 30px;
      padding: 40px;
      box-shadow: 0 20px 60px rgba(0, 0, 0, 0.3);
      position: relative;
    }

    .controller::before,
    .controller::after {
      content: '';
      position: absolute;
      width: 20px;
      height: 20px;
      background: #c0c0c0;
      border-radius: 50%;
      border: 3px solid #808080;
      box-shadow: inset 2px 2px 5px rgba(0, 0, 0, 0.3);
    }

    .controller::before {
      top: 20px;
      left: 20px;
    }

    .controller::after {
      top: 20px;
      right: 20px;
    }

    .screw-bottom-left,
    .screw-bottom-right {
      position: absolute;
      width: 20px;
      height: 20px;
      background: #c0c0c0;
      border-radius: 50%;
      border: 3px solid #808080;
      box-shadow: inset 2px 2px 5px rgba(0, 0, 0, 0.3);
      bottom: 20px;
    }

    .screw-bottom-left {
      left: 20px;
    }

    .screw-bottom-right {
      right: 20px;
    }

    .panel {
      background: linear-gradient(145deg, #5dd9a6, #4bc593);
      border-radius: 20px;
      padding: 30px;
      position: relative;
      box-shadow: inset 0 2px 10px rgba(0, 0, 0, 0.1);
    }

    .header {
      display: flex;
      justify-content: space-between;
      align-items: flex-start;
      margin-bottom: 25px;
    }

    .title-section h1 {
      color: #1a3d5c;
      font-size: 2.2em;
      font-weight: bold;
      margin-bottom: 10px;
    }

    .checkboxes {
      display: flex;
      flex-direction: column;
      gap: 5px;
      color: #1a3d5c;
      font-weight: 600;
    }

    .checkbox-item {
      display: flex;
      align-items: center;
      gap: 8px;
    }

    .checkbox {
      width: 20px;
      height: 20px;
      border: 2px solid #1a3d5c;
      background: white;
      display: flex;
      align-items: center;
      justify-content: center;
      cursor: pointer;
    }

    .checkbox.checked::after {
      content: '✓';
      color: #1a3d5c;
      font-weight: bold;
    }

    .chicken-icon {
      font-size: 2em;
      position: absolute;
      top: 80px;
      right: 40px;
    }

    .main-content {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 30px;
      margin-bottom: 25px;
    }

    .left-section {
      display: flex;
      flex-direction: column;
      gap: 20px;
    }

    .menu-list {
      color: #1a3d5c;
      font-size: 0.85em;
      line-height: 1.6;
    }

    .logo-badge {
      background: white;
      padding: 8px 20px;
      border-radius: 20px;
      text-align: center;
      box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);
      margin-top: auto;
    }

    .logo-badge .agro {
      color: #5dd9a6;
      font-weight: bold;
      font-size: 1.2em;
    }

    .logo-badge .logic {
      color: #2b5c9e;
      font-weight: bold;
      font-size: 1.2em;
    }

    .right-section {
      display: flex;
      flex-direction: column;
      gap: 20px;
    }

    .displays {
      display: flex;
      gap: 15px;
      justify-content: center;
      margin-bottom: 10px;
    }

    .display {
      background: #2a0a0a;
      border: 3px solid #4a1a1a;
      border-radius: 8px;
      padding: 15px 20px;
      font-family: 'Courier New', monospace;
      font-size: 2.5em;
      color: #ff3333;
      text-align: center;
      min-width: 140px;
      box-shadow: inset 0 2px 10px rgba(0, 0, 0, 0.5);
    }

    .display.small {
      font-size: 2em;
      min-width: 80px;
      padding: 15px;
    }

    .keypad {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      gap: 12px;
      margin-bottom: 15px;
    }

    .key {
      background: #2b5c9e;
      color: white;
      border: none;
      border-radius: 50%;
      width: 60px;
      height: 60px;
      font-size: 1.5em;
      font-weight: bold;
      cursor: pointer;
      transition: all 0.2s;
      box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
      margin: 0 auto;
    }

    .key:hover {
      background: #3d7bc9;
      transform: translateY(-2px);
      box-shadow: 0 6px 12px rgba(0, 0, 0, 0.3);
    }

    .key:active {
      transform: translateY(0);
      box-shadow: 0 2px 4px rgba(0, 0, 0, 0.2);
    }

    .key.special {
      font-size: 0.9em;
      padding: 5px;
    }

    .prog-key {
      background: #2b5c9e;
      color: white;
      border: none;
      border-radius: 25px;
      padding: 12px 30px;
      font-size: 1.1em;
      font-weight: bold;
      cursor: pointer;
      transition: all 0.2s;
      box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
      grid-column: 1 / -1;
      width: 150px;
      margin: 0 auto;
    }

    .prog-key:hover {
      background: #3d7bc9;
      transform: translateY(-2px);
    }

    .indicators {
      display: flex;
      flex-direction: column;
      gap: 12px;
    }

    .indicator {
      display: flex;
      align-items: center;
      gap: 12px;
      font-weight: 600;
      color: #1a3d5c;
    }

    .indicator-icon {
      font-size: 1.5em;
    }

    .indicator-light {
      width: 16px;
      height: 16px;
      border-radius: 50%;
      background: #8b0000;
      box-shadow: inset 0 2px 4px rgba(0, 0, 0, 0.3);
      transition: all 0.3s;
    }

    .indicator-light.active {
      background: #ff0000;
      box-shadow: 0 0 10px #ff0000, inset 0 1px 2px rgba(255, 255, 255, 0.3);
    }

    @media (max-width: 768px) {
      .controller {
        padding: 25px;
      }

      .panel {
        padding: 20px;
      }

      .title-section h1 {
        font-size: 1.5em;
      }

      .main-content {
        grid-template-columns: 1fr;
      }

      .display {
        font-size: 2em;
        min-width: 100px;
        padding: 10px 15px;
      }

      .display.small {
        font-size: 1.5em;
        min-width: 70px;
        padding: 10px;
      }

      .key {
        width: 50px;
        height: 50px;
        font-size: 1.2em;
      }

      .chicken-icon {
        position: static;
        display: block;
        text-align: center;
        margin: 15px 0;
      }
    }
  </style>
</head>

<body>
  <div id="app">
    <a href="index.php" style="
      position: fixed;
      top: 20px;
      right: 20px;
      background: rgba(255, 255, 255, 0.2);
      color: white;
      padding: 10px 20px;
      border-radius: 20px;
      text-decoration: none;
      font-weight: bold;
      backdrop-filter: blur(5px);
      border: 1px solid rgba(255,255,255,0.3);
      transition: all 0.3s;
      z-index: 1000;
    ">📊 Dashboard View</a>
    <div class="controller">
      <div class="screw-bottom-left"></div>
      <div class="screw-bottom-right"></div>

      <div class="panel">
        <div class="header">
          <div class="title-section">
            <h1>TempTron 607 A-C</h1>
            <div class="checkboxes">
              <div class="checkbox-item">
                <div class="checkbox checked"></div>
                <span>Used For Broilers</span>
              </div>
              <div class="checkbox-item">
                <div class="checkbox checked"></div>
                <span>Breeders</span>
              </div>
              <div class="checkbox-item">
                <div class="checkbox checked"></div>
                <span>Layers</span>
              </div>
            </div>
          </div>
          <div class="chicken-icon">🐔</div>
        </div>

        <div class="main-content">
          <div class="left-section">
            <div class="menu-list">
              01. Clock<br>
              02. Required Temp<br>
              03. Heat<br>
              04. Fan 1<br>
              05. Fan 2<br>
              06. Fan 3<br>
              07. Fan 4<br>
              08. Fan 5<br>
              09. Fan On Time<br>
              10. Fan Off Time<br>
              11. Humidity Set<br>
              12. Cool Temp<br>
              13. Cool On Time<br>
              14. Cool Off Time<br>
              15. Low Alarm<br>
              16. High Alarm<br>
              17. Water Clock<br>
              18. Feed Mult<br>
              19. Daily Feed<br>
              20. Total Feed<br>
              21. Day 1 Temp<br>
              22. Temp Graph<br>
              31. Grow Day<br>
              32. Reset Time
            </div>
            <div class="logo-badge">
              <span class="agro">Agro</span><span class="logic">Logic</span><sup>®</sup>
            </div>
          </div>

          <div class="right-section">
            <div class="displays">
              <div class="display" id="mainDisplay">28.4</div>
              <div class="display small" id="smallDisplay">88</div>
            </div>

            <div class="keypad">
              <button class="key" data-key="1">1</button>
              <button class="key" data-key="2">2</button>
              <button class="key" data-key="3">3</button>
              <button class="key" data-key="4">4</button>
              <button class="key" data-key="5">5</button>
              <button class="key" data-key="6">6</button>
              <button class="key" data-key="7">7</button>
              <button class="key" data-key="8">8</button>
              <button class="key" data-key="9">9</button>
              <button class="key special" data-key="ENTER">ENTER</button>
              <button class="key" data-key="0">0</button>
              <button class="key special" data-key="DATA">DATA</button>
              <button class="prog-key" data-key="PROG">PROG</button>
            </div>

            <div class="indicators">
              <div class="indicator">
                <span class="indicator-icon">🔊</span>
                <div class="indicator-light" id="alarm"></div>
                <span>ALARM</span>
              </div>
              <div class="indicator">
                <span class="indicator-icon">🔥</span>
                <div class="indicator-light active" id="heat"></div>
                <span>HEAT</span>
              </div>
              <div class="indicator">
                <span class="indicator-icon">🌀</span>
                <div class="indicator-light" id="fan1"></div>
                <span>FAN 1</span>
              </div>
              <div class="indicator">
                <span class="indicator-icon">🌀</span>
                <div class="indicator-light" id="fan2"></div>
                <span>FAN 2</span>
              </div>
              <div class="indicator">
                <span class="indicator-icon">🌀</span>
                <div class="indicator-light" id="fan3"></div>
                <span>FAN 3</span>
              </div>
              <div class="indicator">
                <span class="indicator-icon">🌀</span>
                <div class="indicator-light" id="fan4"></div>
                <span>FAN 4</span>
              </div>
              <div class="indicator">
                <span class="indicator-icon">🌀</span>
                <div class="indicator-light" id="fan5"></div>
                <span>FAN 5</span>
              </div>
              <div class="indicator">
                <span class="indicator-icon">❄️</span>
                <div class="indicator-light" id="cool"></div>
                <span>COOL</span>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>

  <script>
    // ============================================
    // TEMPTRON 607 Controller - Based on Manual
    // ============================================

    // State variables
    let currentInput = ''
    let selectedFunction = 0
    let isEditMode = false
    let isNavigationMode = false
    let hiddenFunctionsUnlocked = false
    let flashInterval = null

    // Function definitions based on TEMPTRON 607 manual
    const functionDefs = {
      0: { name: 'Display', key: 'display', type: 'readonly', unit: '°C' },
      1: { name: 'Clock', key: 'clock', type: 'time', unit: '' },
      2: { name: 'Required Temp', key: 'setpoint', type: 'temp', unit: '°C', min: 10, max: 45 },
      3: { name: 'Heat', key: 'heatTemp', type: 'temp', unit: '°C', min: 10, max: 40 },
      4: { name: 'Fan 1', key: 'fan1Temp', type: 'temp', unit: '°C', min: 15, max: 45 },
      5: { name: 'Fan 2', key: 'fan2Temp', type: 'temp', unit: '°C', min: 15, max: 45 },
      6: { name: 'Fan 3', key: 'fan3Temp', type: 'temp', unit: '°C', min: 15, max: 45 },
      7: { name: 'Fan 4', key: 'fan4Temp', type: 'temp', unit: '°C', min: 15, max: 45 },
      8: { name: 'Fan 5', key: 'fan5Temp', type: 'temp', unit: '°C', min: 15, max: 45 },
      9: { name: 'Fan On Time', key: 'fanOnTime', type: 'number', unit: 's', min: 0, max: 999 },
      10: { name: 'Fan Off Time', key: 'fanOffTime', type: 'number', unit: 's', min: 0, max: 999 },
      11: { name: 'Humidity Set', key: 'humiditySet', type: 'number', unit: '%', min: 0, max: 100 },
      12: { name: 'Cool Temp', key: 'coolTemp', type: 'temp', unit: '°C', min: 20, max: 50 },
      13: { name: 'Cool On Time', key: 'coolOnTime', type: 'number', unit: 's', min: 0, max: 999 },
      14: { name: 'Cool Off Time', key: 'coolOffTime', type: 'number', unit: 's', min: 0, max: 999 },
      15: { name: 'Low Alarm', key: 'lowAlarm', type: 'temp', unit: '°C', min: 5, max: 30 },
      16: { name: 'High Alarm', key: 'highAlarm', type: 'temp', unit: '°C', min: 25, max: 50 },
      17: { name: 'Water Clock', key: 'waterClock', type: 'time', unit: '' },
      18: { name: 'Feed Mult', key: 'feedMult', type: 'number', unit: '', min: 1, max: 99 },
      19: { name: 'Daily Feed', key: 'dailyFeed', type: 'number', unit: 'kg', min: 0, max: 9999 },
      20: { name: 'Total Feed', key: 'totalFeed', type: 'number', unit: 'kg', min: 0, max: 99999 },
      21: { name: 'Day 1 Temp', key: 'day1Temp', type: 'temp', unit: '°C', min: 20, max: 40 },
      22: { name: 'Temp Graph', key: 'tempGraph', type: 'readonly', unit: '' },
      31: { name: 'Grow Day', key: 'growDay', type: 'number', unit: '', min: 1, max: 99 },
      32: { name: 'Reset Time', key: 'resetTime', type: 'time', unit: '' },
      // Hidden functions (33-40) - unlocked with code 3331
      33: { name: 'Lock Code', key: 'lockCode', type: 'code', unit: '', min: 0, max: 9999, hidden: true },
      34: { name: 'Temp Offset', key: 'tempOffset', type: 'number', unit: '°C', min: -9, max: 9, hidden: true },
      35: { name: 'Sensor Type', key: 'sensorType', type: 'number', unit: '', min: 0, max: 3, hidden: true },
      36: { name: 'Alarm Delay', key: 'alarmDelay', type: 'number', unit: 's', min: 0, max: 999, hidden: true },
      37: { name: 'Feed Mode', key: 'feedMode', type: 'number', unit: '', min: 0, max: 2, hidden: true },
      38: { name: 'Fan Mode', key: 'fanMode', type: 'number', unit: '', min: 0, max: 3, hidden: true },
      39: { name: 'Alarm Type', key: 'alarmType', type: 'number', unit: '', min: 0, max: 2, hidden: true },
      40: { name: 'Brightness', key: 'displayBrightness', type: 'number', unit: '', min: 1, max: 15, hidden: true }
    }

    // Parameter values storage (will be loaded from API)
    let parameters = {}

    // DOM elements
    const keys = document.querySelectorAll('.key, .prog-key')
    const mainDisplay = document.getElementById('mainDisplay')
    const smallDisplay = document.getElementById('smallDisplay')

    // Initialize event listeners
    keys.forEach(key => {
      key.addEventListener('click', () => {
        const value = key.dataset.key
        handleKeyPress(value)
      })
    })

    // ============================================
    // MAIN KEY HANDLER - Based on TEMPTRON Manual
    // ============================================
    function handleKeyPress(value) {
      // Handle numeric keys
      if (value >= '0' && value <= '9') {
        handleNumericKey(value)
      }
      // Handle ENTER key
      else if (value === 'ENTER') {
        handleEnterKey()
      }
      // Handle DATA key - go to next function
      else if (value === 'DATA') {
        handleDataKey()
      }
      // Handle PROG key - enter edit mode
      else if (value === 'PROG') {
        handleProgKey()
      }
    }

    // ============================================
    // NUMERIC KEY HANDLER
    // ============================================
    function handleNumericKey(digit) {
      // If first digit is 0 and not in edit mode, start navigation mode
      if (digit === '0' && currentInput === '' && !isEditMode) {
        isNavigationMode = true
        currentInput = ''
        mainDisplay.textContent = '0_'
        return
      }

      // In navigation mode, build function number
      if (isNavigationMode) {
        currentInput += digit
        mainDisplay.textContent = currentInput

        // If we have 2 digits, navigate to that function
        if (currentInput.length >= 2) {
          const funcNum = parseInt(currentInput)
          navigateToFunction(funcNum)
          isNavigationMode = false
          currentInput = ''
        }
        return
      }

      // In edit mode, build value
      if (isEditMode) {
        if (currentInput.length < 5) {
          currentInput += digit
          mainDisplay.textContent = currentInput
        }
        return
      }

      // Default: build input for quick setpoint change (function 0)
      if (selectedFunction === 0) {
        if (currentInput.length < 4) {
          currentInput += digit
          mainDisplay.textContent = currentInput
        }
      }
    }

    // ============================================
    // NAVIGATE TO FUNCTION
    // ============================================
    function navigateToFunction(funcNum) {
      // Check if function exists
      if (!functionDefs[funcNum]) {
        showError('Err')
        return
      }

      // Check if hidden function and not unlocked
      if (functionDefs[funcNum].hidden && !hiddenFunctionsUnlocked) {
        showError('LOC')
        return
      }

      selectedFunction = funcNum
      smallDisplay.textContent = funcNum.toString().padStart(2, '0')

      // Show current value for this function
      displayFunctionValue(funcNum)
    }

    // ============================================
    // DISPLAY FUNCTION VALUE
    // ============================================
    function displayFunctionValue(funcNum) {
      const def = functionDefs[funcNum]
      if (!def) return

      if (funcNum === 0) {
        // Function 0 shows current temperature
        const temp = parameters.temp
        mainDisplay.textContent = temp !== undefined ? temp.toFixed(1) : '---'
      } else {
        // Other functions show their stored value
        const value = parameters[def.key]
        if (value !== undefined) {
          if (def.type === 'time') {
            // Format time as HH:MM or MM:SS
            mainDisplay.textContent = formatTime(value)
          } else if (def.type === 'temp') {
            mainDisplay.textContent = value.toFixed(1)
          } else {
            mainDisplay.textContent = value.toString()
          }
        } else {
          mainDisplay.textContent = '---'
        }
      }
    }

    // ============================================
    // DATA KEY - Go to next function
    // ============================================
    function handleDataKey() {
      if (isEditMode) {
        // Cancel edit mode
        stopFlashing()
        isEditMode = false
        currentInput = ''
        displayFunctionValue(selectedFunction)
        return
      }

      // Go to next function
      let nextFunc = selectedFunction + 1

      // Skip to valid functions
      const maxFunc = hiddenFunctionsUnlocked ? 40 : 32
      while (nextFunc <= maxFunc && !functionDefs[nextFunc]) {
        nextFunc++
      }

      // Wrap around
      if (nextFunc > maxFunc) {
        nextFunc = 0
      }

      selectedFunction = nextFunc
      smallDisplay.textContent = nextFunc.toString().padStart(2, '0')
      currentInput = ''
      displayFunctionValue(nextFunc)
    }

    // ============================================
    // PROG KEY - Enter edit mode
    // ============================================
    function handleProgKey() {
      const def = functionDefs[selectedFunction]

      if (!def || def.type === 'readonly') {
        showError('---')
        return
      }

      if (isEditMode) {
        // Already in edit mode, cancel
        stopFlashing()
        isEditMode = false
        currentInput = ''
        displayFunctionValue(selectedFunction)
        return
      }

      // Enter edit mode
      isEditMode = true
      currentInput = ''

      // Start flashing the function display (as per manual)
      startFlashing()

      // Show current value as starting point
      mainDisplay.textContent = '_'
    }

    // ============================================
    // ENTER KEY - Confirm value
    // ============================================
    async function handleEnterKey() {
      // Check for hidden function unlock code
      if (selectedFunction === 1 && currentInput === '3331') {
        hiddenFunctionsUnlocked = true
        showSuccess('UnL')
        return
      }

      // Check for calibration code
      if (selectedFunction === 1 && currentInput === '4441') {
        showSuccess('CAL')
        return
      }

      if (isNavigationMode) {
        // If in navigation mode with partial input, try to navigate
        if (currentInput.length > 0) {
          const funcNum = parseInt(currentInput.padStart(2, '0'))
          navigateToFunction(funcNum)
        }
        isNavigationMode = false
        currentInput = ''
        return
      }

      if (!isEditMode && selectedFunction === 0) {
        // Quick setpoint change from function 0
        if (currentInput) {
          const val = parseFloat(currentInput)
          if (!isNaN(val)) {
            await saveParameter('setpoint', val)
          }
        }
        return
      }

      if (isEditMode && currentInput) {
        const def = functionDefs[selectedFunction]
        if (!def) return

        let val = parseFloat(currentInput)

        // Validate range
        if (def.min !== undefined && val < def.min) {
          showError('Lo')
          return
        }
        if (def.max !== undefined && val > def.max) {
          showError('Hi')
          return
        }

        // Save the value
        await saveParameter(def.key, val)

        // Exit edit mode
        stopFlashing()
        isEditMode = false
        currentInput = ''
      }
    }

    // ============================================
    // SAVE PARAMETER TO API
    // ============================================
    async function saveParameter(key, value) {
      try {
        mainDisplay.textContent = '...'

        // Change 'api/config' to '/config' to match ESP32 route
        const response = await fetch('/config', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ [key]: value })
        })

        if (response.ok) {
          parameters[key] = value
          showSuccess('Good')
        } else {
          throw new Error('API Error')
        }
      } catch (e) {
        console.error(e)
        showError('Err')
      }
    }

    // ============================================
    // DISPLAY HELPERS
    // ============================================
    function showError(msg) {
      mainDisplay.textContent = msg
      setTimeout(() => {
        currentInput = ''
        isNavigationMode = false
        displayFunctionValue(selectedFunction)
      }, 1500)
    }

    function showSuccess(msg) {
      mainDisplay.textContent = msg
      setTimeout(() => {
        currentInput = ''
        displayFunctionValue(selectedFunction)
      }, 1500)
    }

    function formatTime(value) {
      if (typeof value === 'string' && value.includes(':')) {
        return value
      }
      const hours = Math.floor(value / 60)
      const mins = value % 60
      return `${hours.toString().padStart(2, '0')}:${mins.toString().padStart(2, '0')}`
    }

    // ============================================
    // FLASHING EFFECT (Edit mode indicator)
    // ============================================
    function startFlashing() {
      let visible = true
      flashInterval = setInterval(() => {
        visible = !visible
        smallDisplay.style.opacity = visible ? '1' : '0.3'
      }, 500)
    }

    function stopFlashing() {
      if (flashInterval) {
        clearInterval(flashInterval)
        flashInterval = null
      }
      smallDisplay.style.opacity = '1'
    }

    // ============================================
    // FETCH STATUS FROM API
    // ============================================
    // const apiEndpoint = 'api/status.php'
    const apiEndpoint = '/status' // Update to match ESP32 endpoint

    async function fetchStatus() {
      try {
        const res = await fetch(apiEndpoint, { cache: 'no-store' })
        if (!res.ok) return
        const data = await res.json()
        if (!data) return

        const telemetry = data.telemetry || {}
        const config = data.config || {}

        // Store all parameters
        parameters = { ...config, ...telemetry }

        // Update display if not in edit mode
        if (!isEditMode && !isNavigationMode && !currentInput) {
          displayFunctionValue(selectedFunction)
        }

        // Update indicator lights
        updateIndicators(telemetry, config)
      } catch (e) {
        console.error('Fetch error:', e)
      }
    }

    // ============================================
    // UPDATE INDICATOR LIGHTS
    // ============================================
    function updateIndicators(telemetry, config) {
      const temp = typeof telemetry.temp === 'number' ? telemetry.temp : null
      const setpoint = config.setpoint || 30
      const heatTemp = config.heatTemp || 28
      const coolTemp = config.coolTemp || 32

      if (temp !== null) {
        // Heat indicator
        const heatActive = temp < heatTemp
        document.getElementById('heat').classList.toggle('active', heatActive)

        // Cool indicator
        const coolActive = temp > coolTemp
        document.getElementById('cool').classList.toggle('active', coolActive)

        // Alarm indicators
        const lowAlarm = config.lowAlarm || 15
        const highAlarm = config.highAlarm || 40
        const alarmActive = temp < lowAlarm || temp > highAlarm
        document.getElementById('alarm').classList.toggle('active', alarmActive)

        // Fan indicators (based on temperature thresholds)
        const fan1Temp = config.fan1Temp || setpoint + 1
        const fan2Temp = config.fan2Temp || setpoint + 2
        const fan3Temp = config.fan3Temp || setpoint + 3
        const fan4Temp = config.fan4Temp || setpoint + 4
        const fan5Temp = config.fan5Temp || setpoint + 5

        document.getElementById('fan1').classList.toggle('active', temp >= fan1Temp)
        document.getElementById('fan2').classList.toggle('active', temp >= fan2Temp)
        document.getElementById('fan3').classList.toggle('active', temp >= fan3Temp)
        document.getElementById('fan4').classList.toggle('active', temp >= fan4Temp)
        document.getElementById('fan5').classList.toggle('active', temp >= fan5Temp)
      }
    }

    // ============================================
    // INITIALIZE
    // ============================================
    // Initial display
    smallDisplay.textContent = '00'
    mainDisplay.textContent = '---'

    // Fetch status periodically
    setInterval(fetchStatus, 3000)
    fetchStatus()

    // Keyboard support for testing
    document.addEventListener('keydown', (e) => {
      if (e.key >= '0' && e.key <= '9') {
        handleKeyPress(e.key)
      } else if (e.key === 'Enter') {
        handleKeyPress('ENTER')
      } else if (e.key.toLowerCase() === 'd') {
        handleKeyPress('DATA')
      } else if (e.key.toLowerCase() === 'p') {
        handleKeyPress('PROG')
      }
    })
  </script>
</body>

</html>
)rawliteral";
