/* $Id: final2.c,v 1.11 2001/04/12 13:33:57 jacquet Exp $ */

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
  vertex *face_normals;
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

/* Computes the normalized cross product between vectors p2p1 and p3p1 */
vertex ncrossp(vertex p1, vertex p2, vertex p3) {
  vertex v1, v2, tmp;
  double norm;

  v1.x = p2.x - p1.x;
  v1.y = p2.y - p1.y;
  v1.z = p2.z - p1.z;

  v2.x = p3.x - p1.x;
  v2.y = p3.y - p1.y;
  v2.z = p3.z - p1.z;

  tmp.x = v1.y*v2.z - v1.z*v2.y;
  tmp.y = v1.z*v2.x - v1.x*v2.z;
  tmp.z = v1.x*v2.y - v1.y*v2.x;
  
  norm = sqrt(tmp.x*tmp.x + tmp.y*tmp.y + tmp.z*tmp.z);
  if (fabs(norm) < 1e-10) {
    printf("ncrossp: Trouble\n");
  }

  tmp.x /= norm;
  tmp.y /= norm;
  tmp.z /= norm;
  
  return tmp;
}

/* computes the distance between a point and a plan defined by 3 points */
double distance(vertex point,vertex A,vertex normal)
{
double k;
double dist;

k=-(normal.x*A.x+normal.y*A.y+normal.z*A.z);

dist=fabs(-normal.x*point.x-normal.y*point.y-normal.z*point.z-k);

return dist;

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
cellules* liste(int grid,model *raw_model)
{
cellules *cell;
int h,i,j,k,m,n,o,cellule,state=0;
sample *sample1;
vertex A,B,C,bbox0,bbox1;

bbox0=raw_model->BBOX[0];
bbox1=raw_model->BBOX[1];


raw_model->face_normals=(vertex*)malloc(raw_model->nbfaces*sizeof(vertex));
cell=(cellules *)malloc((raw_model->nbfaces)*sizeof(cellules));

 for(i=0;i<raw_model->nbfaces;i++){
   h=0;
   cell[i].cube=(int *)malloc(sizeof(int));   

   A=raw_model->vertices[raw_model->faces[i].f0];
   B=raw_model->vertices[raw_model->faces[i].f1];
   C=raw_model->vertices[raw_model->faces[i].f2];

   raw_model->face_normals[i]=ncrossp(A,B,C);
   sample1=echantillon(A,B,C,0.05);

   for(j=0;j<sample1->nbsamples;j++){
     state=0;
     m=(sample1->sample[j].x-bbox0.x)*grid/(bbox1.x-bbox0.x);
     n=(sample1->sample[j].y-bbox0.y)*grid/(bbox1.y-bbox0.y);
     o=(sample1->sample[j].z-bbox0.z)*grid/(bbox1.z-bbox0.z);
     
     if(m==grid)
       m=grid-1;
     if(n==grid)
       n=grid-1;
     if(o==grid)
       o=grid-1;

     cellule=m+n*grid+o*grid*grid;

     for(k=0;k<=h;k++){
       if(cellule==cell[i].cube[k]){
         state=1;
         break;
       }
     }
     if(state==0){
       if(h>0){
         if((cell[i].cube=(int *)realloc(cell[i].cube,(h+1)*sizeof(int)))==NULL){
           printf("erreur d'allocation memoire");
           exit(-1);
         }
       }
       cell[i].cube[h]=cellule;
       h++;
     } 
   }
   if(sample1->sample != NULL)
     free(sample1->sample);
   if(sample1 != NULL)
     free(sample1);
   cell[i].nbcube=h;

 }
 /* for(i=0;i<raw_model->nbfaces;i++){
   printf("face %d",i);
   for(j=0;j<cell[i].nbcube;j++){
     printf(" %d",cell[i].cube[j]);
   }
   printf("\n");
   }*/
 
return cell;
}


/*****************************************************************************/
/* fonction qui repertorie pour chaque cellule la liste des faces avec       */
/*      lesquelles elle a une intersection                                   */
/*****************************************************************************/

int** cublist(cellules *cell,int grid,model *raw_model)
{

int **tab,i,j,k;
int *mem;

mem=(int*)calloc(grid*grid*grid,sizeof(int));
tab=(int **)malloc(grid*grid*grid*sizeof(int*));

 for(j=0;j<raw_model->nbfaces;j++){
   for(k=0;k<cell[j].nbcube;k++){
     i=cell[j].cube[k];
     if(mem[i]==0)
       tab[i]=NULL;
     tab[i]=(int *)realloc(tab[i],(mem[i]+1)*sizeof(int));
     tab[i][mem[i]]=j;
     mem[i]++;
   }
 }

 for(i=0;i<grid*grid*grid;i++){
   if(mem[i]==0)
     tab[i]=NULL;
   tab[i]=(int *)realloc(tab[i],(mem[i]+1)*sizeof(int));
   tab[i][mem[i]]=-1;
 }

 /* for(i=0;i<grid*grid*grid;i++){
   j=0;
   printf("cell %d ",i);
   while(tab[i][j]!=-1){
     printf("%d ",tab[i][j]);
     j++;
   }
   printf("\n");
   }*/

return(tab);
}

/****************************************************************************/
/*     fonction qui calcule la distance d'un point a une surface            */
/****************************************************************************/
double pcd(vertex point,model *raw_model,int **repface,int grid)
{
int m,n,o;
int a,b,c;
vertex bbox0,bbox1,A,normal;
int cellule;
int j=0;
double dist,dmin=200;

bbox0=raw_model->BBOX[0];
bbox1=raw_model->BBOX[1];

 m=(point.x-bbox0.x)*grid/(bbox1.x-bbox0.x);
 n=(point.y-bbox0.y)*grid/(bbox1.y-bbox0.y);
 o=(point.z-bbox0.z)*grid/(bbox1.z-bbox0.z);
 
 if(m==grid)
   m=grid-1;
 if(n==grid)
   n=grid-1;
 if(o==grid)
   o=grid-1;
 
 cellule=m+n*grid+o*grid*grid;
 /*printf("cellule: %d\n",cellule);*/

 for(c=o-1;c<=o+1;c++){ 
   for(b=n-1;b<=n+1;b++){
     for(a=m-1;a<=m+1;a++){
       /*printf("mermer");*/
       cellule=a+b*grid+c*grid*grid;
       j=0;
       if(cellule>=0 && cellule<grid*grid*grid){ 
	 while(repface[cellule][j]!=-1){
	   A=raw_model->vertices[raw_model->faces[repface[cellule][j]].f0];
	   normal=raw_model->face_normals[repface[cellule][j]];
	   
	   
	   dist=distance(point,A,normal);
	   /*printf("cellule: %d\n",cellule);
	   printf("%lf %lf %lf\n",A.x,A.y,A.z);
	   printf("%lf %lf %lf\n",B.x,B.y,B.z);
	   printf("%lf %lf %lf\n",C.x,C.y,C.z);
	   
	   printf("face: %d dist: %lf\n",repface[cellule][j],dist);*/
	   if(dist<dmin)
	     dmin=dist;
	   
	   j++;
	 }
       }
     }
   }
 }

 /*printf("dmin: %lf\n",dmin);*/
return dmin;

}
/****************************************************************************/
int main(int argc,char **argv)
{
FILE *f1,*f2;
sample *sample2;
model* raw_model1;
model* raw_model2;
double samplethin,diag,diag2,dcourant,dmax=0,superdmax=0;
int i,j,h=0;
vertex bbox0,bbox1;
cellules *cell;
int **repface;
int grid;


 if (argc!=4) {
   printf("nbre d'arg incorrect\n");
   printf("le 1er argument correspond a l'objet de plus basse resolution\n");
   printf("le 2nd argument correspond a l'objet de plus haute resolution\n");
   printf("le 3eme argument correspond au pas d'echantillonnage\n");
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

raw_model1=readfile(f1);
fclose(f1);
raw_model2=readfile(f2);
fclose(f2);

 if(raw_model1->nbfaces>raw_model2->nbfaces){
   printf("\nle nbre de faces du 1er objet doit etre superieur a celui du 2nd objet\n\n");
   exit(-1);
 }

bbox0=raw_model1->BBOX[0];
bbox1=raw_model1->BBOX[1];

printf("%lf %lf %lf\n",bbox0.x,bbox0.y,bbox0.z);
printf("%lf %lf %lf\n",bbox1.x,bbox1.y,bbox1.z);

bbox0=raw_model2->BBOX[0];
bbox1=raw_model2->BBOX[1];

printf("%lf %lf %lf\n",bbox0.x,bbox0.y,bbox0.z);
printf("%lf %lf %lf\n",bbox1.x,bbox1.y,bbox1.z);

diag=dist(raw_model1->BBOX[0],raw_model1->BBOX[1]);
diag2=dist(raw_model2->BBOX[0],raw_model2->BBOX[1]);

printf("diagBBOX: %lf\n",diag);
printf("diagBBOX2: %lf\n",diag2);


samplethin=atof(argv[3]);
/*samplethin*=diag2/100;*/

if(raw_model2->nbfaces<1000)
  grid=10;
else if(raw_model2->nbfaces<10000)
  grid=20;
else if(raw_model2->nbfaces<100000)
  grid=30;

cell=liste(grid,raw_model2);
repface=cublist(cell,grid,raw_model2);


 for(i=0;i<raw_model1->nbfaces;i++) {
   sample2=echantillon(raw_model1->vertices[raw_model1->faces[i].f0],
		       raw_model1->vertices[raw_model1->faces[i].f1],
		       raw_model1->vertices[raw_model1->faces[i].f2],
		       samplethin); 
   for(j=0;j<sample2->nbsamples;j++){
     dcourant=pcd(sample2->sample[j],raw_model2,repface,grid);
     if(dcourant>dmax)
       dmax=dcourant;
     h++;
   }
   printf("face numero %d: dmax= %lf\n",i+1,dmax);
   if(dmax>superdmax)
     superdmax=dmax;
   dmax=0;

   free(sample2->sample);

   free(sample2);
    
   }
printf("distance maximale: %lf\n",superdmax);
printf("nbsampleteste: %d\n",h);

return 0;
}



