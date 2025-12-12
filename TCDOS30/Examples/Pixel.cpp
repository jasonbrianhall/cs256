// Marcela Ciupe, Vijayanand Bharadwaj
// Part 5 - create two additional Point objects

// Borland C++ - (C) Copyright 1991 by Borland International

/* PIXEL.CPP--Example from Getting Started */

// PIXEL.CPP demonstrates the organiztion of a simple C++ Program
// Its function is showing,moving and hiding a point on screen

//Proprocessor directive
#include <graphics.h>   //Insert "graphics.h" for graphics functions
#include <conio.h>      //Insert "conio.h" for getch() function
#include "point.h"      //Insert "point.h" for declarations of classes
			// of Point and Location

// Main function
// Undefined symbol 'APoint'
int main()

{
   // set graphics mode
   int graphdriver = DETECT, graphmode;
   initgraph(&graphdriver, &graphmode, "..\\bgi");

   // show,move,hide a point across the screen
   Point APoint(50,50);   // Initial point A
   APoint.Show();           // show point A on screen
   getch();                 // Wait for keypress
   APoint.MoveTo(300, 150); // move the point to 620,150
   getch();                 // Wait for keypress
   APoint.Hide();           // hide point A
   getch();                 // Wait for keypress
   closegraph();            // Restore original screen
   return 0;                // Return 0 to main()
}
