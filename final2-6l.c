/* $Id: final2-6l.c,v 1.2 2001/07/09 09:42:22 jacquet Exp $ */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define E 0.0000001

#ifndef min
#define min(x,y) (((x)>(y))?(y):(x))
#endif

#ifndef max
#define max(x,y) (((x)>(y))?(x):(y))
#endif

#define max3(x,y,z) (((x)>(y))?(((x)>(z))?(x):(z)):(((y)>(z))?(y):(z)))
#define min3(x,y,z) (((x)<(y))?(((x)<(z))?(x):(z)):(((y)<(z))?(y):(z)))

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




/* Computes the scalar product between 2 vectors */
double scalprod(vertex v1, vertex v2) {
  return (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z);
}


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

/* Computes the area of the triangle p1,p2, p3 */
double tri_area(vertex p1, vertex p2, vertex p3) {
    vertex u,v,h;
    double nu,nv,uv,nh;
    double tmp;

    u.x = p1.x - p3.x;
    u.y = p1.y - p3.y;
    u.z = p1.z - p3.z;
    
    v.x = p2.x - p3.x;
    v.y = p2.y - p3.y;
    v.z = p2.z - p3.z;
    

    uv = scalprod(u,v);

    nv = norm(v);
    nu = norm(u);

    tmp = uv/(nu*nu);
    h.x = v.x - u.x*tmp;
    h.y = v.y - u.y*tmp;
    h.z = v.z - u.z*tmp;

    nh = norm(h);

    return (nh*nu*0.5);

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

/*****************************************************************************/
/*      compute the distance between a point and a cell                      */
/*****************************************************************************/
double dist_pt_cellule(vertex point, int m, int n, int o, int a, int b, int c, double ccube)
{
double d;
vertex sommet;

  if(a==m && b==n && c==o)
    return -1;

  else if(a==m && b==n){
    if(c>o)
      return (c*ccube-point.z);
    else if(c<o)
      return (point.z-(c+1)*ccube);
  }
  else if(a==m && c==o){
    if(b>n)
      return(b*ccube-point.y);
    else if(b<n)
      return(point.y-(b+1)*ccube);
  }
  else if(b==n && c==o){
    if(a>m)
      return(a*ccube-point.x);
    else if(a<m)
      return(point.x-(a+1)*ccube);
  }

  else if(a==m){
    if(b>n){
      if(c>o){
	sommet.x=point.x;
	sommet.y=b*ccube;
	sommet.z=c*ccube;
	d=dist(point,sommet);
	return d;
      }
      else if(c<o){
	sommet.x=point.x;
	sommet.y=b*ccube;
	sommet.z=(c+1)*ccube;
	d=dist(point,sommet);
	return d;
      }
    }
    else if(b<n){
      if(c>o){
	sommet.x=point.x;
	sommet.y=(b+1)*ccube;
	sommet.z=c*ccube;
	d=dist(point,sommet);
	return d;
      }
      else if(c<o){
	sommet.x=point.x;
	sommet.y=(b+1)*ccube;
	sommet.z=(c+1)*ccube;
	d=dist(point,sommet);
	return d;
      }
    }
  }
  else if(b==n){
    if(a>m){
      if(c>o){
	sommet.x=a*ccube;
	sommet.y=point.y;
	sommet.z=c*ccube;
	d=dist(point,sommet);
	return d;
      }
      else if(c<o){
	sommet.x=a*ccube;
	sommet.y=point.y;
	sommet.z=(c+1)*ccube;
	d=dist(point,sommet);
	return d;
      } 
    }
    else if(a<m){
      if(c>o){
	sommet.x=(a+1)*ccube;
	sommet.y=point.y;
	sommet.z=c*ccube;
	d=dist(point,sommet);
	return d;
      }
      else if(c<o){
	sommet.x=(a+1)*ccube;
	sommet.y=point.y;
	sommet.z=(c+1)*ccube;
	d=dist(point,sommet);
	return d;
      } 
    }
  }
  else if(c==o){
    if(a>m){
      if(b>n){
	sommet.x=a*ccube;
	sommet.y=b*ccube;
	sommet.z=point.z;
	d=dist(point,sommet);
	return d;
      } 	
      else if(b<n){
	sommet.x=a*ccube;
	sommet.y=(b+1)*ccube;
	sommet.z=point.z;
	d=dist(point,sommet);
	return d;
      } 
    }
    else if(a<m){
      if(b>n){
	sommet.x=(a+1)*ccube;
	sommet.y=b*ccube;
	sommet.z=point.z;
	d=dist(point,sommet);
	return d;
      } 	
      else if(b<n){
	sommet.x=(a+1)*ccube;
	sommet.y=(b+1)*ccube;
	sommet.z=point.z;
	d=dist(point,sommet);
	return d;
      } 
    }
  }

  else {
    if(a<m && b<n && c<o){
      sommet.x=(a+1)*ccube;
      sommet.y=(b+1)*ccube;
      sommet.z=(c+1)*ccube;
      d=dist(point,sommet);
      return d;
    }
    else if(a<m && b<n && c>o){
      sommet.x=(a+1)*ccube;
      sommet.y=(b+1)*ccube;
      sommet.z=c*ccube;
      d=dist(point,sommet);
      return d;
    }      
    else if(a<m && b>n && c<o){
      sommet.x=(a+1)*ccube;
      sommet.y=b*ccube;
      sommet.z=(c+1)*ccube;
      d=dist(point,sommet);
      return d;
    }
    else if(a<m && b>n && c>o){
      sommet.x=(a+1)*ccube;
      sommet.y=b*ccube;
      sommet.z=c*ccube;
      d=dist(point,sommet);
      return d;
    }  
    else if(a>m && b<n && c<o){
      sommet.x=a*ccube;
      sommet.y=(b+1)*ccube;
      sommet.z=(c+1)*ccube;
      d=dist(point,sommet);
      return d;
    }    
    else if(a>m && b<n && c>o){
      sommet.x=a*ccube;
      sommet.y=(b+1)*ccube;
      sommet.z=c*ccube;
      d=dist(point,sommet);
      return d;
    }  
    else if(a>m && b>n && c<o){
      sommet.x=a*ccube;
      sommet.y=b*ccube;
      sommet.z=(c+1)*ccube;
      d=dist(point,sommet);
      return d;
    } 
    else if(a>m && b>n && c>o){
      sommet.x=a*ccube;
      sommet.y=b*ccube;
      sommet.z=c*ccube;
      d=dist(point,sommet);
      return d;
    } 
  }
}


/*****************************************************************************/
/*      compute the distance between a point and a surface                   */
/*****************************************************************************/
double dist_pt_surf(vertex A,vertex B,vertex C,vertex point,vertex normal)
{
double k;
int i=0;
double d,dmin=200;
vertex M,AM,AB,AC,BC,AP,BP,PM,CP;
double ah,d1,d2,d3,ab,ac,bc,ap,bp,cp,am,ap_ac,ap_ab,bp_bc;
double yint;
double alpha,beta,Xp,Yp;

   AB.x=B.x-A.x;
   AB.y=B.y-A.y;
   AB.z=B.z-A.z;   
   
   AC.x=C.x-A.x;
   AC.y=C.y-A.y;
   AC.z=C.z-A.z;
   
   BC.x=C.x-B.x;
   BC.y=C.y-B.y;
   BC.z=C.z-B.z;



k=-(normal.x*A.x+normal.y*A.y+normal.z*A.z);
M.x=point.x-normal.x*(+normal.x*point.x+normal.y*point.y+normal.z*point.z+k);
M.y=point.y-normal.y*(+normal.x*point.x+normal.y*point.y+normal.z*point.z+k);
M.z=point.z-normal.z*(+normal.x*point.x+normal.y*point.y+normal.z*point.z+k);

/* printf("%lf %lf %lf\n",M.x,M.y,M.z); */


 if(M.x<=max(A.x,B.x) && M.x>=min(A.x,B.x)){
     yint=(M.x*AB.y-A.x*AB.y+AB.x*A.y)/AB.x;
     if(yint>=M.y && yint<=max(A.y,B.y))
       i++;
   
 }

 if(M.x<=max(A.x,C.x) && M.x>=min(A.x,C.x)){

     yint=(M.x*AC.y-A.x*AC.y+AC.x*A.y)/AC.x;
     if(yint>=M.y && yint<=max(A.y,C.y))
       i++;
 }

 if(M.x<=max(B.x,C.x) && M.x>=min(B.x,C.x)){


     yint=(M.x*BC.y-B.x*BC.y+BC.x*B.y)/BC.x;
     if(yint>=M.y && yint<=max(C.y,B.y))
       i++;
 }



 if(i==1||i==-1) {
   PM.x=point.x-M.x;
   PM.y=point.y-M.y;
   PM.z=point.z-M.z;
   dmin=scalprod(PM,PM);
 }
 else {
   AP.x=point.x-A.x;
   AP.y=point.y-A.y;
   AP.z=point.z-A.z;
   
   BP.x=point.x-B.x;
   BP.y=point.y-B.y;
   BP.z=point.z-B.z;

   CP.x=point.x-C.x;
   CP.y=point.y-C.y;
   CP.z=point.z-C.z;

   /* ce sont des distances au carre */
   ab=scalprod(AB,AB);
   bc=scalprod(BC,BC);
   ap=scalprod(AP,AP);
   bp=scalprod(BP,BP);
   cp=scalprod(CP,CP);
   ac=scalprod(AC,AC);

   ap_ab=scalprod(AP,AB);
   if(ap_ab>0 && ap_ab<ab)
     d1=ap-(ap_ab*ap_ab)/ab;
   else
     d1=min(ap,bp);

   ap_ac=scalprod(AP,AC);
   if(ap_ac>0 && ap_ac<ac)
     d2=ap-(ap_ac*ap_ac)/ac;
   else
     d2=min(ap,cp);

   bp_bc=scalprod(BP,BC);
   if(bp_bc>0 && bp_bc<bc)
     d3=bp-(bp_bc*bp_bc)/bc;
   else
     d3=min(bp,cp);

   dmin=min3(d1,d2,d3);
 }
if(dmin>E)
  return sqrt(dmin);
else 
  return 0;


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
  for (i=0;i<1.000001;i+=k) {
    for (j=0;j<1.000001;j+=k) {
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
cellules* liste(model *raw_model,double samplethin,vertex grille,double ccube)
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
     m=(int)floor((sample1->sample[j].x-bbox0.x)/ccube);
     n=(int)floor((sample1->sample[j].y-bbox0.y)/ccube);
     o=(int)floor((sample1->sample[j].z-bbox0.z)/ccube);

     if(m==(int)grille.x)
       m=(int)grille.x-1;
     if(n==(int)grille.y)
       n=(int)grille.y-1;
     if(o==(int)grille.z)
       o=(int)grille.z-1;

     cellule=m+n*grille.x+o*grille.y*grille.z;

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
 
return cell;
}


/*****************************************************************************/
/* fonction qui repertorie pour chaque cellule la liste des faces avec       */
/*      lesquelles elle a une intersection                                   */
/*****************************************************************************/

int** cublist(cellules *cell,model *raw_model,vertex grille)
{

int **tab,i,j,k;
int *mem;

mem=(int*)calloc((int)grille.x*grille.y*grille.z,sizeof(int));
tab=(int **)malloc((int)grille.x*grille.y*grille.z*sizeof(int*));

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

 for(i=0;i<(int)grille.x*grille.y*grille.z;i++){
   if(mem[i]==0)
     tab[i]=NULL;
   tab[i]=(int *)realloc(tab[i],(mem[i]+1)*sizeof(int));
   tab[i][mem[i]]=-1;
 }

return(tab);
}

/****************************************************************************/
/*     fonction qui calcule la distance d'un point a une surface            */
/****************************************************************************/

double pcd(vertex point,model *raw_model,int **repface,vertex grille,double ccube)
{
int m,n,o;
int mmem,nmem,omem;
int a,b,c;
int face,cell;
vertex bbox0,bbox1,A,B,C,normal,test;
int cellule;
int i,j=0,k=0,l=0;
double dist,dmin=201;
int *memoire,*memoire2;
int rang=0,rang2=0;
int state=0,state2=0;

memoire=NULL;
memoire2=NULL;

bbox0=raw_model->BBOX[0];
bbox1=raw_model->BBOX[1];

test.x=point.x-bbox0.x;
test.y=point.y-bbox0.y;
test.z=point.z-bbox0.z;


 m=(int)floor((point.x-bbox0.x)/ccube);
 n=(int)floor((point.y-bbox0.y)/ccube);
 o=(int)floor((point.z-bbox0.z)/ccube);
 
 if(m==(int)grille.x)
   m=(int)grille.x-1;
 if(n==(int)grille.y)
   n=(int)grille.y-1;
 if(o==(int)grille.z)
   o=(int)grille.z-1;
 
 cellule=m+n*grille.x+o*grille.y*grille.z;
/* printf("cellule: %d\n",cellule); */
/* printf("%lf %lf %lf\n",point.x,point.y,point.z);  */
/* printf("%d %d %d\n",m,n,o);  */

 /* quelles sont les cellules qu'il faut traiter? */
 while(state==0){
   for(c=o-k;c<=o+k;c++){ 
     for(b=n-k;b<=n+k;b++){
       for(a=m-k;a<=m+k;a++){
	 cellule=a+b*grille.x+c*grille.y*grille.z;
	 if(cellule>=0 && cellule<grille.x*grille.y*grille.z){
	   if(repface[cellule][0]!=-1){
	     state=1;
	   }
	 }
       }
     }
   }
   k=k+1;
 }


/*  k=k+1; */
/* printf("k= %d ",k); */

 /* on cherche la distance min dans les cellules qu'on doit obligatoirement traiter */
 k=k-1;
 for(c=o-k;c<=o+k;c++){ 
   for(b=n-k;b<=n+k;b++){
     for(a=m-k;a<=m+k;a++){
       j=0;
       cellule=a+b*grille.x+c*grille.y*grille.z;
       if(cellule>=0 && cellule<grille.x*grille.y*grille.z){

	 /* on garde en memoire les cellules deja traitees */
	 memoire=(int*)realloc(memoire,(rang+1)*sizeof(int));
	 memoire[rang]=cellule;
	 rang++;
	 
	 while(repface[cellule][j]!=-1){
	   state2=0;
	   for(i=0;i<rang2;i++){
	     if(repface[cellule][j]==memoire2[i]){
	       state2=1;
	       break;
	     }
	   }
	   if(state2==0){
	     memoire2=(int*)realloc(memoire2,(rang2+1)*sizeof(int));
	     memoire2[rang2]=repface[cellule][j];
	     rang2++;
	   
	     A=raw_model->vertices[raw_model->faces[repface[cellule][j]].f0];
	     B=raw_model->vertices[raw_model->faces[repface[cellule][j]].f1];
	     C=raw_model->vertices[raw_model->faces[repface[cellule][j]].f2];
	   
	     normal=raw_model->face_normals[repface[cellule][j]];
	     
	     
	     dist=dist_pt_surf(A,B,C,point,normal);
	     
	     
	     if(dist<dmin){
	       dmin=dist;
	     }
	   }
	   j++;
	 }
       }
     }
   }
 }

 /* on cherche la distance min dans les cellules adjacentes */
 k=k+1;
 for(c=o-k;c<=o+k;c++){ 
   for(b=n-k;b<=n+k;b++){
     for(a=m-k;a<=m+k;a++){
       state=0;
       j=0;
       cellule=a+b*grille.x+c*grille.y*grille.z;
       if(cellule>=0 && cellule<grille.x*grille.y*grille.z){
	 /* on regarde si on a pas deja traite cette cellule */
	 for(i=0;i<rang;i++){
	   if(cellule==memoire[i]){
	     state=1;
	     break;
	   }
	 }
	 
	 if(state==0 && dist_pt_cellule(test,m,n,o,a,b,c,ccube)<dmin){
	   while(repface[cellule][j]!=-1){
	     state2=0;
	     for(i=0;i<rang2;i++){
	       if(repface[cellule][j]==memoire2[i]){
		 state2=1;
		 break;
	       }
	     }
	     if(state2==0){
	       memoire2=(int*)realloc(memoire2,(rang2+1)*sizeof(int));
	       memoire2[rang2]=repface[cellule][j];
	       rang2++;
	       
	       A=raw_model->vertices[raw_model->faces[repface[cellule][j]].f0];
	       B=raw_model->vertices[raw_model->faces[repface[cellule][j]].f1];
	       C=raw_model->vertices[raw_model->faces[repface[cellule][j]].f2];
	       
	       normal=raw_model->face_normals[repface[cellule][j]];
	       	       
	       dist=dist_pt_surf(A,B,C,point,normal);
	       
	       
	       if(dist<dmin){
		 dmin=dist;
	       }
	     }
	     j++;
	   }
	 }
       }
     }
   }
 }

free(memoire);
free(memoire2);
/* free(memoire2); */
/* fprintf(f,"%lf ",dmin);  */
/* printf("k= %d dmin=%lf face: %d cellule: %d\n",k,dmin,face,cell); */
/* printf("dmin= %lf ",dmin); */
/* printf("face= %d\n",face);  */
return dmin;
}

/***************************************************************************/
/*      fonction qui permet de calculer l'erreur moyenne sur une face      */
/***************************************************************************/

double err_moy(double **mem_err,sample *sample1,int k)
{
double erreur=0;
double surface1,surface2,surfacetot; 
int i,j;
vertex A,B,C;

A=sample1->sample[0];
B=sample1->sample[1];
C=sample1->sample[k+1];

surface1=2*tri_area(A,B,C);

A=sample1->sample[k-1];
B=sample1->sample[k];
C=sample1->sample[2*k];
surface2=tri_area(A,B,C);


 for(i=0;i<k;i++){
   for(j=0;j<k-i;j++){
     if(j!=k-i-1){
       erreur+=surface1*(mem_err[i][j]+mem_err[i][j+1]+mem_err[i+1][j+1]+mem_err[i+1][j])/4;
       surfacetot+=surface1;
     } else{
       surfacetot+=surface2;
       erreur+=surface2*(mem_err[i][j]+mem_err[i+1][j]+mem_err[i][j+1])/3;
     }
   }
 }

return(erreur);

}



/****************************************************************************/

int main(int argc,char **argv)
{
FILE *f1,*f2;
sample *sample2;
model* raw_model1;
model* raw_model2;
double samplethin,diag,diag2,dcourant,dmax=0,superdmax=0,dmin=200,superdmin=200,dmoy,dmoymax=0,dmoymin=200,meanerror=0,meanerror2=0,meansqrterr;
int nbsamples,sample; 
int i,j,h=0,k,l;
vertex bbox0,bbox1;
cellules *cell;
int **repface;
int grid;
double **mem_err;
double triarea,surfacetot=0;
double ccube;
vertex grille;
int facteur;

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

/* bbox0.x=min(raw_model1->BBOX[0].x,raw_model2->BBOX[0].x); */
/* bbox0.y=min(raw_model1->BBOX[0].y,raw_model2->BBOX[0].y); */
/* bbox0.z=min(raw_model1->BBOX[0].z,raw_model2->BBOX[0].z); */

/* bbox1.x=max(raw_model1->BBOX[1].x,raw_model2->BBOX[1].x); */
/* bbox1.y=max(raw_model1->BBOX[1].y,raw_model2->BBOX[1].y); */
/* bbox1.z=max(raw_model1->BBOX[1].z,raw_model2->BBOX[1].z); */

/* ccube=min3(bbox1.x-bbox0.x,bbox1.y-bbox0.y,bbox1.z-bbox0.z)/20; */




  bbox0.x=min(raw_model1->BBOX[0].x,raw_model2->BBOX[0].x);
  bbox0.y=min(raw_model1->BBOX[0].y,raw_model2->BBOX[0].y);
  bbox0.z=min(raw_model1->BBOX[0].z,raw_model2->BBOX[0].z);
 
  bbox1.x=max(raw_model1->BBOX[1].x,raw_model2->BBOX[1].x);
  bbox1.y=max(raw_model1->BBOX[1].y,raw_model2->BBOX[1].y);
  bbox1.z=max(raw_model1->BBOX[1].z,raw_model2->BBOX[1].z);
  diag = dist(bbox0, bbox1);
  printf("Bbox diag = %f\n", diag);

  printf("mesh1: %d\n",raw_model1->nbfaces);
  printf("mesh2: %d\n",raw_model2->nbfaces);
  /* calcul de la taille de la grille */
  if(raw_model2->nbfaces<100)
    facteur=5;
  else if(raw_model2->nbfaces<1000)
    facteur=10;
  else if(raw_model2->nbfaces<10000)
    facteur=15;
  else if(raw_model2->nbfaces<100000)
    facteur=20;

  if (raw_model2->nbfaces/raw_model1->nbfaces>50)
    facteur/=2;

  ccube=min3(bbox1.x-bbox0.x,bbox1.y-bbox0.y,bbox1.z-bbox0.z)/facteur;

  grille.x=floor((bbox1.x-bbox0.x)/ccube)+1;
  grille.y=floor((bbox1.y-bbox0.y)/ccube)+1;
  grille.z=floor((bbox1.z-bbox0.z)/ccube)+1;


printf("ccube: %lf grille: %lf %lf %lf\n",ccube,grille.x,grille.y,grille.z);

samplethin=atof(argv[3]);
printf("samplethin: %f\n",samplethin);

/* samplethin*=diag2/100; */

k=(int)floor(1.0/atof(argv[3]));
printf("k= %d\n",k);

cell=liste(raw_model2,samplethin,grille,ccube);
repface=cublist(cell,raw_model2,grille);


 for(i=0;i<raw_model1->nbfaces;i++) {
   triarea=tri_area(raw_model1->vertices[raw_model1->faces[i].f0],
		    raw_model1->vertices[raw_model1->faces[i].f1],
		    raw_model1->vertices[raw_model1->faces[i].f2]);
   surfacetot+=triarea;


   mem_err=(double**)malloc((k+1)*sizeof(double*));
   for(j=0;j<k+1;j++)
     mem_err[j]=(double*)malloc((k+1-j)*sizeof(double));

    sample2=echantillon(raw_model1->vertices[raw_model1->faces[i].f0],
			raw_model1->vertices[raw_model1->faces[i].f1],
			raw_model1->vertices[raw_model1->faces[i].f2],
			samplethin);
    sample=0;
    for(j=0;j<k+1;j++){
      for(l=0;l<k+1-j;l++){
	dcourant=pcd(sample2->sample[sample],raw_model2,repface,grille,ccube);
	if(dcourant>dmax)
	  dmax=dcourant;
	if(dcourant<dmin)
	  dmin=dcourant;
	mem_err[j][l]=dcourant;
/* 	printf("sample: %d j=%d l=%d dcourant= %lf\n",sample,j,l,dcourant); */
	sample++;
	h++;
      }
    }
    dmoy=err_moy(mem_err,sample2,k);
    meanerror+=dmoy;
    dmoy=err_moy(mem_err,sample2,k)/triarea;
    meanerror2+=dmoy;
    for(j=0;j<k;j++)
      free(mem_err[j]);
    free(mem_err);

    if(dmoy>dmoymax)
      dmoymax=dmoy;
    if(dmoy<dmoymin)
      dmoymin=dmoy;

    printf("face numero %d: dmax= %lf dmoy= %lf surface: %lf\n",i+1,dmax,dmoy,triarea);
    if(dmax>superdmax)
      superdmax=dmax;
    dmax=0;
    if(dmin<superdmin)
      superdmin=dmin;
    dmin=200;
    
    free(sample2->sample);
    
    free(sample2);
    
 }
/*  meanerror/=surfacetot; */
 printf("distance maximale: %lf \n",superdmax);
 printf("distance minimale: %lf \n",superdmin); 
 printf("nbsampleteste: %d\n",h);
 printf("erreur moyenne1: %lf\n",meanerror2/raw_model1->nbfaces);
 printf("erreur moyenne2: %lf\n",meanerror/surfacetot);
 printf("surface totale: %lf\n",surfacetot);
 
return 0;
}
