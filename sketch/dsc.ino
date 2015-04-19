#include <EEPROM.h>
#include <SoftwareSerial.h>

//TC35
#define rxPin 10  
#define txPin 9
#define igtPin 8
//DSC
#define CLK 2 // Yellow
#define DTA 5 // Green

#define BOOT 99
#define READY 0
#define ARMING 1
#define ARMED 2
#define ALERT 3
#define SYSTEMREADY 4
#define SYSTEMARMED 5

String PHONESNUMBERS= "";
// TC35 Var
SoftwareSerial serialTC35 = SoftwareSerial(rxPin, txPin); 

String st;
int inByte;
int lastSend=99;//0 READY //1 ARMED //2 ALERT //3 SYSTEM
String strIn="";
boolean prog_mode=false;
boolean sendsms_mode=false;
String activateRandomKey="";

byte LedS=0;
byte LedS2=0;
byte LedSB=0;
byte LedZone=0;
byte LedZoneM=0;
byte LedZoneB=0;

void initDSC();
void initTC35();
long lastchangedLed;
unsigned long StatusLeds = 0;
unsigned long ultSentStatusLeds = 0;
void outputTC35();


void setup() {
  Serial.begin(9600);//57600
  Serial.println("DSC.SMSGATEWAY.20150418");
  initDSC();
  initTC35();
  // GET PHONE NUMBER SAVED FROM EPROM
  int i;
  randomSeed(analogRead(1)+millis());
  PHONESNUMBERS = "";
  EEPROM.read(1008);
  for (i=0; i<11; i++) {
    PHONESNUMBERS += Hex8(EEPROM.read(1008+i));
  }
  Serial.print("PHONESNUMBERS:");
  Serial.println(PHONESNUMBERS);


}

void initDSC(){
  //INIT DSC
  pinMode(CLK,INPUT); 
  pinMode(DTA,INPUT);
  attachInterrupt(CLK-2,clock,RISING);
  lastchangedLed=(long)millis()+500;
  ultSentStatusLeds = 0;
}

void initTC35(){

  pinMode(igtPin, INPUT);
  digitalWrite(igtPin, LOW);
  pinMode(igtPin, OUTPUT);
  delay(100);
  pinMode(igtPin, INPUT);
  delay(2000);
  serialTC35.begin(9600);
  delay(1000);
  serialTC35.println("AT+CMGF=1");
  delay(500);
  outputTC35();
  serialTC35.println("AT+CNMI=2,1,0,0,1");
  delay(500);
  outputTC35();
  serialTC35.println("AT^SMGO=1");
  delay(500);
  outputTC35();
  //Save Settings to Modem flash memory permanently
  serialTC35.println("AT&W");
  delay(500);
  outputTC35();
  //TODO: Mettez votre numéro de superviseur
  SendTextMessage("336XXXXXXXX","Lancement la supervision de l'alarme");
}

void loop() {
  outputTC35();
  if(Serial.available()) {  
    int inByte = Serial.read();
    strIn += char(inByte);
    if(inByte == '$') {strIn = ""; prog_mode=true; }
    if (prog_mode) {
      if (inByte == '%') {
        if (strIn.substring(0,4) == "PRG:") {
          char separator = ':';
          String command = strIn.substring(4, strIn.length()-1);
          if (command.indexOf(separator) < 0) {
            Serial.println("ERR:CMD"); // Oups format incorrect
          }
          else {
            int pos = command.substring(0, command.indexOf(separator)).toInt();
            String bytes = command.substring(command.indexOf(separator)+1);
            int len = bytes.length()/2;
            if (pos >= 0 and pos < 1024) {
              Serial.println("Programming EEPROM (" + String(pos) + ") = "+bytes);
              int i;
              for (i=0; i<len; i++) {
                byte sendb = getVal(bytes[i*2+1]) + (getVal(bytes[i*2]) << 4);
                EEPROM.write(pos+i, sendb);
              }
              Serial.println("OK!");
            }
            else
              Serial.println("ERR:POS"); //La zone d'Eprom doit-être entr 1 et 1023
          }
        }
        else if (strIn.substring(0,4) == "SMS:"){
          Serial.print("Send SMS to");
          String command = strIn.substring(4, strIn.length()-1);
          String number = command.substring(0, 11);
          Serial.println(number);
          String message= command.substring(11);
          SendTextMessage(number,message);
        }
        else if (strIn.substring(0,4) == "ATM:"){
          Serial.print("Send Command");
          String command = strIn.substring(4, strIn.length()-1);
          serialTC35.println(command);
          outputTC35();
        }      
      strIn = "";
      prog_mode = false;      
      }      
    }
    else {

      if(inByte == 'T') {
        Serial.print("SYS:STATUS:1");
      }
      
      if(inByte == 'V') {
        activateRandomKey = Hex8(random(256))+Hex8(random(256))+Hex8(random(256))+Hex8(random(256));// Pour etre sur de l'unicité du message        
        Serial.print("SYS:");
        Serial.print("20150418"); //Firmware version
        Serial.print(":1:");
        Serial.print(PHONESNUMBERS);
        Serial.print(":");
        Serial.println(activateRandomKey);
      }
      
      if(inByte == 'L') {
        Serial.println(printLed());
      }
      if(inByte == 'H') {
        Serial.println("'$PRG:1008:NUMBERNUMBER%'   NUMERO POUR L'ALARME (EPROM) NUMBER: numero mobile format 336XXXXXXXX");
        Serial.println("'$SMS:NUMBERMESSAGE%'  ENVOI DE SMS NUMBER: numero mobile format 336XXXXXXXX");
        Serial.println("'$ATM:COMMAND%'        ENVOI DE COMMANDE A LA CARTE");
        Serial.println("T  Test");
        Serial.println("S  Retourne l'etat du systeme");
        Serial.println("L  Retourne le statut des Led de l'alarme");
      }
    }
  }
  // On regarde l'etat du flux de l'alarme
  if (signalChange(1) < 1800) return;     

  String stc = st; 
  st = "";
  
  int cmd = getData(stc,0);
  
  if((!prog_mode) && (strIn.length() > 0)) {
    int inByte = strIn[0];
    strIn = strIn.substring(1);

    if(inByte == 'E') {
      Serial.println("SYS:ECHO");
    }
  }  

  if (cmd == 0) return;     

  if (cmd == 0xa5)
  {
    Serial.print("DSC:");
    if (stc.length() >= 48 )
    {
      Serial.print(Hex8(getData(stc,9)));
      Serial.print(" ");
      Serial.print(Hex8(getData(stc,17)));
      Serial.print(" ");   
      Serial.print(Hex8(getData(stc,25)));
      Serial.print(" ");
      Serial.print(Hex8(getData(stc,33)));
      Serial.print(" ");
      Serial.println(Hex8(getData(stc,41)));
    }  
    else
      Serial.println("ERR");
  }

  if (cmd == 0x05)
  {
    LedS=(byte)getData(stc,8+1);
    LedS2=(byte)getData(stc,8+1+8);    
    if (StatusLeds != StatusLed()) changedLed();   
  } 

  if (cmd == 0x0a)
  {   
    LedZoneM = (byte)getData(stc,8+1+8+8);
    if (StatusLeds != StatusLed()) changedLed();
  }

  if (cmd == 0x5d)
  {
    LedSB=(byte)getData(stc,8+1);
    LedZoneB=(byte)getData(stc,8+1+8);
    if (StatusLeds != StatusLed()) changedLed();
  }   
  
  if (cmd == 0x27)
  { 
    LedZone=(byte)getData(stc,8+1+8+8+8+8);
    if (StatusLeds != StatusLed()) changedLed();   
  } 
  
  if (cmd == 0x64)
  {
    Serial.println("BEP:"+Hex8(getData(stc,9)));
  }
  
  if (cmd == 0x7F)
  {
    Serial.println("BEP:"+Hex8(getData(stc,9)));
  }  
 
  if ((lastchangedLed != 0) && (((long)millis() - lastchangedLed)  >= 0)) {
    if (ultSentStatusLeds != StatusLed()) {
      Serial.println(printLed());      
      ultSentStatusLeds = StatusLed();
    }
    lastchangedLed = 0;
  }
}
  
void changedLed() {
    StatusLeds = StatusLed();
    lastchangedLed=(long)millis()+500;    
}

void clock() 
{
    if (st.length() > 180) return;
    if (digitalRead(DTA)) st += "1"; 
    else st += "0";
} 

unsigned long StatusLed()
{
  unsigned long statusled=(int)LedS+(int)LedSB+(int)LedZone+(int)LedZoneB+(int)LedZoneM+(int)LedS2;
  return statusled;
}

unsigned long signalChange(int current) 
{ 
  unsigned long d = 0; 
  while (digitalRead(CLK) == current) 
  { 
    delayMicroseconds(20); 
    d += 20; 
    if (d > 9000) break;
  } 
  return d; 
} 

unsigned int getData(String st, int position) 
{ 
  int ret = 0; 
  for(int j=0;j<8;j++) 
  { 
    ret <<= 1; 
    if (st[position+j] == '1') ret |= 1; 
  } 
  return ret; 
} 

String Hex8(int num) {
  char res[3];
  sprintf(&res[0], "%02X", num);
  return (res);
}

byte getVal(char c)
{ 
   if(c >= '0' && c <= '9')
     return (byte)(c - '0');
   else
     return (byte)(c-'A'+10);
}

String printLed()
{
  String ret = "LED:";
  byte LedReady = 0;
  byte LedArmed = 0;
  byte LedSystem = 0;  
  if (LedS & 1) LedReady=1;
  if (LedS & 2) LedArmed=1;
  if (LedS & 4) LedSystem=1;
  if (LedS & 8) LedSystem=1;
  if (LedS & 16) LedSystem=1;
  if (LedS & 32) LedSystem=1;
  if (LedSB & 1) LedReady=2;
  if (LedSB & 2) LedArmed=2;
  if (LedSB & 4) LedSystem=2;
  if (LedSB & 8) LedSystem=2;
  if (LedSB & 16) LedSystem=2;
  if (LedSB & 32) LedSystem=2;
  ret += String(LedReady) + String(LedArmed) + String(LedSystem)+"-";
  switch(lastSend)
  {
   case READY:
     if (LedSystem !=0 && LedArmed==0){sendSMS(SYSTEMREADY);}//DEFAULT ALARM
     //if (LedReady==1 && LedArmed==1){sendSMS(ARMING);}
     if (LedReady==0 && LedArmed!=0){sendSMS(ARMED);}
     break;
   case ARMING:
     if (LedReady==0 && LedArmed==1){sendSMS(ARMED);}
     break;
   case ARMED:
     if (LedReady!=0){sendSMS(READY);}
     if (LedSystem ==2){sendSMS(ALERT);}
     if (LedSystem ==1){sendSMS(SYSTEMARMED);}
     break;
   case ALERT:
     if(LedArmed !=1){sendSMS(READY);}
     break;
   case SYSTEMREADY:
     if(LedSystem==0){lastSend=READY;}
      break;     
   case SYSTEMARMED:
     if(LedSystem==0){lastSend=ARMED;}
     break;
   default:
     if(LedArmed==1 && LedReady==0){lastSend=ARMED;}
     else{lastSend=READY;}
     break;
   }
  byte i;
  byte x = LedZone;
  byte y = LedZoneB;
  short int b = 0;
  if (LedS2 & 128) {x = LedZoneM;}
  for(i=0;i<8;i++)
  {
    b=0;
    if (x & 1) b=1;
    if (y & 1) b=2;
    x >>= 1;
    y >>= 1;
    ret += String(b);
  }
  return ret;  
}

void sendSMS(int action)
{
  String msg="";
  switch(action)
  {
  case READY:
    msg="Alarme Desarmee";
    break;
  case ARMING:
    msg="Alarme en cours d'armement";
    break;
  case ARMED:
    msg="Alarme Armee";
    break;
  case ALERT:
    msg="UGRGENT ALARME DECLENCHEE";
    break;
  case SYSTEMARMED:
    msg="Alarme Armee avec Defaut";
    break;
  case SYSTEMREADY:
    msg="Alarme Desarmee avec defaut";
    break;
  }
  Serial.println(msg);
  lastSend = action;
  int p;
  String num="";

  for(p=0;p<4;p++)
  {
    int currentPos = (p==0)?0:p*11+1;
    
    if ((PHONESNUMBERS.substring(p*11,p*11+2)!="33"))
    {
      Serial.println(PHONESNUMBERS.substring(p*11,p*11+2));
     // CE nest pas un numero de telephone
      break;
    }
    num+=PHONESNUMBERS.substring(p*11,p*11+11);
    SendTextMessage(num,msg);
    num="";
  }
}
 boolean SendTextMessage(String NUMBER, String message)  
 {
  String action = "AT+CMGS=\"+"+NUMBER+"\"\r";
  Serial.println(action);
  serialTC35.print(action);  //"AT+CMGS=\"+336xxxxxxxx\"\r"
  delay(500);  
  serialTC35.print(message);  
  delay(500);  
  serialTC35.print((char)26);//the ASCII code of the ctrl+z is 26  
  serialTC35.println();
  delay(1200);
  outputTC35();
  Serial.println("SMSSEND: FIN -----------"); 
  return true;
}  

void outputTC35(){
 char incoming_char=0;
   while(serialTC35.available()!=0)
    {
      incoming_char =char(serialTC35.read());
      switch (incoming_char)
      {
         case '\n':
            Serial.println("");
            break;
         case '\r':   // discard carriage return
            Serial.println("");
            break;
         default:
           Serial.print(incoming_char);
           break;
      }
    }
}
