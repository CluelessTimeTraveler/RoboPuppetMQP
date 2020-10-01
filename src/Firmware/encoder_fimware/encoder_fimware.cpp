#include <SPI.h>
#include "Encoders.h"

/**
 * Private subsystem info
 */
namespace Encoders
{
  // SPI Pins - should be automatically selected
  const uint8_t SPI_MOSI = 11;     // MOSI pin
  const uint8_t SPI_MISO = 12;     // MISO pin
  const uint8_t SPI_SCLK = 12;     // SLCK pin

  //Chip or Slave select
  const uint8_t encoder1 = 2;
  const uint8_t encoder2 = 3;
  const uint8_t encoder3 = 4;
  const uint8_t encoder4 = 5;

  //SPI commands
  const uint8_t AMT22_READ = 0x00;
  const uint8_t AMT22_RESET = 0x60;
  const uint8_t AMT22_ZERO = 0x70;

  // Init flag
  bool init_complete = false;

  // Methods
  uint8_t spiTransmit(uint8_t encoder, uint8_t msg, uint8_t releaseLine);
}

/**
 * Initializes subsystem
 */
void Encoders::init()
{
  if (!init_complete)
  {
    //Set up encoder pins 
    pinMode(encoder1, OUTPUT);
    pinMode(encoder2, OUTPUT);
    pinMode(encoder3, OUTPUT);
    pinMode(encoder3, OUTPUT);
  
    //Set the CS line high which is the default inactive state
    digitalWrite(encoder1, HIGH);
    digitalWrite(encoder2, HIGH);
    digitalWrite(encoder3, HIGH);
    digitalWrite(encoder4, HIGH);
    
    SPI.begin();
    SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
    //SPI.setBitOrder(MSBFIRST);
    //SPI.setDataMode(SPI_MODE0);
  
    //Initalize the serial connection - check baud rate for microcontroller
    Serial.begin(115200);
  
    //Nice screen things
    Serial.println("Encoders Initialized");
    
    SPI.end();

    // Set init flag
    init_complete = true;
  }
}

/**
 * @brief Transmits message to encoder
 * @param encoder number [1,2,3,4], msg (see SPI Commands above), releaseLine [high, low]
 */
uint8_t Encoders::spiTransmit(uint8_t encoder, uint8_t msg, uint8_t releaseLine)  //SPI transmit sequence
{
   //Variable for recieved data
   uint8_t encoder_data;            
   //Enable specific encoder 
   digitalWrite(encoder,LOW); 
   delayMicroseconds(3);
   //Send and recieve data  
   encoder_data = SPI.transfer(msg); 
   //Deselect the encoder
   digitalWrite(encoder,HIGH);
   delayMicroseconds(3);
   //Choose if the encoder is released
   digitalWrite(encoder, releaseLine);
   return(encoder_data);
}

/**
 * @brief Gets encoder position
 * @param encoder number [1,2,3,4], resolution[12,14]
 */
uint16_t Encoders::getPositionSPI(uint8_t encoder, uint8_t resolution)
{
  uint16_t currentPosition;       //16-bit response from encoder
  bool binaryArray[16];           //after receiving the position we will populate this array and use it for calculating the checksum

  //Read in the 16 bit value from the encoder
  currentPosition = spiTransmit(encoder,AMT22_READ, false) << 8;   //get MSB which is the high byte, shift it 8 bits. don't release line.
  delayMicroseconds(3);
  currentPosition |= spiTransmit(encoder,AMT22_READ, true); //OR the low byte with the currentPosition variable to get the LSB. release line after second byte

  //run through the 16 bits of position and put each bit into a slot in the array so we can do the checksum calculation
  for(int i = 0; i < 16; i++){
    binaryArray[i] = (0x01) & (currentPosition >> (i));
  }

  //using the equation on the datasheet we can calculate the checksums and then make sure they match what the encoder sent
  if ((binaryArray[15] == !(binaryArray[13] ^ binaryArray[11] ^ binaryArray[9] ^ binaryArray[7] ^ binaryArray[5] ^ binaryArray[3] ^ binaryArray[1]))
          && (binaryArray[14] == !(binaryArray[12] ^ binaryArray[10] ^ binaryArray[8] ^ binaryArray[6] ^ binaryArray[4] ^ binaryArray[2] ^ binaryArray[0])))
    {
      //we got back a good position, so just mask away the checkbits
      currentPosition &= 0x3FFF;
    }
  else
  {
    currentPosition = 0xFFFF; //bad position
  }

  //If the resolution is 12-bits, and wasn't 0xFFFF, then shift position, otherwise do nothing
  if ((resolution == 12) && (currentPosition != 0xFFFF)) currentPosition = currentPosition >> 2;

  return currentPosition;
}

void setup()
{
    Encoders::init();
}

void loop() 
{
  //create a 16 bit variable to hold the encoders position
  uint16_t encoderPosition;
  //let's also create a variable where we can count how many times we've tried to obtain the position in case there are errors
  uint8_t attempts;

  //once we enter this loop we will run forever
  while(1)
  {
    //set attemps counter at 0 so we can try again if we get bad position    
    attempts = 0;

    //send the function either res12 or res14 for your encoders resolution
    encoderPosition = Encoders::getPositionSPI(Encoders::encoder1, 14); 

    //if the position returned was 0xFFFF we know that there was an error calculating the checksum
    //make 3 attempts for position. we will pre-increment attempts because we'll use the number later and want an accurate count
    while (encoderPosition == 0xFFFF && ++attempts < 3)
    {
      encoderPosition = Encoders::getPositionSPI(Encoders::encoder1, 14); //try again
    }

    if (encoderPosition == 0xFFFF) //position is bad, let the user know how many times we tried
    {
      Serial.print("Encoder 0 error. Attempts: ");
      Serial.print(attempts, DEC); //print out the number in decimal format. attempts - 1 is used since we post incremented the loop
    }
    else //position was good, print to serial stream
    {
      Serial.print("Encoder 0: ");
      Serial.print(encoderPosition, DEC); //print the position in decimal format
    }

  }
}
