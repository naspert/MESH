/* $Id: viewer.cpp,v 1.26 2001/08/07 15:19:09 dsanta Exp $ */

#include <string.h>
#include <qapplication.h>
#include <qevent.h>
#include <qkeycode.h>

#include <compute_error.h>
#include <ScreenWidget.h>
#include <InitWidget.h>

#include <mutils.h>

/*****************************************************************************/
/*             fonction principale                                           */
/*****************************************************************************/


int main( int argc, char **argv )
{
  struct triangle_list *tl;
  char *in_filename1, *in_filename2;
  model *raw_model1, *raw_model2;
  int samples;
  struct cell_list *cl;
  double sampling_step,dcourant,dmax=0,superdmax=0,dmin=200,superdmin=200;
  double dmoy,dmoymax=0,dmoymin=200,meanerror=0;
  int **repface;
  struct face_list *vfl;
  double **error_per_face;
  int i,j,k,l,maxi;
  struct sample_list sample2;
  double **mem_err;
  double triarea, surfacetot=0, surfacemoy=0, diag;
  vertex bbox0,bbox1;
  double ccube;
  struct size3d grid_sz;
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


  k=(int)ceil(1.0/sampling_step);

  /* mise en memoire de chaque point des deux objets */
  raw_model1=read_raw_model(in_filename1);
  raw_model2=read_raw_model(in_filename2);

  bbox0.x=min(raw_model1->bBox[0].x,raw_model2->bBox[0].x);
  bbox0.y=min(raw_model1->bBox[0].y,raw_model2->bBox[0].y);
  bbox0.z=min(raw_model1->bBox[0].z,raw_model2->bBox[0].z);
 
  bbox1.x=max(raw_model1->bBox[1].x,raw_model2->bBox[1].x);
  bbox1.y=max(raw_model1->bBox[1].y,raw_model2->bBox[1].y);
  bbox1.z=max(raw_model1->bBox[1].z,raw_model2->bBox[1].z);
  diag = dist(bbox0, bbox1);
  printf("Bbox diag = %f\n", diag);

  /* Convert the fine model 2 to a triangle list with associated info */
  tl = model_to_triangle_list(raw_model2);

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

  /* calcul de la taille de la grille */
  if(raw_model2->num_faces<100)
    facteur=5;
  else if(raw_model2->num_faces<1000)
    facteur=10;
  else if(raw_model2->num_faces<10000)
    facteur=20;
  else if(raw_model2->num_faces<100000)
    facteur=30;
  else {
    fprintf(stderr,"INTERNAL ERROR: Don't know how to get facteur!\n");
    fprintf(stderr,"At %s:%i\n",__FILE__,__LINE__);
    exit(1);
  }

//   if (raw_model2->num_faces/raw_model1->num_faces>50)
//     facteur/=2;

  ccube=min3(bbox1.x-bbox0.x,bbox1.y-bbox0.y,bbox1.z-bbox0.z)/facteur;

  grid_sz.x=(int)floor((bbox1.x-bbox0.x)/ccube)+1;
  grid_sz.y=(int)floor((bbox1.y-bbox0.y)/ccube)+1;
  grid_sz.z=(int)floor((bbox1.z-bbox0.z)/ccube)+1;

//   printf("bbox0 %f %f %f\n",bbox0.x,bbox0.y,bbox0.z);
  printf("ccube: %f grille: %d %d %d\n",ccube,grid_sz.x,grid_sz.y,grid_sz.z);

  /* on repertorie pour chaque cellule la liste des faces qu'elle contient */
  cl = cells_in_triangles(tl,grid_sz,ccube,bbox0);
  repface = triangles_in_cells(cl,tl,grid_sz);
  for (i=0, maxi=tl->n_triangles; i<maxi; i++) {
    free(cl[i].cell);
  }
  free(cl);
  cl = NULL;

  vfl = faces_of_vertex(raw_model1);
  error_per_face=(double**)xmalloc(raw_model1->num_faces*sizeof(double*));



  /* on calcule pour chaque echantillon la distance a l'objet2 */ 
  sample2.sample = NULL;
  for(i=0;i<raw_model1->num_faces;i++) {
    error_per_face[i]=(double*)xmalloc(2*sizeof(double));
    
    triarea=tri_area(raw_model1->vertices[raw_model1->faces[i].f0],
		     raw_model1->vertices[raw_model1->faces[i].f1],
		     raw_model1->vertices[raw_model1->faces[i].f2]);
    surfacetot+=triarea;
    error_per_face[i][1]=triarea;
//     printf("surfactriangle: %f ",triarea); 
    
    mem_err=(double**)xmalloc((k+1)*sizeof(double*));
    for(j=0;j<k+1;j++)
      mem_err[j]=(double*)xmalloc((k+1-j)*sizeof(double));
    
    sample_triangle(&(raw_model1->vertices[raw_model1->faces[i].f0]),
                    &(raw_model1->vertices[raw_model1->faces[i].f1]),
                    &(raw_model1->vertices[raw_model1->faces[i].f2]),
                    k+1,&sample2);
    samples=0;

    for(j=0;j<k+1;j++){
      for(l=0;l<k+1-j;l++, samples++){
	dcourant = 
	  dist_pt_surf(sample2.sample[samples],tl,repface,grid_sz,ccube,bbox0);

	if(dcourant>dmax)
	  dmax=dcourant;
	if(dcourant<dmin)
	  dmin=dcourant;
	mem_err[j][l]=dcourant;
      }
    }

    
    dmoy=error_stat_triag(mem_err,k+1);
    meanerror += dmoy*triarea;
    error_per_face[i][0]=dmoy;

    for(j=0;j<k;j++)
      free(mem_err[j]);
    free(mem_err);

//      printf("face numero %d: dmax= %f dmoy= %f\n",i+1,dmax,dmoy);
    if(dmax>superdmax)
      superdmax=dmax;
    dmax=0;
    if(dmin<superdmin)
      superdmin=dmin;
    dmin=200;
    
    free(sample2.sample);
    sample2.sample = NULL;
    
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
     for(j=0;j<vfl[i].n_faces;j++){
       dmoy+=error_per_face[vfl[i].face[j]][0];
       surfacemoy+=error_per_face[vfl[i].face[j]][1];
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
     for(j=0;j<vfl[i].n_faces;j++){
       dmoy+=error_per_face[vfl[i].face[j]][0];
       surfacemoy+=error_per_face[vfl[i].face[j]][1];
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
