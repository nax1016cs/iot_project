/*
 * (c)20016-18 Pawel A. Hernik
 * 
  ESP-01 pinout from top:

  GND    GP2 GP0 RX/GP3
  TX/GP1 CH  RST VCC

  MAX7219 5 wires to ESP-1 from pin side:
  Re Br Or Ye
  Gr -- -- --
  capacitor between VCC and GND
  resistor 1k between CH and VCC

  USB to Serial programming
  ESP-1 from pin side:
  FF(GP0) to GND, RR(CH_PD) to GND before upload
  Gr FF -- Bl
  Wh -- RR Vi

  ------------------------
  NodeMCU 1.0/D1 mini pinout:

  D8 - DataIn
  D7 - LOAD/CS
  D6 - CLK

*/

#include <ESP8266WiFi.h>
#include <fstream>
#include <FS.h>
#include <string.h>

#define NUM_MAX 8
#define LINE_WIDTH 32
#define ROTATE  90

// for ESP-01 module
//#define DIN_PIN 2 // D4
//#define CS_PIN  3 // D9/RX
//#define CLK_PIN 0 // D3

// for NodeMCU 1.0/D1 mini
#define DIN_PIN 15  // D8
#define CS_PIN  13  // D7
#define CLK_PIN 12  // D6

#define DEBUG(x)
//#define DEBUG(x) x

#include "max7219.h"
#include "fonts.h"

#define UP_TREE ';' 
#define PERSON '?' 
#define UP_TREE ';' 
#define LOW_TREE '@' 
#define LEFT_HEART '#' 
#define MID_HEART '$' 
#define RIGHT_HEART '%' 
#define PERSON_JUMP '=' 
#define PERSON_LOW_TREE_SIDE '(' 
#define PERSON_LOW_TREE_MID  ')' 
#define PERSON_UP_TREE_SIDE  '*' 
#define PERSON_UP_TREE_MID   '+' 



// =======================================================================
// Your config below!
// =======================================================================
const char* ssid     = "221";     // SSID of local network
const char* password = "123456789";   // Password on network
long utcOffset = 1;                    // UTC for Warsaw,Poland
// =======================================================================

int h, m, s, day, month, year, dayOfWeek;
int summerTime = 0; // calculated in code from date
long localEpoc = 0;
long localMillisAtUpdate = 0;
String date;
String buf="";

int xPos=0, yPos=0;
int clockOnly = 0;

// tree setting
int score = 0;
int temp = 0;
int speed_ = 200;
bool change_time = true;
int interval;
const int tree_num = 5;
int tree_index = 0;
int person_idx = 0;
//int tree [tree_num] = {-1, -1, -1, -1, -1};
std::pair <char,int> tree[tree_num];
int prev_ = 0;

String time_;
String stock;
String msg; // msg from website

//for print string
int firstPos = 0;

char* monthNames[] = {"JAN","FEB","MAR","APR","MAY","JUN","JUL","AUG","SEP","OCT","NOV","DEC"};
char txt[30];

void setup() 
{
  buf.reserve(500);
  Serial.begin(115200);
  initMAX7219();
  sendCmdAll(CMD_SHUTDOWN, 1);
  sendCmdAll(CMD_INTENSITY, 0);
//  DEBUG(Serial.print("Connecting to WiFi ");)
  WiFi.begin(ssid, password);
  clr();
  xPos=0;
  printString("CONNECT..", font3x7);
  refreshAll();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
//    DEBUG(Serial.print("."));
  }
  clr();
  xPos=0;
//  DEBUG(Serial.println(""); Serial.print("MyIP: "); Serial.println(WiFi.localIP());)
  printString((WiFi.localIP().toString()).c_str(), font3x7);
  refreshAll();
  getTime();
  reset();
//  SPIFFS.begin();
}


void drawScore()
{
  yPos = 0;
  xPos = (score/10 < 10) ? 4 : 0;
  sprintf(txt,"%d",score/10);
  printString(txt, font3x7);
}

void reset(){
  tree_index = 1;
  person_idx = 0;
  change_time = true;
  speed_ = 300;
  for (int i=0; i<tree_num; i++){
    tree[i] = std::make_pair(-1, 0);
  }
  char tree_type = (rand() % 2 == 0 ) ? 59 :64;
  tree[0] = std::make_pair(8,tree_type );
  temp = 0;
  score = 0;
}

void create_tree(){
    srand( time(NULL) );
    char temp = (rand() % 2 == 0 ) ? 59 :64;
    tree[tree_index++] = std::make_pair(8, temp);
    tree_index %= tree_num;
}
void play_game(){
//  File f = SPIFFS.open("/t.txt", "r");
//  if (!f) printf("failed to open the file");
//  while(f.available()){
//    Serial.write(f.read());
//  }
//  f.close();
  
  temp ++;
  score++;
  srand( time(NULL) );
  drawScore();
  if (change_time == true){
    interval = int(rand() % 20 + 10);
    change_time = false;
  }
  
  if(temp % interval == 0) {
    change_time = true;
    create_tree();
    temp = 0;
  }
  char person = (digitalRead(D1) == HIGH )? PERSON_JUMP: PERSON;
  printCharX(person,font3x7, 60);
  
  for (int i=0; i<tree_num; i++){
    if ( 60 <= tree[i].first && tree[i].first <= 62 ){
      if ( (person == PERSON && tree[i].second == LOW_TREE)|| (person == PERSON_JUMP && tree[i].second == UP_TREE )){
        delay(3000);
        reset();
      }
    }
    // low tree with side person
    if ( (tree[i].first == 60 || tree[i].first ==  62)  ) {
     if(tree[i].second == LOW_TREE) printChar(PERSON_LOW_TREE_SIDE, font3x7, i);
     else if(tree[i].second == UP_TREE) printChar(PERSON_UP_TREE_SIDE, font3x7, i);
    }
    // low tree with middle person
    else if ( (tree[i].first  == 61)  )  {
      if(tree[i].second == LOW_TREE) printChar(PERSON_LOW_TREE_MID, font3x7, i);
      else if(tree[i].second == UP_TREE) printChar(PERSON_UP_TREE_MID, font3x7, i);
    }
    //only low tree
    else if (tree[i].first != -1 ) {
      if(tree[i].second == LOW_TREE) printChar(LOW_TREE, font3x7, i);
      else if(tree[i].second == UP_TREE) printChar(UP_TREE, font3x7, i);
    }
    
  }
  Serial.println(score);
  refreshAll();
  clr_game();
  person_idx --;
  if (speed_ > 100) speed_--;
  delay(speed_);

}

int decode(int first, int second, int third){
  return third*4  +second*2 +  first;
}
// =======================================================================

void print_NCTU(){
  if(firstPos>NUM_MAX*8)firstPos = 0;
  MyprintChar('N', font3x7);
  int temp_pos = firstPos -3;
  MyprintChar('C', font3x7);
  MyprintChar('T', font3x7);
  MyprintChar('U', font3x7);
  firstPos = temp_pos;
}

void print_iot(){
  if(firstPos>NUM_MAX*8)firstPos = 0;
  MyprintChar('I', font3x7);
  int temp_pos = firstPos -1;
  MyprintChar('#', font3x7);
  firstPos -= 1;
  MyprintChar('$', font3x7);
  firstPos -= 1;
  MyprintChar('%', font3x7);
  MyprintChar('I', font3x7);
  MyprintChar('O', font3x7);
  MyprintChar('T', font3x7);
  firstPos = temp_pos;  
}

void print_msg(String msg){
  if(firstPos>NUM_MAX*8)firstPos = 0;
//  Serial.print("Got message: ");
//  Serial.print(msg);
//  printf("%s", msg);
  MyprintChar(msg[1], font3x7);
  int temp_pos = firstPos -2;
  for (int i = 2; i<msg.length(); i++){
    MyprintChar(msg[i], font3x7);
  }
  firstPos = temp_pos;  
}

//void print_stock(String stock){
//  if(firstPos>NUM_MAX*8)firstPos = 0;
//  MyprintChar(stock[0], font3x7);
//  int temp_pos = firstPos -1;
//  MyprintChar('#', font3x7);
//  firstPos -= 1;
//  MyprintChar('$', font3x7);
//  firstPos -= 1;
//  MyprintChar('%', font3x7);
//  MyprintChar('I', font3x7);
//  MyprintChar('O', font3x7);
//  MyprintChar('T', font3x7);
//  firstPos = temp_pos;  
//}

unsigned int curTime,updTime=0;
int dots,mode;
int ct = 0;
void loop()
{
//  ct++;
//  if (ct==100){
//    getTime();
//    ct = 0;
//  }

  int first = (digitalRead(D2) == HIGH) ? 1:0;
  int second = (digitalRead(D4) == HIGH) ? 1:0;
  int third = ( digitalRead(D5)== HIGH) ? 1:0;
  int number = 0;
  prev_ = number;
  number = decode(first,second ,third );
//  printf("first: %d, second: %d, third %d, number: %d\n", first, second, third, number);
  if (prev_ != number){
    reset();
    clr();
  }
  curTime = millis();
  if(curTime-updTime>600000) {
    updTime = curTime;
    getTime();  // update time every 600s=10m
  }
  updateTime();
  //add
//  getTime();
  
  
  switch(number){
    case 0:
      play_game();
      break;
    case 1:
      dots = (curTime % 1000)<500;
      drawTime0();
      refreshAll();
      delay(100);
      break;
    case 2:
      dots = (curTime % 1000)<500;
      drawTime1();
      refreshAll();
      delay(100);
      break;
    case 3:
      dots = (curTime % 1000)<500;
      drawTime2();
      refreshAll();
      delay(100);
      break;
    case 4:
      print_NCTU();
      refreshAll();
      delay(200);
      break;
    case 5:
      print_iot();
      refreshAll();
      delay(200);
      break;
    case 6:
      getTime();
      print_msg(stock);
      refreshAll();
      delay(100);
      break;
    case 7:
      getTime();
      print_msg(msg);
      refreshAll();
      delay(100);
      break;  
  }
  
}

// =======================================================================

//char* monthNames[] = {"STY","LUT","MAR","KWI","MAJ","CZE","LIP","SIE","WRZ","PAZ","LIS","GRU"};


void drawTime0()
{
  clr();
  yPos = 0;
  xPos = (h>9) ? 0 : 2;
  sprintf(txt,"%d",h);
  printString(txt, digits7x16);
  if(dots) printCharX(':', digits5x16rn, xPos);
  xPos+=(h>=22 || h==20)?1:2;
  sprintf(txt,"%02d",m);
  printString(txt, digits7x16);
}

void drawTime1()
{
  clr();
  yPos = 0;
  xPos = (h>9) ? 0 : 3;
  sprintf(txt,"%d",h);
  printString(txt, digits5x16rn);
  if(dots) printCharX(':', digits5x16rn, xPos);
  xPos+=(h>=22 || h==20)?1:2;
  sprintf(txt,"%02d",m);
  printString(txt, digits5x16rn);
  sprintf(txt,"%02d",s);
  printString(txt, font3x7);
}

void drawTime2()
{
  clr();
  yPos = 0;
  xPos = (h>9) ? 0 : 2;
  sprintf(txt,"%d",h);
  printString(txt, digits5x8rn);
  if(dots) printCharX(':', digits5x8rn, xPos);
  xPos+=(h>=22 || h==20)?1:2;
  sprintf(txt,"%02d",m);
  printString(txt, digits5x8rn);
  sprintf(txt,"%02d",s);
  printString(txt, digits3x5);
  yPos = 1;
  xPos = 1;
  sprintf(txt,"%d&%s&%d",day,monthNames[month-1],year-2000);
  printString(txt, font3x7);
  for(int i=0;i<LINE_WIDTH;i++) scr[LINE_WIDTH+i]<<=1;
}

// =======================================================================

int charWidth(char c, const uint8_t *font)
{
  int fwd = pgm_read_byte(font);
  int fht = pgm_read_byte(font+1);
  int offs = pgm_read_byte(font+2);
  int last = pgm_read_byte(font+3);
  if(c<offs || c>last) return 0;
  c -= offs;
  int len = pgm_read_byte(font+4);
  return pgm_read_byte(font + 5 + c * len);
}

// =======================================================================

int stringWidth(const char *s, const uint8_t *font)
{
  int wd=0;
  while(*s) wd += 1+charWidth(*s++, font);
  return wd-1;
}

// =======================================================================

int stringWidth(String str, const uint8_t *font)
{
  return stringWidth(str.c_str(), font);
}

// =======================================================================

int printCharX(char ch, const uint8_t *font, int x)
{
  int fwd = pgm_read_byte(font);
  int fht = pgm_read_byte(font+1);
  int offs = pgm_read_byte(font+2);
  int last = pgm_read_byte(font+3);
  if(ch<offs || ch>last) return 0;
  ch -= offs;
  int fht8 = (fht+7)/8;
  font+=4+ch*(fht8*fwd+1);
  int j,i,w = pgm_read_byte(font);
  for(j = 0; j < fht8; j++) {
    for(i = 0; i < w; i++) scr[x+LINE_WIDTH*(j+yPos)+i] = pgm_read_byte(font+1+fht8*i+j);
    if(x+i<LINE_WIDTH) scr[x+LINE_WIDTH*(j+yPos)+i]=0;
  }
  return w;
}

// =======================================================================

void printChar(unsigned char c, const uint8_t *font)
{
  if(xPos>NUM_MAX*8) return;
  int w = printCharX(c, font, xPos);
  xPos+=w+1;
}

void MyprintChar(unsigned char c, const uint8_t *font)
{
  if(xPos>NUM_MAX*8) return;
  int w = printCharX(c, font, firstPos);
  firstPos += w+1;
}

void printChar(unsigned char c, const uint8_t *font, unsigned int tree_idx)
{
  if(tree[tree_idx].first>NUM_MAX*8) return;
  int w = printCharX(c, font, tree[tree_idx].first);
  tree[tree_idx].first += 1;
}

// =======================================================================

void printString(const char *s, const uint8_t *font)
{
  while(*s) printChar(*s++, font);
  //refreshAll();
}

void printString(String str, const uint8_t *font)
{
  printString(str.c_str(), font);
}

// =======================================================================
  

void getTime()
{
  WiFiClient client;
//  DEBUG(Serial.print("connecting to www.google.com ...");)
  if(!client.connect("140.113.66.153", 80)) {
//    DEBUG(Serial.println("connection failed.....");)
    return;
  }
  client.print(String("GET / HTTP/1.1\r\n") +
               String("Host: 140.113.66.153\r\n") +
               String("Connection: close\r\n\r\n"));

  int repeatCounter = 10;
  /*while (!client.available() && repeatCounter--) {
    delay(200); DEBUG(Serial.println("y."));
  }*/

  String line;
  client.setNoDelay(false);
  int dateFound = 0;
  while(client.connected() /*&& client.available()*/ && !dateFound) {
    line = client.readStringUntil('\n');
//    line.toUpperCase();
//    Serial.println(line);

    if(line.startsWith("$")){
//      Serial.println(line);
      //time
       time_= line; 
    }
    else if(line.startsWith("@")){
//      Serial.println(line);
      //string
       msg = line;
    }
    else if(line.startsWith("#")){
//      Serial.println(line);
      //string
       stock = line;
    }
    

//     Date: Thu, 19 Nov 2015 20:25:40 GMT
    if(line.startsWith("AATE: ")) {
      localMillisAtUpdate = millis();
      dateFound = 1;
      date = line.substring(6, 22);
      date.toUpperCase();
      decodeDate(date);
//      Serial.println(line);
      h = line.substring(23, 25).toInt();
      m = line.substring(26, 28).toInt();
      s = line.substring(29, 31).toInt();
      summerTime = checkSummerTime();
      if(h+utcOffset+summerTime>23) {
        if(++day>31) { day=1; month++; };  // needs better patch
        if(++dayOfWeek>7) dayOfWeek=1; 
      }
//      DEBUG(Serial.println(String(h) + ":" + String(m) + ":" + String(s)+"   Date: "+day+"."+month+"."+year+" ["+dayOfWeek+"] "+(utcOffset+summerTime)+"h");)
      localEpoc = h * 60 * 60 + m * 60 + s;
    }
    
  }
//  client.stop();
}

// =======================================================================
// MON, TUE, WED, THU, FRI, SAT, SUN
// JAN FEB MAR APR MAY JUN JUL AUG SEP OCT NOV DEC
// Thu, 19 Nov 2015
// decodes: day, month(1..12), dayOfWeek(1-Mon,7-Sun), year
void decodeDate(String date)
{
  switch(date.charAt(0)) {
    case 'M': dayOfWeek=1; break;
    case 'T': dayOfWeek=(date.charAt(1)=='U')?2:4; break;
    case 'W': dayOfWeek=3; break;
    case 'F': dayOfWeek=5; break;
    case 'S': dayOfWeek=(date.charAt(1)=='A')?6:7; break;
  }
  int midx = 6;
  if(isdigit(date.charAt(midx))) midx++;
  midx++;
  switch(date.charAt(midx)) {
    case 'F': month = 2; break;
    case 'M': month = (date.charAt(midx+2)=='R') ? 3 : 5; break;
    case 'A': month = (date.charAt(midx+1)=='P') ? 4 : 8; break;
    case 'J': month = (date.charAt(midx+1)=='A') ? 1 : ((date.charAt(midx+2)=='N') ? 6 : 7); break;
    case 'S': month = 9; break;
    case 'O': month = 10; break;
    case 'N': month = 11; break;
    case 'D': month = 12; break;
  }
  day = date.substring(5, midx-1).toInt();
  year = date.substring(midx+4, midx+9).toInt();
  return;
}

// =======================================================================
// https://en.wikipedia.org/wiki/Summer_Time_in_Europe
// from
// Sunday (31 − ((((5 × y) ÷ 4) + 4) mod 7)) March at 01:00 UTC
// to
// Sunday (31 − ((((5 × y) ÷ 4) + 1) mod 7)) October at 01:00 UTC

int checkSummerTime()
{
  if(month>3 && month<10) return 1;
  if(month==3 && day>=31-(((5*year/4)+4)%7) ) return 1;
  if(month==10 && day<31-(((5*year/4)+1)%7) ) return 1;
  return 0;
}
// =======================================================================

void updateTime()
{
  long curEpoch = localEpoc + ((millis() - localMillisAtUpdate) / 1000);
  long epoch = (curEpoch + 3600 * (utcOffset+summerTime) + 86400L) % 86400L;
  h = (((epoch  % 86400L) / 3600) +6 ) % 24;
  m = (epoch % 3600) / 60;
  s = epoch % 60;
}

// =======================================================================
