#include <stdio.h>
#include <math.h>

typedef struct {
  double x;
  double y;
  double z;
}vertex;

typedef struct {
  int f0;
  int f1;
  int f2;
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
  }

  return(raw_model);
}


/*****************************************************************************/
/*               fonction qui echantillonne un triangle                      */
/*****************************************************************************/

sample* echantillon(vertex a, vertex b, vertex c,double k)
{
  int h=0,nbsamples;
  float i,j;
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
   
  if((sample1->sample=(vertex*)malloc(nbsamples*sizeof(vertex)))==NULL){
    printf("impossible d'allouer de la memoire");
    exit(-1);
  }

  for (i=0;i<=1;i=i+k) {
    for (j=0;j<=1;j=j+k) {
      if (i+j<=1) {
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



/*****************************************************************************/
/*                fonction qui calcule la plus courte distance d'un          */
/*                         a une surface                                     */
/*****************************************************************************/

double pcd(vertex point,model *raw_model2, double k)
{
double d,dmin;
sample *sample1;
int i,j;

 for(j=0;j<raw_model2->nbfaces;j++){
   
   sample1=echantillon(raw_model2->vertices[raw_model2->faces[j].f0],raw_model2->vertices[raw_model2->faces[j].f1],raw_model2->vertices[raw_model2->faces[j].f2],k);

   for(i=0;i<sample1->nbsamples;i++) {

     /*printf("%lf %lf %lf\n",point.x,point.y,point.z);
     printf("%lf %lf %lf\n",sample1->sample[i].x,sample1->sample[i].y,sample1->sample[i].z);
     */
     d=sqrt((point.x-sample1->sample[i].x)*(point.x-sample1->sample[i].x)+(point.y-sample1->sample[i].y)*(point.y-sample1->sample[i].y)+(point.z-sample1->sample[i].z)*(point.z-sample1->sample[i].z));

     /*printf("d= %lf\n",d);*/
     if(j==0)
       dmin=d;
     else if(d<dmin)
       dmin=d;
   }
   free(sample1->sample);
 }
 /*printf("dmin= %lf\n",dmin);*/
return(dmin);  
}

/*****************************************************************************/
/*                        MAIN:fct principale                                */
/*****************************************************************************/

main(int argc,char **argv)
{
FILE *f1,*f2;
sample *sample2;
model* raw_model1;
model* raw_model2;
double samplethin,diag,diag2,dcourant,dmax=0;
int i,j;

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

diag=sqrt((raw_model1->BBOX[0].x-raw_model1->BBOX[1].x)*(raw_model1->BBOX[0].x-raw_model1->BBOX[1].x)+(raw_model1->BBOX[0].y-raw_model1->BBOX[1].y)*(raw_model1->BBOX[0].y-raw_model1->BBOX[1].y)+(raw_model1->BBOX[0].z-raw_model1->BBOX[1].z)*(raw_model1->BBOX[0].z-raw_model1->BBOX[1].z));


diag2=sqrt((raw_model2->BBOX[0].x-raw_model2->BBOX[1].x)*(raw_model2->BBOX[0].x-raw_model2->BBOX[1].x)+(raw_model2->BBOX[0].y-raw_model2->BBOX[1].y)*(raw_model2->BBOX[0].y-raw_model2->BBOX[1].y)+(raw_model2->BBOX[0].z-raw_model2->BBOX[1].z)*(raw_model2->BBOX[0].z-raw_model2->BBOX[1].z));

printf("diagBBOX: %lf\n",diag);
printf("diagBBOX2: %lf\n",diag2);



for(i=0;i<raw_model1->nbfaces;i++) {
     
   sample2=echantillon(raw_model1->vertices[raw_model1->faces[i].f0],raw_model1->vertices[raw_model1->faces[i].f1],raw_model1->vertices[raw_model1->faces[i].f2],samplethin);

   for(j=0;j<sample2->nbsamples;j++){
     dcourant=pcd(sample2->sample[j],raw_model2,samplethin);
     if(dcourant>dmax)
       dmax=dcourant;
       } 
   printf("face numero %d: dmax= %lf\n",i+1,dmax);
   dmax=0;
   free(sample2->sample);
   }

}
