/* $Id: compute_error.c,v 1.1 2001/04/27 07:13:27 jacquet Exp $ */
#include <3dmodel.h>
#include <geomutils.h>




/* computes the distance between a point and a plan defined by 3 points */
double distance(vertex point,vertex A,vertex normal)
{
double k;
double dist;

k=-(normal.x*A.x+normal.y*A.y+normal.z*A.z);

dist=fabs(-normal.x*point.x-normal.y*point.y-normal.z*point.z-k);

return dist;

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

bbox0=raw_model->bBox[0];
bbox1=raw_model->bBox[1];


raw_model->face_normals=(vertex*)malloc(raw_model->num_faces*sizeof(vertex));
cell=(cellules *)malloc((raw_model->num_faces)*sizeof(cellules));

 for(i=0;i<raw_model->num_faces;i++){
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
 /* for(i=0;i<raw_model->num_faces;i++){
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

 for(j=0;j<raw_model->num_faces;j++){
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
int j=0,k=0;
double dist,dmin=200;

bbox0=raw_model->bBox[0];
bbox1=raw_model->bBox[1];

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

 while(dmin==200){
   for(c=o-k;c<=o+k;c++){ 
     for(b=n-k;b<=n+k;b++){
       for(a=m-k;a<=m+k;a++){
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
   k++;
 }
 /*printf("dmin: %lf\n",dmin);*/
return dmin;

}


