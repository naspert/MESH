/* $Id: final2.c,v 1.10 2001/04/10 13:16:03 jacquet Exp $ */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

typedef struct {
  double x;
  double y;
  double z;
}vertex;

typedef struct {
  int f0;
  int f1;
  int f2;
  vertex min,max;
}face;

typedef struct {
  int nbfaces;
  int nbvertex;
  vertex* vertices;
  vertex BBOX[2];
  face* faces;
} model;

typedef struct {
  vertex* sample;
  int nbsamples;
}sample;

typedef struct {
  int *cube;
  int nbcube;
}cellules;

/* Computes the norm of vector v */
double norm(vertex v) {
  return (sqrt(v.x*v.x + v.y*v.y + v.z*v.z));
}

/* Computes the distance between v1 and v2 */
 double dist(vertex v1, vertex v2) {
  vertex tmp;
  
  tmp.x = v1.x - v2.x;
  tmp.y = v1.y - v2.y;
  tmp.z = v1.z - v2.z;

  return norm(tmp);
  
}

/****************************************************************************/
/*         fonctions qui retournent le max ou le min entre 3 points         */
/****************************************************************************/

int max(int a,int b,int c)
{
int max=a;

if (b>c && b>a)
  max=b;
if (c>b && c>a)
  max=c;

return(max);
}


int min(int a,int b,int c)
{
int min =a;

if(b<a && b<c)
  min=b;
else if (c<a && c<b)
  min=c;

return(min);
}


/****************************************************************************/
/*                lecture des fichiers de donnees                           */
/****************************************************************************/
model* readfile(FILE *f)
{
  model *raw_model;
  double x,y,z;
  int v0,v1,v2;
  int nbvertex,nbfaces,i;

  fscanf(f,"%d %d",&nbvertex,&nbfaces);

  raw_model = (model*)malloc(sizeof(model));
  raw_model->nbfaces = nbfaces;
  raw_model->nbvertex = nbvertex;
  raw_model->faces = (face*)malloc(nbfaces*sizeof(face));
  raw_model->vertices = (vertex*)malloc(nbvertex*sizeof(vertex));

  for (i=0; i<nbvertex; i++) {
    fscanf(f,"%lf %lf %lf",&x, &y, &z);
    raw_model->vertices[i].x = 1.0*x;
    raw_model->vertices[i].y = 1.0*y;
    raw_model->vertices[i].z = 1.0*z;
    if (i==0) {
      raw_model->BBOX[0]=raw_model->vertices[0];
      raw_model->BBOX[1]=raw_model->vertices[0];
    }
    if (raw_model->vertices[i].x > raw_model->BBOX[1].x) 
      raw_model->BBOX[1].x = raw_model->vertices[i].x;
    else if (raw_model->vertices[i].x < raw_model->BBOX[0].x)
      raw_model->BBOX[0].x = raw_model->vertices[i].x;

    if (raw_model->vertices[i].y > raw_model->BBOX[1].y) 
      raw_model->BBOX[1].y = raw_model->vertices[i].y;
    else if (raw_model->vertices[i].y < raw_model->BBOX[0].y)
      raw_model->BBOX[0].y = raw_model->vertices[i].y;

    if (raw_model->vertices[i].z > raw_model->BBOX[1].z) 
      raw_model->BBOX[1].z = raw_model->vertices[i].z;
    else if (raw_model->vertices[i].z < raw_model->BBOX[0].z)
      raw_model->BBOX[0].z = raw_model->vertices[i].z;
  }
  
  for (i=0; i<nbfaces; i++) {
    fscanf(f,"%d %d %d",&v0, &v1, &v2);
    raw_model->faces[i].f0 = v0;
    raw_model->faces[i].f1 = v1;
    raw_model->faces[i].f2 = v2;

    raw_model->faces[i].min.x=min(raw_model->vertices[raw_model->faces[i].f0].x,
				  raw_model->vertices[raw_model->faces[i].f1].x,
				  raw_model->vertices[raw_model->faces[i].f2].x);
    
    raw_model->faces[i].min.y=min(raw_model->vertices[raw_model->faces[i].f0].y,
				  raw_model->vertices[raw_model->faces[i].f1].y,
				  raw_model->vertices[raw_model->faces[i].f2].y);
    
    raw_model->faces[i].min.z=min(raw_model->vertices[raw_model->faces[i].f0].z,
				  raw_model->vertices[raw_model->faces[i].f1].z,
				  raw_model->vertices[raw_model->faces[i].f2].z);

    raw_model->faces[i].max.x=max(raw_model->vertices[raw_model->faces[i].f0].x,
				  raw_model->vertices[raw_model->faces[i].f1].x,
				  raw_model->vertices[raw_model->faces[i].f2].x);
    raw_model->faces[i].max.y=max(raw_model->vertices[raw_model->faces[i].f0].y,
				  raw_model->vertices[raw_model->faces[i].f1].y,
				  raw_model->vertices[raw_model->faces[i].f2].y);
    raw_model->faces[i].max.z=max(raw_model->vertices[raw_model->faces[i].f0].z,
				  raw_model->vertices[raw_model->faces[i].f1].z,
				  raw_model->vertices[raw_model->faces[i].f2].z);

  }
return(raw_model);
}


/*****************************************************************************/
/*               fonction qui echantillonne un triangle                      */
/*****************************************************************************/

sample* echantillon(vertex a, vertex b, vertex c,double k)
{
  int h=0,nbsamples;
  double i,j;
  vertex l1,l2;
  sample *sample1;

  if((sample1=(sample*)malloc(sizeof(sample)))==NULL){
  printf("impossible d'allouer de la memoire");
  exit(-1);
  }

  l1.x=b.x-a.x;
  l1.y=b.y-a.y;
  l1.z=b.z-a.z;

  l2.x=c.x-a.x;
  l2.y=c.y-a.y;
  l2.z=c.z-a.z;

  nbsamples=(1+1/k)*(1+1/k)/2+1+1/k; 
   
  if((sample1->sample=(vertex*)malloc(sizeof(vertex)))==NULL){
    printf("impossible d'allouer de la memoire");
    exit(-1);
  }
  for (i=0;i<=1;i+=k) {
    for (j=0;j<=1;j+=k) {
      if (i+j<1.000001) {
	if(h>0)
	  sample1->sample=(vertex*)realloc(sample1->sample,(h+1)*sizeof(vertex));
        sample1->sample[h].x=a.x+i*l1.x+j*l2.x;
        sample1->sample[h].y=a.y+i*l1.y+j*l2.y;
        sample1->sample[h].z=a.z+i*l1.z+j*l2.z;
        h++;
	}
    }
  }
  sample1->nbsamples=h;
  
  return(sample1);

}

/****************************************************************************/
/* fonction qui repertorie pour chaque face les cellules avec lesquelles    */
/*     elle a une intersection                                              */
/****************************************************************************/
vertex** liste(model *raw_model,double samplethin)
{
vertex **groupts;
int i,j,k,m,n,o,cellule;
sample *sample1;
vertex A,B,C,bbox0,bbox1,test;
int mem[1000][1]={0};

groupts=(vertex**)malloc(1000*sizeof(vertex*));

bbox0=raw_model->BBOX[0];
bbox1=raw_model->BBOX[1];

test.x=bbox0.x-1;
test.y=0;
test.z=0; 


 for(i=0;i<raw_model->nbfaces;i++){

   A=raw_model->vertices[raw_model->faces[i].f0];
   B=raw_model->vertices[raw_model->faces[i].f1];
   C=raw_model->vertices[raw_model->faces[i].f2];

   sample1=echantillon(A,B,C,samplethin);

   for(j=0;j<sample1->nbsamples;j++){

     m=(sample1->sample[j].x-bbox0.x)*10/(bbox1.x-bbox0.x);
     n=(sample1->sample[j].y-bbox0.y)*10/(bbox1.y-bbox0.y);
     o=(sample1->sample[j].z-bbox0.z)*10/(bbox1.z-bbox0.z);
     
     if(m==10)
       m=9;
     if(n==10)
       n=9;
     if(o==10)
       o=9;

     cellule=m+n*10+o*100;
     
     if(mem[cellule][0]==0)
       groupts[cellule]=NULL;
     groupts[cellule]=(vertex*)realloc(groupts[cellule],
				       (mem[cellule][0]+1)*sizeof(vertex));
     groupts[cellule][mem[cellule][0]]=sample1->sample[j];
     mem[cellule][0]++;

   }

   free(sample1->sample);
   free(sample1);   
 }

 for(i=0;i<1000;i++){
   if(mem[i][0]==0)
     groupts[i]=NULL;
   groupts[i]=(vertex*)realloc(groupts[i],(mem[i][0]+1)*sizeof(vertex));
   groupts[i][mem[i][0]]=test;
 }
return groupts;
}




/*****************************************************************************/
/*                fonction qui calcule la plus courte distance d'un          */
/*                       point a une surface                                 */
/*****************************************************************************/

double pcd(vertex point,model *raw_model2, double k,vertex **groupts)
{
double d,dmin;
int m,n,o,i=0,j,cellule,mem;
sample *sample1;
vertex bbox0,bbox1;
int a,b,c;

bbox0=raw_model2->BBOX[0];
bbox1=raw_model2->BBOX[1];

m=(point.x-bbox0.x)*10/(bbox1.x-bbox0.x);
n=(point.y-bbox0.y)*10/(bbox1.y-bbox0.y);
o=(point.z-bbox0.z)*10/(bbox1.z-bbox0.z);

if(m==10)
  m=9;
if(n==10)
  n=9;
if(o==10)
  o=9; 
cellule=m+n*10+o*100;

d=dist(point,groupts[cellule][0]);
dmin=d;

 for(c=o-1;c<=o+1;c++){ 
   for(b=n-1;b<=n+1;b++){
     for(a=m-1;a<=m+1;a++){
       
       cellule=a+b*10+c*100;
       j=0;
       if(cellule>=0 && cellule<1000){
	 while(groupts[cellule][j].x>bbox0.x-1){
	   
	   d=dist(point,groupts[cellule][j]);
	   
	   if(d<dmin)
	     dmin=d;
	   j++;
	 }       
       } 
     }
   }
 }
 

/*printf("nb face test: %d;dmin: %lf\n",h,dmin);*/    
/*printf("%lf\n ",dmin);*/
return(dmin);  
}

/****************************************************************************/
int main(int argc,char **argv)
{
FILE *f1,*f2;
sample *sample2;
model* raw_model1;
model* raw_model2;
cellules *cell;
double samplethin,diag,diag2,dcourant,dmax=0,superdmax=0;
int i,j;
vertex bbox0,bbox1;
vertex **groupts;

 if (argc!=4) {
   printf("nbre d'arg incorrect\n");
   exit(-1);
 }

 if((f1=fopen(argv[1],"r"))==NULL){
   printf("impossible d'ouvrir fichier1\n");
   exit(-1);
 }
 if((f2=fopen(argv[2],"r"))==NULL){
   printf("impossible d'ouvrir fichier2\n");
   exit(-1);
 }

samplethin=atof(argv[3]);

raw_model1=readfile(f1);
fclose(f1);
raw_model2=readfile(f2);
fclose(f2);

bbox0=raw_model1->BBOX[0];
bbox1=raw_model1->BBOX[1];

printf("%lf %lf %lf\n",bbox0.x,bbox0.y,bbox0.z);
printf("%lf %lf %lf\n",bbox1.x,bbox1.y,bbox1.z);

bbox0=raw_model2->BBOX[0];
bbox1=raw_model2->BBOX[1];

printf("%lf %lf %lf\n",bbox0.x,bbox0.y,bbox0.z);
printf("%lf %lf %lf\n",bbox1.x,bbox1.y,bbox1.z);


groupts=liste(raw_model2,samplethin);

diag=dist(raw_model1->BBOX[0],raw_model1->BBOX[1]);
diag2=dist(raw_model2->BBOX[0],raw_model2->BBOX[1]);

printf("diagBBOX: %lf\n",diag);
printf("diagBBOX2: %lf\n",diag2);

 for(i=0;i<raw_model1->nbfaces;i++) {
   sample2=echantillon(raw_model1->vertices[raw_model1->faces[i].f0],
		       raw_model1->vertices[raw_model1->faces[i].f1],
		       raw_model1->vertices[raw_model1->faces[i].f2],
		       samplethin); 
   for(j=0;j<sample2->nbsamples;j++){
     dcourant=pcd(sample2->sample[j],raw_model2,samplethin,groupts);
     if(dcourant>dmax)
       dmax=dcourant;
   }
   printf("face numero %d: dmax= %lf\n",i+1,dmax);
   if(dmax>superdmax)
     superdmax=dmax;
   dmax=0;

   free(sample2->sample);

   free(sample2);
    
 }
printf("distance maximale: %lf\n",superdmax);
return 0;
}



