/* $Id: rawview.c,v 1.36 2003/03/24 12:16:38 aspert Exp $ */
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>


#include <3dutils.h>
#include <rawview.h>
#include <rawview_misc.h>
#include <subdiv.h>
#include <subdiv_methods.h>
#include <assert.h>
#ifdef DEBUG
# include <debug_print.h>
#endif
#include <stdarg.h>

/* ****************** */
/* Useful Global vars */
/* ****************** */

/* GL renderer context */
static struct gl_render_context gl_ctx;

/* Subdiv functions structure */
static struct subdiv_methods sm;

/* storage for mouse stuff */
static struct mouse_state mouse;

/* display lists indices */
static struct display_lists_indices dl_idx;

/* prototypes */
static void gfx_init(void);
static void display(void);

/* ************************************************************ */
/* Here is the callback function when mouse buttons are pressed */
/* or released. It does nothing else than store their state     */
/* ************************************************************ */
static void mouse_button(int button, int state, int x, int y) {
  switch(button) {
  case GLUT_LEFT_BUTTON:
    if (state==GLUT_DOWN) {
      mouse.oldx = x;
      mouse.oldy = y;
      mouse.left_button_state = 1;
    } else if (state == GLUT_UP)
      mouse.left_button_state = 0;
    break;
  case GLUT_MIDDLE_BUTTON:
    if (state==GLUT_DOWN) {
      mouse.oldx = x;
      mouse.oldy = y;
      mouse.middle_button_state = 1;
    } else if (state == GLUT_UP)
      mouse.middle_button_state = 0;
    break;
  case GLUT_RIGHT_BUTTON:
    if (state==GLUT_DOWN) {
      mouse.oldx = x;
      mouse.oldy = y;
      mouse.right_button_state = 1;
    } else if (state == GLUT_UP)
      mouse.right_button_state = 0;
    break;
  default:
    break;
  }
}


/* ********************************************************* */
/* Callback function when the mouse is dragged in the window */
/* Only does sthg when a button is pressed                   */
/* ********************************************************* */
static void motion_mouse(int x, int y) {
  int dx, dy;
  GLfloat dth, dph, dpsi;

  dx = x - mouse.oldx;
  dy = y - mouse.oldy;

  if (mouse.left_button_state == 1) {
    dth = dx*ANGLE_STEP; 
    dph = dy*ANGLE_STEP;
    glPushMatrix(); /* Save transform context */
    glLoadIdentity();
    glRotated(dth, 0.0, 1.0, 0.0); /* Compute new rotation matrix */
    glRotated(dph, 1.0, 0.0, 0.0);
    glMultMatrixd(gl_ctx.mvmatrix); /* Add the sum of the previous ones */
    glGetDoublev(GL_MODELVIEW_MATRIX, gl_ctx.mvmatrix); /* Get the
                                                         * final matrix */
    glPopMatrix(); /* Reload previous transform context */
    glutPostRedisplay();
  }
  else if (mouse.middle_button_state == 1) {
    gl_ctx.distance += dy*gl_ctx.dstep;
    glutPostRedisplay();
  }
  else if (mouse.right_button_state == 1) { 
    dpsi = -dx*ANGLE_STEP;
    glPushMatrix(); /* Save transform context */
    glLoadIdentity();
    glRotated(dpsi, 0.0, 0.0, 1.0); /* Modify roll angle */
    glMultMatrixd(gl_ctx.mvmatrix);
    glGetDoublev(GL_MODELVIEW_MATRIX, gl_ctx.mvmatrix); /* Get the
                                                         * final matrix */
    glPopMatrix(); /* Reload previous transform context */
    glutPostRedisplay();
  }
  mouse.oldx = x;
  mouse.oldy = y;
}

/* ********************************************************** */
/* Reshape callbak function. Only sets correct values for the */
/* viewport and projection matrices.                          */
/* ********************************************************** */
static void reshape(int width, int height) {
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(FOV, (GLdouble)width/(GLdouble)height, gl_ctx.distance/10.0, 
		 10.0*gl_ctx.distance);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
}


/* ******************************************** */
/* Initial settings of the rendering parameters */
/* ******************************************** */
static void gfx_init() {
  const char *glverstr;

  glverstr = (const char*)glGetString(GL_VERSION);
  verbose_printf(gl_ctx.verbose, "GL_VERSION = %s\n", glverstr);




  glEnable(GL_DEPTH_TEST);
  glShadeModel(GL_SMOOTH); 

  glClearColor(0.0, 0.0, 0.0, 0.0);
  glColor3f(1.0, 1.0, 1.0); /* Settings for wireframe model */
  glFrontFace(GL_CCW);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  rebuild_list(&gl_ctx, &dl_idx);    

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(FOV, 1.0, gl_ctx.distance/10.0, 10.0*gl_ctx.distance);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  if (gl_ctx.load_coord == 0)
    glGetDoublev(GL_MODELVIEW_MATRIX, gl_ctx.mvmatrix); /* Initialize
                                                         * the temp
                                                         * matrix */
}



/* *****************************************************************
 * Display callback - This just calls the *true* function which lies
 * inside rawview_misc.c 
 * ***************************************************************** */
static void display() {
  display_wrapper(&gl_ctx, &dl_idx);
}



/* **************************** */
/* Callback for the normal keys */
/* **************************** */
static void norm_key_pressed(unsigned char key, int x, int y) {
  struct model *sub_model;


  switch(key) {
  case 'b':
  case 'B':
    verbose_printf(gl_ctx.verbose, "Butterfly subdivision...\n");
    sub_model = subdiv(gl_ctx.raw_model, &(sm.butterfly));
    if (sub_model != NULL) {
      sub_model->bBox[0] = gl_ctx.raw_model->bBox[0];
      sub_model->bBox[1] = gl_ctx.raw_model->bBox[1];
      __free_raw_model(gl_ctx.raw_model);
      gl_ctx.normals_done = 0;
      gl_ctx.curv_done = 0;
      gl_ctx.disp_curv = 0;
      gl_ctx.draw_normals = 0;
      set_light_off();
      gl_ctx.raw_model = sub_model;
      rebuild_list(&gl_ctx, &dl_idx);
      glutPostRedisplay();
    } else 
      fprintf(stderr, "Butterfly subdivision failed\n");
    break;
  case 'd':
  case 'D':
    verbose_printf(gl_ctx.verbose, "Dumping viewing info\n");
    coord_grab(&gl_ctx);
    break;
  case 'g':
  case 'G': /* Enable Gaussian curvature display */
    if (gl_ctx.disp_curv != 1) {
      if (!gl_ctx.curv_done) { /* Compute curvatures at each vertex */
        if (do_curvature(&gl_ctx)) {
          fprintf(stderr, "Unable to compute curvatures...\n");
          return;
        }
        gl_ctx.curv_done = 1;
      }
      verbose_printf(gl_ctx.verbose, "Displaying Gauss curvature\n");
      verbose_printf(gl_ctx.verbose, "min_kg = %f max_kg = %f\n", 
                     gl_ctx.min_kg, gl_ctx.max_kg);
      gl_ctx.disp_curv = 1;
      set_light_off();
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    } else {
      gl_ctx.disp_curv = 0;
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    
    rebuild_list(&gl_ctx, &dl_idx);
    glutPostRedisplay();
    break;
  case 'i':
  case 'I':
    fprintf(stderr, "\nModel Info :\n %d vertices and %d triangles\n\n", 
            gl_ctx.raw_model->num_vert, gl_ctx.raw_model->num_faces);
    break;
  case 'k':
  case 'K':
    verbose_printf(gl_ctx.verbose, "Kobbelt-sqrt3 subdivision...\n");
    sub_model = subdiv_sqrt3(gl_ctx.raw_model, &(sm.kob_sqrt3));
    if (sub_model != NULL) {
      sub_model->bBox[0] = gl_ctx.raw_model->bBox[0];
      sub_model->bBox[1] = gl_ctx.raw_model->bBox[1];
      __free_raw_model(gl_ctx.raw_model);
      gl_ctx.normals_done = 0;
      gl_ctx.curv_done = 0;
      gl_ctx.disp_curv = 0;
      gl_ctx.draw_normals = 0;
      set_light_off();
      gl_ctx.raw_model = sub_model;
      rebuild_list(&gl_ctx, &dl_idx);
      glutPostRedisplay();
    } else 
      fprintf(stderr, "Kobbelt-sqrt3 subdivision failed\n");
    break;
  case 'l':
  case 'L':
    verbose_printf(gl_ctx.verbose, "Loop subdivision...\n");
    sub_model = subdiv(gl_ctx.raw_model, &(sm.loop));
    if (sub_model != NULL) {
      sub_model->bBox[0] = gl_ctx.raw_model->bBox[0];
      sub_model->bBox[1] = gl_ctx.raw_model->bBox[1];
      __free_raw_model(gl_ctx.raw_model);
      gl_ctx.normals_done = 0;
      gl_ctx.curv_done = 0;
      gl_ctx.disp_curv = 0;
      gl_ctx.draw_normals = 0;
      set_light_off();
      gl_ctx.raw_model = sub_model;
      rebuild_list(&gl_ctx, &dl_idx);
      glutPostRedisplay();
    } else 
      fprintf(stderr, "Loop subdivision failed\n");
    break;
  case 'm':
  case 'M': /* Enable Mean curvature display */
    if (gl_ctx.disp_curv != 2) {
      if (!gl_ctx.curv_done) { /* Compute curvatures at each vertex */
        if (do_curvature(&gl_ctx)) {
          fprintf(stderr, "Unable to compute curvatures...\n");
          return;
        }
        gl_ctx.curv_done = 1;
      }
      verbose_printf(gl_ctx.verbose, "Displaying mean curvature\n");
      verbose_printf(gl_ctx.verbose, "min_km = %f max_km = %f\n", 
                     gl_ctx.min_km, gl_ctx.max_km);
      gl_ctx.disp_curv = 2;
      set_light_off();
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    } else {
      gl_ctx.disp_curv = 0;
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    
    rebuild_list(&gl_ctx, &dl_idx);
    glutPostRedisplay();
    break;
  case 'q':
  case 'Q':
    if (gl_ctx.raw_model->tree != NULL) {
      destroy_tree(gl_ctx.raw_model->tree, gl_ctx.raw_model->num_faces);
      gl_ctx.raw_model->tree = NULL;
    }
    if (gl_ctx.info != NULL)
      free(gl_ctx.info);

    __free_raw_model(gl_ctx.raw_model);
    exit(0);
    break;
  case 's':
  case 'S':
    verbose_printf(gl_ctx.verbose, "Spherical subdivision...\n");
    if (!gl_ctx.normals_done) {
      if (!do_normals(gl_ctx.raw_model, gl_ctx.verbose)) 
        gl_ctx.normals_done = 1;
      else {
        fprintf(stderr, "Unable to compute normals ...\n");
        break;
      }
    }
    sub_model = subdiv(gl_ctx.raw_model, &(sm.spherical));
    if (sub_model != NULL) {
      sub_model->bBox[0] = gl_ctx.raw_model->bBox[0];
      sub_model->bBox[1] = gl_ctx.raw_model->bBox[1];
      __free_raw_model(gl_ctx.raw_model);
      gl_ctx.normals_done = 0;
      gl_ctx.curv_done = 0;
      gl_ctx.disp_curv = 0;
      gl_ctx.draw_normals = 0;
      set_light_off();
      gl_ctx.raw_model = sub_model;
      rebuild_list(&gl_ctx, &dl_idx);
      glutPostRedisplay();
    } else 
      fprintf(stderr, "Loop subdivision failed\n");
    break;
  default:
    break;
  }
}



/* ********************************************************* */
/* Callback function for the special keys (arrows, function) */
/* ********************************************************* */
static void sp_key_pressed(int key, int x, int y) {


  GLboolean light_mode;
  int i;

  switch(key) {
  case GLUT_KEY_F1:/* Print Help */
    fprintf(stderr, "\n***********************\n");
    fprintf(stderr, "* Rawview    -   Help *\n");
    fprintf(stderr, "***********************\n\n");
    fprintf(stderr, "b/B      :\tPerforms Butterfly subdivision\n");
    fprintf(stderr, "d/D      :\tSaves the viewing coordinates in a file (coordxxx.mat)\n");
    fprintf(stderr, "g/G      :\tDisplay Gaussian curvature\n");
    fprintf(stderr, "i/I      :\tDisplay model information\n");
    fprintf(stderr, "l/L      :\tPerforms Loop subdivision\n");
    fprintf(stderr, "m/M      :\tDisplays mean curvature\n");
    fprintf(stderr, "s/S      :\tPerforms Spherical subdivision\n\n");
    fprintf(stderr, "F1       :\tDisplays this help\n");
    fprintf(stderr, "F2       :\tToggles lighted/wireframe mode\n");
    fprintf(stderr, "F3       :\tInvert normals (if any)\n");
    fprintf(stderr, "F4       :\tDraw vertex normals (if any)\n");
    fprintf(stderr, "F5       :\tSave model (incl. normals)\n");
    fprintf(stderr, "F6       :\tGrab the frame to a PPM file (grabxxx.ppm)\n");
    fprintf(stderr, "F7       :\tToggle triangle/point mode\n");
    fprintf(stderr, "F8       :\tDraw vertices' labels (be careful !)\n");
    fprintf(stderr, "F9       :\tDraw spanning tree (if any)\n");
    fprintf(stderr, "F10      :\tRender in a EPS file (uses 'gl2ps')\n");
    fprintf(stderr, "Shift+F10:\tSame as F10 but in negative\n");
    fprintf(stderr, "F11      :\tToggle backface culling\n\n\n");
    fprintf(stderr, "Send bugs to Nicolas.Aspert@epfl.ch\n\t\t\tHave fun.\n");
    return;
  case GLUT_KEY_F2: /* Toggle Light+filled mode */
    light_mode = glIsEnabled(GL_LIGHTING);
    if (light_mode == GL_FALSE) {
      verbose_printf(gl_ctx.verbose, "Lighted mode\n");
      if (gl_ctx.normals_done) {
        set_light_on();
        rebuild_list(&gl_ctx, &dl_idx);
      }
      else { /* We have to build the normals */
	if (!do_normals(gl_ctx.raw_model, gl_ctx.verbose)) { /* success */
          gl_ctx.normals_done = 1;
          gl_ctx.disp_curv = 0;
          set_light_on();
          rebuild_list(&gl_ctx, &dl_idx);
        } else 
          fprintf(stderr, "Unable to compute normals... non-manifold model\n");
      }
    } else { /* light_mode == GL_TRUE */
      set_light_off();
      rebuild_list(&gl_ctx, &dl_idx);
    } 
    break;
  case GLUT_KEY_F3: /* invert normals */
    light_mode = glIsEnabled(GL_LIGHTING);
    if (light_mode || gl_ctx.draw_normals) {
      verbose_printf(gl_ctx.verbose, "Inverting normals\n");
      for (i=0; i<gl_ctx.raw_model->num_vert; i++) 
	neg_v(&(gl_ctx.raw_model->normals[i]), 
              &(gl_ctx.raw_model->normals[i]));

      rebuild_list(&gl_ctx, &dl_idx);
    }
    break;
  case GLUT_KEY_F4: /* draw normals */
    if (gl_ctx.draw_normals == 0) {
      gl_ctx.draw_normals = 1;
      verbose_printf(gl_ctx.verbose, "Draw normals\n");
      if (gl_ctx.normals_done) {
        rebuild_list(&gl_ctx, &dl_idx);
      }
      else { /* We have to build the normals */
	if (!do_normals(gl_ctx.raw_model, gl_ctx.verbose)) { /* success */
          gl_ctx.normals_done = 1;
          rebuild_list(&gl_ctx, &dl_idx);
        } else {
          fprintf(stderr, "Unable to compute normals... non-manifold model\n");
          gl_ctx.draw_normals = 0;
        }
      }
    } 
    else { /* gl_ctx.draw_normals == 1 */
      gl_ctx.draw_normals = 0;
      rebuild_list(&gl_ctx, &dl_idx);
    }
    break;
  case GLUT_KEY_F5: /* Save model... useful for normals */
    verbose_printf(gl_ctx.verbose, "Write model...\n");
    write_raw_model(gl_ctx.raw_model, gl_ctx.in_filename);
    return;
  case GLUT_KEY_F6: /* Frame grab */
    frame_grab(&gl_ctx);
    return;
  case GLUT_KEY_F7: /* switch from triangle mode to point mode */
    if(gl_ctx.tr_mode == 1) {/*go to point mode*/
      gl_ctx.tr_mode = 0;
      verbose_printf(gl_ctx.verbose, "Going to point mode\n");

    } else if(gl_ctx.tr_mode == 0) {
      gl_ctx.tr_mode = 1;
      verbose_printf(gl_ctx.verbose, "Going to triangle mode\n");
    }
    rebuild_list(&gl_ctx, &dl_idx);
    break;
  case GLUT_KEY_F8: /* draw labels for vertices */
    if (gl_ctx.draw_vtx_labels == 1) {
      gl_ctx.draw_vtx_labels = 0;
      verbose_printf(gl_ctx.verbose, "Stop drawing labels\n");
    } else if (gl_ctx.draw_vtx_labels == 0) {
      gl_ctx.draw_vtx_labels = 1;
      verbose_printf(gl_ctx.verbose, "Drawing labels\n");
    }
    rebuild_list(&gl_ctx, &dl_idx);
    break;

  case GLUT_KEY_F9: /* Draw the spanning tree */
    if(gl_ctx.draw_spanning_tree == 1) {
      gl_ctx.draw_spanning_tree = 0;
      verbose_printf(gl_ctx.verbose, "Stop drawing spanning tree\n");
    } else if (gl_ctx.draw_spanning_tree == 0) {
      gl_ctx.draw_spanning_tree = 1;
      verbose_printf(gl_ctx.verbose, "Drawing spanning tree\n");
      if (gl_ctx.raw_model->tree == NULL) { /* We need to build this ...*/
        if (do_spanning_tree(gl_ctx.raw_model, gl_ctx.verbose))
          gl_ctx.draw_spanning_tree = 0;
      }
    }
    rebuild_list(&gl_ctx, &dl_idx);
    break;

  case GLUT_KEY_F10:
    verbose_printf(gl_ctx.verbose, "Rendering to a PostScript file...\n");
    if (glutGetModifiers() ==  GLUT_ACTIVE_SHIFT)
      ps_grab(&gl_ctx, &dl_idx, 0); /* render negative */
    else 
      ps_grab(&gl_ctx, &dl_idx, 1);

    verbose_printf(gl_ctx.verbose, "done\n");
    break;

  case GLUT_KEY_F11: /* backface culling when in wf mode */
    if (gl_ctx.wf_bc)  /* goto classic wf mode */
      gl_ctx.wf_bc = 0;
    else 
      gl_ctx.wf_bc = 1;
    
    glDisable(GL_LIGHTING);
    glColor3f(1.0, 1.0, 1.0);
    glFrontFace(GL_CCW);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    rebuild_list(&gl_ctx, &dl_idx);
    break;

  case GLUT_KEY_UP:
    glPushMatrix(); /* Save transform context */
    glLoadIdentity();
    glRotated(-5.0, 1.0, 0.0, 0.0);
    glMultMatrixd(gl_ctx.mvmatrix); /* Add the sum of the previous ones */
    glGetDoublev(GL_MODELVIEW_MATRIX, gl_ctx.mvmatrix); /* Get the
                                                         * final matrix */
    glPopMatrix(); /* Reload previous transform context */
    break;
  case GLUT_KEY_DOWN:
    glPushMatrix(); 
    glLoadIdentity();
    glRotated(5.0, 1.0, 0.0, 0.0);
    glMultMatrixd(gl_ctx.mvmatrix); 
    glGetDoublev(GL_MODELVIEW_MATRIX, gl_ctx.mvmatrix); 
    glPopMatrix();
    break;
  case GLUT_KEY_LEFT:
    glPushMatrix();
    glLoadIdentity();
    glRotated(-5.0, 0.0, 1.0, 0.0);
    glMultMatrixd(gl_ctx.mvmatrix); 
    glGetDoublev(GL_MODELVIEW_MATRIX, gl_ctx.mvmatrix); 
    glPopMatrix();
    break;
  case GLUT_KEY_RIGHT:
    glPushMatrix();
    glLoadIdentity();
    glRotated(5.0, 0.0, 1.0, 0.0);
    glMultMatrixd(gl_ctx.mvmatrix); 
    glGetDoublev(GL_MODELVIEW_MATRIX, gl_ctx.mvmatrix); 
    glPopMatrix();
    break;
  case GLUT_KEY_PAGE_DOWN:
    glPushMatrix();
    glLoadIdentity();
    glRotated(-5.0, 0.0, 0.0, 1.0);
    glMultMatrixd(gl_ctx.mvmatrix); 
    glGetDoublev(GL_MODELVIEW_MATRIX, gl_ctx.mvmatrix); 
    glPopMatrix();
    break;
  case GLUT_KEY_END:
    glPushMatrix();
    glLoadIdentity();
    glRotated(5.0, 0.0, 0.0, 1.0);
    glMultMatrixd(gl_ctx.mvmatrix); 
    glGetDoublev(GL_MODELVIEW_MATRIX, gl_ctx.mvmatrix); 
    glPopMatrix();
    break;
  default:
    break;
  }
  glutPostRedisplay();
}


/* ************************************************************ */
/* Main function : read model, compute initial bounding box/vp, */
/* perform callback registration and go into the glutMainLoop   */
/* ************************************************************ */
int main(int argc, char **argv) {

  int i, rcode=0;
  char *title=NULL;
  const char s_title[]="Raw Mesh Viewer $Revision: 1.36 $ - ";
  vertex_t center;
  struct model* raw_model;


  /* Init GL renderer context */
  memset(&gl_ctx, 0, sizeof(struct gl_render_context));

  if (argc == 2) {
    gl_ctx.in_filename = malloc((strlen(argv[1])+1)*sizeof(char));
    strcpy(gl_ctx.in_filename, argv[1]);
  }
  else if (argc == 3 || argc == 4 || argc == 5) {
    for (i=1; i<argc-1; i++) {
      if (strcmp(argv[i], "--verbose") == 0)
        gl_ctx.verbose = 1;
      else if (strcmp(argv[i], "--coordload") == 0) 
        coord_load(argv[++i], &gl_ctx);
    }

    gl_ctx.in_filename = malloc((strlen(argv[argc-1])+1)*sizeof(char));
    strcpy(gl_ctx.in_filename, argv[argc-1]);
  } else {
#ifdef DONT_USE_ZLIB
    fprintf(stderr, "Usage:%s [--verbose,--coordload coordfile] file.[raw, wrl, smf, ply, iv]\n", 
            argv[0]);
#else
    fprintf(stderr, 
            "Usage:%s [--verbose,--coordload coordfile] file.[raw, wrl, smf, ply, iv][.gz]\n", 
            argv[0]);
#endif
    exit(-1);

  }


  rcode = read_fmodel(&raw_model, gl_ctx.in_filename, MESH_FF_AUTO, 0);
  if (rcode < 0) {
    fprintf(stderr, "Unable to read model - error code %d\n", rcode);
    exit(-1);
  }  


  if (raw_model->builtin_normals == 1) {
    gl_ctx.normals_done = 1;
    verbose_printf(gl_ctx.verbose, "The model has builtin normals\n");
  }

  
#ifdef DEBUG
  DEBUG_PRINT("bbox_min = %f %f %f\n", raw_model->bBox[0].x, 
              raw_model->bBox[0].y, raw_model->bBox[0].z);
  DEBUG_PRINT("bbox_max = %f %f %f\n", raw_model->bBox[1].x, 
              raw_model->bBox[1].y, raw_model->bBox[1].z);
#endif

  add_v(&(raw_model->bBox[0]), &(raw_model->bBox[1]), &center);
  prod_v(0.5, &center, &center);


  /* Center the model around (0, 0, 0) */
  for (i=0; i<raw_model->num_vert; i++) 
    substract_v(&(raw_model->vertices[i]), &center, &(raw_model->vertices[i]));

  gl_ctx.tr_mode = 1; /* default -> wireframe w. triangles */
  
  if (gl_ctx.load_coord == 0)
    gl_ctx.distance = dist_v(&(raw_model->bBox[0]), &(raw_model->bBox[1]))/
      tan(FOV*M_PI_2/180.0);
  
  gl_ctx.dstep = gl_ctx.distance*TRANSL_STEP;

  gl_ctx.raw_model = raw_model;

  title = (char*)malloc((strlen(gl_ctx.in_filename)+strlen(s_title)+1)*
                        sizeof(char));
  strcpy(title, s_title);
  strcat(title, gl_ctx.in_filename);

  /* Init mouse state */
  memset(&mouse, 0, sizeof(struct mouse_state));
  
  /* Init display lists indices */
  memset(&dl_idx, 0, sizeof(struct display_lists_indices));

  /* Init subdiv function structure */
  INIT_SUBDIV_METHODS(sm);

  /* Init the rendering window */
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(500, 500);
  glutCreateWindow(title);
  free(title);

  /* Callback registration */
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutSpecialFunc(sp_key_pressed);
  glutKeyboardFunc(norm_key_pressed); 
  glutMouseFunc(mouse_button);
  glutMotionFunc(motion_mouse);

  /* 1st frame + build model */
  gfx_init();

  /* Go for it */
  glutMainLoop();

  /* should never get here */
  return 0;
}
