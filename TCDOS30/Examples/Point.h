// Marcela Ciupe, Vijayanand Bharadwaj
// Part 2  -  Modified POINT.H

// Borland C++ - (C) Copyright 1991 by Borland International

/* point.h--Example from Getting Started */

// point.h contains two classes:
// class Location describes screen locations in X and Y coordinates
// class Point describes whether a point is hidden or visible

enum Boolean {false, true};

class Location {
protected:          // allows derived class to access private data
   int X;
   int Y;

public:             // these functions can be accessed from outside
   Location(int InitX, int InitY);   // change function "Location" with "location"
   int GetX();
   int GetY();
};              // semicolon deleted at the end of the class Location
class Point : public Location {      // derived from class Location
// public derivation means that X and Y are protected within Point

protected:
   Boolean Visible;  // classes derived from Point will need access

public:
   Point(int InitX, int InitY);      // constructor
			     // delete the "int" keyword located
			     // in front of each argument
   void Show();
   void Hide();
   Boolean IsVisible();
   void MoveTo(int NewX, int NewY);
};
