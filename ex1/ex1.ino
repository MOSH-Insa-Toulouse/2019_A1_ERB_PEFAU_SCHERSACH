#include <rn2xx3.h>
#include <SoftwareSerial.h>
#include <avr/sleep.h>

SoftwareSerial mySerial(10, 11); // RX, TX

//create an instance of the rn2xx3 library,
//giving the software serial as port to use
rn2xx3 myLora(mySerial);

//Boolean value to hold the fact that
//the analog comp was triggered
bool triggered = false;


void onGazTooHigh(){
  
  if(digitalRead(2) == HIGH)  
        triggered = true;
        
    //TODO: save value on trigger ?
}




// the setup routine runs once when you press reset:
void setup()
{
 
  
  //output LED pin
  pinMode(13, OUTPUT);
  led_on();

  // Open serial communications and wait for port to open:
  Serial.begin(57600); //serial port to computer
  mySerial.begin(9600); //serial port to radio
  Serial.println("Startup");

  initialize_radio();

  //transmit a startup message
  myLora.tx("TTN Mapper on TTN Enschede node");

  led_off();
  delay(2000);

 pinMode(2, INPUT);
 attachInterrupt(digitalPinToInterrupt(2),onGazTooHigh,CHANGE); 
  
}

void initialize_radio()
{
  //reset rn2483
  pinMode(8, OUTPUT);
  digitalWrite(8, LOW);
  delay(500);
  digitalWrite(8, HIGH);

  delay(100); //wait for the RN2xx3's startup message
  mySerial.flush();

  //Autobaud the rn2483 module to 9600. The default would otherwise be 57600.
  myLora.autobaud();

  //check communication with radio
  String hweui = myLora.hweui();
  while(hweui.length() != 16)
  {
    Serial.println("Communication with RN2xx3 unsuccessful. Power cycle the board.");
    Serial.println(hweui);
    delay(20000);
    hweui = myLora.hweui();
  }

  //print out the HWEUI so that we can register it via ttnctl
  Serial.println("When using OTAA, register this DevEUI: ");
  Serial.println(myLora.hweui());
  Serial.println("RN2xx3 firmware version:");
  Serial.println(myLora.sysver());

  //configure your keys and join the network
  Serial.println("Trying to join TTN");
  bool join_result = false;


  /*
   * ABP: initABP(String addr, String AppSKey, String NwkSKey);
   * Paste the example code from the TTN console here:
   */
  const char *devAddr = "26011E01";
  const char *nwkSKey = "C33682E4FC23BA0A6910C0D00F2E4630";
  const char *appSKey = "81E1FCDE0C90BE4A5CEBF35134523ACE";

  join_result = myLora.initABP(devAddr, appSKey, nwkSKey);

  /*
   * OTAA: initOTAA(String AppEUI, String AppKey);
   * If you are using OTAA, paste the example code from the TTN console here:
   */
  //const char *appEui = "70B3D57ED00001A6";
  //const char *appKey = "A23C96EE13804963F8C2BD6285448198";

  //join_result = myLora.initOTAA(appEui, appKey);


  while(!join_result)
  {
    Serial.println("Unable to join. Are your keys correct, and do you have TTN coverage?");
    delay(60000); //delay a minute before retry
    join_result = myLora.init();
  }
  Serial.println("Successfully joined TTN");

}

// the loop routine runs over and over again forever:
void loop()
{
    led_on();

    float sensorVoltage;
    float sensorValue;
   
    sensorValue = analogRead(A0);
    sensorVoltage = sensorValue/1024*5.0;
   
    Serial.print("sensor voltage = ");
    Serial.print(sensorVoltage);
    Serial.println(" V");
    //delay(1000)

    Serial.println("TXing");
    char charVal[10];                
  
  //4 is mininum width, 3 is precision; float value is copied onto buff
    dtostrf(sensorVoltage, 4, 3, charVal);
    Serial.print(charVal);
    myLora.tx(charVal); //one byte, blocking function

    led_off();

    if (triggered == true ) {
      triggered = false; 
      Serial.println("hey");
    }
    
    Serial.println("Will go to sleep...");
    

    // TODO test
    // Puts the board to sleep, will return on interrupt
    putBoardSleep();

    delay(200);

}

void led_on()
{
  digitalWrite(13, 1);
}

void led_off()
{
  digitalWrite(13, 0);
}

//Puts the arduino to sleep
//Returns when MCU gets awaken by interrupt
void putBoardSleep()
{
    sleep_enable();
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    // TODO check if other stuff are to be stopped
    sleep_cpu();
}
