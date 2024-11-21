#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID      "TMPL3X_oMQPqZ"
#define BLYNK_TEMPLATE_NAME    "Hadipa"
#define BLYNK_AUTH_TOKEN       "-TiMDTpTiy8KoNg96q2HhxXWTiL_HR2E"
#define SS_PIN 21
#define RST_PIN 22
#define SIZE_BUFFER     18
#define MAX_SIZE_BLOCK  16
#define LED_PIN 2 // Change this to the appropriate pin for your LED
#define BUZZER_LED 27


#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
MFRC522::StatusCode status;
byte nuidPICC[4];

char ssid[] = "LC";
char pass[] = "lc@tiet1";

Servo myservo;
int pos = 0;
int servoPin = 33;
bool GateOpen = false;
void setup()
{
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // Make sure the LED is off initially
  pinMode(BUZZER_LED, OUTPUT);
  digitalWrite(BUZZER_LED, LOW);

  ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
	ESP32PWM::allocateTimer(3);
	myservo.setPeriodHertz(50);    // standard 50 hz servo
	myservo.attach(servoPin, 500, 2400);
  myservo.write(0);
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

void loop()
{
  
  Blynk.run();

  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  if ( ! rfid.PICC_ReadCardSerial())
    return;

  bool val = readingData();
  Serial.println(val);

  if(val == true){
    if(GateOpen == false){
      rotateServoOpen();
      GateOpen = true;
      blinkBuz();
    }else{
      rotateServoClose();
      GateOpen = false;
    }
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  return;

}

bool readingData()
{
  rfid.PICC_DumpDetailsToSerial(&(rfid.uid)); 
  
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
  
  byte buffer[SIZE_BUFFER] = {0};
 
  byte block = 1;
  byte size = SIZE_BUFFER;
  status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(rfid.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(rfid.GetStatusCodeName(status));
    return false;
  }

  status = rfid.MIFARE_Read(block, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(rfid.GetStatusCodeName(status));
    return false;
  }
  return true;
}

BLYNK_WRITE(V0) {
    int buttonState = param.asInt();
    Serial.println(buttonState);
    if (buttonState == 1) {
      if(GateOpen == false){
        rotateServoOpen();
        GateOpen = true;
        blinkBuz();
      }
    }else if(buttonState == 0){
      if(GateOpen == true){
        rotateServoClose();
        GateOpen = false;
      }
    }
  }

void rotateServoOpen() {
  for (pos = 0; pos <= 180; pos += 1) {
		myservo.write(pos);
		delay(15);
	}
}

void rotateServoClose() {
  for (pos = 180; pos >= 0; pos -= 1) {
		myservo.write(pos);
		delay(15);
	}
}

void blinkBuz() {
  digitalWrite(BUZZER_LED, HIGH);
  delay(1500);
  digitalWrite(BUZZER_LED, LOW);
}
