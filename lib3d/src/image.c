/* $Id: image.c,v 1.1 2001/03/12 14:50:32 aspert Exp $ */
#include "image.h"


/* Pseudo - destructors of images
   These functions free the 3D arrays of the image_int
   or image_uchar */
void free_image_uchar(image_uchar* image)
{
  int i;
  for (i=0;i<image->nc;i++)
    {
      free(image->data[i][0]);
      free(image->data[i]);
    }
  free(image->data);
  free(image);
}


void free_image_int(image_int* image)
{
  int i;
   for (i=0;i<image->nc;i++)
    {
      free(image->data[i][0]);
      free(image->data[i]);
    }
  free(image->data);
  free(image);
}


/* Read and allocates the memory for the image in "in_file"
   nc can only be 1 and 3 (number of components in the image
   This function reads a file in "binary" PGM or PPM format
   and returns a pointer to the image_uchar */
image_uchar* image_uchar_read(FILE *in_file)
{
  image_uchar *image;
  unsigned char ***data;
  unsigned char *line_buffer;
  int width,height,nc,max_val;
  int i,chkerr,j,k;
  char buffer[100]; /*temp buffer for reading the header of the file*/
  char imtype;

  fscanf(in_file,"%s",buffer);
 
  assert(buffer[0]=='P'&&(buffer[1]=='5'||buffer[1]=='6'));

  imtype=buffer[1];
  fscanf(in_file,"%s",buffer);
  while (buffer[0]=='#') { /* Ignore comment lines */
   fgets(buffer,100,in_file);
   fscanf(in_file,"%s",buffer);
  }
  width=atoi(buffer); /*No more comments -> read size*/
 

  fscanf(in_file,"%s",buffer);
  height=atoi(buffer);


  fscanf(in_file,"%s\n",buffer); 
  while(buffer[0]=='#'){ /*Ignore comments located after size tags*/
    fgets(buffer,100,in_file);
    fscanf(in_file,"%s\n",buffer);
  }
  max_val=atoi(buffer);

  if (max_val>255)
    {
      fprintf(stderr,"Unsupported maximum pixel value\n");
      exit(0);
    }
  image=(image_uchar*)malloc(sizeof(image_uchar));

 /* Memory allocation */
  /* The line/column allocation is done so that a pixel value
     can be accessed by data[comp_number][i][j] and a whole line
     of the image  can be accessed by data[comp_number][j] */
  if (imtype=='5') /* PGM file */
    {
      nc=1;
      line_buffer=(unsigned char*)malloc(width*sizeof(unsigned char));
      data=(unsigned char***)malloc(sizeof(unsigned char**));/*Only one component*/
      assert(data!=(unsigned char***)0);

      data[0]=(unsigned char**)malloc(height*sizeof(unsigned char*));
      assert(data[0]!=(unsigned char**)0);

      data[0][0]=(unsigned char*)malloc(width*height*sizeof(unsigned char));
      assert(data[0][0]!=(unsigned char*)0);
      for(i=1;i<height;++i) /* Load adresses of each line */
	data[0][i]=&(data[0][0][i*width*sizeof(unsigned char)]);
    }
  else /*PPM file*/
    {
      nc=3;
      line_buffer=(unsigned char*)malloc(3*width*sizeof(unsigned char));
      assert(line_buffer!=(unsigned char*)0);
      data=(unsigned char***)malloc(3*sizeof(unsigned char**));
      assert(data!=(unsigned char***)0);
      for (i=0;i<3;i++)
	{
	  data[i]=(unsigned char**)malloc(height*sizeof(unsigned char*));
	  assert(data[i]!=(unsigned char**)0);
	  data[i][0]=(unsigned char*)malloc(width*height*sizeof(unsigned char));
	  assert(data[i][0]!=(unsigned char*)0);
	  
	   for(j=1;j<height;++j) /* Load adresses of each line  */
 	    data[i][j]=&(data[i][0][j*width*sizeof(unsigned char)]); 
	  
	}
    }
  /* End of memory allocation */

  /*Reading unit*/
  for(i=0;i<height;i++)
    {
      chkerr=fread(line_buffer,sizeof(unsigned char),nc*width,in_file);
      assert(chkerr==nc*width); 
      /* if (chkerr!=nc*width) {
	fprintf (stderr, "Line %i/%i\n", i+1, height);
	fprintf (stderr, "#=%i/%i\n", chkerr, nc*width);
	exit (1);
	} */
      for (j=0;j<width;j++)
	for(k=0;k<nc;k++)
	  data[k][i][j]=line_buffer[j*nc+k];
    }
  free(line_buffer);
  /*End of reading unit*/

  /*Allocating structure */
  image->width=width;
  image->height=height;
  image->nc=nc;
  image->max_val=max_val;
  image->data=data;
  return(image);
}/* image_read */

/* Writes an image in out_file. nc must be 1 or 3 and defines
   the format of the written image
   This function also writes PPM or PGM header  */
void image_uchar_write(image_uchar *image,FILE *out_file)
{
  int chkerr,i,j,k;
  unsigned char *line_buffer;

  line_buffer=(unsigned char*)malloc(image->nc*image->width*sizeof(unsigned char));
  assert(line_buffer!=(unsigned char*)0);
  /*Writing unit*/
  /*Writing the header*/
  assert((image->nc==1)||(image->nc==3));
  if (image->nc==1) /*PGM header*/
    fprintf(out_file,"P5\n");
  else /*PPM header*/
    fprintf(out_file,"P6\n");
 
  
  assert(image->max_val<=255);
  
  fprintf(out_file,"%d %d\n",image->width,image->height); /*Size info*/
  fprintf(out_file,"%d\n",image->max_val); /*Max val info*/

  /*Data writing*/
  for (i=0;i<image->height;i++)
    {
      for (j=0;j<image->width;j++)
	for (k=0;k<image->nc;k++)
	  line_buffer[j*image->nc+k]=image->data[k][i][j];

      chkerr=fwrite(line_buffer,sizeof(unsigned char),image->width*image->nc,out_file);
      assert(chkerr==(image->nc*image->width));
    }
  free(line_buffer);
  /*End of writing unit*/
}/* image_write */


/* This function allocates a image_uchar structure with the provided 
   parameters and returns a pointer to the allocated image */
image_uchar* image_uchar_alloc(int width,int height,int nc,int max_val)
{
  unsigned char ***data;
  int i,j;
  image_uchar *image;

  image=(image_uchar*)malloc(sizeof(image_uchar));
  data=(unsigned char***)malloc(nc*sizeof(unsigned char**));
  assert(data!=(unsigned char***)0);
  for (i=0;i<nc;i++)
    {
      data[i]=(unsigned char**)malloc(height*sizeof(unsigned char*));
      assert(data[i]!=(unsigned char**)0);

      data[i][0]=(unsigned char*)malloc(width*height*sizeof(unsigned char));
      assert(data[i][0]!=(unsigned char*)0);

      for(j=1;j<height;++j) /* Load adresses of each line */
	data[i][j]=&(data[i][0][j*width*sizeof(unsigned char)]);
    }
  image->data=data;
  image->width=width;
  image->height=height;
  image->nc=nc;
  image->max_val=max_val;
  return(image);
}

/* This function allocates a image_int structure with the provided 
   parameters and returns a pointer to the allocated image */
image_int* image_int_alloc(int width,int height,int nc)
{
  int ***data,i,j;
  image_int *image;

  image=(image_int*)malloc(sizeof(image_int));
  assert(image!=(image_int*)0);
  data=(int***)malloc(nc*sizeof(int**));
  assert(data!=(int***)0);

  for (i=0;i<nc;i++)
    {
      data[i]=(int**)malloc(height*sizeof(int*));
      assert(data[i]!=(int**)0);
      data[i][0]=(int*)malloc(width*height*sizeof(int));
      assert(data[i][0]!=(int*)0);

      for(j=1;j<height;++j) /* Load adresses of each line */
	data[i][j]=&(data[i][0][j*width*sizeof(unsigned char)]);
    }

  image->data=data;
  image->width=width;
  image->height=height;
  image->nc=nc;
  return(image);
}

/* This function copies the part of an image_uchar, starting at (xoff,yoff)
   and of size width*height into an image_int. It returns a pointer
   to an image_int that is allocated in this function */
image_int* image_int_extract_uchar(image_uchar *image,int width,int height,int xoff, int yoff)
{
  image_int *out_image;
  int i,j,k;

  out_image=image_int_alloc(width,height,image->nc);

  for (i=0;i<image->nc;i++)
    for (j=0;j<height;j++)
      for(k=0;k<width;k++)
 	  out_image->data[i][j][k]=(int)image->data[i][j+yoff][k+xoff];
 
  return(out_image);
  
}

/* This function copies the part of an image_int, starting at (xoff,yoff)
   and of size width*height into an image_int. It returns a pointer
   to an image_int that is allocated in this function */
image_int* image_int_extract(image_int *image,int width,int height,int xoff,int yoff)
{
  image_int* out_image;
  int i,j;

  out_image=image_int_alloc(width,height,image->nc);

  for (i=0;i<image->nc;i++)
    for (j=0;j<height;j++)
       memcpy(out_image->data[i][j],image->data[i][j+yoff]+xoff,width*sizeof(int)); 

  return(out_image);  
}

/* This function copy the part of an image_int, starting at (xoff,yoff) */
/* and of size width*height into an EXISTING image_int structure. */
void image_int_put(image_int *image_in,image_int* image_out,int width,int height,int xoff,int yoff)
{
  int i,j;

  for (i=0;i<image_in->nc;i++)
    for (j=0;j<height;j++)
      memcpy(image_out->data[i][j],image_in->data[i][j+yoff]+xoff,width*sizeof(int));
}

