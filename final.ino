#include <Adafruit_NeoPixel.h> //Neopixel函式庫
#include "SeeedGroveMP3.h" //Grove_MP3函式庫
#include "KT403A_Player.h" //Grove_MP3音樂撥放晶片函式庫

//Grove_MP3函式庫設定
#ifdef __AVR__
  #include <avr/power.h>
#endif

#ifdef __AVR__
    #include <SoftwareSerial.h>
    SoftwareSerial SSerial(12, 13); // RX, TX
    #define COMSerial SSerial
    #define ShowSerial Serial

    KT403A<SoftwareSerial> Mp3Player;
#endif

#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
    #define COMSerial Serial1
    #define ShowSerial SerialUSB

    KT403A<Uart> Mp3Player;
#endif

#ifdef ARDUINO_ARCH_STM32F4
    #define COMSerial Serial
    #define ShowSerial SerialUSB

    KT403A<HardwareSerial> Mp3Player;
#endif

#define CycleDelay 20
#define BlinkDelay 100
#define LEDPin A5

const int scanInPin[]={3,4};
const int scanOutPin[]={8,9,10,11};

//LED顏色設定
const int colors[2][3]={{255,0,0},{0,0,255}};

const int layout[7][6]={
  { 5, 4, 3, 2, 1, 0},
  { 6, 7, 8, 9,10,11},
  {17,16,15,14,13,12},
  {18,19,20,21,22,23},
  {29,28,27,26,25,24},
  {30,31,32,33,34,35},
  {41,40,39,38,37,36}};

int readBtn();
int *check(int posX, int posY, int turn, int bd[7][6]);

Adafruit_NeoPixel pixels(43, LEDPin, NEO_GRB + NEO_KHZ800);
void setup(){
  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
    clock_prescale_set(clock_div_1);
  #endif;
  pinMode(scanInPin[0],INPUT);
  pinMode(scanInPin[1],INPUT);
  pinMode(scanOutPin[0],OUTPUT);
  pinMode(scanOutPin[1],OUTPUT);
  pinMode(scanOutPin[2],OUTPUT);
  pinMode(scanOutPin[3],OUTPUT);
  pixels.begin();
  Serial.begin(9600);
  ShowSerial.begin(9600);
  COMSerial.begin(9600);
  while (!ShowSerial);
  while (!COMSerial);
  Mp3Player.init(COMSerial);

}


void loop(){
  int  i, tim;
  bool isLock=0, isOver=0;
  int  btn, turn=0, musitim=0;
  int  dropLED[4], *resultLED;
  int  board[7][6];
  
  //重設變數+清空LED
  for(i=0;i<42;i++){
    board[i/6][i%6]=0;
    pixels.setPixelColor(i,pixels.Color(0,0,0));
  }
  Mp3Player.playSongSpecify(1,1);
  
  while(1){
    //讀按鈕輸入，沒按為-1
    btn=readBtn();
    //Serial.println(btn);
    //檢查btn是否為reset、當行是否已滿
    if(btn==7) break;
    if(board[btn][5]!=0) btn=-1;

    //更新指示燈
    if((!isLock)&&(!isOver)){
      pixels.setPixelColor(42,pixels.Color(colors[turn%2][0], colors[turn%2][1],colors[turn%2][2]));
    }else{
      pixels.setPixelColor(42,pixels.Color(0,0,0));
    }
    
    
    if((!isLock)&&(!isOver)&&(btn!=-1)){
      //更新btn到board
      for(i=0;i<6;i++){
        if(board[btn][i]==0){
          board[btn][i]=turn%2+1;

          //更新btn到dropLED
          dropLED[0]=turn; //turn
          dropLED[1]=btn;  //x
          dropLED[2]=i;    //end Y
          dropLED[3]=5;    //cur Y
          //Serial.println(*(dropLED+2));
          //檢查遊戲是否結束
          resultLED = check(btn,i,turn,board);
          if(resultLED[0]!=0) isOver=1;
          
          tim=0;
          isLock=1;
          turn++;
          break;
        }
      }

    }else if(isLock){
      //掉落動畫
      if(tim%BlinkDelay==0){
        if(dropLED[3]!=5){
          pixels.setPixelColor(layout[dropLED[1]][dropLED[3]+1], pixels.Color(0,0,0));
        }
        pixels.setPixelColor(layout[dropLED[1]][dropLED[3]], pixels.Color(colors[(turn+1)%2][0], colors[(turn+1)%2][1],colors[(turn+1)%2][2]));
        if(dropLED[3]==dropLED[2]){
          isLock=0;
        }else{
          dropLED[3]-=1;
        }
      }
      tim++;
 
    }else if(isOver){
      //結束動畫
      if(tim%BlinkDelay==0){
        for(i=1;i<=*resultLED;i++){
          pixels.setPixelColor(*(resultLED+i), pixels.Color(0,0,0));
        }
        if(tim%(BlinkDelay*2)==0){
          for(i=1;i<=resultLED[0];i++){
            pixels.setPixelColor(*(resultLED+i), pixels.Color(colors[(turn+1)%2][0], colors[(turn+1)%2][1],colors[(turn+1)%2][2]));
          } 
        }
      }
      musitim=7850;
      tim++;
    }
    
    if(!isOver){
      if(musitim==7850){
         Mp3Player.playSongSpecify(1,1);
         musitim=0;
      }
    }else{
      if(musitim==7850){
         Mp3Player.playSongSpecify(1,2);
         musitim=0;
      }
    }
    musitim++;
    pixels.show();
    delayMicroseconds(CycleDelay); 
  }
}

/*
const int scanInPin[]={2,3};
const int scanOutPin[]={8,9,10,11};
 */


int readBtn(){
  int i;
  for(i=0;i<4;i++){
    digitalWrite(scanOutPin[0],1);
    digitalWrite(scanOutPin[1],1);
    digitalWrite(scanOutPin[2],1);
    digitalWrite(scanOutPin[3],1);
    digitalWrite(scanOutPin[i],0);
    
    if(digitalRead(scanInPin[0])==0) return i;
    if(digitalRead(scanInPin[1])==0) return i+4;
  }
  return -1;
}



int *check(int posX, int posY, int turn, int bd[7][6]){
  int static res[23];
  memset(res, 0, sizeof(res));
  int val=(turn%2+1)*(turn%2+1)*(turn%2+1)*(turn%2+1);
  int x=posX, y=posY;
  int i,cont;
 
  //row (can only be 4 in a row : check 3 ones under)
  if(y>=3){
    if(bd[x][y] *bd[x][y-1] *bd[x][y-2] *bd[x][y-3] == val){
      res[res[0]+1] = layout[x][y];
      res[res[0]+2] = layout[x][y-1];
      res[res[0]+3] = layout[x][y-2];
      res[res[0]+4] = layout[x][y-3];
      res[0] += 4;
    }
  }
  
  //column (can be more than 4 : find the leftest one and check all on the same column)
  cont=0;
  x=posX; y=posY;
  for(i=0;i<4;i++){
    if(bd[i][y] *bd[i+1][y] *bd[i+2][y] *bd[i+3][y] == val){
      if(cont){
        res[res[0]+1] = layout[i+3][y];
        res[0]++;
      }else{
        res[res[0]+1] = layout[i][y];
        res[res[0]+2] = layout[i+1][y];
        res[res[0]+3] = layout[i+2][y];
        res[res[0]+4] = layout[i+3][y];
        res[0] += 4;
        cont=1;
      }
    }else cont=0;
  }
  
  //up-right (can be more than 4 : find the lowest one and check up and right)
  cont=0;
  x=posX; y=posY;
  if(!((x>=4&&y<=2)||(x<=2&&y>=3))){
    for(i=0;i<6;i++){
      if(x-i<0||y-i<0){
        x=x-i+1;
        y=y-i+1;
        break;
      }
    }
    for(i=0;i<3;i++){
      if(x+i+3<=6 && y+i+3<=6){
        if(bd[x+i][y+i] *bd[x+i+1][y+i+1] *bd[x+i+2][y+i+2] *bd[x+i+3][y+i+3] == val){
          if(cont){
            res[res[0]+1] = layout[x+i+3][y+i+3];
            res[0]++;
          }else{
            res[res[0]+1] = layout[x+i][y+i];
            res[res[0]+2] = layout[x+i+1][y+i+1];
            res[res[0]+3] = layout[x+i+2][y+i+2];
            res[res[0]+4] = layout[x+i+3][y+i+3];
            res[0]+=4;
            cont=1;
          }
        }else cont=0;
      }
    }
  }
  
  //up-left (can be more than 4 : find the lowest one and check up and left)
  cont=0;
  x=posX; y=posY;
  if(!((x<=2&&y<=2)||(x>=4&&y>=3))){
    for(i=0;i<6;i++){
      if(x+i>6 || y-i<0){
        x=x+i-1;
        y=y-i+1;
        break;
      }
    }
    for(i=0;i<3;i++){
      if(x-i-3>=0 && y+i+3<=6){
        if(bd[x-i][y+i] *bd[x-i-1][y+i+1] *bd[x-i-2][y+i+2] *bd[x-i-3][y+i+3] == val){
          if(cont){
            res[res[0]+1] = layout[x-i-3][y+i+3];
            res[0]++;
          }else{
            res[res[0]+1] = layout[x-i][y+i];
            res[res[0]+2] = layout[x-i-1][y+i+1];
            res[res[0]+3] = layout[x-i-2][y+i+2];
            res[res[0]+4] = layout[x-i-3][y+i+3];
            res[0]+=4;
            cont=1;
          }
        }else cont=0;
      }
    }
  }
  return res;
}
