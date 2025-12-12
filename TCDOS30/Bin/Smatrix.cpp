//SMATRIX.CPP  Main program to test the matrix class.
//Written by Jenny and Xueming.

#include <iostream.h>
#include "matrix.h"

void main()
{
   double x;
   int row, col;

   Sparse_matrix m(5,5);             //Object m of Sparse_matrix.
   if(m.put_element(2.5,1,2)==0)     //Put 2.5 in 2nd row, 3rd column.
   cout << "An item was put in 2nd row, 3rd column.\n";
   m.get_element(x, 1, 2);           //Get an element.
   cout<<"The item in 2nd row, 3rd column is: "<<x<<endl<<endl;  //Show.

   m.put_element(0.0,2,3);           //Try to put a zero in the matrix.
   m.put_element(3.5,6,7);           //Try to put an item out of range.
   m.delete_element(6, 7);           //Try to delete an item out of range.

   if(m.delete_element(1, 2)==0)     //Delete an element.
   cout <<endl<<"The item in 2nd row, 3rd column was deleted.\n";
   m.Sparse_matrix::~Sparse_matrix();
}
