/* $Id: viewer.cpp,v 1.4 2001/05/03 10:57:31 jacquet Exp $ */

#include <qapplication.h>
#include <ScreenWidget.h>
#include <compute_error.h>

/*****************************************************************************/
/***********   fonction principale                                      ******/
/*****************************************************************************/


int main( int argc, char **argv )
{
  char *in_filename1, *in_filename2;
  model *raw_model1, *raw_model2;
  int grid;
  cellules *cell;
  double samplethin,dcourant,dmax=0,superdmax=0,dmin=200,superdmin=200;
  double dmoy,dmoymax=0,dmoymin=200;
  int **repface;
  int **list_face;
  int *nbfaces;
  double *moyfacerror;
  int i,j,h=0;
  sample *sample2;
  FILE *f;



  f=fopen("dida","w");

  if (argc!=4) {
    printf("nbre d'arg incorrect\n");
    printf("le 1er argument correspond a l'objet de plus basse resolution\n");
    printf("le 2nd argument correspond a l'objet de plus haute resolution\n");
    printf("le 3eme argument correspond au pas d'echantillonnage\n");
    exit(-1);
  }

  in_filename1 = argv[1];
  in_filename2 = argv[2];
  samplethin=atof(argv[3]);


  raw_model1=read_raw_model(in_filename1);
  raw_model2=read_raw_model(in_filename2);
  

  if(raw_model2->num_faces<1000)
    grid=10;
  else if(raw_model2->num_faces<10000)
    grid=20;
  else if(raw_model2->num_faces<100000)
    grid=30;

  cell=liste(grid,raw_model2);
  repface=cublist(cell,grid,raw_model2);

  moyfacerror=(double *)malloc(raw_model1->num_faces*sizeof(double));
  list_face=(int **)malloc(raw_model1->num_vert*sizeof(int *));
  nbfaces=(int *)calloc(raw_model1->num_vert,sizeof(int));

  listoffaces(raw_model1,nbfaces,list_face);
  
  for(i=0;i<raw_model1->num_faces;i++) {
    sample2=echantillon(raw_model1->vertices[raw_model1->faces[i].f0],
			raw_model1->vertices[raw_model1->faces[i].f1],
			raw_model1->vertices[raw_model1->faces[i].f2],
			samplethin);
    dmoy=0;
    for(j=0;j<sample2->nbsamples;j++){
      dcourant=pcd(sample2->sample[j],raw_model2,repface,grid,f);
      dmoy+=dcourant;
      if(dcourant>dmax)
       dmax=dcourant;
     if(dcourant<dmin)
       dmin=dcourant;
     h++;
    }
    
    dmoy/=sample2->nbsamples;
    moyfacerror[i]=dmoy;
    if(dmoy>dmoymax)
      dmoymax=dmoy;
    if(dmoy<dmoymin)
      dmoymin=dmoy;

    printf("face numero %d: dmax= %lf dmoy= %lf\n",i+1,dmax,dmoy);
    if(dmax>superdmax)
      superdmax=dmax;
    dmax=0;
    if(dmin<superdmin)
      superdmin=dmin;
    dmin=200;
    
    free(sample2->sample);
    
    free(sample2);
    
  }
  printf("distance maximale: %lf dist moy max: %lf\n",superdmax,dmoymax);
  printf("distance minimale: %lf dist moy min: %lf\n",superdmin,dmoymin); 
  printf("nbsampleteste: %d\n",h);

  
  raw_model1->error=(int *)malloc(raw_model1->num_vert*sizeof(int));
  raw_model2->error=(int *)calloc(raw_model2->num_vert,sizeof(int));
 

  for(i=0;i<raw_model1->num_vert;i++) {
    dcourant=0;  
    for(j=0;j<nbfaces[i];j++){ 
      dcourant+=(moyfacerror[list_face[i][j]]-dmoymin)/(dmoymax-dmoymin);
    }
    dcourant/=nbfaces[i];
    raw_model1->error[i]=dcourant*15;
    if(raw_model1->error[i]>15)
      raw_model1->error[i]=15;
    //raw_model1->error[i]=(int)floor(dcourant); 
    fprintf(f,"%d ",raw_model1->error[i]); 
    //fprintf(f,"%lf ",dcourant);
  }

  QApplication::setColorSpec( QApplication::CustomColor );
  QApplication a( argc, argv );  
  
  ScreenWidget w(raw_model1,raw_model2);
  a.setMainWidget( &w );
  w.show();
  return a.exec();
}
