/** \file lcddemo.c
 *  \brief A simple demo that draws a string and square
 */

#include <libTimer.h>
#include "lcdutils.h"
#include "lcddraw.h"

/** Initializes everything, clears the screen, draws "hello" and a square */
int
main()
{
  configureClocks();
  lcd_init();
  u_char width = screenWidth, height = screenHeight;

  clearScreen(COLOR_BLUE);
    
   // int col = 0, row= 0 ;
  //  int centerCol = width/2, centerRow = height/2;
 //   for (row = 0 ;row<20;row++){
   //   for(col = -row; col <=(2* row); col+=2)
   //     drawPixel(centerCol + col,centerRow+ row, COLOR_PINK);
  //  }
    
  drawString5x7(20,20, "hello", COLOR_GREEN, COLOR_RED);

  //fillRectangle(30,30, 60, 60, COLOR_ORANGE);
  
}
