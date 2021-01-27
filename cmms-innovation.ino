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
const char* ssid = "CANALBOX-B53A";
const char* password = "6383905460";

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

float tempThreshold = 32.00;
float humiThreshold = 100.00;
float lpgThreshold = 1000.00;
float coThreshold = 70000.00;
float smokeThreshold = 5200.00;

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
    Serial.print("Temperature: ");
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
    Serial.print("Humidity: ");
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
    Serial.print("Fahrenheit: ");
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
    Serial.print("LPG: ");
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
    Serial.print("CO: ");
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
    Serial.print("Smoke: ");
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
  Serial.println(" ");

  // give access to the client to Async webserver.
  WiFiClient client = server.available();

  if (client) {
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");
    String currentLine = "";
    while (client.connected() && currentTime - previousTime <= timeoutTime) {
      currentTime = millis();
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        header += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

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
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");

            // Web Page Heading
            client.println("<body><h1>CMMS Web Server</h1>");
            client.println("<body><h1>CMMS Data Collected</h1>");
            client.println("<p>Temperature Value: " + tempValue + "</p>");
            client.println("<p>Humidity Value: " + humiValue + "</p>");
            client.println("<p>LPG Value: " + lpgValue + "</p>");
            client.println("<p>CO2 Value: " + coValue + "</p>");
            client.println("<p>Smoke Value: " + smokeValue + "</p>");
            client.println("<p>Fahrenheit Value: " + tempFValue + "</p>");

            // Give Access to the buzzer
            client.println("<p>GPIO 26 - State " + output26State + "</p>");
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
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
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

  if ((temperature.toFloat() > tempThreshold) || (humidity.toFloat() > humiThreshold) || (lpg.toFloat() > lpgThreshold) || (co.toFloat() > coThreshold) || (smoke.toFloat() > smokeThreshold)) {
    output26State = "on";
    digitalWrite(output26, HIGH);
  } else {
    output26State = "off";
    digitalWrite(output26, LOW);
  }

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
