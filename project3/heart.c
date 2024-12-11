#include <msp430.h>
#include <libTimer.h>
#include "lcdutils.h"
#include "lcddraw.h"
#include "buzzer.h"

// WARNING: LCD DISPLAY USES P1.0.  Do not touch!!!

#define LED BIT6        /* note that bit zero req'd for display */

#define SW1 1
#define SW2 2
#define SW3 4
#define SW4 8

#define SWITCHES 15


#define NOTE_C4   1911
#define NOTE_Cs4  1806
#define NOTE_D4   1703
#define NOTE_Ds4  1605
#define NOTE_E4   1517
#define NOTE_F4   1432
#define NOTE_Fs4  1366
#define NOTE_G4   1275
#define NOTE_Gs4  1207
#define NOTE_A4   1136
#define NOTE_As4  1073
#define NOTE_B4   1012

#define NOTE_C5   955
#define NOTE_Cs5  903
#define NOTE_D5   851
#define NOTE_Ds5  800
#define NOTE_E5   758
#define NOTE_F5   715
#define NOTE_Fs5  643
#define NOTE_G5   637
#define NOTE_Gs5  603
#define NOTE_A5   568
#define NOTE_As5  537
#define NOTE_B5   506

#define NOTE_C6   478
#define NOTE_Cs6  452
#define NOTE_D6   425
#define NOTE_Ds6  402
#define NOTE_E6   379

#define REST 0

#define DEBOUNCE_DELAY 5
volatile int debounceCount = 0;
int switches = 0 ;

//For lcd heart
unsigned short oldheartColor= COLOR_WHITE;
unsigned short newheartColor= COLOR_RED;
unsigned char step = 0;
short initSize = 2 , controlSize = 3;
short growVelocity = 1, growLimit[2]={2,12};
short drawPos[2] = {screenWidth/2 ,screenHeight/2};
short redrawScreen = 1;

//for songs
short currentNote = 0;
short durationCount = 0;
char songP= 1;
const short *song;

//TEST SONG 1
const short  song1[] = {NOTE_E4, NOTE_D4, NOTE_C4, NOTE_D4, NOTE_E4, REST, NOTE_E4, REST, NOTE_E4, NOTE_D4, REST, NOTE_D4, REST, NOTE_D4, NOTE_E4, NOTE_G4,  REST, NOTE_G4};
const short durations1[] = {100, 100, 100, 100, 100, 10, 100, 10, 100, 100, 10, 100, 10, 100, 100, 100, 10, 100};

//SONG 2
const short  song2[] ={NOTE_C5, NOTE_G4 , NOTE_F4, NOTE_D5, NOTE_C5, NOTE_G4 , NOTE_F4, NOTE_D5, NOTE_Ds5, NOTE_D5 , NOTE_G5, NOTE_F5, NOTE_G5 ,NOTE_F5, NOTE_Ds5, NOTE_D5 , NOTE_C5, NOTE_As4};
const short durations2[] = {100, 100, 100, 100, 100, 100, 100, 100,100, 100, 100, 40, 40, 40, 100, 100, 100, 100};

//SONG 3
const short  song3[]= {NOTE_F5, NOTE_A5, NOTE_F5, NOTE_E5, NOTE_F5, NOTE_A5, NOTE_F5, NOTE_D5, NOTE_D5, NOTE_F5, NOTE_D5, NOTE_C5,NOTE_D5,NOTE_A5,NOTE_G5, NOTE_D5,NOTE_A5,NOTE_G5,NOTE_F5,NOTE_D5 , REST};
const short durations3[]={100, 100, 100, 150, 100, 100, 100, 100, 100, 100,100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 200};

//SONG 4
const short  song4[]= {NOTE_Fs4, NOTE_G4, NOTE_B4, NOTE_D5, NOTE_Fs5,
    NOTE_Fs5 ,NOTE_G5, NOTE_A5, NOTE_Fs5, NOTE_B4,
    NOTE_Fs5 ,NOTE_G5, NOTE_A5,NOTE_B5,NOTE_Cs6, NOTE_B5,
    NOTE_Cs6, NOTE_D6, NOTE_E6 , NOTE_Cs6, NOTE_Fs5,
    };
const short durations4[]={100, 100, 100, 100, 250, 100, 100, 100, 200, 180, 100, 100, 100, 80, 20, 150, 100, 100, 100, 200, 200};

//functions
static char switch_update_interrupt_sense();
void switch_init();
void switch_interrupt_handler();
void drawHeart(u_char size, u_char x, u_char y, unsigned short color);
void screen_update_heart();
void wdt_c_handler();
void update_shape();



static char switch_update_interrupt_sense(){
    char p2val = P2IN;
    /* update switch interrupt to detect changes from current buttons */
    P2IES |= (p2val & SWITCHES);    /* if switch up, sense down */
    P2IES &= (p2val | ~SWITCHES);    /* if switch down, sense up */
    return p2val;
}

void switch_init(){           /* setup switch */
    P2REN |= SWITCHES;        /* enables resistors for switches */
    P2IE |= SWITCHES;        /* enable interrupts from switches */
    P2OUT |= SWITCHES;        /* pull-ups for switches */
    P2DIR &= ~SWITCHES;        /* set switches' bits for input */
    switch_update_interrupt_sense();
}


void switch_interrupt_handler() {
    if(debounceCount == 0){
        char p2val = switch_update_interrupt_sense(); //p2val = p2in
        switches = ~p2val & SWITCHES;
        debounceCount = DEBOUNCE_DELAY;
        // changes song according to switch
        if (switches & SW1){
            songP=1;
        }else if (switches & SW2){
            songP=2;
        }else if (switches & SW3){
            songP=3;
        }
    }
}



//draws heart of size int on x and y cordinate of screen
 void drawHeart(u_char size,u_char x, u_char y, unsigned short color ){
     u_char size2= size*4;
     for (int row = size2 ;row >= 0 ;row--){
         for(int col = 0 ; col < row; col++){
             drawPixel(x - col, y - row + size2 , color);
             drawPixel(x + col, y - row + size2, color);
         }
     }
     
     for (int row = 1;row <=size ;row++){
         for(int col = size; col >= row; col--){
             //40,40
             drawPixel(x+col, y -row , color);
             drawPixel(x-col, y -row , color);
             drawPixel(x+col - (size*4 +1), y -row , color);
             drawPixel(x-col + (size*4 + 1), y -row , color);
         }}
     fillRectangle(x+size +1, y -size, size*2 , size , color);
     fillRectangle(x-size*3, y-size , size*2, size , color);
 }


void screen_update_heart() {
    if (initSize != controlSize || oldheartColor != newheartColor ) /* size change / color change */
        goto redraw;
    return;            /* does nothing */
 redraw:
    drawHeart(initSize, drawPos[0] ,drawPos[1], COLOR_WHITE); /* erase */
    initSize = controlSize;
    oldheartColor = newheartColor;
    drawHeart(initSize, drawPos[0] ,drawPos[1], oldheartColor); /* draw */
}
  


void wdt_c_handler(){
    //debouncer
    if(debounceCount>0){
        debounceCount--;
    }
    
    // for heart drawing
    static int secCount = 0;
    
    secCount ++;
    if (secCount >= 50) {
        /* grow heart */
        short oldSize = controlSize;
        short newSize = oldSize + growVelocity;
        if (newSize <= growLimit[0] || newSize >= growLimit[1])
            growVelocity = -growVelocity;
        else{
            controlSize = newSize;
        }
        redrawScreen = 1;
        secCount = 0;
    }
    
    //updates colors
    switch (switches) {
        case SW1:
            newheartColor = COLOR_GREEN;
            redrawScreen = 1;
            break;
        case SW2:
            newheartColor = COLOR_RED;
            redrawScreen = 1;
            break;
        case SW3:
            newheartColor = COLOR_PURPLE;
            redrawScreen = 1;
            break;
        default:
            break;
    }
    /* pauses music */
    if (switches & SW4) return;
    
    
    //for song
    const short *song = song4;
    const short *durations= durations4;
    short songL = sizeof(song4)/sizeof(short);
    if (songP == 1){
        song = song4;
        durations= durations4;
        songL = sizeof(song4)/sizeof(short);
    }else if(songP == 2){
        song = song2;
        durations= durations2;
        songL = sizeof(song2)/sizeof(short);
    }
    else if(songP == 3){
        song = song3;
        durations= durations3;
        songL = sizeof(song3)/sizeof(short);
    }
    if (durationCount == 0) { // If the note duration is complete
        buzzer_set_period(song[currentNote]); // note frequency
        durationCount = durations[currentNote];            // time note needs to be played
        currentNote++;                                      // plays next note
        if (currentNote >= songL) {
            currentNote = 0; // Restart the song so it loops
        }
    }
    durationCount--;
}
  
void update_shape() {
    screen_update_heart();
}

void main() {
    P1DIR |= LED;        /**< Green led on when CPU on */
    P1OUT |= LED;
    configureClocks();
    lcd_init();
    switch_init();
    buzzer_init();
    
    enableWDTInterrupts();      /**< enable periodic interrupt */
    or_sr(0x8);                  /**< GIE (enable interrupts) */
  
  clearScreen(COLOR_WHITE);
  drawString5x7(20,50, "PROJECT 3 ", COLOR_BLACK, COLOR_WHITE);
  while (1) {            /* forever */
    if (redrawScreen) {
      redrawScreen = 0;
      update_shape();
    }
    P1OUT &= ~LED;    /* led off */
    or_sr(0x10);    /**< CPU OFF */
    P1OUT |= LED;    /* led on */
  }
}

void __interrupt_vec(PORT2_VECTOR) Port_2(){
    if (P2IFG & SWITCHES) {          /* did a button cause this interrupt? */
        P2IFG &= ~SWITCHES;              /* clear pending sw interrupts */
        switch_interrupt_handler();    /* single handler for all switches */
    }
    
}
