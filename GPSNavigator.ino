#include <Adafruit_GFX.h>    //                         Core graphics library
#include <Adafruit_TFTLCD.h> //                         Hardware-specific library
#include <SD.h>//                                       SD library
#include <SPI.h>//                                      SPI library
#include <EEPROM.h>//                                   EEPROM library
#include <TinyGPS.h>//                                  Tiny GPS library

#define LCD_CS A3 //                                    Chip Select goes to Analog 3
#define LCD_CD A2 //                                    Command/Data goes to Analog 2
#define LCD_WR A1 //                                    LCD Write goes to Analog 1
#define LCD_RD A0 //                                    LCD Read goes to Analog 0
#define SD_CS 10     //                                 Set the chip select line to whatever you use (10 doesnt conflict with the library)
#define BUFFPIXEL 80//                                  To increase speed for display we use a buffer

float    latitude=51.687994;//Sint Jan; will be used as long as no GPS signal is recieved
float    longitude=5.308396;

float    oldlat,oldlon;//                               In order to check if the position has changed we need the old position 
float    Lat1,Lat2,Lon1,Lon2;//                         Position of left top and right bottom of BMP, read from boundarykl.dat
int      XPos,YPos;
File     boundaryfile;
String   FileNameMap;
char     FileNameString[8]; //                          char array to copy the String into
unsigned long fix_age;

TinyGPS gps; //                                         create gps object
Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, A4);

void setup(){
  Serial.begin(9600);
  tft.reset();
  uint16_t identifier = tft.readID();
  tft.begin(identifier);
  tft.fillScreen(0x0000);
}

float arraytofloat1(char input[]){
  float b=0.0;
  float c=10.0;
  for(int t=0;t<10;t++){
    float a=0.0;
    a=input[t]-'0';
    b=b+c*a;
    c=c*0.1;
  }
  return b;
}

void Readline(){
  char Buf1[12];
  char Dummy[1];
  boundaryfile.read(Buf1,12);
  Lat1=arraytofloat1(Buf1);//                           Top latitude
  boundaryfile.read(Dummy,1);//                         Comma separator
  boundaryfile.read(Buf1,12);
  Lat2=arraytofloat1(Buf1);//                           Bottom latitude
  boundaryfile.read(Dummy,1);//                         Comma separator 
  boundaryfile.read(Buf1,12);
  Lon1=arraytofloat1(Buf1);//                           Left longitude            
  boundaryfile.read(Dummy,1);//                         Comma separator
  boundaryfile.read(Buf1,12);
  Lon2=arraytofloat1(Buf1);//                           Right longitude
  boundaryfile.read(Dummy,1);//                         Comma separator
  boundaryfile.read(Buf1,7);
  for(int teller=0;teller<7;teller++){//                Filename
   FileNameMap+=Buf1[teller];
  } 
  boundaryfile.read(Dummy,1);
  boundaryfile.read(Dummy,1);
}

uint16_t read16(File f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); //                LSB
  ((uint8_t *)&result)[1] = f.read(); //                MSB
  return result;
}

uint32_t read32(File f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); //                LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); //                MSB
  return result;
}

void bmpDraw(char *filename) {
  File     bmpFile;
  uint32_t bmpImageoffset;//                            Start of image data in file
  uint32_t bmpWidth;// 
  uint32_t bmpHeight;//  
  uint8_t  sdbuffer[BUFFPIXEL]; //                      pixel in buffer (R+G+B per pixel)
  uint16_t lcdbuffer[BUFFPIXEL];  //                    pixel out buffer (16-bit per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); //               Current position in sdbuffer
  boolean  flip = true;//                               BMP is stored bottom-to-top
  int      row, col;
  uint8_t  b, hi, lo;
  uint32_t pos = 0;
  uint8_t  lcdidx = 0;
  boolean  first = true;
  uint16_t ColDef;
  uint8_t dum,blue,green,red;
  if ((bmpFile = SD.open(filename)) == NULL) { //       Open requested file on SD card
      tft.setCursor(0, 0);
      tft.setTextColor(0xFFFF);  tft.setTextSize(2);
      tft.println("Map not found");
    return;
  }
  if(read16(bmpFile) == 0x4D42) { //                    2  BMP signature 
    (void)read32(bmpFile);//                            4  File size
    (void)read32(bmpFile); //                           4  creator bytes
    bmpImageoffset = read32(bmpFile); //                4  Start of image data
    (void)read32(bmpFile);//                            4  Header size
    bmpWidth  = read32(bmpFile);//                      4  bmp Width
    bmpHeight = read32(bmpFile);//                      4  bmp Height
    if(read16(bmpFile) == 1) { //                       2  # planes -- must be '1'
      (void)read16(bmpFile);//                          2  bits per pixel
      (void)read32(bmpFile);//                          4  0 = uncompressed
      for (int t=0;t<20;t++){
        dum=bmpFile.read();
      }
      for (int t=0;t<255;t++){//                        start reading color definitions
        blue=bmpFile.read();//                          As we do not want to write to the EEPROM too often
        green=bmpFile.read();//                         we check if the stored colordefinition is changed
        red=bmpFile.read();//                           otherwise we do not write to EEPROM!
        dum=bmpFile.read();
        ColDef=tft.color565(red,green,blue);//          convert to 16 bit color
        if(EEPROM.read(t*2) != highByte(ColDef)){//     if highbyte differs, write to EEPROM
          EEPROM.write(t*2,highByte(ColDef));
        }
        if(EEPROM.read(t*2+1) != lowByte(ColDef)) {//   if lowbyte differs, write to EEPROM
          EEPROM.write(t*2+1,lowByte(ColDef));
        }
      }  
      int hx=120;
      int hy=160;
      XPos=(longitude-Lon1)*bmpWidth/(Lon2-Lon1)-hx;
      if(XPos<0){
        XPos=XPos+hx;
      }
      if(XPos>bmpWidth){
        XPos=XPos-hx;
      }
      YPos=-1*(latitude-Lat1)*bmpHeight/(Lat1-Lat2)-hy;
      if(YPos<0){
        YPos=YPos+hy;
      }
      if(YPos>bmpHeight){
        YPos=YPos-hy;
      }
      tft.setCursor(0, 300);
      tft.setTextColor(0xFFFF);  tft.setTextSize(2);
      tft.print("Map in use: ");
      tft.println(filename);
      tft.setAddrWindow(0,0,tft.width()-1,tft.height()-22);// Set TFT address window to clipped image bounds
      for (row=0; row<tft.height()-21; row++) { 
        if(flip) //                                     Bitmap is stored bottom-to-top order (normal BMP)
          pos = bmpImageoffset + (bmpHeight - 1 - row-YPos) * bmpWidth+XPos;
        else     //                                     Bitmap is stored top-to-bottom
          pos = bmpImageoffset + row * bmpWidth;
        if(bmpFile.position() != pos) { //              Need seek?
          bmpFile.seek(pos);
          buffidx = sizeof(sdbuffer); //                Force buffer reload
        }
        for (col=0; col<tft.width(); col++) { //        For each column...
          if (buffidx >= sizeof(sdbuffer)) { //         Time to read more pixel data?
            if(lcdidx > 0) {//                          Push LCD buffer to the display first
              tft.pushColors(lcdbuffer, lcdidx, first);
              lcdidx = 0;
              first  = false;
            }
            bmpFile.read(sdbuffer, sizeof(sdbuffer));
            buffidx = 0; //                             Set index to beginning
          }
          b = sdbuffer[buffidx++];//                    read pixelcolor
          hi=EEPROM.read(b*2);//                        read highbyte of 16 bit colordefinition
          lo=EEPROM.read(b*2+1);//                      read lowbyte of 16 bit colordefinition
          ColDef=256*hi+lo;//                           set color
          lcdbuffer[lcdidx++] = ColDef;
        } //                                            end column
      } //                                              end row
      if(lcdidx > 0) {//                                Write any remaining data to LCD
        tft.pushColors(lcdbuffer, lcdidx, first);
      } 
    }
  }
  bmpFile.close();
}

void findbmp(){
  SD.begin(SD_CS);
  boundaryfile = SD.open("boundkl.dat");
  for(int teller=0;teller<304;teller++){
    FileNameMap="";
    Readline();
    if((latitude<=Lat1) && (latitude>=Lat2) && (longitude>=Lon1) && (longitude<=Lon2)){
      boundaryfile.close();
      return;
    }
  }
  tft.fillScreen(0x0000);
  tft.setCursor(10,100);
  tft.println("No map found!");
  FileNameMap="";
  boundaryfile.close();
}

void check(){
  findbmp();
  FileNameMap.toCharArray(FileNameString, FileNameMap.length() + 1); //Copy the string (+1 is to hold the terminating null char)  
  bmpDraw(FileNameString);
  int16_t x0=100;
  int16_t y0=160;
  int16_t w0=40;
  uint16_t color=0xF0F0;
  for(int t=-1;t<2;t++){
    tft.drawFastHLine(x0, y0+t, w0,  color);
    tft.drawFastVLine(x0+20+t,y0-20,w0,color);
  }  
}

void loop(){
  if (Serial.available() > 0) {
    //                                                  read the incoming byte:
    if(gps.encode(Serial.read())){ //                   encode gps data
      gps.f_get_position(&latitude,&longitude,&fix_age); // get latitude and longitude
    }
  }
  if(latitude!=oldlat || longitude!=oldlon){//          Only renew if position has changed
  check();
  oldlat=latitude;
  oldlon=longitude;
  }
}
