/*  Matrix Encryption by Jason Hall */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define repconst 32

int simple_matrix_in( char *,  char *, int x,  char[], int);
int simple_matrix_out( char *,  char *, int x,  char[], int);
int xor_next_byte_with_prev(char *inmatrix);


void main(void)
{
char infile[256], outfile[80], sc[256], out[256], password[256], pass[256], data[256];
FILE *input;
FILE *output;
int i, j;
printf("Welcome to Jason Hall's decrypt program\n\n");

strcpy(data, "");
printf("input:  ");
scanf("%s", infile);
printf("output:  ");
scanf("%s", outfile);
printf("password:  ");
strcpy(pass, "");
strcpy(password, "");
scanf("%s", pass);
input=fopen(infile, "rb");
output=fopen(outfile, "wb");
simple_matrix_in(pass, password, 256, data, 128);
strcpy(sc, "");
do
{
        j=0;
        do
        {
                fgets(sc+j, 2, input);
                //past=sc[j]-past;
                j++;
        }while(j<256 && !feof(input));
        if(j==256)
        {
                xor_next_byte_with_prev(out);
                for(i=0;i<=repconst;i++)
                {
                        simple_matrix_out(sc, out, 256, password,i);
                        strcpy(pass, password);
                        simple_matrix_in(pass, password, 256, pass, i);
                }
                for(i=0;i<=255;i++)
                {
                        fputc(out[i]/*-password[255-i])^0xc5*/, output);
                        //fputc((out[i]-i-pass[i])^0x66, output);

                }
        }
        else
        {
        xor_next_byte_with_prev(out);

                for(i=0;i<=j-2;i++)
                {
                                fputc((sc[i]-i-pass[i])^0x66, output);
                }
        }
}while(!feof(input));
fclose(input);
fclose(output);
}

int simple_matrix_in( char *inmatrix,  char *outmatrix, int x,  char init_offset[], int init)
{
        unsigned char *used;
        int i, cater=0;
        unsigned char offset=init_offset[init];
        strcpy(outmatrix, "");
        used=(unsigned char *) malloc(x);

        for(i=0;i<=(x)-1;i++)
        {
                used[i]=0;
        }
        do
        {
                if(used[offset]==0)
                {

                        //printf("A");
                        outmatrix[offset]=(inmatrix[cater]^0xff)+init_offset[cater];
                        used[offset]=1;
                        offset=inmatrix[cater]^0xff/*+offset*/;
                        offset=offset+offset+init_offset[cater];
                        cater++;
                }
                else
                {
                        if(used[(offset^0xff)]==0)
                        {
                                //printf("B");
                                outmatrix[(offset^0xff)]=inmatrix[cater]/*+init_offset[cater]^0xff*/;

                                used[(offset^0xff)]=1;
                                offset=inmatrix[cater]/*+offset*/;
                                offset=offset+offset+init_offset[cater];
                                cater++;
                        }
                        else
                        {
                                //printf("C");
                                offset++;
                                //printf("offset %i\n", offset);
                        }
                }
        }while(cater<(x));
//        fclose(log);
        free(used);
return 0;
}

int simple_matrix_out( char *inmatrix,  char *outmatrix, int x,  char init_offset[], int init)
{
        unsigned char *used;

        int i, cater=0;
        unsigned char offset=init_offset[init];

        strcpy(outmatrix, "");
        used=(unsigned char *) malloc(x);

        for(i=0;i<=(x)-1;i++)
        {
                used[i]=0;
        }
        do
        {
                if(used[offset]==0)
                {

                        outmatrix[cater]=((inmatrix[offset]^0xff)+init_offset[cater]);

                        used[offset]=1;
                        offset=inmatrix[offset]/*+offset*/;
                        offset=(offset+offset-init_offset[cater]^0xff)^0xff;
                        cater++;



                        /*outmatrix[offset]=(inmatrix[cater]^0xff)+init_offset[cater];
                        fprintf(log, "%i a %i %i\n", offset, inmatrix[cater], inmatrix[cater]^0xff);
                        used[offset]=1;
                        offset=inmatrix[cater]^0xff/*+offset;
                        offset=offset+offset+init_offset[cater];
                        cater++;                           */


                }
                else
                {
                        if(used[(offset^0xff)]==0)
                        {

                                outmatrix[cater]=inmatrix[offset^0xff]/*+init_offset[cater]*/;
                                //fprintf(log, "%i b %i %i\n", offset^0xff, outmatrix[offset^0xff], outmatrix[offset^0xff]^0xff);

                                used[(offset^0xff)]=1;
                                offset=inmatrix[offset^0xff];
                                //offset=(offset+offset+init_offset[cater]^0xff);
                                offset=offset+offset+init_offset[cater];

                                //offset=(offset+offset+init_offset[cater])^0xff;

                                cater++;
                        }
                        else
                        {
                                offset++;
                        }
                }
        }while(cater<(x));
        free(used);
//        fclose(log2);
        return 0;
}

int xor_next_byte_with_prev(char *inmatrix)
{
        int j;
        char byte1=inmatrix[255];
        for(j=0;j<=254;j++)
        {
             inmatrix[j]=inmatrix[j]^byte1;
             byte1=inmatrix[j];

        }
return 0;
}

