/* $Id: compute_error.c,v 1.13 2001/07/12 08:40:32 aspert Exp $ */

#include <compute_error.h>

#define E 1e-10



/* computes the distance between a point and a plan defined by 3 points */
double distance(vertex point,vertex A,vertex normal)
{
  double k;
  double dist;
  
  k = scalprod(A,normal);

  dist=fabs(scalprod(point, normal) + k);

  return dist;
  
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


/*****************************************************************************/
/*      compute the distance between a point and a surface                   */
/*****************************************************************************/
double dist_pt_surf(vertex A,vertex B,vertex C,vertex point,vertex normal)
{
  double k,tmp;
  int i=0;
  double d, dmin=FLT_MAX;
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
  
  
  k=-scalprod(normal,A);
  
  tmp=scalprod(normal,point)+k;
  M.x=point.x-normal.x*tmp;
  M.y=point.y-normal.y*tmp;
  M.z=point.z-normal.z*tmp;
  



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



 if(i==1) {
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
     d1 = ap - (ap_ab*ap_ab)/ab;
   else
     d1 = min(ap,bp);

   ap_ac = scalprod(AP,AC);
   if(ap_ac>0 && ap_ac<ac)
     d2 = ap - (ap_ac*ap_ac)/ac;
   else
     d2 = min(ap,cp);

   bp_bc = scalprod(BP,BC);
   if(bp_bc>0 && bp_bc<bc)
     d3 = bp - (bp_bc*bp_bc)/bc;
   else
     d3 = min(bp,cp);

   dmin=min3(d1,d2,d3);
 }
if(dmin>E)
  return sqrt(dmin);
else 
  return 0;


}
/*****************************************************************************/
/*      compute the distance between a point and a cell                      */
/*****************************************************************************/
double dist_pt_cellule(vertex point, int m, int n, int o, int a, int b, int c, 
		       double ccube) {
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
  sample1->sample = NULL;

  l1.x=b.x-a.x;
  l1.y=b.y-a.y;
  l1.z=b.z-a.z;

  l2.x=c.x-a.x;
  l2.y=c.y-a.y;
  l2.z=c.z-a.z;

  nbsamples=(1 + 1/k)*(1 + 1/k)/2 + 1 + 1/k; 
   

  for (i=0; i<1+E; i+=k) {
    for (j=0; j<1+E; j+=k) {
      if (i+j<1+E) {
	sample1->sample=(vertex*)realloc(sample1->sample,(h+1)*sizeof(vertex));
        sample1->sample[h].x=a.x+i*l1.x+j*l2.x;
        sample1->sample[h].y=a.y+i*l1.y+j*l2.y;
        sample1->sample[h++].z=a.z+i*l1.z+j*l2.z;

      }
    }
  }
  sample1->nbsamples=h;
  
  return(sample1);

}


int compar_cpe(const void *face0, const void *face1) {
  int *f0, *f1;

  f0 = (int*)face0;
  f1 = (int*)face1;

  if (*f0 == *f1)
    return 0;
  else if (*f0 < *f1)
    return -1;
  else
    return 1;
}

/****************************************************************************/
/* fonction qui repertorie pour chaque face les cellules avec lesquelles    */
/*     elle a une intersection                                              */
/****************************************************************************/
cellules* liste(model *raw_model,double samplethin,vertex grille,double ccube,vertex bbox0,vertex bbox1)
{
  cellules *cell;
  int h,i,j,k,m,n,o,cellule, cellule_old,state=0;
  int size_cell;
  sample *sample1;
  vertex A,B,C;
  int *tmp;
  /* bbox0=raw_model->bBox[0]; */
  /* bbox1=raw_model->bBox[1]; */
  

  raw_model->face_normals=(vertex*)malloc(raw_model->num_faces*sizeof(vertex));
  cell=(cellules *)malloc((raw_model->num_faces)*sizeof(cellules));
  
 for(i=0;i<raw_model->num_faces;i++){
   h = 0;
   tmp = NULL;
   cell[i].cube = NULL;
   A=raw_model->vertices[raw_model->faces[i].f0];
   B=raw_model->vertices[raw_model->faces[i].f1];
   C=raw_model->vertices[raw_model->faces[i].f2];

   raw_model->face_normals[i] = ncrossp(A,B,C);
   sample1=echantillon(A,B,C,0.1);
   cellule_old = -1;
   for(j=0;j<sample1->nbsamples;j++){
     state=0;
     
     m=(int)floor((sample1->sample[j].x-bbox0.x)/ccube);
     n=(int)floor((sample1->sample[j].y-bbox0.y)/ccube);
     o=(int)floor((sample1->sample[j].z-bbox0.z)/ccube);
     
     if(m == (int)grille.x)
       m = (int)grille.x - 1;
     else if (m == -1)
       m = 0;

     if(n == (int)grille.y)
       n = (int)grille.y - 1;
     else if (n == -1)
       n = 0;

     if(o == (int)grille.z)
       o = (int)grille.z - 1;
     else if (o == -1)
       o = 0;


     cellule = m + n*grille.x + o*grille.y*grille.x;    

     if (cellule != cellule_old) {
       tmp = (int *)realloc(tmp, (h+1)*sizeof(int));
       tmp[h++] = cellule;
       cellule_old = cellule;
     }
   }

   qsort(tmp, h, sizeof(int), compar_cpe);

   cellule_old = -1;
   size_cell = 0;
   for (j=0; j<h; j++) {
     if (tmp[j] != cellule_old) {
       cell[i].cube = (int*)realloc(cell[i].cube, (size_cell+1)*sizeof(int));
       cell[i].cube[size_cell++] = tmp[j];
       cellule_old = tmp[j];

     }
   }
   cell[i].nbcube = size_cell;
   free(tmp);
   if(sample1 != NULL) {
     if(sample1->sample != NULL)
       free(sample1->sample);
     free(sample1);
   }


 }
/*  for(i=0;i<raw_model->num_faces;i++) */
/*    printf("nbcell[%d] = %d\n",i, cell[i].nbcube); */
/*    for(j=0;j<cell[i].nbcube;j++){ */
/*      printf(" %d",cell[i].cube[j]); */
/*    } */
/*    printf("\n"); */
/*    } */
 
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

  mem=(int*)calloc((int)(grille.x*grille.y*grille.z),sizeof(int));
  tab=(int **)malloc((int)(grille.x*grille.y*grille.z)*sizeof(int*));

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
double pcd(vertex point,model *raw_model,int **repface,vertex grille,double ccube,vertex bbox0,vertex bbox1) {
  int m, n, o;
  int mmem, nmem, omem;
  int a, b, c;
  int face, cell;
  vertex A, B, C, normal, test;
  int cellule;
  int i, j=0, k=0, l=0;
  double dist, dmin=FLT_MAX;
  int *memoire=NULL, *memoire2=NULL;
  int rang=0, rang2=0;
  int state=0, state2=0;





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
 
  cellule = m + n*grille.x + o*grille.y*grille.x;


 /* quelles sont les cellules qu'il faut traiter? */
  while(state==0){
    for(c=o-k;c<=o+k;c++){ 
      for(b=n-k;b<=n+k;b++){
	for(a=m-k;a<=m+k;a++){
	  cellule = a + b*grille.x + c*grille.y*grille.x;
	  if(a<grille.x && b<grille.y && c<grille.z && a>0 && b>0 && c>0 &&
	     repface[cellule][0]!=-1 ){
	    state=1;
	  }
	}
      }
    }
    k++;
  }
  
  

  /* on cherche la distance min dans les cellules qu'on doit obligatoirement traiter */
   k--; 
  for(c=o-k; c<=o+k; c++){ 
    for(b=n-k; b<=n+k; b++){
      for(a=m-k; a<=m+k; a++){
	j=0;
	cellule = a + b*grille.x + c*grille.y*grille.x;
	if(a<grille.x && b<grille.y && c<grille.z && a>0 && b>0 && c>0){

	 /* on garde en memoire les cellules deja traitees */
	  memoire=(int*)realloc(memoire,(rang+1)*sizeof(int));
	  memoire[rang++]=cellule;
	 
	  while(repface[cellule][j]!=-1){
	    state2=0;
	    for(i=0;i<rang2;i++){
	      if(repface[cellule][j]==memoire2[i]){
		state2 = 1;
		break;
	      }
	    }
	    if(state2 == 0){
	      memoire2 = (int*)realloc(memoire2,(rang2+1)*sizeof(int));
	      memoire2[rang2++] = repface[cellule][j];
	      
	      A=raw_model->vertices[raw_model->faces[repface[cellule][j]].f0];
	      B=raw_model->vertices[raw_model->faces[repface[cellule][j]].f1];
	      C=raw_model->vertices[raw_model->faces[repface[cellule][j]].f2];
	      
	      normal=raw_model->face_normals[repface[cellule][j]];
	      
	      dist=dist_pt_surf(A,B,C,point,normal);
	      
	      
	      if(dist<dmin)
		dmin=dist;
	    }
	    j++;
	  }
	}
      }
    }
  }

 /* on cherche la distance min dans les cellules adjacentes */
 k++;
 for(c=o-k;c<=o+k;c++){ 
   for(b=n-k;b<=n+k;b++){
     for(a=m-k;a<=m+k;a++){
       state=0;
       j=0;
       cellule = a + b*grille.x + c*grille.y*grille.x;
       if(cellule>=0 && cellule<grille.x*grille.y*grille.z){
	 /* on regarde si on a pas deja traite cette cellule */
	 for(i=0;i<rang;i++){
	   if(cellule==memoire[i]){
	     state=1;
	     break;
	   }
	 }
	 
	 if(state==0 && dist_pt_cellule(test,m,n,o,a,b,c,ccube)<dmin){
	   while(repface[cellule][j] != -1){
	     state2=0;
	     for(i=0;i<rang2;i++){
	       if(repface[cellule][j]==memoire2[i]){
		 state2=1;
		 break;
	       }
	     }
	     if(state2==0){
	       memoire2 = (int*)realloc(memoire2,(rang2+1)*sizeof(int));
	       memoire2[rang2++] = repface[cellule][j];
	       
	       A=raw_model->vertices[raw_model->faces[repface[cellule][j]].f0];
	       B=raw_model->vertices[raw_model->faces[repface[cellule][j]].f1];
	       C=raw_model->vertices[raw_model->faces[repface[cellule][j]].f2];
	       
	       normal=raw_model->face_normals[repface[cellule][j]];
	       	       
	       dist=dist_pt_surf(A,B,C,point,normal);
	       
	       
	       if(dist<dmin)
		 dmin=dist;

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
 
 return dmin;
}



/****************************************************************************/
/*     fonction qui retourne la liste des faces pour chaque vertex          */
/****************************************************************************/

void listoffaces(model *raw_model,int *nbfaces,int **list_face)
{
int i,j;


/*  For each vertex, build list of faces */
 for (i=0; i<raw_model->num_vert; i++) {
   list_face[i]=NULL;
   for(j=0; j<raw_model->num_faces; j++){
     if(raw_model->faces[j].f0 == i ||
	raw_model->faces[j].f1 == i ||
	raw_model->faces[j].f2 == i) {
	 list_face[i] = (int*)realloc(list_face[i], 
				      (nbfaces[i] + 1)*sizeof(int));	 
	 list_face[i][nbfaces[i]] = j;
	 nbfaces[i]++;
     }
   }
 }  
}


