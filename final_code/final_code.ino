//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
String get_Coordinates();
void init_gsm();
void gprs_connect();
boolean gprs_disconnect();
boolean is_gprs_connected();
void post_to_firebase(String data);
boolean waitResponse(String expected_answer = "OK", unsigned int timeout = 2000);
int state = 0;
const int pin = 9;
float gpslat, gpslon;

//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
#include <SoftwareSerial.h>
#include <TinyGPS.h>
//GSM Module RX pin to Arduino 3
//GSM Module TX pin to Arduino 2
#define rxPin 2
#define txPin 3
#define gpPinA 4
#define gpPinB 5
TinyGPS gps;
SoftwareSerial SIM800(rxPin, txPin);
SoftwareSerial sgsm(rxPin, txPin);
SoftwareSerial sgps(gpPinA, gpPinB);
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

const String APN  = "ENTER_APN";
const String USER = "ENTER_USER_NAME";
const String PASS = "ENTER_PASSWORD";

const String FIREBASE_HOST  = "ENTER_FIREBASE_HOST_URL";
const String FIREBASE_SECRET  = "ENTER_FIREBASE_SECRET_KEY";


#define USE_SSL true
#define DELAY_MS 500


//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
//Void function initialization
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
void setup() {

  //Begin serial communication with Serial Monitor
  Serial.begin(115200);

  //Begin serial communication with SIM800
  SIM800.begin(9600);
  sgsm.begin(9600);
  sgps.begin(9600);

  Serial.println("Initializing SIM800...");
  init_gsm();
}

void loop() {
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //Loop for sending SMS
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  sgps.listen();
  while (sgps.available())
  {
    int c = sgps.read();
    if (gps.encode(c))
    {
      gps.f_get_position(&gpslat, &gpslon);
    }
  }
  if (digitalRead(pin) == HIGH && state == 0) {
    sgsm.listen();
    sgsm.print("\r");
    delay(1000);
    sgsm.print("AT+CMGF=1\r");
    delay(1000);
    /*Replace XXXXXXXXXX to 10 digit mobile number &
      ZZ to 2 digit country code*/
    sgsm.print("AT+CMGS=\"+254702677999\"\r");
    delay(1000);
    //The text of the message to be sent.
    sgsm.println("Hello,i'm in danger please find me.");
    sgsm.print("Latitude :");
    sgsm.println(gpslat, 6);
    sgsm.print("Longitude:");
    sgsm.println(gpslon, 6);
    delay(1000);
    sgsm.write(0x1A);
    delay(1000);
    state = 1;
  }
  if (digitalRead(pin) == LOW) {
    state = 0;
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //End of sending SMS
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //End of sending Data to the satabase
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  String data = get_Coordinates();
  Serial.print(data);

  if (!is_gprs_connected()) {
    gprs_connect();
  }
  post_to_firebase(data);

  delay(1000);

}
String get_Coordinates() {
  sgps.listen();
  while (sgps.available())
  {
    int c = sgps.read();
    if (gps.encode(c))
    {
      gps.f_get_position(&gpslat, &gpslon);
    }
  }
  if (digitalRead(pin) == HIGH && state == 0) {
    sgsm.listen();
    String Lat = String(gpslat, 6);
    String Lng = String(gpslon, 6);

    String Data = "{";

    Data += "\"Latitude\":\"" + Lat + "\",";

    Data += "\"Longitude\":\"" + Lng + "\"";

    Data += "}";

    return Data;
    state = 1;
  }
  if (digitalRead(pin) == LOW) {
    state = 0;
  }
}



//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//Function: post_to_firebase() start
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
void  post_to_firebase(String data) {
  // MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //Start HTTP connection
  SIM800.println("AT+HTTPINIT");
  waitResponse();
  delay(DELAY_MS);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //Enabling SSL 1.0
  if (USE_SSL == true) {
    SIM800.println("AT+HTTPSSL=1");
    waitResponse();
    delay(DELAY_MS);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //Setting up parameters for HTTP session
  SIM800.println("AT+HTTPPARA=\"CID\",1");
  waitResponse();
  delay(DELAY_MS);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //Set the HTTP URL - Firebase URL and FIREBASE SECRET
  SIM800.println("AT+HTTPPARA=\"URL\"," + FIREBASE_HOST + ".json?auth=" + FIREBASE_SECRET);
  waitResponse();
  delay(DELAY_MS);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //Setting up re direct
  SIM800.println("AT+HTTPPARA=\"REDIR\",1");
  waitResponse();
  delay(DELAY_MS);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //Setting up content type
  SIM800.println("AT+HTTPPARA=\"CONTENT\",\"application/json\"");
  waitResponse();
  delay(DELAY_MS);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //Setting up Data Size
  //+HTTPACTION: 1,601,0 - error occurs if data length is not correct
  SIM800.println("AT+HTTPDATA=" + String(data.length()) + ",10000");
  waitResponse("DOWNLOAD");
  //delay(DELAY_MS);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //Sending Data
  SIM800.println(data);
  waitResponse();
  delay(DELAY_MS);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //Sending HTTP POST request
  SIM800.println("AT+HTTPACTION=1");

  for (uint32_t start = millis(); millis() - start < 20000;) {
    while (!SIM800.available());
    String response = SIM800.readString();
    if (response.indexOf("+HTTPACTION:") > 0)
    {
      Serial.println(response);
      break;
    }
  }

  delay(DELAY_MS);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //+HTTPACTION: 1,603,0 (POST to Firebase failed)
  //+HTTPACTION: 0,200,0 (POST to Firebase successfull)
  //Read the response
  SIM800.println("AT+HTTPREAD");
  waitResponse("OK");
  delay(DELAY_MS);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //Stop HTTP connection
  SIM800.println("AT+HTTPTERM");
  waitResponse("OK", 1000);
  delay(DELAY_MS);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
}

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// Initialize GSM Module
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
void init_gsm()
{
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //Testing AT Command
  SIM800.println("AT");
  waitResponse();
  delay(DELAY_MS);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //Checks if the SIM is ready
  SIM800.println("AT+CPIN?");
  waitResponse("+CPIN: READY");
  delay(DELAY_MS);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //Turning ON full functionality
  SIM800.println("AT+CFUN=1");
  waitResponse();
  delay(DELAY_MS);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //Turn ON verbose error codes
  SIM800.println("AT+CMEE=2");
  waitResponse();
  delay(DELAY_MS);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //Enable battery checks
  SIM800.println("AT+CBATCHK=1");
  waitResponse();
  delay(DELAY_MS);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //Register Network (+CREG: 0,1 or +CREG: 0,5 for valid network)
  //+CREG: 0,1 or +CREG: 0,5 for valid network connection
  SIM800.println("AT+CREG?");
  waitResponse("+CREG: 0,");
  delay(DELAY_MS);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //setting SMS text mode
  SIM800.print("AT+CMGF=1\r");
  waitResponse("OK");
  delay(DELAY_MS);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
}


//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//Connect to the internet
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
void gprs_connect()
{
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //DISABLE GPRS
  SIM800.println("AT+SAPBR=0,1");
  waitResponse("OK",60000);
  delay(DELAY_MS);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //Connecting to GPRS: GPRS - bearer profile 1
  SIM800.println("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
  waitResponse();
  delay(DELAY_MS);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //sets the APN settings for your sim card network provider.
  SIM800.println("AT+SAPBR=3,1,\"APN\","+APN);
  waitResponse();
  delay(DELAY_MS);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //sets the user name settings for your sim card network provider.
  if(USER != ""){
    SIM800.println("AT+SAPBR=3,1,\"USER\","+USER);
    waitResponse();
    delay(DELAY_MS);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //sets the password settings for your sim card network provider.
  if(PASS != ""){
    SIM800.println("AT+SAPBR=3,1,\"PASS\","+PASS);
    waitResponse();
    delay(DELAY_MS);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //after executing the following command. the LED light of 
  //sim800l blinks very fast (twice a second) 
  //enable the GPRS: enable bearer 1
  SIM800.println("AT+SAPBR=1,1");
  waitResponse("OK", 30000);
  delay(DELAY_MS);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //Get IP Address - Query the GPRS bearer context status
  SIM800.println("AT+SAPBR=2,1");
  waitResponse("OK");
  delay(DELAY_MS);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
}



/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
* Function: gprs_disconnect()
* AT+CGATT = 1 modem is attached to GPRS to a network. 
* AT+CGATT = 0 modem is not attached to GPRS to a network
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
boolean gprs_disconnect()
{
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //Disconnect GPRS
  SIM800.println("AT+CGATT=0");
  waitResponse("OK",60000);
  //delay(DELAY_MS);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //DISABLE GPRS
  //SIM800.println("AT+SAPBR=0,1");
  //waitResponse("OK",60000);
  //delay(DELAY_MS);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

  return true;
}
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
* Function: gprs_disconnect()
* checks if the gprs connected.
* AT+CGATT = 1 modem is attached to GPRS to a network. 
* AT+CGATT = 0 modem is not attached to GPRS to a network
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
boolean is_gprs_connected()
{
  SIM800.println("AT+CGATT?");
  if(waitResponse("+CGATT: 1",6000) == 1) { return false; }

  return true;
}


//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//Handling AT COMMANDS
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//boolean waitResponse(String expected_answer="OK", unsigned int timeout=2000) //uncomment if syntax error (arduino)
boolean waitResponse(String expected_answer, unsigned int timeout) //uncomment if syntax error (esp8266)
{
  uint8_t x=0, answer=0;
  String response;
  unsigned long previous;
    
  //Clean the input buffer
  while( SIM800.available() > 0) SIM800.read();
  
  //NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
  previous = millis();
  do{
    //if data in UART INPUT BUFFER, reads it
    if(SIM800.available() != 0){
        char c = SIM800.read();
        response.concat(c);
        x++;
        //checks if the (response == expected_answer)
        if(response.indexOf(expected_answer) > 0){
            answer = 1;
        }
    }
  }while((answer == 0) && ((millis() - previous) < timeout));
  //NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
  
  Serial.println(response);
  return answer;
}
