//#include <LiquidCrystal_I2C.h>
#include <Tone32.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>
#include "config.h"
#include "WiFi.h"
#include <PubSubClient.h>
#include <Streaming.h>

#define R0 5            // Relay 0 D1
#define R1 4            // Relay 1 D2
#define R2 0            // Relay 2 D3
#define R3 2            // Relay 3 D4



#define BUZZER_PIN  23 // ESP32 pin GIOP18 connected to piezo buzzer
#define BUZZER_CHANNEL 0

// constants won't change :
const long interval = 5000;
const int HotAirGunAnalogPin = 35;
const int HotAirGunFan = 19;
const int HotAirGunPower = 5;
const int SolderIonHeater = 18;

int readingIn = 0;
unsigned long previousMillis = 0;      
unsigned long frqpreviousMillis = 0;

WiFiClient espClient;
PubSubClient client(espClient);


//LiquidCrystal_I2C lcd(0x27,16,2);
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0); 

int HotAirGunValue;
int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};


void setup() {
  // setup WiFi
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  u8g2.begin();
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode ( HotAirGunPower, OUTPUT );
  pinMode ( 18, OUTPUT );
  pinMode ( HotAirGunFan, OUTPUT );
  pinMode ( HotAirGunAnalogPin, INPUT );
/*
  tone(BUZZER_PIN, NOTE_C4, 500, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_D4, 500, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_E4, 500, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_F4, 500, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_G4, 500, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_A4, 500, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
*/
  tone(BUZZER_PIN, NOTE_B4, 500, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);

}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  char message_buff[100];
  int i = 0;
  int x = 0;
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    //Serial.print((char)payload[i]);
    message_buff[i] = payload[i];
    x++;
  }
  message_buff[x] = '\0';
  String msgString = String(message_buff);
  Serial.println("Payload: " + msgString);

   switch (msgString[0]){
    case '0':     //Det er et alle kald
    if(msgString[2]=='1'){
      digitalWrite(R0, LOW);
      digitalWrite(R1, LOW);
      digitalWrite(R2, LOW);
      digitalWrite(R3, LOW);
    }
    else{
      digitalWrite(R0, HIGH);
      digitalWrite(R1, HIGH);
      digitalWrite(R2, HIGH);
      digitalWrite(R3, HIGH);
    }
    case '1':
    if(msgString[2]=='1'){
      digitalWrite(R0, LOW);
    }
    else{
      digitalWrite(R0, HIGH);
    }
    break;
    case '2':
    if(msgString[2]=='1'){
      digitalWrite(R1, LOW);
    }
    else{
      digitalWrite(R1, HIGH);
    }
    break;
    case '3':
    if(msgString[2]=='1'){
      digitalWrite(R2, LOW);
    }
    else{
      digitalWrite(R2, HIGH);
    }
    break;
    case '4':
    if(msgString[2]=='1'){
      digitalWrite(R3, LOW);
    }
    else{
      digitalWrite(R3, HIGH);
    }
    break;
  }
  send_messure_data();
}

void send_messure_data(){
 readingIn = analogRead(HotAirGunAnalogPin);    // read the input pin

  //  Convert data to JSON string 
  String json =
  "{"
  "\"analog\": \"" + String(readingIn) + "\","
  "\"relay_1\": \"" + digitalRead(R0) + "\","
  "\"relay_2\": \"" + digitalRead(R1) + "\","
  "\"relay_3\": \"" + digitalRead(R2) + "\","
  "\"relay_4\": \"" + digitalRead(R3) + "\"}";
  // Convert JSON string to character array
  // Serial.print("json length: ");
  // Serial.println(json.length()+1);
  char jsonChar[200];
  json.toCharArray(jsonChar, json.length()+1);
  // Publish JSON character array to MQTT topic
  // mqtt_topic = "/Gasmads/Outdoor/07/json/";

  if( client.publish(mqtt_topic,jsonChar)){
    Serial.println(json); 
  }
  else{
    Serial.println("Ikke Sendt");      
  }

}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    Serial.println (mqtt_username);
    Serial.println (mqtt_password);
    if (client.connect("Gasmads53", mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe("/Gasmads/Indoor/53/input/#");
      Serial.println("subscribe");
      //client.publish(mqtt_topic, "Så er vi igang");
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void loop() {
  /*
  delay(1000);
  digitalWrite(HotAirGunPower, LOW );
  digitalWrite(18, HIGH );
  digitalWrite(HotAirGunFan, HIGH );
  delay(3000);
  digitalWrite(HotAirGunPower, HIGH );
  digitalWrite(18, LOW );
  digitalWrite(HotAirGunFan, LOW );
  HotAirGunValue = analogRead(HotAirGunAnalogPin);
  Serial.print("Value: ");
  Serial.println(HotAirGunValue);  
  
   u8g2.clearBuffer();          // clear the internal memory
   u8g2.setFont(u8g2_font_logisoso28_tr);  // choose a suitable font at https://github.com/olikraus/u8g2/wiki/fntlistall
   u8g2.drawStr(8,29,"Ln-Spc.");  // write something to the internal memory
   u8g2.sendBuffer();         // transfer internal memory to the display
  */
 
  unsigned long currentMillis = millis();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  // read the input pin

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    send_messure_data();
  }


/*
  lcd.init();
  lcd.clear();
  lcd.backlight();

  lcd.setCursor(2,0);
  lcd.print("Hello World");

  lcd.setCursor(2,1);
  lcd.print("Ln - Spc");
*/
  
}
