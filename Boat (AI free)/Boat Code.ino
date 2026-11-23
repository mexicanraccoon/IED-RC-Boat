#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>

const int CE_PIN = 9;
const int CSN_PIN = 10;

const int LESC_PIN = 5;
const int RESC_PIN = 6;

const int LED_PIN = 3;

const int HORN_PIN = A0;
int HORN_FREQ = 500;


RF24 radio(CE_PIN, CSN_PIN);
const byte address[6] = "80085";


struct PacketData {
  int motorLeftSpeed;
  int motorRightSpeed;
  bool leftButton;
  bool rightButton;
};

PacketData rxData = {0, 0, false, false};

Servo LESC;
Servo RESC;

bool lightOn = false;

void setup() {
  Serial.begin(9600);

  pinMode(LED_PIN, OUTPUT);
  pinMode(HORN_PIN, OUTPUT);

  radio.begin();
  
  if (!radio.begin()) {
    Serial.println("Radio hardware error!");
    while (1);
  }

  radio.setPALevel(RF24_PA_LOW);
  radio.setPayloadSize(sizeof(PacketData));
  radio.openReadingPipe(0, address);
  radio.startListening();

  LESC.attach(LESC_PIN, 1000, 2000);
  RESC.attach(RESC_PIN, 1000, 2000);

  digitalWrite(LED_PIN, LOW);


}

void loop() {

  if (radio.available()) {
    radio.read(&rxData, sizeof(PacketData));
    int leftPower = rxData.motorLeftSpeed;
    int rightPower = rxData.motorRightSpeed;
    bool lightButton = rxData.leftButton;
    bool hornStatus = rxData.rightButton;

    LESC.write(leftPower);
    RESC.write(rightPower);

    if (lightButton == true) {

      if (lightOn == true) {
        lightOn = false;
      }
      else lightOn = true;
    }
    if (lightOn == true) digitalWrite(LED_PIN, HIGH);
    if (hornStatus == true) tone(HORN_PIN, HORN_FREQ);
    delay(10);
  }
  else {
    RESC.write(0);
    LESC.write(0);
    if (lightOn == false) {
      digitalWrite(LED_PIN, HIGH);
      tone(HORN_PIN, HORN_FREQ);
      lightOn = true;
    }
    delay(500);
    if (lightOn == true) {
      lightOn = false;
      digitalWrite(LED_PIN, LOW);
      noTone(HORN_PIN);
    }
    delay(500);
  }
}
