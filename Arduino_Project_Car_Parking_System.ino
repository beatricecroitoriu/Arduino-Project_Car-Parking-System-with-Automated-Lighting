#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); 
#include <Servo.h> 

Servo myservo1;

#define sensorPin A0 
#define light1 8
#define light2 9
#define buzzer 4
#define ledVerde 7
#define ledRosu 6

int IR1 = 2; 
int IR2 = 5;

int Slot = 4;           
int flag1 = 0;
int flag2 = 0;

int sensorValue;
int manualLight = 0;
int manualServo = 0;
int manualLCD = 0;

bool alarmON = false; // Stare alarmă
unsigned long lastBuzzTime = 0; // Timpul pentru ciclul buzzer-ului
bool buzzerState = false; // Starea actuală a buzzer-ului

bool isAuthenticated = false;
const String username = "admin"; // Numele de utilizator
const String password = "1234";  // Parola

String lcdLine1 = " ";
String lcdLine2 = " ";

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; 
byte ip[] = { 192, 168, 1, 50 }; // IP address in LAN – need to change according to your Network address

EthernetServer server(80); 

const char loginPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<title>Login</title>
<style>
body {
    font-family: Arial, sans-serif;
    display: flex;
    justify-content: center;
    align-items: center;
    height: 100vh; /* Înălțime completă a paginii */
    margin: 0;
    background-color: #f9f9f9; /* Fundal deschis */
}
.container {
    text-align: center;
    border: 1px solid #ccc;
    padding: 30px;
    border-radius: 10px;
    background-color: white;
    box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
}
h1 {
    color: #28a745;
}
input[type="text"], input[type="password"] {
    padding: 10px;
    width: 80%;
    margin: 10px 0;
    font-size: 18px;
    border: 1px solid #ccc;
    border-radius: 5px;
}
button {
    padding: 10px 20px;
    font-size: 18px;
    margin: 10px;
    border: none;
    border-radius: 5px;
    cursor: pointer;
    color: white;
    background-color: #007bff;
}
button:hover {
    background-color: #0056b3;
}
</style>
</head>
<body>
<div class="container">
    <h1>Login</h1>
    <form action="/" method="GET">
        <input type="text" name="username" placeholder="Username"><br>
        <input type="password" name="password" placeholder="Password"><br>
        <button type="submit">Login</button>
    </form>
</div>
</body>
</html>
)rawliteral";

const char loginFailedPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<title>Login Failed</title>
<style>
body {
    font-family: Arial, sans-serif;
    display: flex;
    justify-content: center;
    align-items: center;
    height: 100vh;
    margin: 0;
    background-color: #f9f9f9;
}
.container {
    text-align: center;
    border: 1px solid #ccc;
    padding: 30px;
    border-radius: 10px;
    background-color: white;
    box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
}
h1 {
    color: #dc3545; /* Roșu pentru eroare */
}
p {
    font-size: 18px;
    margin-bottom: 20px;
}
button {
    padding: 10px 20px;
    font-size: 18px;
    margin: 10px;
    border: none;
    border-radius: 5px;
    cursor: pointer;
    color: white;
    background-color: #007bff;
}
button:hover {
    background-color: #0056b3;
}
</style>
</head>
<body>
<div class="container">
    <h1>Login Failed</h1>
    <p>Invalid credentials. Please try again.</p>
    <button onclick="window.location.href='/'">Try Again</button>
</div>
</body>
</html>
)rawliteral";


const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<title>Smart Parking System</title>
<style>
body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; }
h1 { color: #28a745; }
button, input[type="text"] {
  padding: 10px 20px;
  font-size: 18px;
  margin: 10px;
  border: none;
  border-radius: 5px;
  cursor: pointer;
  color: black; /* Textul va fi negru */
  background-color: white; /* Fundal alb pentru câmp */
  border: 1px solid #ccc; /* Adaugă o margine subțire pentru vizibilitate */
}
.on { background-color: #28a745; }
.off { background-color: #dc3545; }
.auto { background-color: #007bff; }
.servo-up { background-color: #17a2b8; }
.servo-down { background-color: #ffc107; }
.servo-auto { background-color: #6f42c1; }
.alarm-on { background-color: #ff5733; }
.alarm-off { background-color: #5a5a5a; }
.submit { background-color: #17a2b8; }
.reset { background-color: #dc3545; }
.on:hover { background-color: #218838; }
.off:hover { background-color: #c82333; }
.auto:hover { background-color: #0056b3; }
.submit:hover { background-color: #138496; }
.reset:hover { background-color: #b02a37; }
</style>
<script>
function toggleLight(mode) {
    fetch('/?MODE=' + mode);
}
function controlServo(action) {
    fetch('/?SERVO=' + action);
}
function controlAlarm(state) {
    fetch('/?ALARM=' + state);
}
function updateLCDMessage() {
    fetch('/lcd')
      .then(response => response.text())
      .then(data => {
        document.getElementById('lcd-message').innerText = data;
      });
}
function setLCDMessage() {
    const line1 = document.getElementById('line1').value;
    const line2 = document.getElementById('line2').value;
    fetch('/setlcd?line1=' + encodeURIComponent(line1) + '&line2=' + encodeURIComponent(line2));
}
function resetLCDMessage() {
    fetch('/resetlcd');
}
setInterval(updateLCDMessage, 2000); // Actualizare la fiecare 2 secunde
</script>
</head>
<body>
<h1>Smart Parking System</h1>
<button class='on' onclick="toggleLight('ON')">Street Lights ON</button>
<button class='off' onclick="toggleLight('OFF')">Street Lights OFF</button>
<button class='auto' onclick="toggleLight('AUTO')">Automated Mode</button>
<br>
<button class='servo-up' onclick="controlServo('UP')">Barrier UP</button>
<button class='servo-down' onclick="controlServo('DOWN')">Barrier DOWN</button>
<button class='servo-auto' onclick="controlServo('AUTO')">Automated Mode</button>
<br>
<button class='alarm-on' onclick="controlAlarm('ON')">Start Alarm</button>
<button class='alarm-off' onclick="controlAlarm('OFF')">Stop Alarm</button>
<br>
<h2>LCD Message:</h2>
<div id="lcd-message" style="
    font-family: monospace; 
    border: 1px solid #ccc; 
    padding: 10px; 
    width: 112px; 
    height: 35px; 
    white-space: pre; 
    background-color: #f9f9f9; 
    text-align: left; 
    margin: 0 auto;"> <!-- Centrează secțiunea LCD Message -->
    Loading...
</div>
<br>
<div style="text-align: center;"> <!-- Container pentru câmpuri și butoane, centrat -->
    <input type="text" id="line1" placeholder="Line 1 (max 16 chars)" maxlength="16" style="display: block; margin: 10px auto;">
    <input type="text" id="line2" placeholder="Line 2 (max 16 chars)" maxlength="16" style="display: block; margin: 10px auto;">
    <button class="submit" onclick="setLCDMessage()" style="margin: 10px;">Set Message</button>
    <button class="reset" onclick="resetLCDMessage()" style="margin: 10px;">Reset Message</button>
</body>
</html>
)rawliteral";

void setup() {
  pinMode(light1, OUTPUT);
  pinMode(light2, OUTPUT);
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, LOW); 
  pinMode(ledVerde, OUTPUT);
  pinMode(ledRosu, OUTPUT);
  pinMode(sensorPin, INPUT);
  lcd.begin(16, 2);
  lcd.backlight();
  pinMode(IR1, INPUT);
  pinMode(IR2, INPUT);
  
  myservo1.attach(3);
  myservo1.write(100);

  digitalWrite(buzzer, LOW); 

  lcd.setCursor(0, 0);
  lcd.print("PROIECT PARCARE");
  lcdLine1 = "PROIECT PARCARE";
  lcd.setCursor(0, 1);
  lcd.print("  INTELIGENTA  ");
  lcdLine2 = "  INTELIGENTA  ";
  delay(3000);
  lcd.clear();  
  //start Ethernet
  Ethernet.begin(mac, ip);
  server.begin();
}

String URLDecode(String input) {
  String decoded = "";
  char temp[] = "00";
  unsigned int i, len = input.length();
  for (i = 0; i < len; i++) {
    if (input[i] == '+') {
      decoded += ' ';
    } else if (input[i] == '%') {
      if (i + 2 < len) {
        temp[0] = input[i + 1];
        temp[1] = input[i + 2];
        decoded += (char)strtol(temp, NULL, 16);
        i += 2;
      }
    } else {
      decoded += input[i];
    }
  }
  return decoded;
}

void loop() {

   
      lcd.setCursor(0, 0);
      lcd.print(lcdLine1);
      lcd.setCursor(0, 1);
      lcd.print(lcdLine2);
    

   if (alarmON) {
      unsigned long currentMillis = millis();
      if (currentMillis - lastBuzzTime >= (buzzerState ? 300 : 200)) { // 300ms ON, 200ms OFF
        buzzerState = !buzzerState; // Comută starea buzzer-ului
        digitalWrite(buzzer, buzzerState ? HIGH : LOW); // Pornește sau oprește buzzer-ul
        lastBuzzTime = currentMillis; // Actualizează timpul ultimului ciclu
      }
  } else {
    digitalWrite(buzzer, LOW); // Asigură că buzzer-ul este oprit
  }
  
  if(digitalRead (IR1) == LOW && flag1==0 && manualServo == 0)
  {
    if(Slot>0)
    {
      flag1=1;
      if(flag2==0)
      {
        myservo1.write(0); 
        digitalWrite(buzzer, HIGH); 
        delay(300); 
        digitalWrite(buzzer, LOW); 
        delay(100);
        Slot = Slot-1;
      }
     
    }
    else if (manualLCD == 0)
    {
      lcd.setCursor (0,0);
      lcd.print("  NE PARE RAU!  "); 
      lcdLine1 = "  NE PARE RAU!  "; 
      lcd.setCursor (0,1);
      lcd.print("PARCAREA OCUPATA"); 
      lcdLine2 = "PARCAREA OCUPATA"; 
      delay (3000);
      lcd.clear();     
    }
  }

  if(digitalRead (IR2) == LOW && flag2==0 && manualServo == 0)
  {
    flag2=1;
    if(flag1==0)
      {
        myservo1.write(0); 
        digitalWrite(buzzer, HIGH); 
        delay(300); 
        digitalWrite(buzzer, LOW); 
        delay(100);
        Slot = Slot+1;
        if(Slot==1)
          digitalWrite(ledVerde,LOW);
      }
  }

  if(flag1==1 && flag2==1 && manualServo == 0)
  {
    delay (1000);
    myservo1.write(100);
    flag1=0;
    flag2=0;
    if(Slot==0)
      digitalWrite(ledRosu,HIGH);
    else
      digitalWrite(ledVerde,HIGH);
  }

  if(flag1==0 && flag2==0)
    if(Slot!=0)
      digitalWrite(ledVerde,HIGH);
      
  if (manualLCD == 0){    
    lcd.setCursor (0,0);
    lcd.print("   BUN VENIT!   ");
    lcdLine1 = "   BUN VENIT!   ";
    lcd.setCursor (0,1);
    lcd.print("LOCURI LIBERE: ");
    lcdLine2 = "LOCURI LIBERE: " + String(Slot);
    lcd.print(Slot);
  }

  if(Slot==0)
    digitalWrite(ledVerde,LOW);
  else
    digitalWrite(ledRosu,LOW);

  sensorValue = analogRead(sensorPin);

  if (manualLight == 0) {
    if (sensorValue <= 100) {
      digitalWrite(light1, HIGH);
      digitalWrite(light2, HIGH);
    } else {
      digitalWrite(light1, LOW);
      digitalWrite(light2, LOW);
    }
  }

  EthernetClient client = server.available();
  if (client) {
    boolean currentLineIsBlank = true;
    String request = "";

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        request += c;

        if (c == '\n' && currentLineIsBlank) {
          if (request.indexOf("GET /lcd") >= 0) {
                        // Trimite mesajul curent de pe LCD
                        client.println(F("HTTP/1.1 200 OK"));
                        client.println(F("Content-Type: text/plain"));
                        client.println();
                        client.print(lcdLine1);
                        client.print("\n");
                        client.println(lcdLine2);
                        client.stop();
                        return;
          } else if (request.indexOf("GET /setlcd") >= 0) {
                          manualLCD = 1;
                          int line1Index = request.indexOf("line1=") + 6;
                          int line2Index = request.indexOf("line2=") + 6;
                          String encodedLine1 = request.substring(line1Index, request.indexOf("&line2="));
                          String encodedLine2 = request.substring(line2Index, request.indexOf(" HTTP/"));
                          lcdLine1 = URLDecode(encodedLine1);
                          lcdLine2 = URLDecode(encodedLine2);
                          lcd.clear();
                          lcd.setCursor(0, 0);
                          lcd.print(lcdLine1);
                          lcd.setCursor(0, 1);
                          lcd.print(lcdLine2);
                          client.println(F("HTTP/1.1 200 OK"));
                          client.println(F("Content-Type: text/plain"));
                          client.println();
                          client.println("LCD updated");
                          client.stop();
                          return;
          } else if (request.indexOf("GET /resetlcd") >= 0) {
                        // Resetează mesajul la cel automat
                        manualLCD = 0;
                        lcd.clear();
                        client.println(F("HTTP/1.1 200 OK"));
                        client.println(F("Content-Type: text/plain"));
                        client.println();
                        client.println("LCD reset to default");
                        client.stop();
                        return;
                      
          }else if (request.indexOf("GET /?username=") >= 0 && request.indexOf("&password=") > 0) {
            String receivedUsername = request.substring(request.indexOf("username=") + 9, request.indexOf("&password="));
            String receivedPassword = request.substring(request.indexOf("password=") + 9, request.indexOf(" HTTP/"));

            if (receivedUsername == username && receivedPassword == password) {
              // Login reușit - redirect către pagina principală
              client.println(F("HTTP/1.1 302 Found"));
              client.println(F("Location: /smart-parking-system"));
              client.println(F("Content-Type: text/html"));
              client.println();
            } else {
              // Login eșuat
              client.println(F("HTTP/1.1 200 OK"));
              client.println(F("Content-Type: text/html"));
              client.println();
              for (size_t i = 0; i < strlen_P(loginFailedPage); i++) {
                client.print((char)pgm_read_byte(&loginFailedPage[i]));
              }
            }
          } else if (request.indexOf("GET /smart-parking-system") >= 0) {
            // Pagina principală
            client.println(F("HTTP/1.1 200 OK"));
            client.println(F("Content-Type: text/html"));
            client.println();
            for (size_t i = 0; i < strlen_P(webpage); i++) {
              client.print((char)pgm_read_byte(&webpage[i]));
            }
          } else {
            // Procesarea comenzilor
            if (request.indexOf("ALARM=ON") > -1) {
              alarmON = true;
            } else if (request.indexOf("ALARM=OFF") > -1) {
              alarmON = false;
            } else if (request.indexOf("MODE=ON") > -1) {
              manualLight = 1;
              digitalWrite(light1, HIGH);
              digitalWrite(light2, HIGH);
            } else if (request.indexOf("MODE=OFF") > -1) {
              manualLight = 1;
              digitalWrite(light1, LOW);
              digitalWrite(light2, LOW);
            } else if (request.indexOf("MODE=AUTO") > -1) {
              manualLight = 0;
            } else if (request.indexOf("SERVO=UP") > -1) {
              manualServo = 1;
              myservo1.write(0);
            } else if (request.indexOf("SERVO=DOWN") > -1) {
              manualServo = 1;
              myservo1.write(100);
            } else if (request.indexOf("SERVO=AUTO") > -1) {
              manualServo = 0;
            }

            // Default: pagina de login implicită
            client.println(F("HTTP/1.1 200 OK"));
            client.println(F("Content-Type: text/html"));
            client.println();
            for (size_t i = 0; i < strlen_P(loginPage); i++) {
              client.print((char)pgm_read_byte(&loginPage[i]));
            }
          }
          break;
        }
        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    delay(1);
    client.stop();
  }
}
