#include <3dmodel.h>
#include <geomutils.h>
#include <subdiv_methods.h>

void compute_face_midpoint_kobsqrt3(const struct ring_info* rings, 
                                    const int fidx, 
                                    const struct model *raw_model, 
                                    vertex_t *vout)
{
  int v0, v1, v2;
  vertex_t p;
  v0 = raw_model->faces[fidx].f0;
  v1 = raw_model->faces[fidx].f1;
  v2 = raw_model->faces[fidx].f2;

  __add_v(raw_model->vertices[v0], raw_model->vertices[v1], p);
  __add_v(raw_model->vertices[v2], p, p);
  __prod_v(1/3.0, p, *vout);
  
}


void update_vertices_kobsqrt3(const struct model *or_model, 
                              struct model *subdiv_model, 
                              const struct ring_info *rings) 
{
  int i, j, n;
  float alpha;
  vertex_t tmp;

  for (i=0; i<or_model->num_vert; i++) {
    if (rings[i].type != 0)
      abort();
    n = rings[i].size;
    alpha = (4. - 2.*cos(2*M_PI/n))/(9.*n);
    __prod_v(1.-n*alpha, or_model->vertices[i], tmp);
    for (j=0; j< rings[i].size; j++) 
      __add_prod_v(alpha, or_model->vertices[rings[i].ord_vert[j]], tmp, tmp);
    
    subdiv_model->vertices[i] = tmp;
  }
  
}
