/* $Id: viewer.cpp,v 1.25 2001/08/07 08:31:44 aspert Exp $ */
#include <string.h>
#include <qapplication.h>
#include <qevent.h>
#include <qkeycode.h>

#include <compute_error.h>
#include <ScreenWidget.h>
#include <InitWidget.h>




/*****************************************************************************/
/*             fonction principale                                           */
/*****************************************************************************/


int main( int argc, char **argv )
{
  char *in_filename1, *in_filename2;
  model *raw_model1, *raw_model2;
  int samples;
  cellules *cell;
  double sampling_step,dcourant,dmax=0,superdmax=0,dmin=200,superdmin=200;
  double dmoy,dmoymax=0,dmoymin=200,meanerror=0;
  int **repface;
  int **list_face;
  int *nbfaces;
  double **error_per_face;
  int i,j,k,l;
  sample *sample2;
  double **mem_err;
  double triarea, surfacetot=0, surfacemoy=0, diag;
  vertex bbox0,bbox1;
  double ccube;
  vertex grille;
  int facteur=-1;
  info_vertex *curv;
  QString m1,n1,o1;
  int text=1;

 /* affichage de la fenetre graphique */
 QApplication::setColorSpec( QApplication::CustomColor );
 QApplication a( argc, argv ); 



 if ((argc == 5) && !strcmp("-t", argv[1])) {
     text = 0;     
     in_filename1 = argv[2];
     in_filename2 = argv[3];
     sampling_step=atof(argv[4]);
 } else {
   InitWidget *b;
   b = new InitWidget() ;
   a.setMainWidget( b );
   b->show(); 
   a.exec();
  
   in_filename1 = b->mesh1;
   in_filename2 = b->mesh2;
   sampling_step=atof(b->step);
 }


  k=(int)floor(1.0/sampling_step);

  /* mise en memoire de chaque point des deux objets */
  raw_model1=read_raw_model(in_filename1);
  raw_model2=read_raw_model(in_filename2);

  
  raw_model2->area = (double*)malloc(raw_model2->num_faces*sizeof(double));
  curv = (info_vertex*)malloc(raw_model2->num_vert*sizeof(info_vertex));
  
  if (raw_model2->face_normals==NULL) {
    raw_model2->face_normals = compute_face_normals(raw_model2,curv);
        
    if (raw_model2->face_normals != NULL){
      compute_vertex_normal(raw_model2, curv, raw_model2->face_normals);
      for (i=0; i<raw_model2->num_vert; i++) 
	free(curv[i].list_face);
      free(curv);
    }
  }

  bbox0.x=min(raw_model1->bBox[0].x,raw_model2->bBox[0].x);
  bbox0.y=min(raw_model1->bBox[0].y,raw_model2->bBox[0].y);
  bbox0.z=min(raw_model1->bBox[0].z,raw_model2->bBox[0].z);
 
  bbox1.x=max(raw_model1->bBox[1].x,raw_model2->bBox[1].x);
  bbox1.y=max(raw_model1->bBox[1].y,raw_model2->bBox[1].y);
  bbox1.z=max(raw_model1->bBox[1].z,raw_model2->bBox[1].z);
  diag = dist(bbox0, bbox1);
  printf("Bbox diag = %f\n", diag);
  /* calcul de la taille de la grille */
  if(raw_model2->num_faces<100)
    facteur=5;
  else if(raw_model2->num_faces<1000)
    facteur=10;
  else if(raw_model2->num_faces<10000)
    facteur=20;
  else if(raw_model2->num_faces<100000)
    facteur=30;

//   if (raw_model2->num_faces/raw_model1->num_faces>50)
//     facteur/=2;

  ccube=min3(bbox1.x-bbox0.x,bbox1.y-bbox0.y,bbox1.z-bbox0.z)/facteur;

  grille.x=floor((bbox1.x-bbox0.x)/ccube)+1;
  grille.y=floor((bbox1.y-bbox0.y)/ccube)+1;
  grille.z=floor((bbox1.z-bbox0.z)/ccube)+1;

//   printf("bbox0 %f %f %f\n",bbox0.x,bbox0.y,bbox0.z);
  printf("ccube: %f grille: %f %f %f\n",ccube,grille.x,grille.y,grille.z);

  /* on repertorie pour chaque cellule la liste des faces qu'elle contient */
  cell=liste(raw_model2,sampling_step,grille,ccube,bbox0,bbox1);
  repface=cublist(cell,raw_model2,grille);
  
  nbfaces=(int*)calloc(raw_model1->num_vert,sizeof(int));
  list_face=(int**)malloc(raw_model1->num_vert*sizeof(int*));


  listoffaces(raw_model1,nbfaces,list_face);
  error_per_face=(double**)malloc(raw_model1->num_faces*sizeof(double*));



  /* on calcule pour chaque echantillon la distance a l'objet2 */ 
  for(i=0;i<raw_model1->num_faces;i++) {
    error_per_face[i]=(double*)malloc(2*sizeof(double));
    
    triarea=tri_area(raw_model1->vertices[raw_model1->faces[i].f0],
		     raw_model1->vertices[raw_model1->faces[i].f1],
		     raw_model1->vertices[raw_model1->faces[i].f2]);
    surfacetot+=triarea;
    error_per_face[i][1]=triarea;
//     printf("surfactriangle: %f ",triarea); 
    
    mem_err=(double**)malloc((k+1)*sizeof(double*));
    for(j=0;j<k+1;j++)
      mem_err[j]=(double*)malloc((k+1-j)*sizeof(double));
    
    sample2=echantillon(raw_model1->vertices[raw_model1->faces[i].f0],
			raw_model1->vertices[raw_model1->faces[i].f1],
			raw_model1->vertices[raw_model1->faces[i].f2],
			sampling_step);
    samples=0;

    for(j=0;j<k+1;j++){
      for(l=0;l<k+1-j;l++, samples++){
	dcourant = 
	  pcd(sample2->sample[samples],raw_model2,repface,grille,ccube,bbox0,bbox1);

	if(dcourant>dmax)
	  dmax=dcourant;
	if(dcourant<dmin)
	  dmin=dcourant;
	mem_err[j][l]=dcourant;
// 	samples++;
// 	h++;
      }
    }

    
    dmoy=err_moy(mem_err,sample2,k); /* dmoy= erreur moy * surface triangle */
    meanerror+=dmoy;
    dmoy=err_moy(mem_err,sample2,k)/triarea;
    error_per_face[i][0]=dmoy;

    for(j=0;j<k;j++)
      free(mem_err[j]);
    free(mem_err);

     printf("face numero %d: dmax= %f dmoy= %f\n",i+1,dmax,dmoy);
    if(dmax>superdmax)
      superdmax=dmax;
    dmax=0;
    if(dmin<superdmin)
      superdmin=dmin;
    dmin=200;
    
    free(sample2->sample);
    
    free(sample2);
    
 }
 meanerror/=surfacetot;

 printf("\ndistance maximale: %f \n",superdmax);
 printf("distance minimale: %f \n",superdmin); 
//  printf("nbsampleteste: %d\n",h);
 printf("erreur moyenne: %f\n",meanerror);
 printf("err_max_rel = %f %%\nerr_moyenne_rel = %f %%\n", superdmax*100/diag, 
	meanerror*100/diag);

 if(text == 1){
   /* on assigne une couleur a chaque vertex qui est proportionnelle */
   /* a la moyenne de l'erreur sur les faces incidentes */
   raw_model1->error=(int *)malloc(raw_model1->num_vert*sizeof(int));
   raw_model2->error=(int *)calloc(raw_model2->num_vert,sizeof(int));


   for(i=0;i<raw_model1->num_vert;i++){
     dmoy=0;
     surfacemoy=0;
     for(j=0;j<nbfaces[i];j++){
       dmoy+=error_per_face[list_face[i][j]][0];
       surfacemoy+=error_per_face[list_face[i][j]][1];
     }
     dmoy/=surfacemoy/*nbfaces[i]*/;
     if(dmoy>dmoymax)
       dmoymax=dmoy;
     if(dmoy<dmoymin)
       dmoymin=dmoy;
   }
   
   for(i=0;i<raw_model1->num_vert;i++){
     surfacemoy=0;
     dmoy=0;
     for(j=0;j<nbfaces[i];j++){
       dmoy+=error_per_face[list_face[i][j]][0];
       surfacemoy+=error_per_face[list_face[i][j]][1];
     }
     dmoy/=surfacemoy/*nbfaces[i]*/;
     raw_model1->error[i]=(int)floor(7*(dmoy-dmoymin)/(dmoymax-dmoymin));
     if(raw_model1->error[i]<0)
       raw_model1->error[i]=0;
   }
   
   
   
   ScreenWidget c(raw_model1,raw_model2,superdmin,superdmax);
   a.setMainWidget( &c );
   c.show(); 
   a.exec();

 }
 
}
