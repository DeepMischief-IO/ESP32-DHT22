///***This Program contains I2C display with pin 0x27 ,AM2302 Sensor and ESP 32
///***With MQTT and Local client
/// by DeepMisChief-IO

#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define DHTPIN 2
#define DHTTYPE DHT22

// Update these with values suitable for your network.
const char *ssid = "redmi";
const char *password = "12345678";
const char *mqtt_server = "192.168.222.98";
LiquidCrystal_I2C lcd(0x27, 16, 2);

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

/// WIFI
void setup_wifi()
{
  delay(10);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String message;
  for (unsigned int i = 0; i < length; i++)
  {
    message += (char)payload[i];
  }
  Serial.println(message);
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client"))
    {
      Serial.println("connected");
      // Subscribe
      if (client.subscribe("your/topic", 1))
      {
        Serial.println("Subscription at QoS 1 successful");
      }
      else
      {
        Serial.println("Subscription at QoS 1 failed");
      }
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  lcd.init();
  dht.begin(); // begin temp sensor
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
  delay(2000);
  
  //For Temperature Sensor data read
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);
  if (isnan(h) || isnan(t) || isnan(f))
  {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);
//for I2C display
  lcd.setCursor(0, 0);       // set the cursor to column 0, line 0
  lcd.print("Temperature:"); // Print a message to the LCD
  lcd.println(t);


  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));
  client.publish("your/temperature", String(t).c_str(), true);

  client.publish("your/humidity", String(h).c_str(), true);
  client.subscribe("button/state");
  delay(5000);//delay for temperature
}
