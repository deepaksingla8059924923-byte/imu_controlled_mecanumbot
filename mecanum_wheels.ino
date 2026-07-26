#include <WiFi.h>
#include <WebServer.h>

// ==========================================
// 1. CONFIGURATION
// ==========================================
const char* ssid = "ESP32_Mecanum_Rover"; 
const char* password = "password123";     

WebServer server(80);

// Front Motors: Cytron MDD3A (PWM on both A and B)
#define M1A 26  // Front Right A
#define M1B 27  // Front Right B
#define M2A 12  // Front Left A
#define M2B 13  // Front Left B

// Back Motors: Cytron SmartDriveDuo-10 (PWM + DIR mode)
#define PWM_BL 32  // Back Left PWM 
#define DIR_BL 33  // Back Left Direction 
#define PWM_BR 17  // Back Right PWM 
#define DIR_BR 18  // Back Right Direction 


// ==========================================
// 2. WEB PAGE INTERFACE (MATLAB UI STYLE)
// ==========================================
const char* webpageCode = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
  <title>Mecanum Control Dashboard</title>
  <style>
    /* MATLAB Style CSS */
    body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background-color: #F0F0F0; color: #000; margin: 15px; user-select: none; }
    h2 { font-size: 18px; color: #333; margin-top: 0; }
    
    .panel { 
      border: 1px solid #999; background: #F4F4F4; margin-bottom: 25px; 
      position: relative; padding: 20px 10px 10px 10px; box-shadow: 1px 1px 3px rgba(0,0,0,0.1);
    }
    .panel-title { 
      position: absolute; top: -10px; left: 10px; background: #F0F0F0; 
      padding: 0 5px; font-size: 12px; color: #000; border: 1px solid #F0F0F0; 
    }
    
    .row { display: flex; justify-content: space-between; align-items: center; margin-bottom: 8px; }
    .label { font-size: 13px; width: 80px; text-align: left; }
    
    input[type=number] { 
      width: 60px; height: 24px; font-family: 'Consolas', monospace; 
      text-align: right; border: 1px solid #999; 
    }
    
    button { 
      background: #E1E1E1; border: 1px solid #ADADAD; border-radius: 2px;
      cursor: pointer; color: #000; font-size: 13px; padding: 5px 10px; transition: 0.1s;
    }
    button:hover { background: #E5F1FB; border-color: #0078D7; }
    button:active { background: #CCE4F7; border-color: #005499; }
    
    .btn-red { background: #FFC0C0; border-color: #FF0000; }
    .btn-red:hover { background: #FF9999; }
    
    /* D-Pad Grid */
    .d-pad-container { display: flex; flex-direction: column; align-items: center; }
    .master-speed { margin-bottom: 15px; font-size: 13px; }
    .d-pad { display: grid; grid-template-columns: repeat(3, 60px); gap: 5px; }
    .d-pad button { height: 60px; font-size: 18px; font-weight: bold; }
    .empty { background: transparent; border: none; }
    
    /* Command Window */
    textarea { 
      width: 100%; height: 80px; font-family: 'Consolas', monospace; 
      font-size: 12px; background: #FFF; border: 1px solid #999; resize: none; box-sizing: border-box;
    }
  </style>
</head>
<body>
  <h2>Mecanum Controller</h2>

  <!-- Panel 1: Directional Control -->
  <div class="panel">
    <div class="panel-title">Kinematic Controller</div>
    <div class="d-pad-container">
      <div class="master-speed">
        <label>Master PWM Speed: </label>
        <input type="number" id="masterSpeed" value="150" min="0" max="255">
      </div>
      
      <div class="d-pad">
        <button onmousedown="sendKinematic('rotateLeft')" onmouseup="sendKinematic('stop')" ontouchstart="sendKinematic('rotateLeft')" ontouchend="sendKinematic('stop')">&#8634;</button>
        <button onmousedown="sendKinematic('front')" onmouseup="sendKinematic('stop')" ontouchstart="sendKinematic('front')" ontouchend="sendKinematic('stop')">&#8593;</button>
        <button onmousedown="sendKinematic('rotateRight')" onmouseup="sendKinematic('stop')" ontouchstart="sendKinematic('rotateRight')" ontouchend="sendKinematic('stop')">&#8635;</button>
        
        <button onmousedown="sendKinematic('left')" onmouseup="sendKinematic('stop')" ontouchstart="sendKinematic('left')" ontouchend="sendKinematic('stop')">&#8592;</button>
        <button class="btn-red" onmousedown="sendKinematic('stop')" ontouchstart="sendKinematic('stop')">STOP</button>
        <button onmousedown="sendKinematic('right')" onmouseup="sendKinematic('stop')" ontouchstart="sendKinematic('right')" ontouchend="sendKinematic('stop')">&#8594;</button>
        
        <div class="empty"></div>
        <button onmousedown="sendKinematic('back')" onmouseup="sendKinematic('stop')" ontouchstart="sendKinematic('back')" ontouchend="sendKinematic('stop')">&#8595;</button>
        <div class="empty"></div>
      </div>
    </div>
  </div>

  <!-- Panel 2: Workspace / Individual Control -->
  <div class="panel">
    <div class="panel-title">Workspace (Individual RPM)</div>
    
    <div class="row">
      <div class="label">Front Left</div>
      <button onclick="setSign('fl', -1)">- (REV)</button>
      <input type="number" id="fl" value="0" min="-255" max="255">
      <button onclick="setSign('fl', 1)">+ (FWD)</button>
    </div>
    <div class="row">
      <div class="label">Front Right</div>
      <button onclick="setSign('fr', -1)">- (REV)</button>
      <input type="number" id="fr" value="0" min="-255" max="255">
      <button onclick="setSign('fr', 1)">+ (FWD)</button>
    </div>
    <div class="row">
      <div class="label">Back Left</div>
      <button onclick="setSign('bl', -1)">- (REV)</button>
      <input type="number" id="bl" value="0" min="-255" max="255">
      <button onclick="setSign('bl', 1)">+ (FWD)</button>
    </div>
    <div class="row">
      <div class="label">Back Right</div>
      <button onclick="setSign('br', -1)">- (REV)</button>
      <input type="number" id="br" value="0" min="-255" max="255">
      <button onclick="setSign('br', 1)">+ (FWD)</button>
    </div>

    <div style="display: flex; gap: 10px; margin-top: 15px;">
      <button style="flex: 2; height: 35px; font-weight: bold;" onclick="sendWorkspace()">Send to Workspace</button>
      <button class="btn-red" style="flex: 1;" onclick="stopAll()">Clear/Stop</button>
    </div>
  </div>

  <!-- Panel 3: Command Window -->
  <div class="panel" style="background: #FFF;">
    <div class="panel-title" style="background: #FFF;">Command Window</div>
    <textarea id="console" readonly>>> System ready...</textarea>
  </div>

  <script>
    // MATLAB-style console logger
    function logCmd(msg) {
      let consoleEl = document.getElementById('console');
      consoleEl.value += '\n>> ' + msg;
      consoleEl.scrollTop = consoleEl.scrollHeight;
    }

    // Prevents mobile scrolling while using D-Pad
    document.body.addEventListener('touchmove', function(e) { if(e.target.tagName == 'BUTTON') e.preventDefault(); }, { passive: false });

    // D-Pad / Kinematic Control Sender
    function sendKinematic(action) {
      let speed = document.getElementById('masterSpeed').value || 150;
      fetch(`/action?cmd=${action}&speed=${speed}`);
      logCmd(`Kinematics: ${action} @ PWM ${speed}`);
    }

    // Workspace: Flips number to positive/negative
    function setSign(id, sign) {
      let el = document.getElementById(id);
      let val = Math.abs(parseInt(el.value) || 0);
      el.value = val * sign;
    }

    // Workspace: Send individual RPMs
    function sendWorkspace() {
      let fl = document.getElementById('fl').value;
      let fr = document.getElementById('fr').value;
      let bl = document.getElementById('bl').value;
      let br = document.getElementById('br').value;
      fetch(`/setMotors?fl=${fl}&fr=${fr}&bl=${bl}&br=${br}`);
      logCmd(`Set RPM -> FL:${fl} FR:${fr} BL:${bl} BR:${br}`);
    }

    // Stop and Zero out
    function stopAll() {
      document.getElementById('fl').value = 0;
      document.getElementById('fr').value = 0;
      document.getElementById('bl').value = 0;
      document.getElementById('br').value = 0;
      fetch('/action?cmd=stop&speed=0');
      logCmd('System Stopped.');
    }
  </script>
</body>
</html>
)rawliteral";


// ==========================================
// 3. MOTOR CONTROL MODULE
// ==========================================
void initMotors() {
    ledcAttach(M1A, 1000, 8);
    ledcAttach(M1B, 1000, 8);
    ledcAttach(M2A, 1000, 8);
    ledcAttach(M2B, 1000, 8);

    ledcAttach(PWM_BL, 1000, 8);
    pinMode(DIR_BL, OUTPUT);
    ledcAttach(PWM_BR, 1000, 8);
    pinMode(DIR_BR, OUTPUT);
    
    setIndividualMotors(0, 0, 0, 0);
}

// ------------------------------------------
// DYNAMIC MOVEMENT LOGIC
// ------------------------------------------
void setIndividualMotors(int fl, int fr, int bl, int br) {
    fl = constrain(fl, -255, 255);
    fr = constrain(fr, -255, 255);
    bl = constrain(bl, -255, 255);
    br = constrain(br, -255, 255);

    // Front Left (M2)
    if (fl >= 0) { ledcWrite(M2A, fl); ledcWrite(M2B, 0); } 
    else         { ledcWrite(M2A, 0);  ledcWrite(M2B, -fl); }

    // Front Right (M1)
    if (fr >= 0) { ledcWrite(M1A, 0);   ledcWrite(M1B, fr); } 
    else         { ledcWrite(M1A, -fr); ledcWrite(M1B, 0); }

    // Back Left (MDDS10)
    if (bl >= 0) { digitalWrite(DIR_BL, LOW);  ledcWrite(PWM_BL, bl); } 
    else         { digitalWrite(DIR_BL, HIGH); ledcWrite(PWM_BL, -bl); }

    // Back Right (MDDS10)
    if (br >= 0) { digitalWrite(DIR_BR, LOW);  ledcWrite(PWM_BR, br); } 
    else         { digitalWrite(DIR_BR, HIGH); ledcWrite(PWM_BR, -br); }
}


// ==========================================
// 4. WEB SERVER MODULE
// ==========================================
void handleRoot() {
    server.send(200, "text/html", webpageCode);
}

// Endpoint 1: Individual Workspace Variables
void handleSetMotors() {
    if (server.hasArg("fl") && server.hasArg("fr") && server.hasArg("bl") && server.hasArg("br")) {
        int fl = server.arg("fl").toInt();
        int fr = server.arg("fr").toInt();
        int bl = server.arg("bl").toInt();
        int br = server.arg("br").toInt();
        
        Serial.printf("Workspace Update -> FL:%d FR:%d BL:%d BR:%d\n", fl, fr, bl, br);
        setIndividualMotors(fl, fr, bl, br);
    }
    server.send(200, "text/plain", "OK");
}

// Endpoint 2: Kinematic D-Pad Commands
void handleAction() {
    if (server.hasArg("cmd") && server.hasArg("speed")) {
        String cmd = server.arg("cmd");
        int s = server.arg("speed").toInt(); // Master speed variable
        
        Serial.println("Kinematics Update: " + cmd + " @ " + String(s));
        
        // Standard Mecanum Kinematics Matrix mapped directly to individual speeds
        if      (cmd == "front")       setIndividualMotors(s,  s,  s,  s);
        else if (cmd == "back")        setIndividualMotors(-s, -s, -s, -s);
        else if (cmd == "left")        setIndividualMotors(-s,  s,  s, -s);
        else if (cmd == "right")       setIndividualMotors(s, -s, -s,  s);
        else if (cmd == "rotateLeft")  setIndividualMotors(-s,  s, -s,  s);
        else if (cmd == "rotateRight") setIndividualMotors(s, -s,  s, -s);
        else if (cmd == "stop")        setIndividualMotors(0, 0, 0, 0);
    }
    server.send(200, "text/plain", "OK");
}

void initServer() {
    server.on("/", handleRoot);
    server.on("/setMotors", handleSetMotors); 
    server.on("/action", handleAction); 
    server.begin();
    Serial.println("HTTP server started");
}

void setupWiFiAP() {
    Serial.println("Setting up Access Point...");
    WiFi.softAP(ssid, password);
    
    IPAddress IP = WiFi.softAPIP();
    Serial.print("Access Point Ready. IP: ");
    Serial.println(IP);
}

// ==========================================
// 5. MAIN LOGIC (SETUP & LOOP)
// ==========================================
void setup() {
    Serial.begin(115200);
    initMotors();
    setupWiFiAP();
    initServer();
}

void loop() {
    server.handleClient();
}