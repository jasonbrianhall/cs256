//MATRIX.H  Sparse_matrix class definition.
//Written by Jenny and Xueming.

#include <iostream.h>

class Sparse_matrix            //Define a sparse matrix.
{
	  int no_rows, no_cols;    //Size of the sparse matrix.
	  struct element {
			 double value;     //Nonzero element value.
			 int col_no;       //The element column position.
			 element * Next;   //Pointer to next nonzero element.
			 };
	  element **matrix;        //Pointer to the matrix.
 public:
	  Sparse_matrix(int, int); //Constructor.
	  ~Sparse_matrix();        //Destructor.
	  int put_element(double, int, int); //Put a value into the specific
										 //position in the matrix.
	  int get_element(double&, int ,int); //Get an element value from the
										 //specific position in the matrix.
	  int delete_element(int, int);      //Delete an element in the matrix.
};