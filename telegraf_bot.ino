
#include <WiFiSSLClient.h>
#include <WiFi101.h>
#include <SPI.h>
#include <TelegramBot.h>
#include <DHT.h>


#include "pitches.h"
#include "configure.h"

//initialize Humidity/Temperature Sensor
#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE);

char ssid[] = WIFI_SSID;   //  your network SSID (name)
char pass[] = WIFI_PASS;   // your network password

// notes in the melody:
int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};

// Initialize Telegram BOT
const String BotToken = BOT_TOKEN;
const String BotName = BOT_NAME;
const String BotUsername = BOT_USERNAME;


WiFiSSLClient client;
TelegramBot bot (BotToken, BotName, BotUsername, client);

const int ledPin =  6;  // the number of the LED pin

// named constant for the pin the sensor is connected to
const int sensorPin = A0;
// room temperature in Celcius
const float baselineTemp = 20.0;


void setup() {

  //connext to serial console
  Serial.begin(115200);
  //while (!Serial) {}
  //delay(3000);


  // attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected");

  _playMusic();

  bot.begin();
  pinMode(ledPin, OUTPUT);

  dht.begin();

}

void loop() {

  message m = bot.getUpdates(); // Read new messages

  if (m.text.equals("")) {
    //nothing to do
    delay(10000);
    return;
  }

  Serial.print("Got Command:");
  Serial.println(m.text);


  if (m.text.equals("/on")) {
    digitalWrite(ledPin, HIGH);
    bot.sendMessage(m.chat_id, "The Led is now ON");

    _playMusic();

  } else if ( m.text.equals("/off")) {
    digitalWrite(ledPin, LOW);
    bot.sendMessage(m.chat_id, "The Led is now OFF");
    noTone(8);
  } else if ( m.text.equals("/music")) {

    _playMusic();
    bot.sendMessage(m.chat_id, "music played");

  } else if ( m.text.equals("/ping")) {

    bot.sendMessage(m.chat_id, "pong");

  } else if ( m.text.equals("/water")) {

    bot.sendMessage(m.chat_id, "water = ?? cm");

  } else if ( m.text.equals("/t")) {


    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float humidity = dht.readHumidity();

    // Read temperature as Celsius
    float temperature = dht.readTemperature();

    // Check if any reads failed and exit early (to try again).
    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Failed to read from DHT sensor!");
      bot.sendMessage(m.chat_id, "Failed to read from DHT sensor!");
      return;
    }

    char temperatureMsg[25];
    sprintf(temperatureMsg, "t = %f C°, h = %f", temperature, humidity);
    Serial.println(temperatureMsg);

    bot.sendMessage(m.chat_id, temperatureMsg);



  } else if ( m.text.equals("/t2")) {

    // read the value on AnalogIn pin 0
    // and store it in a variable
    int sensorVal = analogRead(sensorPin);

    // send the 10-bit sensor value out the serial port
    Serial.print("sensor Value: ");
    Serial.print(sensorVal);

    // convert the ADC reading to voltage
    float voltage = (sensorVal / 1024.0) * 5.0;

    // Send the voltage level out the Serial port
    Serial.print(", Volts: ");
    Serial.print(voltage);

    // convert the voltage to temperature in degrees C
    // the sensor changes 10 mV per degree
    // the datasheet says there's a 500 mV offset
    // ((volatge - 500mV) times 100)
    Serial.print(", degrees C: ");
    float temperature = (voltage - .5) * 100;
    Serial.println(temperature);

    char temperatureMsg[25];
    sprintf(temperatureMsg, "t = %f C°", temperature);
    //Serial.println(temperatureMsg);

    bot.sendMessage(m.chat_id, temperatureMsg);

  } else if ( m.text.equals("/help")) {

    bot.sendMessage(m.chat_id,
                    "/help - this help,\
    /ping - get \"pong\",\
    /on - LED on,\
    /off - LED off,\
    /t - get temperature,\
    /w - get water level,\
    /music - play music");

  } else {
    Serial.print("Unknown Command:");
  }

  //delay 10 seconds
  delay(10000);
}

void _playMusic() {
  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < 8; thisNote++) {

    // to calculate the note duration, take one second
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(8, melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(8);
  }
}


