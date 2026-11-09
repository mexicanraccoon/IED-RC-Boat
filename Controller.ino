
#include <SPI.h> //built in library enabling communication with Serial Peripheral Interface (SPI) devices. Used to connect microcontrollers to peripherals like SD cards, sensors, LCD screens, and flash memory.
#include <nRF24L01.h> //low level hardware registry mapping and constant defining for Nordic semiconductor's 2.4 Ghz trancievers.
#include <RF24.h> //library for handling SPI communication, packet instruction, and radio settings between a microcontroller and the nRF24L01 chip.

//CE and CSN pins for nRF24l01
RF24 radio(9, 10); //CE stands for chip enable. controls whether the rf24 is in transmit or receive mode., CSN stands for chip select not. When pin is pulled LOW, rf24 starts listening for SPI command signals on MOSI/MISO/SCK bus 
const uint64_t pipeAddress = 0xE8E8F0F0E1LL; //sets the pipe address for the outgoing tranceiver to emit on. The receiving tranceiver will also need to be set to this pipe address in order to receive the signal


struct PacketData {  //creates a structure for the packet data that will be sent at the pipe address. struct is similar in functionality to a class, where the structure is given a name and has keys that expect values to be initialized and potentially changed in it.
  int motorLeftSpeed; //Range of -255 (full reverse) to 255 (full forward)
  int motorRightSpeed; //Range of -255 (full reverse) to 255 (full forward)
  bool leftButton; //left joysitck button state (true = pressed)
  bool rightButton; //right joystick button state (true = pressed)
};

PacketData txData; //Creates a PacketData structure named "txData". txData will eventually need it's motorLeftSpeed, motorRightSpeed, leftButton, and rightButton initialized.

const int leftSWPin = 2; //designates the pin that reads the button input for left stick as digital 2
const int rightSWPin = 3; //designates the pin that reads the button input for left stick as digital 3

const int statusLED = 6; //designates the pin that the status LED will be connected to.


int statusLEDcount = 0;

void setup() { //defines function that is run exactly once upon arduino power-up or reset
  // put your setup code here, to run once:
  pinMode(statusLED, OUTPUT); //
  
  Serial.begin(9600); //initializes the serial communication (sets the transmission speed rate in bits per second for the serial port (USB))

  if (!radio.begin()) { //sets the behavior of the controller if the radio fails to initialize. 
    Serial.println("Radio hardware error!"); //print to the serial terminal that there is 
    digitalWrite(statusLED, HIGH); //If radio is not working, LED will power solid
    while (1); //continuous loop until arduino is reset so transmitter can be repaired
    }
  radio.openWritingPipe(pipeAddress); //sets the address to which the radio is attuned to
  radio.setPALevel(RF24_PA_LOW); //configures the power output level of the radio
  
  /*a note on setPAlevels:
   * RF24_PA_MIN = -18dBm which is the lowest power and the shortest range)
   * RF24_PA_LOW = -12 dBm
   * RF24_PA_HIGH = -6 dBm
   * RF24_PA_MAX = 0dBm which is the highest power and longest range
   * dBm stands for decibel milliwatts. it is measured on a logarithmic scale, so very small numbers will be negative and very large numbers will be positive */
  radio.stopListening(); //sets the tranciever to end receiving mode (default setting) and prepare to transmit;
}

void loop() {
  // put your main code here, to run repeatedly:

  if ((statusLEDcount) >= 5) {
    digitalWrite(statusLED, HIGH);
  }else {
    digitalWrite(statusLED, LOW);
  }

  int rawLeftY = analogRead(A1);
  int rawRightY = analogRead(A0);


  int leftMapped = map(rawLeftY, 0, 1023, -255, 255);
  int rightMapped = map(rawRightY, 0, 1023, -255, 255);

  if (abs(leftMapped) < 25) leftMapped = 0;
  if (abs(rightMapped) <25) rightMapped = 0;

  txData.motorLeftSpeed = leftMapped;
  txData.motorRightSpeed = rightMapped;

  txData.leftButton = (digitalRead(leftSWPin) == LOW);
  txData.rightButton = (digitalRead(rightSWPin) == LOW);

  radio.write(&txData, sizeof(PacketData));

  statusLEDcount = (statusLEDcount + 1);

  if (statusLEDcount > 10) statusLEDcount = 0;
  
  delay(20);
  
}
