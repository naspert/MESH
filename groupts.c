vertex** groupts(model *raw_model)
{
vertex **cellpt;
sample *sample1;
int i,j;
int m,n,o;
vertex bbox0,bbox1,point,test;
int cellule;
int memoire[1000][1];

cellpt=(vertex**)malloc(1000*sizeof(vertex*));

bbox0=raw_model2->BBOX[0];
bbox1=raw_model2->BBOX[1];

test.x=-1;
test.y=-1;
test.z=-1;

 for(i=0;i<raw_model->nbfaces;i++){
   sample1=echantillon(raw_model->vertices[raw_model->faces[i].f0],
		       raw_model->vertices[raw_model->faces[i].f1],
		       raw_model->vertices[raw_model->faces[i].f2],
		       samplethin); 
   for(j=0;j<sample1->nbsamples;j++){
     point=sample1->sample[j];
   
     m=(point.x-bbox0.x)*10/(bbox1.x-bbox0.x);
     n=(point.y-bbox0.y)*10/(bbox1.y-bbox0.y);
     o=(point.z-bbox0.z)*10/(bbox1.z-bbox0.z);
     cellule=m+n*10+o*100;
     
     if(memoire[cellule][0]==0)
       cellpt[cellule]=NULL;
     cellpt[cellule]=(vertex*)realloc(cellpt[cellule],(memoire[cellule][0]+1)*sizeof(vertex));
     cellpt[cellule][memoire[cellule][0]]=point;
     memoire[cellule][0]++;
   }
 }

 for(i=0;i<1000;i++){
   if(memoire[i][0]==0)
     cellpt[i]=NULL;
   cellpt[i]=(vertex*)realloc(cellpt[i],(memoire[i][0]+1)*sizeof(vertex));
   cellpt[i][memoire[i][0]]=test;
 }

return(cellpt);

}

