%{H_TEMPLATE}

// gba.h has all the register definitions
#include "gba.h"     

extern u16 font_Tiles[]; // the letters as 8x8 tiles
extern u16 Master_Palette[]; // the color palette 

void print(char* text,u16 x,u16 y);
void initTextMode();


