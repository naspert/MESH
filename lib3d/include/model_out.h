/* $Id: model_out.h,v 1.4 2004/04/30 07:50:21 aspert Exp $ */


/*
 *
 *  Copyright (C) 2001-2004 EPFL (Swiss Federal Institute of Technology,
 *  Lausanne)
 *  You are not allowed to redistribute this program without the explicit
 *  permission of the author.
 *
 *  Author : Nicolas Aspert
 *
 *  Contact : 
 *     Nicolas Aspert
 *     Signal Processing Institute (ITS)
 *     Swiss Federal Institute of Technology (EPFL)
 *     1015 Lausanne
 *     Switzerland
 *
 *     Tel : +41 21 693 3632
 *     E-Mail : Nicolas.Aspert@epfl.ch
 *
 *
 */



#ifndef _MODEL_OUT_PROTO
#define _MODEL_OUT_PROTO

#ifdef __cplusplus
extern "C" {
#endif

  void write_wrl_model(struct model*, char*);

#ifdef __cplusplus
}
#endif

#endif
