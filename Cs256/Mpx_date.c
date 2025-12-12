/* Created 02/04/2001 by Kenneth A. Heck				    */
/* This contains procedures mpx_date, show_date, change_date, char_to_long  */
/* and string_upper.							    */
/* It will provide all functionality necessary for viewing and updating     */
/* the system date.							    */

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "mpx_supt.h"
#include "mpx_date.h"

date_rec date_record;

/*** PROTOTYPES ***/
/* Provide necessary prototypes */
int show_date();
int change_date(int, int, int);
int char_to_long(char*, int, long*);
int string_upper(char*);
int mpx_date(char*);

int mpx_date(char *param)
{
/*** INITIALIZATIONS ***/
	/* Dimension and initialize all needed variables */
	char month[80], day[80], year[80];
	int change_date_error=0, char_to_long_error=0;
	int size_month=80, size_day=80, size_year=80;
	int valid_date=0;
	int valid_param=0;
	long lng_month=0, lng_day=0, lng_year=0;

	month[0]=month[1]=0;
	day[0]=day[1]=0;
	year[0]=year[1]=year[2]=year[3]=0;



	string_upper(param);

	/* Display the current date */
	if (strcmp(param,"")==0 || strcmp(param,"GET")==0)
	{
		printf("\nThe current system date is: ");
		show_date();
		valid_param=1;
	}


	if (strcmp(param,"SET")==0)
	{
		valid_param=1;
/*** MONTH ***/
		/* Display the prompt for the month */
		printf("\nEnter the month(1-12): ");

		/* Obtain the input for the new value of month */
		sys_req(READ,TERMINAL,month,&size_month);

/*** DAY ***/
		/* Display the prompt for the day */
		printf("\nEnter the day(1-31): ");

		/* Obtain the input for the new value of day */
		sys_req(READ,TERMINAL,day,&size_day);

/*** YEAR ***/
		/* Display the prompt for the year */
		printf("\nEnter the year(1000-3000): ");

		/* Obtain the input for the new value of year */
		sys_req(READ,TERMINAL,year,&size_year);

/*** INPUT SIZE REDUCTION ***/
		/* Reduce the size variable to a length that will fit within */
		/* a long.						     */
		size_month=9;
		size_day=9;
		size_year=9;

/*** INPUT CORRECTION ***/
		/* Change the character input to long values */
		char_to_long_error=char_to_long(month,size_month-1,&lng_month);
		char_to_long_error=char_to_long(day,size_day-1,&lng_day);
		char_to_long_error=char_to_long(year,size_year-1,&lng_year);
		if (char_to_long_error!=0)
		{
			printf("\nThere was an error on input conversion.  The date could not be set.\n\n");
		}

/*** INPUT CHECK ***/
		/* Do initial checks for date accuracy */
		if (lng_month>0 && lng_month<13)
		{
			/* January, March, May, July, August, October, December: 31 days */
			/* April, June, September, November: 30 days */
			/* February: 28 days, leap year: 29 days */
			if (((lng_month==1 || lng_month==3 || lng_month==5 || lng_month==7
				|| lng_month==8 || lng_month==10 || lng_month==12)
				&& (lng_day>0 && lng_day<32))
			|| ((lng_month==4 || lng_month==6 || lng_month==9 || lng_month==11)
				&& (lng_day>0 && lng_day<31))
			|| ((lng_month==2) && (lng_day>0 && lng_day<29))
			|| ((lng_month==2) && (lng_day>0 && lng_day<30 &&
				(lng_year%4==0 && (lng_year%100!=0 || lng_year%400==0)))))
			{
				if (lng_year>999 && lng_year<3001)
				{
					valid_date=1;
				}
				else
				{
					valid_date=0;
				}
			}
			else
			{
				valid_date=0;
			}
		}
		else
		{
			valid_date=0;
		}

/*** DATE CHANGE ***/
		if (valid_date==1)
		{
			/* Pass the integer input to the change date function */
			change_date_error=change_date(lng_month,lng_day,lng_year);
			if (change_date_error!=0)
			{
				printf("\nThere was an error changing the date.  The date could not be set.\n\n");
			}
		}
		else
		{
			printf("\nYou entered an invalid date.\n\n");
		}
	}

	if (valid_param==0)
	{
		printf("\nYou have entered an invalid date parameter.\n");
		printf("\nValid parameters are GET or SET.\n\n");
	}

/*** SUCCESS RETURN ***/
	/* Return a value of zero to indicate successful completion */
	return 0;
}

/*** FUNCTION TO SHOW THE CURRENT SYSTEM DATE ***/
int show_date()
{
	/* Call the provided sys_get_date to obtain the current system date */
	sys_get_date(&date_record);

	/* Display the date to the screen */
	printf("%i %i %i\n",date_record.month,date_record.day,date_record.year);

	/* Return zero to indicate successful completion of the function */
	return 0;
}


/*** FUNCTION TO CHANGE THE SYSTEM DATE TO THE DATE PASSED ***/
int change_date(int new_month, int new_day, int new_year)
{
	int change_date_error=0;

	/* Insert the passed values into the date structure before making the update */
	date_record.month=new_month;
	date_record.day=new_day;
	date_record.year=new_year;

	/* Update the date utilizing the provided sys_set_date function */
	change_date_error=sys_set_date(&date_record);
	if (change_date_error!=0)
	{
		printf("\nThere was an error changing the date.  The date could not be set.\n\n");
	}

	/* Display the date to show the user that the new date has been accepted */
	printf("\nThe system date has been updated to: ");
	show_date();

	/* Return zero to indicate successful completion of the function */
	return 0;
}


/*** FUNCTION TO CHANGE AN ARRAY OF CHARACTER NUMBERS INTO A SINGLE LONG ***/
int char_to_long(char *in_char, int in_char_length, long *out_lng)
{
	int counter=0, new_in_char_length=0;

	/* Loop through the number of char records until a carriage return (ASCII 10) is found */
	for(counter=0;counter<in_char_length;counter++)
	{
		/* Check for new_in_char_length==0 to make sure it obtains */
		/* the first and only the first carriage return.	   */
		if ((int)in_char[counter]==10 && new_in_char_length==0)
		{
			new_in_char_length=counter;
		}
	}

	/* Loop through the number of char records that need to be concatenated into a single long */
	for(counter=0;counter<new_in_char_length;counter++)
	{
		/* out_int is the final converted integer */
		/* 48 is subtracted to account for the ascii offset */
		/* Since each character occupies one place in the decimal system, it is raised to the */
		/*    power of 10 that matches its original position.  This position is obtain by     */
		/*    taking the total number of records, subtracting the current counter and then    */
		/*    subtracting one.						      		      */
		if ((int)in_char[counter]>47 && (int)in_char[counter]<58)
		{
			*out_lng=*out_lng+(((int)(in_char[counter])-48)*(pow10(new_in_char_length-counter-1)));
		}
	}

	/* Return zero to indicate successful completion of the function */
	return 0;
}

int string_upper(char *in_char)
{
	int counter=0, string_length=0;

	string_length = strlen(in_char);
	for (counter=0; counter<string_length; counter++)
	{
		in_char[counter] = toupper(in_char[counter]);
	}

	return 0;
}