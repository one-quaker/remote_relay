#include <SoftwareSerial.h>

#define tel1 "380991234567" // sample
#define tel2 "380981234567" // sample

#define R1 4
#define R2 5

String r1State = "OFF";
String r2State = "OFF";
String tel;
bool smsSend = false;

//Input String
String input;
String input1;


SoftwareSerial gsm(2, 3); // RX, TX


void setup() {
  Serial.begin(38400);
  delay(5000);
  gsm.begin(38400);
  delay(1000);
  gsmSetup();

  pinMode(R1, OUTPUT);
  pinMode(R2, OUTPUT);
}

void loop() {
  if (smsSend == true) {
    smsSend = false;
    sendSms(tel, "Relay status: #1 " + r1State + ", #2 " + r2State);
  }
    
  if (gsm.available()) {
    input1 = "";
    input1 = gsm.readStringUntil('\r');
    gsm.read();
    delay(50);
    
    if (input1.length() > 0) {
      parseCmd(input1);
    }
    
    Serial.println(input1);
  }
  if (Serial.available()) {
    input = "";
  
    //Read Serial input to the end of line
    input = Serial.readStringUntil('\n');
    Serial.read();
    delay(50);

    if (input.length() > 0) {
      parseCmd(input);
      gsm.print(input + '\r');
    }
    
  }

}

void gsmSetup() {
  gsm.print("AT+CMGF=1\r");  
  delay(100);
  gsm.print("AT+CSCS=\"GSM\"\r");   
  delay(100);
  gsm.print("AT+CNMI=2,2,0,0,0\r");
  delay(100);
//  gsm.println("AT+CSMS=1"); // SMS GSM03.40 and GSM03.41;SMS related AT commands support GSM07.05 Phase 2+
//  delay(100);
  gsm.println("AT+CLIP=1");  // turn on Caller ID
  delay(100);
//  gsm.println("AT+CMEE=0");
//  delay(100);
//  gsm.println("ATE0"); // set ECHO 0
//  delay(100);
}

void sendSms(String num, String msg) {
  delay(5000);
  gsm.println("AT+CMGF=1"); // AT command to send SMS message
  delay(100);
  gsm.print("AT+CMGS=");                                     // recipient's mobile number, in international format E.123
  delay(100);
  gsm.println("\"" + num + "\"");
  delay(100);
  gsm.println(msg); // message to send
  delay(100);
  gsm.println((char)26);   // End AT command with a ^Z, ASCII code 26
  delay(100);
}

void parseCmd(String in) {
  String arg1 = getValue(in, ' ', 0);
  String arg2 = getValue(in, ' ', 1);
  String arg3 = getValue(in, ' ', 2);

  mainControl(arg1, arg2, arg3);
}

void mainControl(String arg1, String arg2, String arg3) {
  if (arg1.equalsIgnoreCase("on") && arg2.equals("1")) {
    relayControl(R1, HIGH);
    printState();
  }
  else if (arg1.equalsIgnoreCase("off") && arg2.equals("1")) {
    relayControl(R1, LOW);
    printState();
  }
  else if ((arg1.equalsIgnoreCase("on") && arg2.equals("all")) || (arg1.equalsIgnoreCase("o") && arg2.equals(""))) {
    relayControl(R1, HIGH);
    relayControl(R2, HIGH);
    printState();
  }
  else if ((arg1.equalsIgnoreCase("off") && arg2.equals("all")) || (arg1.equalsIgnoreCase("r") && arg2.equals(""))) {
    relayControl(R1, LOW);
    relayControl(R2, LOW);
    printState();
  }
  else if (arg1.equalsIgnoreCase("stat")) {
    printState();
    smsSend = true;
  }
  else if (arg1.equals("+CMT:")) {
    tel = arg2.substring(2, 14); // detect phone number from incoming sms
  }
  else if (arg1.equals("+CLIP:")) {
    tel = arg2.substring(1, 13); // detect phone number from incoming call
    // Serial.println(tel);
    if ((tel == tel1 || tel == tel2)) {
      Serial.println("Phone number in white list!");
      checkRelays();
      gsm.println("AT+CLIP=0");
      delay(100);
      gsm.println("ATH0");
      delay(100);
    }
    else {
      Serial.println("Phone number not in white list!");
    }
  }
  else if (arg1.equals("ATH0")) {
    gsm.println("AT+CLIP=1");
    delay(100);
    smsSend = true;
    printState();
  }
//  else {
//    Serial.println("Invalid input!");
//  }
}

void checkRelays() {
  if (r1State == "ON") {
    relayControl(R1, LOW);
  }
  else if (r1State == "OFF") {
    relayControl(R1, HIGH);
  }
  if (r2State == "ON") {
    relayControl(R2, LOW);
  }
  else if (r2State == "OFF") {
    relayControl(R2, HIGH);
  }
}

void relayControl(int pin, int state) {
  digitalWrite(pin, state);

  String s;
  if (state == 0) {
    s = "OFF";  
  }
  else if (state == 1) {
    s = "ON";  
  }

  if (pin == R1) {
    r1State = s;
  }
  else if (pin == R2) {
    r2State = s;
  }
}

void printState() {
  Serial.println("#R1 " + r1State + ", #R2 " + r2State);
}

// http://stackoverflow.com/questions/9072320/split-string-into-string-array
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}
