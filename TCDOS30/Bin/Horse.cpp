// horse.cpp
// models a horse race

#include <iostream.h>
#include <dos.h>                    // for delay()
#include <conio.h>                  // for kbhit()
#include <stdlib.h>                 // for random()
#include <time.h>                   // for randomize()

const int CPF = 5;                  // screen columns per furlong

class horse
   {
   private:
      // track characteristics (declarations only)
      static horse* hptr;           // pointer to horse memory
      static int total;             // total number of horses
      static int count;             // horses created so far
      static int track_length;      // track length in furlongs
      static float elapsed_time;    // time since start of race

      // horse characteristics
      int horse_number;             // this horse's number
      float finish_time;            // this horse's finish time
      float distance_run;           // distance since start
   public:
      static void init_track(float l, int t); // initialize track
      static void create_horses()   // create horses
         {
         hptr = new horse[total];   // get memory for all horses
         }
      static void track_tick();     // time tick for entire track
      horse()                       // constructor for each horse
         {
         horse_number = count++;    // set our horse's number
         distance_run = 0.0;        // haven't moved yet
         }
      void horse_tick();            // time tick for one horse
   };

horse* horse::hptr;                 // define static (track) vars
int horse::total;
int horse::count = 0;
int horse::track_length;
float horse::elapsed_time = 0.0;

void horse::init_track(float l, int t)  // static (track) function
   {
   total = t;                       // set number of horses
   track_length = l;                // set track length
   randomize();                     // initialize random numbers
   clrscr();                        // clear screen
                                    // display track
   for(int f=0; f<=track_length; f++)    // for each furlong
      for(int r=1; r<=total*2 + 1; r++)  // for each screen row
         {
         gotoxy(f*CPF + 5, r);
         if(f==0 || f==track_length)
            cout << '\xDE';         // draw start or finish line
         else
            cout << '\xB3';         // draw furlong marker
         }
   }

void horse::track_tick()            // static (track) function
   {
   elapsed_time += 1.75;            // update time

   for(int j=0; j<total; j++)       // for each horse,
      (hptr+j)->horse_tick();       // update horse
   }

void horse::horse_tick()            // for each horse
   {                                // display horse & number
   gotoxy( 1 + int(distance_run * CPF), 2 + horse_number*2 );
   cout << " \xDB" << horse_number << "\xDB";
   if(distance_run < track_length + 1.0/CPF)  // until finish,
      {
      if( random(3) % 3 )           // skip about 1 of 3 ticks
         distance_run += 0.2;       // advance 0.2 furlongs
      finish_time = elapsed_time;   // update finish time
      }
   else
      {                             // display finish time
      int mins = int(finish_time)/60;
      int secs = int(finish_time) - mins*60;
      cout << " Time=" << mins << ":" << secs;
      }
   }

void main()
   {
   float length;
   int total;

   cout << "\nEnter track length (furlongs): ";
   cin >> length;
   cout << "\nEnter number of horses (1 to 10): ";
   cin >> total;
                             // initialize track
   horse::init_track(length, total);
   horse::create_horses();   // create horses
   while( !kbhit() )         // exit on keypress
      {
      horse::track_tick();   // move and display all horses
      delay(500);            // wait 1/2 second
      }
   }
