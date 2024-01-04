#define F_CPU 15000000
#define __AVR_ATmega328__
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <util/delay.h>
#include "ili9341.h"
#include "font.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#define SPI_DDR DDRB
#define SPI_PORT PORTB
#define SPI_DC_PIN PB1
#define SPI_SS_PIN PB2
#define SPI_MOSI_PIN PB3
#define SPI_RST_PIN PB4
#define SPI_SCK_PIN PB5
#define N_LAYERS 2;

#define BUTTON_PORT PORTC
static const uint8_t initcmd[] = {
  0xEF, 3, 0x03, 0x80, 0x02,
  0xCF, 3, 0x00, 0xC1, 0x30,
  0xED, 4, 0x64, 0x03, 0x12, 0x81,
  0xE8, 3, 0x85, 0x00, 0x78,
  0xCB, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
  0xF7, 1, 0x20,
  0xEA, 2, 0x00, 0x00,
  ILI9341_PWCTR1  , 1, 0x23,             // Power control VRH[5:0]
  ILI9341_PWCTR2  , 1, 0x10,             // Power control SAP[2:0];BT[3:0]
  ILI9341_VMCTR1  , 2, 0x3e, 0x28,       // VCM control
  ILI9341_VMCTR2  , 1, 0x86,             // VCM control2
  ILI9341_MADCTL  , 1, 0x48,             // Memory Access Control
  ILI9341_VSCRSADD, 1, 0x00,             // Vertical scroll zero
  ILI9341_PIXFMT  , 1, 0x55,
  ILI9341_FRMCTR1 , 2, 0x00, 0x18,
  ILI9341_DFUNCTR , 3, 0x08, 0x82, 0x27, // Display Function Control
  0xF2, 1, 0x00,                         // 3Gamma Function Disable
  ILI9341_GAMMASET , 1, 0x01,             // Gamma curve selected
  ILI9341_GMCTRP1 , 15, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, // Set Gamma
    0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00,
  ILI9341_GMCTRN1 , 15, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, // Set Gamma
    0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F,
  ILI9341_SLPOUT  , 0x80,                // Exit Sleep
  ILI9341_DISPON  , 0x80,                // Display on
  0x00                                   // End of list
};
inline void SPI_MasterInit(void) {
    SPI_DDR |= (1 << SPI_SS_PIN) | (1 << SPI_MOSI_PIN) | (1 << SPI_DC_PIN) | (1 << SPI_RST_PIN) | (1 << SPI_SCK_PIN);
    SPI_PORT |= (1 << SPI_SS_PIN);  // Set SS pin high (inactive)
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);
}
inline void SPI_MasterTransmit(const uint8_t data) {
    SPDR = data;
    while (!(SPSR & (1 << SPIF)));
}
inline void ILI9341_WriteData8(const uint8_t data){
    SPI_PORT |= (1 << SPI_DC_PIN);
   
    SPI_MasterTransmit(data);
}
inline void ILI9341_WriteData16(const uint16_t data){
    SPI_PORT |= (1 << SPI_DC_PIN);

    SPI_MasterTransmit(data   >> 8);
    SPI_MasterTransmit(data & 0xff);
}
inline void ILI9341_WriteData8_array(const uint8_t* data, const uint8_t n){
    SPI_PORT |= (1 << SPI_DC_PIN);
    
    uint8_t i=0;
    for (i=0;i<n;i++){
        SPI_MasterTransmit(data[i]);
    }
}
inline void ILI9341_WriteData8_array_lpad(const uint8_t* data, const uint8_t n){
    SPI_PORT |= (1 << SPI_DC_PIN);
   
    uint8_t i=0;
    for (i=0;i<n;i++){
        SPI_MasterTransmit(0);
        SPI_MasterTransmit(data[i]);
    }
   
}
inline void ILI9341_WriteData8_array_rpad(const uint8_t* data, const uint8_t n){
    SPI_PORT |= (1 << SPI_DC_PIN);
   
    int i;
    for (i=0;i<n;i++){
        SPI_MasterTransmit(data[i]);
        SPI_MasterTransmit(0);
    }
   
}
inline void ILI9341_WriteData8_array_double(const uint8_t* data, const uint8_t n){
    SPI_PORT |= (1 << SPI_DC_PIN);
    
    int i;
    for (i=0;i<n;i++){
        SPI_MasterTransmit(data[i]);
        SPI_MasterTransmit(data[i]);
    }
   
}
inline void ILI9341_WriteData16_array(const uint16_t* data, const uint8_t n){
    SPI_PORT |= (1 << SPI_DC_PIN);
   
    int i;
    for (i=0;i<n;i++){
        SPI_MasterTransmit(data[i]>>8);
        SPI_MasterTransmit(data[i]&0xff);
    }
   
}
inline void ILI9341_WriteCommand(uint8_t cmd) {
    SPI_PORT &= ~(1 << SPI_DC_PIN); // set transmission type to COMMAND
    SPI_MasterTransmit(cmd);
    
}


void ILI9341_Init(void) {
    // Reset display
    SPI_PORT &= ~(1 << SPI_SS_PIN);  // Set SS pin low (active)
//    SPI_PORT &= ~(1 << SPI_RST_PIN);  // Set SS pin low (active)
    _delay_ms(300);
    SPI_PORT |= (1 << SPI_RST_PIN);  // Set SS pin high (inactive)
//    SPI_PORT |= (1 << SPI_SS_PIN);  // Set SS pin high (inactive)
    _delay_ms(10);

    // Software reset
//    SPI_PORT &= ~(1 << SPI_SS_PIN);  // Set SS pin low (active)
//    ILI9341_WriteCommand(ILI9341_SWRESET);
//    SPI_PORT |= (1 << SPI_SS_PIN);  // Set SS pin high (inactive)
    _delay_ms(100);


    uint8_t cmd, x, numArgs;
    const uint8_t *addr = initcmd;
  

    while ((cmd = *(addr++)) > 0) {
        x = *(addr++);
        numArgs = x & 0x7F;
        SPI_PORT &= ~(1 << SPI_SS_PIN);  // Set SS pin low (active)
                                         //
        ILI9341_WriteCommand(cmd);
        ILI9341_WriteData8_array(addr,numArgs);
        
        SPI_PORT |= (1 << SPI_SS_PIN);  // Set SS pin high (inactive)
        addr += numArgs;
        if (x & 0x80)
          _delay_ms(150);
    }
}

const uint16_t colors[19] PROGMEM= {
ILI9341_BLACK,
ILI9341_NAVY,
ILI9341_DARKGREEN,
ILI9341_DARKCYAN,
ILI9341_MAROON,
ILI9341_PURPLE,
ILI9341_OLIVE,
ILI9341_LIGHTGREY,
ILI9341_DARKGREY,
ILI9341_BLUE,
ILI9341_GREEN,
ILI9341_CYAN,
ILI9341_RED,
ILI9341_MAGENTA,
ILI9341_YELLOW,
ILI9341_WHITE,
ILI9341_ORANGE,
ILI9341_GREENYELLOW,
ILI9341_PINK,
    };
uint8_t textcol=0;
int8_t str2[30*20];
int8_t* str=str2+1;
uint16_t render_cursor=0;
uint16_t prev_render_cursor=0;
uint16_t cursor=0;
uint16_t strln = 0;
uint16_t linelen = 26;
uint8_t charmap[]="wertyuiosdfghjklzxcvbnmqap .\1\2\3\4"
                  "9753102468+-/*        #=() .\1\2\3\4";
typedef struct token {
    int8_t* id;
    int8_t len;
}token;
token tokens[128];
int8_t tokens_n=0;
void draw_cursor(uint16_t render_cursor, uint16_t color){
    SPI_PORT &= ~(1 << SPI_SS_PIN);  // Set SS pin low (active)
    ILI9341_WriteCommand(0x2A);  // Set column address
    ILI9341_WriteData16((render_cursor % linelen) * 9);
    ILI9341_WriteData16((render_cursor % linelen) * 9);

    ILI9341_WriteCommand(0x2B);  // Set row address
    ILI9341_WriteData16((render_cursor / linelen) * 18);
    ILI9341_WriteData16((render_cursor / linelen) * 18 + 16);
    uint8_t i;
    for (i=0 ; i< 18 ; i++) {
        ILI9341_WriteData16(color);
    }
    SPI_PORT |= (1 << SPI_SS_PIN);  // Set SS pin high (inactive)
}

void erase_text(uint16_t cursor, uint16_t length) {
    SPI_PORT &= ~(1 << SPI_SS_PIN);  // Set SS pin low (active)
    
    uint16_t i=0;
    uint16_t c=0;
    uint16_t row=0;
    uint16_t startofline=cursor;
    uint16_t col = 0;
    for (row = cursor / linelen ; c < length ;row++){
        uint16_t c2= 0;
        for (c2=startofline;(c2==startofline || (c2 %linelen)) && c2 <startofline + linelen && c2 < startofline+length; c2++);
        ILI9341_WriteCommand(0x2A);  // Set column address
        ILI9341_WriteData16((cursor % linelen) * 9);
        ILI9341_WriteData16((cursor % linelen) * 9 +(c2-startofline)*9 );

        ILI9341_WriteCommand(0x2B);  // Set row address
        ILI9341_WriteData16(18* row );
        ILI9341_WriteData16(18*(row+1));
        for (i=0; i < c2 -startofline;i++){
//            ILI9341_WriteData16_array((uint16_t*)"\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2",9);
            ILI9341_WriteData16_array((uint16_t*)"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0",9);
        }
        for (i=0;i<16;i++){
            ILI9341_WriteData16(0);
            
            for (c=startofline; (c==startofline || (c%linelen)) && c < startofline+linelen && c < startofline+length /*&& str[c] != '\n'*/ ;c++){
                ILI9341_WriteData16( 0 );
                ILI9341_WriteData16( 0 );
                ILI9341_WriteData16( 0 );
                ILI9341_WriteData16( 0 );

                ILI9341_WriteData16( 0 );
                ILI9341_WriteData16( 0 );
                ILI9341_WriteData16( 0 );
                ILI9341_WriteData16( 0 );
                ILI9341_WriteData16(0);
            }
        }
        for (i=0; i<length && i < c2 -startofline;i++){
            ILI9341_WriteData16_array((uint16_t*)"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0",9);
        }
        startofline=c + (str[c] == '\n' && c2 - startofline < linelen);
        cursor += linelen - cursor%linelen;
    }

    //ILI9341_WriteData8_array_double(font+(-0x20 + 'a')*9*16 ,9*16);  // Memory write
    SPI_PORT |= (1 << SPI_SS_PIN);  // Set SS pin high (inactive)
}
void draw_text(int8_t* str, uint16_t cursor, uint16_t start_of_text,uint16_t end_of_text) {
    SPI_PORT &= ~(1 << SPI_SS_PIN);  // Set SS pin low (active)
    
    //ILI9341_WriteData8_array_double(symbol+(-'#' + '#')*9*16 ,9*16);  // Memory write
    uint16_t i=0;
    uint16_t c=0;
    uint16_t row=0;
    uint16_t startofline=start_of_text;
    uint16_t col = pgm_read_word(colors+textcol);
    for (row = cursor / linelen ; c < end_of_text ;row++){
//        col = pgm_read_word(colors+row+1);
        uint16_t c2= 0;
        for (c2=startofline;(c2==startofline || ((c2 -startofline+cursor) %linelen)) && c2 <startofline + linelen && str[c2] != '\n' && c2 < end_of_text; c2++);
        //for (c2=startofline;(c2==startofline || (c2 %linelen)) && c2 <startofline + linelen && str[c2] != '\n' && c2 < end_of_text; c2++);
        ILI9341_WriteCommand(0x2A);  // Set column address
        ILI9341_WriteData16((cursor % linelen) * 9);
        ILI9341_WriteData16((cursor % linelen) * 9 +(c2-startofline)*9 );
        
        ILI9341_WriteCommand(0x2B);  // Set row address
        ILI9341_WriteData16(18* row );
        ILI9341_WriteData16(18*(row+1));
        for (i=0; i < (c2 -startofline)*9;i++){
              ILI9341_WriteData16(0);
//            ILI9341_WriteData16_array((uint16_t*)"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0",9);
        }
        for (i=0;i<16;i++){
            ILI9341_WriteData16(0);
            
            for (c=startofline; (c==startofline || ((c-startofline+cursor)%linelen)) &&  c < end_of_text && str[c] != '\n' ;c++){
                uint8_t drawbyte = pgm_read_byte(&font[(str[c]-0x20)*16+i]);
                            
                ILI9341_WriteData16( ((drawbyte>>7) & 1) * col );
                ILI9341_WriteData16( ((drawbyte>>6) & 1) * col );
                ILI9341_WriteData16( ((drawbyte>>5) & 1) * col );
                ILI9341_WriteData16( ((drawbyte>>4) & 1) * col );

                ILI9341_WriteData16( ((drawbyte>>3) & 1) * col );
                ILI9341_WriteData16( ((drawbyte>>2) & 1) * col );
                ILI9341_WriteData16( ((drawbyte>>1) & 1) * col );
                ILI9341_WriteData16( ((drawbyte>>0) & 1) * col );
                ILI9341_WriteData16(0);
            }
        }
        for (i=0; i<end_of_text && i < c2 -startofline;i++){
            ILI9341_WriteData16_array((uint16_t*)"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0",9);
        }
        startofline=c + (str[c] == '\n' && c2 + cursor - startofline < linelen);
        cursor += linelen - cursor%linelen;
    }

    //ILI9341_WriteData8_array_double(font+(-0x20 + 'a')*9*16 ,9*16);  // Memory write
    SPI_PORT |= (1 << SPI_SS_PIN);  // Set SS pin high (inactive)
}
void draw_text_with_erase(int8_t * str, uint16_t cursor, uint16_t start_of_text,uint16_t end_of_text) {
    SPI_PORT &= ~(1 << SPI_SS_PIN);  // Set SS pin low (active)
    //ILI9341_WriteData8_array_double(symbol+(-'#' + '#')*9*16 ,9*16);  // Memory write
    uint16_t i=0;
    uint16_t c=0;
    uint16_t row=0;
    uint16_t startofline=start_of_text;
    for (row = cursor / linelen ; c < end_of_text ;row++){
        uint16_t col = pgm_read_word(colors+textcol);
        uint16_t c2= 0;
        for (c2=startofline;(c2==startofline || (c2 %linelen)) && c2 <startofline + linelen && str[c2] != '\n' && c2 < end_of_text; c2++);
        ILI9341_WriteCommand(0x2A);  // Set column address
        ILI9341_WriteData16((cursor % linelen) * 9);
        ILI9341_WriteData16( (linelen * 9) );

        ILI9341_WriteCommand(0x2B);  // Set row address
        ILI9341_WriteData16(18* row );
        ILI9341_WriteData16(18*(row+1));
        for (i=cursor%linelen; i < linelen;i++){
            ILI9341_WriteData16_array((uint16_t*)"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0",9);
        }
        for (i=0;i<16;i++){
            ILI9341_WriteData16(0);
            
            for (c=startofline; (c==startofline || ((c-startofline+cursor)%linelen)) &&  c < end_of_text && str[c] != '\n' ;c++){
                uint8_t drawbyte = pgm_read_byte(&font[(str[c]-0x20)*16+i]);
                            
                ILI9341_WriteData16( ((drawbyte>>7) & 1) * col ); 
                ILI9341_WriteData16( ((drawbyte>>6) & 1) * col );
                ILI9341_WriteData16( ((drawbyte>>5) & 1) * col );
                ILI9341_WriteData16( ((drawbyte>>4) & 1) * col );

                ILI9341_WriteData16( ((drawbyte>>3) & 1) * col );
                ILI9341_WriteData16( ((drawbyte>>2) & 1) * col );
                ILI9341_WriteData16( ((drawbyte>>1) & 1) * col );
                ILI9341_WriteData16( ((drawbyte>>0) & 1) * col );
                ILI9341_WriteData16(0);
            }
            uint16_t iii;
            if (str[c] == '\n' && str[c-1] == '\n' && str[c+1] != '\n' && str[c+1] != 0 ||
                str[c] == '\n' && str[c-1] == '\n' && str[c-2] != '\n'// || str[c-1] == '\n'
            ){
                for (iii = 0; iii< linelen; iii++){
//                    ILI9341_WriteData16_array((uint16_t*)"\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2",9);
                    ILI9341_WriteData16_array((uint16_t*)"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0",9);
                }
            }
            else {
                for (iii = cursor + c - startofline; iii%linelen; iii++){
                    ILI9341_WriteData16_array((uint16_t*)"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0",9);
                }
            }
        }
        for (i=cursor%linelen; i < linelen;i++){
            ILI9341_WriteData16_array((uint16_t*)"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0",9);
        }
        if (str[c] == '\n' && ((c-startofline+cursor)%linelen) == 0  && str[c-1] != '\n') {
//            uint16_t col = pgm_read_word(colors+ (textcol= (textcol+1)%8+1 ));
            ILI9341_WriteCommand(0x2A);  // Set column address
            ILI9341_WriteData16( 0);
            ILI9341_WriteData16( (linelen * 9) );

            ILI9341_WriteCommand(0x2B);  // Set row address
            ILI9341_WriteData16(18* (row+1) );
            ILI9341_WriteData16(18* (row+2));
            row++;
            uint16_t xyz=0;
            for (xyz= 0 ; xyz< linelen * 18*9; xyz++)ILI9341_WriteData16(0);
        } 
        startofline=c + (str[c] == '\n' &&  c2 - startofline < linelen) - (c2-startofline >= linelen);
        cursor += linelen - cursor%linelen;
    }

    //ILI9341_WriteData8_array_double(font+(-0x20 + 'a')*9*16 ,9*16);  // Memory write
    SPI_PORT |= (1 << SPI_SS_PIN);  // Set SS pin high (inactive)
}

uint16_t find_cursor_at_position(uint16_t* screen_position){
    uint16_t c = 0;
    uint16_t pos=0;

    for (c = 0 ; c < strln; c++){
        if (pos == *screen_position){
            return c; 
        }
        if (str[c] == '\n') {
            uint16_t pos2 = pos + linelen - (pos % linelen);
            if (pos2 > *screen_position) {
                *screen_position = pos;
                return c;
            }
            pos= pos2;
        }else pos++;
    }
    *screen_position = pos;
    return c;
}
uint16_t find_position_of_text_offset(uint16_t text_offset){
    uint16_t c = 0;
    uint8_t pos = 0;
    for (c = 0 ; c < text_offset; c++){
        if (str[c] == '\n')pos+= linelen - pos%linelen;
        else pos++;
    }
    return pos;
}


void debug_print(uint16_t position, char* pattern, ...){
   va_list argList;
   char debug_buf[linelen*4];
   //sprintf(debug_buf, "cursor: %i\nrender_cursor: %i\nstr[cursor]: %c", cursor, render_cursor, str[cursor] == '\n' ? '\\' : str[cursor]);
   va_start(argList,pattern);
    vsprintf(debug_buf, pattern , argList );
    va_end(argList);
   draw_text_with_erase((int8_t * )debug_buf, position*linelen, 0,strlen(debug_buf));
}

char** tokenize(int8_t* string){
    tokens_n=0;
    for (int8_t* c = string; *c != 0; c++){
        while (*c == ' ' || *c == '\n')c++;
        if (*c >= 'a' && *c <= 'z' || *c >= 'A' && *c <= 'Z'){
            tokens[tokens_n].id = c;
            int8_t* c_end=0;
            for (c_end = c; *c_end != 0 ; c_end++){
                if (! ( *c_end >= 'a' && *c_end <= 'z' || *c_end >= 'A' && *c_end <= 'Z')){
                    break;
                }
            }
            tokens[tokens_n++].len = c_end-c;
            c= c_end;
        }

        if (*c >= '0' && *c <= '9'){
            tokens[tokens_n].id = c;
            int8_t* c_end=0;
            for (c_end = c; *c_end != 0 ; c_end++){
                if (! ( *c_end >= '0' && *c_end <= '9')){
                    break;
                }
            }
            tokens[tokens_n++].len = c_end-c;
            c= c_end;
        }

        if (*c == '*' || *c == '/'  || *c == '+'  || *c == '-'  || *c == '=' || *c == '#' || *c == '(' || *c == ')'){
            tokens[tokens_n].id = c;
            int8_t* c_end=c+1;
            tokens[tokens_n++].len = c_end-c;
            c= c_end;
        }
        
        if (*c >= '0' && *c <= '9'){
            tokens[tokens_n].id = c;
            int8_t* c_end=0;
            for (c_end = c; *c_end != 0 ; c_end++){
                if (! ( *c_end >= '0' && *c_end <= '9')){
                    break;
                }
            }
            tokens[tokens_n++].len = c_end-c;
            c= c_end;
        }

    }


}

int main(void) {
    memset ( str, '\0',30*20) ;
    str[-1]='\n';
    SPI_MasterInit();
    ILI9341_Init();
    PORTC =  0xFF;
    DDRC |=  0xF;
    
    DDRD=0;
    PORTD=0xFF;
    
    uint8_t arg = 7;

    textcol=3;
    uint8_t change = 1;
    uint8_t cursor_change = 1;
    uint8_t keys_down[4]= {0,0,0,0};

    uint8_t shift = 0;
    uint8_t mod1 = 0;
    uint8_t layer = 0;
    
    uint16_t erase_cursor=0;
    uint16_t erase_len=0;
    uint16_t start_of_line=0;
    uint16_t end_of_line=0;
    uint16_t render_position=0;
    uint8_t  draw_with_erase=0;
    uint16_t textcursor_at_render_pos=0; 

    while (1){
        render_position= prev_render_cursor;
        shift = (keys_down[3] & 0x80) >> 7;
        mod1  = (keys_down[3] & 0x40) >> 6;
        for (uint8_t i = 0 ; i < 4 ; i++){
            PORTC = ~(1<<i);
            uint8_t buttonstate = ~PIND;
            if(keys_down[i] != buttonstate){
                cursor_change=1;
                for (uint8_t keyboard_column = 0 ; keyboard_column < 8 ; keyboard_column++){
                    if (keys_down[i] & 1<<keyboard_column  &&  !(buttonstate & 1<<keyboard_column) ) {
                        char c = charmap[32*layer + 8*i + keyboard_column];
                        if (c >= 'a' && c <= 'z') c &= ~(shift << 5);
                        if (c == '\1' && mod1){
                            textcol = (textcol +1) %16;
                            change=1;
                        } 
                        else if (c == '\2' && mod1){
                            textcol = (textcol -1) %16;
                            change=1;
                        } 
                        else if (c == '\1'){
                            layer = (layer +1) % N_LAYERS;
                        } 
                        else if (c == '\2'){
                            layer = (layer -1) % N_LAYERS;
                        } else if (mod1 && c == 'p'){
                            cursor_change=1;
                            change=1;
                            uint16_t after_cursor = 0;
                            
                            start_of_line=cursor - render_cursor % linelen;
                            for (end_of_line = cursor; str[end_of_line]!='\n' && end_of_line < strln ;end_of_line++);

                            if (str[cursor-1] == '\n') {
                                for (end_of_line = cursor; str[end_of_line]!='\n' && end_of_line < strln ;end_of_line++);
                                uint16_t aftercursor = end_of_line %linelen;
                                
                                strln--;
                                erase_len = strln;
                                render_cursor--;
                                cursor = find_cursor_at_position(&render_cursor);  
                                render_position=render_cursor;
                                memmove(str+cursor,str+cursor+1, strln-cursor + 1 );
                                
                                uint16_t end_of_text=linelen*30;
                                erase_cursor=find_cursor_at_position(&end_of_text);
                                erase_cursor=end_of_text + linelen - (end_of_text%linelen) - linelen*(end_of_text%linelen == 0);
                                erase_len=linelen ;

                                draw_with_erase=1;
                            }else{
                                memmove(str+cursor-1,str+cursor, strln-cursor + 1 );
                                
                                strln--;
                                cursor--;
                                erase_len = 1;

                                start_of_line=cursor - render_cursor % linelen;
                                for (end_of_line = cursor; str[end_of_line]!='\n' && end_of_line < strln; end_of_line++);
                                erase_cursor = render_cursor + end_of_line-cursor -1;
                                render_cursor--;
                                cursor = find_cursor_at_position(&render_cursor);  
                                if (! ((end_of_line- start_of_line) % linelen)){
                                    draw_with_erase=1;
                                    uint16_t end_of_text=linelen*30;
                                    erase_cursor=find_cursor_at_position(&end_of_text);
                                    erase_cursor=end_of_text + linelen - (end_of_text%linelen);
                                    erase_len=linelen;

                                    draw_with_erase=1;
                                 }
                            }
                            textcursor_at_render_pos=cursor;
                        }
                        else if (mod1 && c == 'h') {
                            if (cursor > 0){
                                cursor_change  = 1;
                                render_cursor -= 1;
                                cursor = find_cursor_at_position(&render_cursor);
                            }
                        }
                        else if (mod1 && c == 'l') {
                            if (cursor < strln ){
                                cursor_change=1;
                                cursor++;
                                if (str[cursor-1] == '\n'){
                                    render_cursor += linelen - (render_cursor %linelen);
                                } else render_cursor ++;
                            }
                        }
                        else if (mod1 && c == 'k') {
                            if (render_cursor > linelen){
                                cursor_change=1;
                                render_cursor -= linelen;
                                cursor = find_cursor_at_position(&render_cursor);
                            } else {
                                render_cursor=0;
                                cursor=0;
                            }
                        }
                        else if (mod1 && c == 'j') {
                            render_cursor += linelen * (cursor_change=1);
                            cursor = find_cursor_at_position(&render_cursor);
                            
                        }
                        else if (c != '\4' && c != '\3'){
                            if (mod1 && c == 'n') c =' ';
                            if (mod1 && c == 'v'){
                                c = '\n';
                                start_of_line=cursor - render_cursor % linelen;
                                for (end_of_line = cursor;  str[end_of_line]!='\n' && end_of_line < strln ;end_of_line++);
                                erase_cursor = render_cursor;
                                erase_len    = end_of_line - cursor;
                                render_position  = render_cursor - render_cursor%linelen ;
                                render_cursor   += linelen - render_cursor % linelen ;
                                change=1;
                                draw_with_erase=1;
                                if(textcursor_at_render_pos == 0xffff)textcursor_at_render_pos = start_of_line;
                                if(str[start_of_line - 1] != '\n' && cursor == start_of_line && start_of_line > 0) {
                                    render_position-=linelen;
                                }
                            }else {
                                erase_len=0;
                                render_cursor ++;
                                change=1;
                            }
                            if(textcursor_at_render_pos == 0xffff)textcursor_at_render_pos = cursor;
                            cursor_change=1;
                            memmove(str+cursor+1,str+cursor, strln - cursor +1 );
                            str[cursor++] = c;
                            strln++;
                            start_of_line=cursor - render_cursor % linelen;
                            for (end_of_line = textcursor_at_render_pos; str[end_of_line]!='\n' && end_of_line < strln ;end_of_line++);
                            if (! ((end_of_line- start_of_line) % linelen))draw_with_erase=1;
                            if (! ((end_of_line- start_of_line) % linelen)  && str[end_of_line] == '\n' ){
                                draw_with_erase=1;
                            }

                        }
                    }
                }
            }
            keys_down[i] = buttonstate;
        }
        for (end_of_line = textcursor_at_render_pos;  str[end_of_line]!='\n' && end_of_line < strln ;end_of_line++);
        
        if (change){
            if ( draw_with_erase ){
                draw_text_with_erase(str,render_position,textcursor_at_render_pos,strln); 
            } else {
                draw_text(str,render_position, textcursor_at_render_pos, end_of_line);        
            }
       
            tokenize(str);
        }
        if (erase_len){
            erase_text(erase_cursor,erase_len);
        }
        erase_len=0;
        if (cursor_change){
            draw_cursor(prev_render_cursor, ILI9341_BLACK);
            draw_cursor(render_cursor, ILI9341_WHITE);
            prev_render_cursor = render_cursor;
        }
        if (change){
            for (char i = 0 ; i < 3; i++){
                int8_t tmpchar = *(tokens[i].id + tokens[i].len);
                tokens[i].id[tokens[i].len]=0;
                debug_print(13+i,"%s\n",tokens[i]);
                tokens[i].id[tokens[i].len]=tmpchar;
            }
            debug_print(16,"%i" , tokens_n);
        }
        draw_with_erase=0;
        cursor_change=0;
        change= 0;
        textcursor_at_render_pos = 0xffff;
    }
    return 0;
}
