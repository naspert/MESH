/* calcul de la distance minimale d'une surface a un ensemble de surface */

/* lecture du second fichier */

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


/*************************************************************************/

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


/*************************************************************************/
void echantillon(FILE *f,vertex a, vertex b, vertex c,double k)
{
  vertex m,l1,l2;
  float i,j;
  

  l1.x=b.x-a.x;
  l1.y=b.y-a.y;
  l1.z=b.z-a.z;

  l2.x=c.x-a.x;
  l2.y=c.y-a.y;
  l2.z=c.z-a.z;

  for (i=0;i<=1;i=i+k) {
    for (j=0;j<=1;j=j+k) {
      if (i+j<=1) {
        m.x=a.x+i*l1.x+j*l2.x;
        m.y=a.y+i*l1.y+j*l2.y;
        m.z=a.z+i*l1.z+j*l2.z;
        fprintf(f,"%f %f %f\n",m.x,m.y,m.z); 
      }
    }    
  }
}


/**************************************************************************/
void echobjet(FILE *f,model* raw_model,double k)
{
  int i;

  for (i=0;i<raw_model->nbfaces;i++) {

    echantillon(f,raw_model->vertices[raw_model->faces[i].f0],raw_model->vertices[raw_model->faces[i].f1],raw_model->vertices[raw_model->faces[i].f2],k);
  }
									       
}


/****************************************************************************/
double pcd(vertex point)
{
   double d,dmin,dmax=0;
   int i=0;
   vertex echantillon;
   FILE *f;

   f=fopen("dida3","r");
   if (f==NULL)
     {
     printf("impossible d'ouvrir le fichier Dida\n");
     exit(-1);
     }

  
   while(fscanf(f,"%lf %lf %lf",&(echantillon.x),&(echantillon.y),&(echantillon.z))!=EOF)
     {
     d=sqrt((point.x-echantillon.x)*(point.x-echantillon.x)+(point.y-echantillon.y)*(point.y-echantillon.y)+(point.z-echantillon.z)*(point.z-echantillon.z));

     if (i==0)
       {
        dmin=d;
       }
     if (d<=dmin)
       {
        dmin=d;
       }
   
     i++;

     }
   fclose(f); 
   return(dmin);
   
}


/***************************************************************************/
double distance()
{

FILE *file;
vertex sample;
double dmax=0,dcourant=0;
printf("dmax %lf ",dmax);

file=fopen("dida4","r");
if (file==NULL)
  {
   printf("erreur de lecture du fichier");
   exit(-1);
  }

while( fscanf(file,"%lf %lf %lf",&(sample.x),&(sample.y),&(sample.z))!=EOF)
  {
   dcourant=pcd(sample);
   if (dcourant>dmax)
     {
      dmax=dcourant;
     }
  }

printf("dmax=%f\n",dmax);
return(dmax);
}


/****************************************************************************/
main(int argc,char **argv)
{

int i;
FILE *f1,*f2,*f3,*f4;
model* raw_model1;
model* raw_model2;
double samplethin,diag,diag2;

 if (argc!=4) {
   printf("nbre d'arg incorrect\n");
   exit(-1);
 }

samplethin=atof(argv[3]);

f1=fopen(argv[1],"r");
f2=fopen(argv[2],"r");
f3=fopen("dida3","w");

 if (f1==NULL || f2==NULL || f3==NULL) {
   printf("impossible ouverture fichier\n");
   exit(-1);
 }

raw_model1=readfile(f1);
fclose(f1);

 diag=sqrt((raw_model1->BBOX[0].x-raw_model1->BBOX[1].x)*(raw_model1->BBOX[0].x-raw_model1->BBOX[1].x)+(raw_model1->BBOX[0].y-raw_model1->BBOX[1].y)*(raw_model1->BBOX[0].y-raw_model1->BBOX[1].y)+(raw_model1->BBOX[0].z-raw_model1->BBOX[1].z)*(raw_model1->BBOX[0].z-raw_model1->BBOX[1].z));

raw_model2=readfile(f2);
fclose(f2);

 diag2=sqrt((raw_model2->BBOX[0].x-raw_model2->BBOX[1].x)*(raw_model2->BBOX[0].x-raw_model2->BBOX[1].x)+(raw_model2->BBOX[0].y-raw_model2->BBOX[1].y)*(raw_model2->BBOX[0].y-raw_model2->BBOX[1].y)+(raw_model2->BBOX[0].z-raw_model2->BBOX[1].z)*(raw_model2->BBOX[0].z-raw_model2->BBOX[1].z));

printf("diagBBOX: %lf\ndiagBBOX2: %lf\n",diag,diag2);




echobjet(f3,raw_model2,samplethin);
fclose(f3);

for (i=0;i<raw_model1->nbfaces;i++) {
    f4=fopen("dida4","w");
    echantillon(f4,raw_model1->vertices[raw_model1->faces[i].f0],raw_model1->vertices[raw_model1->faces[i].f1],raw_model1->vertices[raw_model1->faces[i].f2],samplethin);
    fclose(f4);
    printf("face numero %d ",i+1);
    distance();
  }  
   
  
}
