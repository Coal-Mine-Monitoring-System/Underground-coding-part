#include <HTTPClient.h>
#include <Adafruit_Sensor.h>
#include <WiFi.h>
#include <DHT.h>
#include <MQ2.h>

// define DHTTYPE in our case is DHT11
#define DHTTYPE DHT11
// define required pins
#define DHTPIN 4 // Digital pin connected to the DHT sensor
#define MQGASPIN 35
//#define BUZZERPIN 26

// initialize MQ2 and DHT
DHT dht(DHTPIN, DHTTYPE);
MQ2 mq2(MQGASPIN);

// Replace with WIFI network credentials
const char* ssid = "junior";
const char* password = "junior121212";

// HTTP status
HTTPClient http;
// creation of webser
// check Web server port for work
WiFiServer server(80);
// we have to set variable to store the HTTP request
String header;
// declaration of output state(Auxiliar variables to store the current output state
String output26State = "off";
// Assign output variables to GPIO poins
const int output26 = 26;

// we need to keep track with time.
// current time
unsigned long currentTime = millis();
// previous time
unsigned long previousTime = 0;
// define timeout time in milliseconds
const long timeoutTime = 2000;

// end of Emmanuel try

// declaration of variables required.
String temperature;
String humidity;
String lpg;
String co;
String smoke;
String buzzerState;
// url to make a request
String url = "https://cmms-app.herokuapp.com/monitoring/update-data?data=";

//required function to collecting data
// temperature sensing function that return value of temperature
String readDHTTemperature() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  // Read temperature as Celsius
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  //float t = dht.readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else {
    Serial.println(t);
    return String(t);
  }
}

// Humidity sensing function that return value of humidity
String readDHTHumidity() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else {
    Serial.println(h);
    return String(h);
  }
}

String readDHTTemperatureF() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float f = dht.readTemperature(true);
  if (isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else {
    Serial.println(f);
    return String(f);
  }
}

// LPG sensing function with help of MQ2 gas sensor that return value of lpg value
String readMQLPG() {
  // Sensor reading require few seconds to read.
  float lpg = mq2.readLPG();
  if (isnan(lpg)) {
    Serial.println("Failed to read the value of lpg from MQ2 sensor!");
    return "--";
  }
  else {
    Serial.println(lpg);
    return String(lpg);
  }
}

// CO2 sensing funcdtion with help of MQ2 gas sensor that return value of co value
String readMQCo() {
  // Sensor reading with few seconds
  float co = mq2.readCO();
  if (isnan(co)) {
    Serial.println("Failed to read the value of co2 from MQ2 sensor!");
    return "--";
  }
  else {
    Serial.println(co);
    return String(co);
  }
}

String readMQSmoke() {
  // Sensor reading with few seconds
  float smoke = mq2.readSmoke();
  if (isnan(smoke)) {
    Serial.println("Failed to read the value of smoke from MQ2 sensor!");
    return "--";
  }
  else {
    Serial.println(smoke);
    return String(smoke);
  }
}
void setup()
{
  // buzzer pin setting
  pinMode(output26, OUTPUT);
  digitalWrite(output26, LOW);
  // setup serial to print required strings
  Serial.begin(115200);

  dht.begin();
  mq2.begin();
  delay(500);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }

  // start Async webserver
  server.begin();
}

void loop()
{
  // read all required data from functions repetitively to display on Async webserver.
  String tempValue = readDHTTemperature();
  String humiValue = readDHTHumidity();
  String lpgValue = readMQLPG();
  String coValue = readMQCo();
  String smokeValue = readMQSmoke();
  String tempFValue = readDHTTemperatureF();
  // give access to the client to Async webserver.
  WiFiClient client = server.available();

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // turns the GPIOs on and off
            if (header.indexOf("GET /26/on") >= 0) {
              Serial.println("GPIO 26 on");
              output26State = "on";
              digitalWrite(output26, HIGH);
            } else if (header.indexOf("GET /26/off") >= 0) {
              Serial.println("GPIO 26 off");
              output26State = "off";
              digitalWrite(output26, LOW);
            }

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");

            // Web Page Heading
            client.println("<body><h1>CMMS Web Server</h1>");

            // Emmanuel try
            client.println("<p>Temperature Value: " + tempValue + "</p>");
            client.println("<p>Humidity Value: " + humiValue + "</p>");
            client.println("<p>LPG Value: " + lpgValue + "</p>");
            client.println("<p>CO2 Value: " + coValue + "</p>");
            client.println("<p>Smoke Value: " + smokeValue + "</p>");
            client.println("<p>Fahrenheit Value: " + tempFValue + "</p>");

            // Display current state, and ON/OFF buttons for GPIO 26
            client.println("<p>GPIO 26 - State " + output26State + "</p>");
            // If the output26State is off, it displays the ON button
            if (output26State == "off") {
              client.println("<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>");
            }

            client.println("</body></html>");

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }

  // read value generated by functions
  temperature = readDHTTemperature();
  humidity = readDHTHumidity();
  lpg = readMQLPG();
  co = readMQCo();
  smoke = readMQSmoke();
  delay(1000);

  // reconnect just in case
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  String path = url + "11111111" + "*" + temperature + "*" + humidity + "*" + lpg + "*" + co + "*" + smoke;

  http.begin(path.c_str());

  //send get request to the web

  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println(payload);
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
  delay(15000);
}
