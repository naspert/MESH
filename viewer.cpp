/* $Id: viewer.cpp,v 1.29 2001/08/09 12:43:57 aspert Exp $ */

#include <time.h>
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
  clock_t start_time;
  char *in_filename1, *in_filename2;
  model *raw_model1, *raw_model2;
  double sampling_step;
  double dmoy,dmoymax=0,dmoymin=DBL_MAX;
  struct face_list *vfl;
  int i,j,k;
  double surfacemoy=0;
  QString m1,n1,o1;
  int text=1;
  struct face_error *fe = NULL;
  struct dist_surf_surf_stats stats;
  double bbox1_diag,bbox2_diag;
  double *tmp_error;

  /* affichage de la fenetre graphique */
  QApplication::setColorSpec( QApplication::CustomColor );
  QApplication a( argc, argv ); 


  /* Get arguments */
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
  
  /* Get the number of samples per triangle in each direction */
  k=(int)ceil(1.0/sampling_step);

  /* Read models from input files */
  raw_model1=read_raw_model(in_filename1);
  raw_model2=read_raw_model(in_filename2);

  /* Compute the distance from one model to the other */
  start_time = clock();
  dist_surf_surf(raw_model1,raw_model2,k+1,&fe,&stats,0);

  /* Print results */
  bbox1_diag = dist(raw_model1->bBox[0],raw_model1->bBox[1]);
  bbox2_diag = dist(raw_model2->bBox[0],raw_model2->bBox[1]);
  printf("\n              Model information\n\n");
  printf("                \t    Model 1\t    Model 2\n");
  printf("Number of vertices:\t%11d\t%11d\n",
         raw_model1->num_vert,raw_model2->num_vert);
  printf("Number of triangles:\t%11d\t%11d\n",
         raw_model1->num_faces,raw_model2->num_faces);
  printf("BoundingBox diagonal:\t%11g\t%11g\n",
         bbox1_diag,bbox2_diag);
  printf("Surface area:   \t%11g\t%11g\n",
         stats.m1_area,stats.m2_area);
  printf("\n       Distance from model 1 to model 2\n\n");
  printf("        \t   Absolute\t%% BBox diag\n");
  printf("        \t           \t  (Model 2)\n");
  printf("Min:    \t%11g\t%11g\n",
         stats.min_dist,stats.min_dist/bbox2_diag);
  printf("Max:    \t%11g\t%11g\n",
         stats.max_dist,stats.max_dist/bbox2_diag);
  printf("Mean:   \t%11g\t%11g\n",
         stats.mean_dist,stats.mean_dist/bbox2_diag);
  printf("RMS:    \t%11g\t%11g\n",
         stats.rms_dist,stats.rms_dist/bbox2_diag);
  printf("\n");
  printf("Calculated error in %g seconds\n",
         (double)(clock()-start_time)/CLOCKS_PER_SEC);
  printf("Used %d samples per triangle of model 1\n",(k+1)*(k+2)/2);
  printf("Size of partitioning grid (X,Y,Z): %d %d %d\n",stats.grid_sz.x,
         stats.grid_sz.y,stats.grid_sz.z);
  fflush(stdout);

  if(text == 1){
    /* Get the faces incident on each vertex to assign error values to each
     * vertex for display. */
    vfl = faces_of_vertex(raw_model1);

   /* on assigne une couleur a chaque vertex qui est proportionnelle */
   /* a la moyenne de l'erreur sur les faces incidentes */
   raw_model1->error=(int *)malloc(raw_model1->num_vert*sizeof(int));
   raw_model2->error=(int *)calloc(raw_model2->num_vert,sizeof(int));



   tmp_error = (double*)malloc(raw_model1->num_vert*sizeof(double));

   for(i=0; i<raw_model1->num_vert; i++){
     dmoy = 0;
     surfacemoy = 0;
     for(j=0; j<vfl[i].n_faces; j++) {
       dmoy += fe[vfl[i].face[j]].mean_error*fe[vfl[i].face[j]].face_area;
       surfacemoy += fe[vfl[i].face[j]].face_area;
     }
     dmoy /= surfacemoy;
     tmp_error[i] = dmoy;

     if(dmoy>dmoymax)
       dmoymax = dmoy;

     if(dmoy<dmoymin)
       dmoymin = dmoy;
   }
   
   for(i=0;i<raw_model1->num_vert;i++){
     raw_model1->error[i] = 
       (int)floor(7*(tmp_error[i] - dmoymin)/(dmoymax - dmoymin));
     if(raw_model1->error[i]<0)
       raw_model1->error[i]=0;
   }
   
   free(tmp_error);

   
   ScreenWidget c(raw_model1, raw_model2, dmoymin, dmoymax);
   a.setMainWidget( &c );
   c.show(); 
   a.exec();

 }
 
}
