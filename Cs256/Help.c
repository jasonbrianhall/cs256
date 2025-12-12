/*  Help.c by Jason Hall

Takes in a string, returns 1 if help.dat does not exist, returns 0 if everything
is just perfect and 2 if file is found but
no help topics on intended subject.  3=list of topics*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int help(char input_string[])
{
	FILE *help_file;
	char single_char[2];
	char long_char[80], temp[80];
	int returned, counter;
	if((help_file=fopen("c:\\cs256\\help.dat", "r"))==NULL)
	{
		returned=1;
	}
	else
	{
		help_file=fopen("c:\\cs256\\help.dat", "r");
		/*  Assures Beginning of File -- Not Needed   */
		returned=2;
		fseek(help_file, 0, SEEK_SET);
		if(strcmp(input_string, "LIST")==0)
		{
			printf("\n");
			printf("Command List is as follows:\n\n");
			returned=3;
			counter=0;
		}
		do
		{
			if(strcmp(input_string, "LIST")==0)
			{
				do
				{
					strcpy(single_char, "");
					fgets(single_char, 2, help_file);
				} while(!feof(help_file) & strcmp(single_char, ":")!=0);
				counter++;
				if(counter==7)
				{
				printf("\tPress enter to continue");
				getch();
				counter=0;
				printf("\n\n");
				}
				strcpy(temp, "");
				fgets(temp, 79, help_file);
				sscanf(temp, "%s", temp);
				if(strcmp(temp, "")!=0)
					printf("%s:\n", temp);
				strcpy(temp, "");
				fgets(temp, 79, help_file);
				printf("\t%s\n", temp);

			}
			else
			{
				do
				{
                                        strcpy(single_char, "");
                                        fgets(single_char, 2, help_file);

                                } while(!feof(help_file) & strcmp(single_char, ":")!=0);

                                        strcpy(long_char, "");
                                        strcpy(temp, "");
                                        fgets(temp, 79, help_file);
					/*  I'm Trying to avoid using FSCANF, would be a lot easier */
                                        sscanf(temp, "%s", long_char);
                                        if(strcmp(input_string, long_char)==0)
                                        {
                                        /* Gets and Ignores Description   */
                                        strcpy(temp, "");
                                        fgets(temp, 79, help_file);
					printf("Description of %s:\n\n", input_string);

                                        do
                                        {
					fgets(single_char, 2, help_file);
                                                if(!feof(help_file) & strcmp(single_char, ":")!=0)
                                                {

                                                        printf("%s", single_char);
                                                        returned=0;
                                                }
                                        }while(!feof(help_file) & strcmp(single_char, ":")!=0);
                                }
                        }
                }while(!feof(help_file));
        fclose(help_file);
        }

return returned;
}
