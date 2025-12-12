#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char current_path[80];

int cd(char dir_name[])
{

	int a=0, b=0, i;
	char temp[80], temp2[2], dir_now[80];
	int slashcounter;
	strcpy(dir_now, "");
	strcpy(dir_now, current_path);
	if(strcmp(dir_name, "..")==0)
	{
		slashcounter=0;
		strcpy(temp, "");
		for(i=0;i<=strlen(current_path);i++)
		{
		strcpy(temp2, "");
		strncat(temp2, current_path+i, 1);
		if(strcmp(temp2, "\\")==0)
		{
			slashcounter=i;
		}
		}
		strncat(temp, current_path, slashcounter);
		a=sys_close_dir();
		a=sys_open_dir(temp);
		if(a==0 & strcmp(current_path, "")!=0)
		{
		printf("Directory Changed Successfully\n");
		}
		if(a==0 & strcmp(current_path, "")==0)
		{
		printf("No Directories Currently Opened\n");
		a=sys_open_dir(dir_now);
		}
		if(a!=0)
                {
		printf("Change Directory Operation Can Not Be Performed\n");
		a=sys_open_dir(dir_now);
		}
		b=1;
	}
	
	if(strcmp(dir_name, "")==0 & b==0)
	{
		if(strcmp(current_path, "")!=0)
		{

			printf("Current Directory is %s\n", dir_now);
		}
		else
		{
			printf("No Directories Currently Opened!\n");
		}
	b=1;
	}
	if(strcmp(dir_name, "")!=0 & strcmp(dir_name, "..")!=0 & b==0)
        {
	sys_close_dir();
	a=sys_open_dir(dir_name);

	if(a==0)
	{
		printf("Directory Changed Successfully\n");
	}
	else
        {
		printf("Directory %s Does Not Exist\n", dir_name);
		a=sys_open_dir(dir_now);
	}
	}
return a;
}

int cdquiet(char dir_name[])
{

	int a=0, b=0, i;
	char temp[80], temp2[2], dir_now[80];
	int slashcounter;
	strcpy(dir_now, "");
	strcpy(dir_now, current_path);
	if(strcmp(dir_name, "..")==0)
	{
		slashcounter=0;
		strcpy(temp, "");
		for(i=0;i<=strlen(current_path);i++)
		{
		strcpy(temp2, "");
		strncat(temp2, current_path+i, 1);
		if(strcmp(temp2, "\\")==0)
		{
			slashcounter=i;
		}
		}
		strncat(temp, current_path, slashcounter);
		a=sys_close_dir();
		a=sys_open_dir(temp);
		if(a==0 & strcmp(current_path, "")!=0)
		{
		}
		if(a==0 & strcmp(current_path, "")==0)
		{
		a=sys_open_dir(dir_now);
		}
		if(a!=0)
                {
		a=sys_open_dir(dir_now);
		}
		b=1;
	}
	
	if(strcmp(dir_name, "")==0 & b==0)
	{
		if(strcmp(current_path, "")!=0)
		{
		}
		else
		{
		}
	b=1;
	}
	if(strcmp(dir_name, "")!=0 & strcmp(dir_name, "..")!=0 & b==0)
        {
	sys_close_dir();
	a=sys_open_dir(dir_name);

	if(a==0)
	{
	}
	else
        {
		a=sys_open_dir(dir_now);
	}
	}
return a;
}





int list()
{
char a[80];
int i,c, k=0;
long int *b;
strcpy(a, "");
printf("\nList of files\n\n");
printf("File Name\tSize\n\n");

do
{
c=sys_get_entry(a, 11, b);
if(c==0)
{
k++;
if(strlen(a)>=5)
{
printf("%s.MPX\t%ld\n", a, *b);
}
else
{
printf("%s.MPX\t\t%ld\n", a, *b);
}
}
}while(c==0);
printf("\nNumber of Entries:  %i\n\n", k);
cdquiet("..");
return 0;
}


