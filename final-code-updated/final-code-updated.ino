#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <SoftwareSerial.h>
//#include <RTClib.h>

#define ldrtopL A2
#define ldrtopR A3
#define ldrbotL A7
#define ldrbotR A6

#define r1 7 //  motor Y
#define r2 12 //
#define r3 7  // motor X
#define r4 8  //
#define r5 5  // cut-off relay

#define rightlimit 260
#define leftlimit 0
#define uplimit 260
#define botlimit 0

#define deg 2  // degree factor

SoftwareSerial mySerial(4, 3); // 4-> rx ,  3-> tx
LiquidCrystal_I2C lcd(0x27, 20, 4);
//RTC_DS1307 RTC;

String server = "184.106.153.149";
String writeurl = "/update?api_key=2L17KW0KYAVIFQPB&";
String dataPost;
int countTrueCommand;
int countTimeCommand;
boolean found = false;

const int currentPin = 0;
const int voltageSensorsolar = 1;
unsigned int topL = 0, topR = 0, botL = 0, botR = 0;
int diffelev = 0, diffazi = 0;

int  hor = 18, mIn = 0, Hor = 0, Min = 0;

// Cloud Parameters
double batteryvoltage = 0.0 , panelvoltage = 0.0, batterycurrent = 0.0, panelpower = 0.0;
int xdeg = 0 , ydeg = 43, charginglevel = 0;
boolean chargingstatus = 1;


unsigned long previousMillis = 0;
String cstatus = " ", str = " ";
unsigned long previoustime = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("Initializing .... ");
  mySerial.begin(115200);
    resets();
  Wire.begin();
//  RTC.begin();
//  if (! RTC.isrunning())
//  {
//    RTC.adjust(DateTime(__DATE__, __TIME__));
//  }
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" SMART    SOLAR ");
  lcd.setCursor(0, 1);
  lcd.print("TRACKER PROJECT");
  lcd.setCursor(0, 2);
  lcd.print("LOADING...");
  pinMode(r1, OUTPUT);
  pinMode(r2, OUTPUT);
  pinMode(r3 , OUTPUT);
  pinMode(r4, OUTPUT);
  pinMode(r5, OUTPUT);
  pinMode(ldrtopR, INPUT);
  pinMode(ldrtopL, INPUT);
  pinMode(ldrbotL, INPUT);
  pinMode(ldrbotR, INPUT);
  pinMode(currentPin , INPUT);
  pinMode(voltageSensorsolar , INPUT);
  stopall();
//  DateTime now = RTC.now();
//    Hor = now.hour(), DEC;
//    Serial.println(Hor);
//      panel_right();
//      digitalWrite(10,LOW);
//    delay(3000);
//  
//    stopall();
//  //  wifi();

}

unsigned long prevtime = millis() , prtime = 0;
int angle = 0;



void loop() {

//  if (timer(5 * 60 * 1000)) {
//    DateTime now = RTC.now();
//    Hor = now.hour(), DEC;
//    Min = now.minute(), DEC;
//    TimeCheck();
//  }

  if (millis() - previoustime >= 5000) {
    previoustime = millis();
    batteryvoltage = Voltage(voltageSensorsolar) - 1.0;
    panelvoltage = Voltage(voltageSensorsolar);
    batterycurrent = Current(currentPin);
    panelpower = batterycurrent * panelvoltage;
    chargelevel();
    if(Hor > 19){ panelvoltage = 5.0;}
    if (batterycurrent > 0.25 || batteryvoltage < 11.0) {
      cstatus = "Charging";
      chargingstatus = 1;
      digitalWrite(r5, LOW);
    }
    else {
      cstatus = "NotConnect"; chargingstatus = 0; digitalWrite(r5, HIGH);
    }
    LCDupdate();
          wifi();
    //    delay(500);
  }

  solar_tracker();
//       delay(1000);
}

// *************************************************************************************************************************************


boolean timer(int t) {
  if (millis() - previoustime >= t) {
    previoustime = millis();
    return true;
  }
}

void test() {
  digitalWrite(13, HIGH);
  delay(100);
  digitalWrite(13, LOW);
  delay(100);
}

void solar_tracker() {

  topL = analogRead(ldrtopL);
  topR = analogRead(ldrtopR);
  botL = analogRead(ldrbotL);
  botR = analogRead(ldrbotR);

  // Calculating Averages
  // Y-axis movement
  unsigned int avgtop = (topL + topR) / 2; //average of top LDRs
  unsigned int avgbot = (botL + botR) / 2; //average of bottom LDRs
  // X-axis movement
  unsigned int avgleft = (topL + botL) / 2; //average of left LDRs
  unsigned int avgright = (topR + botR) / 2; //average of right LDRs

  diffelev = avgtop - avgbot;     //Get the different average betwen LDRs top and LDRs bot
  diffazi = avgright - avgleft;    //Get the different average betwen LDRs right and LDRs left

//        Serial.print("TopLeft : " + String(topL) + " \t");
//        Serial.print("TopRight : " + String(topR) + " \t");
//        Serial.print("BotLeft : " + String(botL) + " \t");
//        Serial.print("BotRight : " + String(botR) + " \t");
//        Serial.println();
//        Serial.print("Top : " + String(avgtop) + " \t");
//        Serial.print("Bottom : " + String(avgbot) + " \t");
//        Serial.print("Left : " + String(avgleft) + " \t");
//        Serial.print("Right : " + String(avgright) + " \t");
//        Serial.println();
  Serial.print("Y-axis : " + String(diffelev) + " \t");
  Serial.print("X-axis : " + String(diffazi) + " \t");
//  Serial.println();
      delay(100);


  //left-right movement of solar tracker
//  if (abs(diffazi) > 25) {
    if (diffazi > rightlimit) {
      Serial.println("Moving in Right Direction");
      panel_right();
    }
    else if (diffazi <  leftlimit) {
      Serial.println("Moving in Left Direction");
      panel_left();
    }
    else {
      stopall_x();
    }
//  }

  //up-down movement of solar tracker
//  if (abs(diffelev) > 25) {
//    if (diffelev < 0) {
//      Serial.println("Moving in Down Direction");
//      panel_down();
//
//    }
//    else if (diffelev >  260) {
//      Serial.println("Moving in Upward Direction");
//      panel_up();
//    }
//    else {
//      stopall_y();
//    }
//  }

}

unsigned long prev = 0;
void degrotate(int x , int y) {

}

//long innn = 0;
//void TimeCheck()
//{
//  if (hor == Hor && mIn == Min)
//  {
//    lcd.clear();
//    lcd.print("Alarm is Achieved");
//    digitalWrite(r1, LOW);
//    digitalWrite(r2, HIGH);
//    if (millis() - innn >= 3000) {   // 1deg = 360;
//      innn = millis();
//    }
//  }
//}

void stopall_x() {
  digitalWrite(r3, HIGH);
  digitalWrite(r4, HIGH);
}

void stopall_y() {
  digitalWrite(r1, HIGH);
  digitalWrite(r2, HIGH);
}

// solar panel moves with the help of LDRs connected to another nano
void panel_left() {
  //   digitalWrite(r1, HIGH);
  //  digitalWrite(r2, HIGH);
  // motor X
  digitalWrite(r3, LOW);
  digitalWrite(r4, HIGH);
  if (millis() - prevtime >= 360) {   // 1deg = 360;
    prevtime = millis();
    xdeg = xdeg - deg;
  }
}
void panel_right() {
  //   digitalWrite(r1, HIGH);
  //  digitalWrite(r2, HIGH);
  // motor X
  digitalWrite(r3, HIGH);
  digitalWrite(r4, LOW);
  if (millis() - prevtime >= 460) {   // 1deg = 360;
    prevtime = millis();
    xdeg = xdeg + deg;
  }
}

void panel_up() {
  //  digitalWrite(r3, HIGH);
  //  digitalWrite(r4, HIGH);
  // motor Y
  digitalWrite(r1, LOW);
  digitalWrite(r2, HIGH);
  if (millis() - prevtime >= 360) {   // 1deg = 360;
    prevtime = millis();
    ydeg = ydeg + deg;
  }
}

void panel_down() {
  //  digitalWrite(r3, HIGH);
  //  digitalWrite(r4, HIGH);
  // motor Y
  digitalWrite(r1, HIGH);
  digitalWrite(r2, LOW);
  if (millis() - prevtime >= 460) {   // 1deg = 360;
    prevtime = millis();
    ydeg = ydeg - deg;
  }
}

boolean i = 0;
void LCDupdate() {
  if (i == 0) {
    i = 1;
    lcd.setCursor(0, 0);
    lcd.print("Panel Parameters");
    lcd.setCursor(0, 3);
    lcd.print("V=");
    lcd.print(panelvoltage, 1);
    lcd.print("v ");
    lcd.print("P=");
    lcd.print(panelpower, 1);
    lcd.print("W   ");

    lcd.setCursor(0, 2);
    lcd.print("X Deg=");
    lcd.print(xdeg);
    lcd.print("   ");
    lcd.print("Y Deg=");
    lcd.print(ydeg);
    lcd.print("      ");
    lcd.setCursor(0, 1);
    lcd.print("                ");
  }
  else if (i == 1) {
    i = 0;
    lcd.setCursor(0, 0);
    lcd.print("Bat Parameters");
    lcd.setCursor(0, 1);
    lcd.print("V=");
    lcd.print(batteryvoltage, 1);
    lcd.print("v ");
    lcd.print("   I=");
    lcd.print(batterycurrent, 1);
    lcd.print("A ");

    lcd.setCursor(0, 2);
    lcd.print("Charging=");
    lcd.print(cstatus);
    lcd.print(" ");
    lcd.setCursor(0, 3);
    lcd.print("Level=");
    lcd.print(charginglevel);
    lcd.print("% ");
  }

}
void chargelevel() {
  if (batteryvoltage > 12.3) {
    charginglevel = 100;
  }
  else if (batteryvoltage >= 12.0 && batteryvoltage < 12.1) {
    charginglevel = 90;
  }
  else if (batteryvoltage >= 11.0 && batteryvoltage < 11.9) {
    charginglevel = 80;
  }
  else if (batteryvoltage >= 10.0 && batteryvoltage < 10.9) {
    charginglevel = 60;
  }
  else if (batteryvoltage >= 9.0 && batteryvoltage < 9.9) {
    charginglevel = 40;
  }
  else if (batteryvoltage >= 8.0 && batteryvoltage < 8.9) {
    charginglevel = 20;
  }
  else if (batteryvoltage < 6.0) {
    charginglevel = 0;
  }
}

double Voltage(int pin)
{
  uint16_t Raw = 0;
  double Volt;
  for (int i = 0; i < 20; i++)
  {
    Raw = (Raw + analogRead(pin));
    delay(1);
  }
  Raw = Raw / 20;
  Volt = ((Raw / 4.075) / 10);
  return Volt;
}

double Current(int pin)
{
  double Voltage = 0;
  double Current = 0;
  // Voltage is Sensed 1000 Times for precision
  for (int i = 0; i < 25; i++)
  {
    Voltage = (Voltage + (.0049 * analogRead(pin))); // (5 V / 1024 (Analog) = 0.0049) which converter Measured analog input voltage to 5 V Range
    delay(2);
  }
  Voltage = ( Voltage / 25);
  Current = ((Voltage - 2.4) / 0.200); // Sensed voltage is converter to current
  return Current;
}

//float floatMap(float x, float in_min, float in_max, float out_min, float out_max) {
//  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
//}

void stopall() {
  digitalWrite(r1, HIGH);
  digitalWrite(r2, HIGH);
  digitalWrite(r3, HIGH);
  digitalWrite(r4, HIGH);
//  digitalWrite(r5, HIGH);
}

//void espcheck() {
//  dataPost = "field1=" + String(temp) + "&field2=" + String(hum) + "&field3=" + String(spo2) + "&field4=" + String(heartrate) + "&field5=" + String(modes);
//  httppost();
//  Serial.println("Post 1");
//  delay(50);
//}


void wifi() {
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += "184.106.153.149"; // api.thingspeak.com connection IP
  cmd += "\",80";           // api.thingspeak.com connection port, 80
  mySerial.println(cmd);

  if (mySerial.find("Error")) {
    Serial.println("AT+CIPSTART error");
    return;
  }
  Serial.println("Starting..");
  // Set String, Data to send by GET method
  String getStr = "GET /update?api_key=Q252WGU80F874GWA";
  //  getStr += apiKey;

  getStr += "&field1=";
  getStr += String(panelvoltage);
  getStr += "&field2=";
  getStr += String(panelpower);
  getStr += "&field3=";
  getStr += String(xdeg);
  getStr += "&field4=";
  getStr += String(ydeg);
  getStr += "&field5=";
  getStr += String(batteryvoltage);
  getStr += "&field6=";
  getStr += String(batterycurrent);
  getStr += "&field7=";
  getStr += String(chargingstatus);
  getStr += "&field8=";
  getStr += String(charginglevel);
  getStr += "\r\n\r\n";

  // Send Data
  cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  mySerial.println(cmd);

  if (mySerial.find(">")) {
    mySerial.print(getStr);
  }
  else {
    mySerial.println("AT+CIPCLOSE");
    // alert user
    Serial.println("AT+CIPCLOSE");
  }
}

void resets()
{
  sendCommand("AT", 50, "OK");
  sendCommand("AT+RST", 50, "OK");
  sendCommand("AT+CWMODE=1", 50, "OK");
  sendCommand("AT+CWJAP=\"solar\",\"12345678\"\r\n", 1000, "OK");
}

void sendCommand(String command, int maxTime, char readReplay[]) {
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while (countTimeCommand < (maxTime * 1))
  {
    mySerial.println(command);//at+cipsend
    if (mySerial.find(readReplay)) //ok
    {
      found = true;
      break;
    }

    countTimeCommand++;
  }

  if (found == true)
  {
    Serial.println("OK");
    countTrueCommand++;
    countTimeCommand = 0;
  }

  if (found == false)
  {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }

  found = false;
}

//void httppost () {
//  Serial3.println("AT+CIPSTART=\"TCP\",\"" + server + "\",80");//start a TCP connection.
//  delay(50);
//  if ( Serial3.find("OK"))
//  {
//    Serial.println(F("TCP connection ready"));
//  }
//  delay(50);
//  String postRequest =
//
//    "POST " + writeurl + " HTTP/1.0\r\n" +
//
//    "Host: " + server + "\r\n" +
//
//    "Accept: *" + "/" + "*\r\n" +
//
//    "Content-Length: " + String(dataPost.length()) + "\r\n" +
//
//    "Content-Type: application/x-www-form-urlencoded\r\n" +
//
//    "\r\n" + dataPost;
//
//  String sendCmd = "AT+CIPSEND=";//determine the number of caracters to be sent.
//  Serial3.print(sendCmd);
//  Serial3.println(postRequest.length());
//  delay(50);
//  Serial.println(dataPost);
//  if (Serial3.find(">")) {
//    Serial.println("Sending..");
//    Serial3.print(postRequest);
//    Serial.println(postRequest);
//    delay(50);
//    if ( Serial3.find("SEND OK")) {
//      Serial.println(F("Packet sent"));
//      delay(50);
//      while (Serial3.available()) {
//        String tmpRSerial3 = Serial3.readString();
//        Serial.println(tmpRSerial3);
//      }
//      delay(50);
//      // close the connection
//      Serial3.println(F("AT+CIPCLOSE"));
//    }
//  }
//}
//}

//String tp = " ", bt = " ", ri = " ", li = " ", flag = " ";
//void nano_receive() {
//  //    mySerial.end();
//  //    nano.begin(9600);
//  if (nano.available() > 0) {
//    str = nano.readStringUntil('$');
//    flag = str.substring(0, 1);
//
//    if (flag == "<") {
//      tp = str.substring(1, 2);
//      delay(10);
//      bt = str.substring(3, 4);
//      delay(10);
//      li = str.substring(5, 6);
//      delay(10);
//      ri = str.substring(7);
//      delay(10);
//      Serial.println("From Nano : " + str);
//      top = tp.toInt();
//      bot = bt.toInt();
//      left = li.toInt();
//      right = ri.toInt();
//      Serial.print( String(top) + "    " + String(bot));
//      Serial.println( "  "+String(left) + "    " + String(right));
//    }
//  }
//    nano.end();
//  delay(100);

//    mySerial.begin(115200);
//}
