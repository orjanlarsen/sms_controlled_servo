
#include <SoftwareSerial.h>
#include <Servo.h>

SoftwareSerial mySerial(7, 8);
Servo servo1; Servo servo2; 

// EN: String buffer for the GPRS shield message
// FR: Mémoire tampon de type string pour les messages du shield GPRS 
String SmsStorePos = String("");
String msg = String("");
String snTmp = String("");
String snFull = String("");
  
int SERVO_ON_POSITION = 180;
int SERVO_OFF_POSITION = 0;
int SERVO_NEUTRAL_POSITION = 90;
    
// EN: Set to 1 when the next GPRS shield message will contains the SMS message
// FR: Est mis à 1 quand le prochain message du shield GPRS contiendra le contenu du SMS
int SmsContentFlag = 0;

// EN: Pin of the LED to turn ON and OFF depending on the received message
// FR: Pin de la LED a allumer/éteindre en fonction du message reçu
int ledPin = 5;
int shield_powered_on = 0;
int SERVO_PIN = 14;

unsigned char simmbuffer[64];
int bindex=0;

void setup()
{ 
  p("booting");
  mySerial.begin(19200);               // the GPRS baud rate   
  mySerial.print("\r");
  delay(1000);
  Serial.begin(19200);                 // the GPRS baud rate


  servo1.attach(SERVO_PIN);

  servo1.write(90);
  delay(500);
  servo1.detach();

  //make shure shield is off

  //delay(10000);

  
  
    //poweroffsim900();
  
    //gms shield  should be ready by now
    //Clear garbage messages

   powerupsim900();

    
    
    DeleteAllMessagesBootup();
    ActivateSleep_mode();

    p("Ready");
}

void loop()
{
    
    char SerialInByte;

   //relay sersial to sim900
    if (Serial.available()){
     mySerial.write(Serial.read());
    }

/*if (mySerial.available()){
  while(mySerial.available()){
    simmbuffer[bindex++]=mySerial.read();
         if(bindex == 64)break;
      }
    Serial.write(simmbuffer,bindex);
    for (int i=0; i<bindex;i++){
    simmbuffer[i]=NULL;
  
  }
  bindex = 0;
}
*/   
    if(Serial.available())
    {
       mySerial.write((unsigned char)Serial.read());
     }  
    else if(mySerial.available())
    {
        char SerialInByte;
        SerialInByte = (unsigned char)mySerial.read();

        // EN: Relay to Arduino IDE Monitor
        // FR: Relayer l'information vers le moniteur Serie Arduino
        Serial.print( SerialInByte );

        // -------------------------------------------------------------------
        // EN: Program also listen to the GPRS shield message.
        // FR: Le programme écoute également les messages issus du GPRS Shield.
        // -------------------------------------------------------------------

        // EN: If the message ends with <CR> then process the message
        // FR: Si le message se termine par un <CR> alors traiter le message 
        if( SerialInByte == 13 ){
          // EN: Store the char into the message buffer
          // FR: Stocké le caractère dans le buffer de message
         // p("debug: going into processgprsmessage"+msg);
          ProcessGprsMsg();
         }
         if( SerialInByte == 10 ){
            // EN: Skip Line feed
            // FR: Ignorer les Line Feed 
         }
         else {
           // EN: store the current character in the message string buffer
           // FR: stocker le caractère dans la mémoire tampon réservé au message
           msg += String(SerialInByte);
         }
     }
     /*else{
       
        if(shield_powered_on == 0){
         
          }
          
      }
       */ 
}

// EN: Make action based on the content of the SMS. 
//     Notice than SMS content is the result of the processing of several GPRS shield messages.
// FR: Execute une action sur base du contenu d'un SMS.
//     Notez que le contenu du SMS est le résultat du traitement de plusieurs messages du shield GPRS.
void ProcessSms( String sms ){
  sms.toLowerCase();
  Serial.print( "ProcessSms for [" );
  
  
  Serial.print( sms );
  Serial.println( "]" );

  if( sms.indexOf("pa") >= 0 ){
    Serial.println("-----Skru på------");
    
    //turnon
    servo1.attach(SERVO_PIN);
    servo1.write(SERVO_ON_POSITION);
    delay(1000);
    servo1.write(SERVO_NEUTRAL_POSITION);
    delay(1000);
    servo1.detach();
   
    //digitalWrite( ledPin, HIGH );
    //Serial.println( "LED IS ON" );
    return;
  }
  if( sms.indexOf("av") >= 0 ){
    servo1.attach(SERVO_PIN);
    servo1.write(SERVO_OFF_POSITION);
    delay(1000);
    servo1.write(SERVO_NEUTRAL_POSITION);
    delay(1000);
    servo1.detach();
    
    Serial.println("-----Skru AV------");
    //digitalWrite( ledPin, LOW );
    //Serial.println( "LED IS OFF" );
    return;
  } else {
   
   
    Serial.println("-----Vet ikke hva jeg skal gjøre------"); 
    
    return;
  }
  
}
// EN: Request Text Mode for SMS messaging
// FR: Demande d'utiliser le mode Text pour la gestion des messages
void GprsTextModeSMS(){
  mySerial.println( "AT+CMGF=1" );
  
}

void GprsReadSmsStore( String SmsStorePos ){
  // Serial.print( "GprsReadSmsStore for storePos " );
  // Serial.println( SmsStorePos ); 
  mySerial.print( "AT+CMGR=" );
  mySerial.println( SmsStorePos );
}

// EN: Clear the GPRS shield message buffer
// FR: efface le contenu de la mémoire tampon des messages du GPRS shield.
void ClearGprsMsg(){
  msg = "";
}

// EN: interpret the GPRS shield message and act appropiately
// FR: interprete le message du GPRS shield et agit en conséquence
void ProcessGprsMsg() {
  p("----------");
  Serial.println("");
  Serial.print( "GPRS Message: [" );
  Serial.print( msg );
  Serial.println( "]" );
  

  if( msg.indexOf( "POWER DOWN" ) >= 0 ){
    powerupsim900();
    }
    
  if( msg.indexOf( "RING" ) >= 0 ){
    Serial.println("-----Skru på------");
    
    //turnon
    servo1.attach(SERVO_PIN);
    servo1.write(SERVO_ON_POSITION);
    delay(1000);
    servo1.write(SERVO_NEUTRAL_POSITION);
    delay(1000);
    servo1.detach();
    mySerial.println("AT+CHUP");
    


    
    }
  if( msg.indexOf( "Call Ready" ) >= 0 ){
     Serial.println( "*** GPRS Shield registered on Mobile Network ***" );
     GprsTextModeSMS();
  }

  // EN: unsolicited message received when getting a SMS message
  // FR: Message non sollicité quand un SMS arrive
  if( msg.indexOf( "+CMTI" ) >= 0 ){
     Serial.println( "*** SMS Received ***" );
     // EN: Look for the coma in the full message (+CMTI: "SM",6)
     //     In the sample, the SMS is stored at position 6
     // FR: Rechercher la position de la virgule dans le message complet (+CMTI: "SM",6) 
     //     Dans l'exemple, le SMS est stocké à la position 6
     int iPos = msg.indexOf( "," );
     SmsStorePos = msg.substring( iPos+1 );
     Serial.print( "SMS stored at " );
     Serial.println( SmsStorePos );

     // EN: Ask to read the SMS store
     // FR: Demande de lecture du stockage SMS
     GprsReadSmsStore( SmsStorePos );
  }

  // EN: SMS store readed through UART (result of GprsReadSmsStore request)  
  // FR: Lecture du stockage SMS via l'UART (résultat de la requete GprsReadSmsStore)
  if( msg.indexOf( "+CMGR:" ) >= 0 ){
    // get number of sender
    int snPos = msg.indexOf("+1");
    Serial.print("SMS From: ");
    snTmp = msg.substring(snPos+1);
    snFull = "";
    for (int i = 0; i < 11; i++){
      snFull += snTmp[i];    
    }
    Serial.println(snFull);

    // EN: Next message will contains the BODY of SMS
    // FR: Le prochain message contiendra le contenu du SMS
    SmsContentFlag = 1;
    // EN: Following lines are essentiel to not clear the flag!
    // FR: Les ligne suivantes sont essentielle pour ne pas effacer le flag!
    ClearGprsMsg();
    return;
  }

  // EN: +CMGR message just before indicate that the following GRPS Shield message 
  //     (this message) will contains the SMS body
  // FR: le message +CMGR précédent indiquait que le message suivant du Shield GPRS 
  //     (ce message) contient le corps du SMS 
  if( SmsContentFlag == 1 ){
    Serial.println( "*** SMS MESSAGE CONTENT ***" );
    Serial.println( msg );
    Serial.println( "*** END OF SMS MESSAGE ***" );
    ProcessSms( msg );
    delSMS();
  }

  ClearGprsMsg();
  // EN: Always clear the flag
  // FR: Toujours mettre le flag à 0
  SmsContentFlag = 0; 
  p("----------");
  p("----------");
}
void delSMS() {
  mySerial.print("AT+CMGD=");
  mySerial.println(SmsStorePos);
}  

void DeleteAllMessagesBootup(){
  
  delay(7000);
  mySerial.println("AT");
  delay(1000);
  mySerial.println("AT+CMGD=1,4");
  delay(1000);
  
 }

void ActivateSleep_mode(){
    //alternative from http://forum.arduino.cc/index.php?topic=106721.0 and https://www.soselectronic.cz/productdata/13/78/09/137809/SIM900R.pdf
    // activate sleep mode
    mySerial.println("AT+CSCLK=2"); //2:Module decides itself if go to sleep mode if no data on serial port.
    delay(500);
    //tipper denne kan lage en bug?
   
}
void poweroffsim900(){
//make shure shield is ready to recive
 p("powering shield off");
 
  mySerial.println("AT");
  delay(1000);
  
  mySerial.println("AT+CPOWD=1");
  delay(500);
}

void powerupsim900(){
    p("powering shield on");
           pinMode(9, OUTPUT); 
         digitalWrite(9,LOW);
         delay(1000);
         digitalWrite(9,HIGH);
         delay(2000);
         digitalWrite(9,LOW);
         
  }
 void p(String string){
  Serial.println(string);
  }
