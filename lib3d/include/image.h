/* $Id: image.h,v 1.1 2001/03/12 14:50:32 aspert Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <limits.h>

#ifndef _IMAGE
#define _IMAGE

/* This structure defines an image containing usigned char data
   It is mainly used for PGM/PPM input/output */
typedef struct
{
  unsigned char*** data;/*3D array containing the image data */
  /* The 1st dimension is for the component number
     The 2nd dimension is for the line number
     The 3rd dimension is for the column number*/
  int width;
  int height;/* These 2 parameters keep track of the size of the image */
  int nc;/* Number of components in the image */
  int max_val;/* Max value in the image. Only used to write PGM/PPM format*/
}image_uchar;


/* This structure defines an image containing int data
   All the comments made for image_uchar apply to image_int
   It is mainly used when performing the wavelet transforms
   since its output produces some negative coefficients */
typedef struct
{
  int ***data;
  int width;
  int height;
  int nc;
  int max_val;
}image_int;

/* Some useful template declarations follow */

/*Destructors*/
void free_image_uchar(image_uchar*);
void free_image_int(image_int*);

/*allocation+IO images*/
image_uchar* image_uchar_read(FILE*);
void image_uchar_write(image_uchar*,FILE*);
image_uchar* image_uchar_alloc(int,int,int,int);
image_int* image_int_alloc(int,int,int);
image_int* image_int_extract(image_int*,int,int,int,int);
image_int* image_int_extract_uchar(image_uchar*,int,int,int,int);
void image_int_put(image_int*,image_int*,int,int,int,int);

#endif
