%{CPP_TEMPLATE}

#include "main.h"

int main(){
  initTextMode();
  print("HELLO WORLD",9,9);
  while(1){}	//loop forever
   
}


void print(char* text,u16 x,u16 y){
  u16 i=0;
  u16* ScreenBase = (u16*)0x6004000;
  ScreenBase = ScreenBase + x + y*32;
  while ( *text != '\0' ){

    *ScreenBase++ = (u16)*text-'A'+1;
    i++;
    text++;
  }
}

void initTextMode(){
  REG_DISPCNT = (MODE0 | BG2_ENABLE);  //this sets the screen mode to mode 0 and enables background 2
  
  // 256 colors and charscreenbase 0, screenbase 8
  // default size is 256x256 pixel
  REG_BG2CNT = (1 << 7 | 0 << 2 | 8 << 8); 
  
  
  u16* palDest=(u16*)BGPaletteMem; //0x5000000
  u16* palSource = (u16*)Master_Palette;
  // copy the palette info (256 colors a 15/16 Bit) into the desired address 
  u16 i=0;
  for (;i<256;i++) {
    *palDest++ = *palSource++;
  }
  
  // copy the tiles to charbase 0, (0x6000000 start of the videobuffer)
  u16* fontDest = (u16*)VideoBuffer; //this is the start of video memory
  u16* fontSource = (u16*)font_Tiles;
  i=0;
  for(;i<1920;i=i+2){
    *fontDest++ = *fontSource++;
  }
}


