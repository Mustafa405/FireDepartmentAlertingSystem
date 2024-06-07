#include <ESP8266WiFi.h>
//#include <BlynkSimpleEsp8266.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include "DHT.h"

LiquidCrystal lcd (16, 5, 4, 0, 2, 14);

#define DHTPIN 15     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 22  (AM2302), AM2321

int fire = 12;
int buzzer = 13;

const char* ssid = "projectshub";
const char* password = "projectshub";
const char *host = "maker.ifttt.com";
const char *privateKey = "bcIobygfgfKB8xo1-ilmim0pbtRa2Rm9AJToohYUpOp";
ESP8266WebServer server(80); //Server on port 80
void send_event(const char *event);

DHT dht(DHTPIN, DHTTYPE);

const int sensor=A0; // Assigning analog pin A5 to variable 'sensor'
float tempc; //variable to store temperature in degree Celsius
float tempf; //variable to store temperature in Fahreinheit
float vout; //temporary variable to hold sensor reading

String Message;
const char MAIN_page[] PROGMEM = R"=====(
<!doctype html>
<html>
<head>
  <title>Data Logger</title>
  <h2 style="text-align:center;">FIRE DEPARTMENT ALERTING SYSTEM</h2>
  <style>
  canvas{
    -moz-user-select: none;
    -webkit-user-select: none;
    -ms-user-select: none;
  }
  /* Data Table Styling*/ 
  #dataTable {
    font-family: "Trebuchet MS", Arial, Helvetica, sans-serif;
    border-collapse: collapse;
    width: 100%;
    text-align: center;
  }
  #dataTable td, #dataTable th {
    border: 1px solid #ddd;
    padding: 8px;
  }
  #dataTable tr:nth-child(even){background-color: #f2f2f2;}
  #dataTable tr:hover {background-color: #ddd;}
  #dataTable th {
    padding-top: 12px;
    padding-bottom: 12px;
    text-align: center;
    background-color: #050505;
    color: white;
  }
  </style>
</head>
<body>   
<div>
  <table id="dataTable">
    <tr><th>Time</th><th>Activity</th></tr>
  </table>
</div>
<br>
<br>  
<script>
var Avalues = [];
//var timeStamp = [];
var dateStamp = [];
setInterval(function() {
  // Call a function repetatively with 5 Second interval
  getData();
}, 3000); //5000mSeconds update rate
function getData() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
     //Push the data in array
 // var time = new Date().toLocaleTimeString();
  var date = new Date();
  var txt = this.responseText;
  var obj = JSON.parse(txt); 
      Avalues.push(obj.Activity);
     // timeStamp.push(time);
      dateStamp.push(date);
  //Update Data Table
    var table = document.getElementById("dataTable");
    var row = table.insertRow(1); //Add after headings
    var cell1 = row.insertCell(0);
    var cell2 = row.insertCell(1);
    cell1.innerHTML = date;
    //cell2.innerHTML = time;
    cell2.innerHTML = obj.Activity;
    }
  };
  xhttp.open("GET", "readData", true); //Handle readData server on ESP8266
  xhttp.send();
}    
</script>
</body>
</html>

)====="; 
void handleRoot() {
 String s = MAIN_page; //Read HTML contents
 server.send(200, "text/html", s); //Send web page
}

void readData() {
  int fire_state = digitalRead(fire); //Continuously check the state of PIR sensor

  int t = dht.readTemperature();

  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.setCursor(2, 0);
  lcd.print(t);
  lcd.print("  ");
  delay(100);

    if(fire_state == LOW){ 
    lcd.setCursor(0, 1);
    lcd.print("fire:D    ");
    digitalWrite (buzzer, HIGH);    //If intrusion detected ring the buzzer
    delay(1000);
    digitalWrite (buzzer, LOW);
    Message = "fire Deteced";
    String data = "{\"Activity\":\""+ String(Message) +"\"}";
    server.send(200, "text/plane", data); //Send ADC value, temperature and humidity JSON to client ajax request
//    send_event("motion_event");               
//    Serial.println("Motion detected!");
    }

    if(fire_state == HIGH){ 
    lcd.setCursor(0, 1);
    lcd.print("fire:UD   ");
    }
//    Message = temperature;
//    String data = "{\"Activity\":\""+ String(Message) +"\"}";
//    server.send(200, "text/plane", data); //Send ADC value, temperature and humidity JSON to client ajax request

  delay(500);                         //Check state of PIR after every half second
}




void setup() {
  Serial.begin(115200);  /* Define baud rate for serial communication */
  pinMode(fire, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(sensor,INPUT); // Configuring sensor pin as input
  
  digitalWrite(fire, 1);

//  lcd.init();                      // initialize the lcd
//  lcd.init();

//  lcd.backlight();

  dht.begin();

  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print(" WELCOME TO THE");
  lcd.setCursor(0, 1);
  lcd.print("   PROJECT   ");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" FIRE ALERTING ");
  lcd.setCursor(0, 1);
  lcd.print("   SYSTEM      ");
  delay(2000);
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("SSID:");
  lcd.setCursor(0, 1);
  lcd.print("projectshub");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PSWD:");
  lcd.setCursor(0, 1);
  lcd.print("projectshub");
  delay(2000);
  lcd.clear();
  lcd.print("Wifi Connecting");
  
 WiFi.begin(ssid, password);
  int wifi_ctr = 0;
  while (WiFi.status() != WL_CONNECTED) {
    lcd.print(".");
  delay(500);
  }
  lcd.clear();
  lcd.print("Wifi Connected");
  delay(1000);
  lcd.clear();
 lcd.print(WiFi.localIP()); delay(2000);
 lcd.clear();
  
 Serial.println("");
 Serial.println("Successfully connected to WiFi.");
 Serial.println("IP address is : ");
 Serial.println(WiFi.localIP());
 server.on("/", handleRoot);      //Which routine to handle at root location. This is display page
 server.on("/readData", readData); //This page is called by java Script AJAX
 server.begin();                  //Start server
 Serial.println("HTTP server started");
}

void loop() 
{
  server.handleClient();          //Handle client requests 

/*
  float temperature = analogRead(A0);
  temperature = (temperature * resolution);
  temperature = temperature * 100;

  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.setCursor(2, 0);
  lcd.print(temperature);
  delay(100);

  if (digitalRead(fire) == 0)
  {
    lcd.setCursor(9, 1);
    lcd.print("fire:D  ");
  }
  if (digitalRead(fire) == 1)
  {
    lcd.setCursor(9, 1);
    lcd.print("fire:UD");
  }
  delay(200);
*/
}

void send_event(const char *event)
{
  Serial.print("Connecting to "); 
  Serial.println(host); 
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("Connection failed");
    return;
  } 
  // We now create a URI for the request
  String url = "/trigger/";
  url += event;
  url += "/with/key/";
  url += privateKey; 
  Serial.print("Requesting URL: ");
  Serial.println(url);  
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  while(client.connected())
  {
    if(client.available())
    {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    } else {
      // No data yet, wait a bit
      delay(50);
    };
  }  
  Serial.println();
  Serial.println("closing connection");
  client.stop();
}
