/* $Id$ */


/*
 *
 *  Copyright (C) 2001-2004 EPFL (Swiss Federal Institute of Technology,
 *  Lausanne) This program is free software; you can redistribute it
 *  and/or modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 *  USA.
 *
 *  In addition, as a special exception, EPFL gives permission to link
 *  the code of this program with the Qt non-commercial edition library
 *  (or with modified versions of Qt non-commercial edition that use the
 *  same license as Qt non-commercial edition), and distribute linked
 *  combinations including the two.  You must obey the GNU General
 *  Public License in all respects for all of the code used other than
 *  Qt non-commercial edition.  If you modify this file, you may extend
 *  this exception to your version of the file, but you are not
 *  obligated to do so.  If you do not wish to do so, delete this
 *  exception statement from your version.
 *
 *  Authors : Nicolas Aspert, Diego Santa-Cruz and Davy Jacquet
 *
 *  Web site : http://mesh.epfl.ch
 *
 *  Reference :
 *   "MESH : Measuring Errors between Surfaces using the Hausdorff distance"
 *   in Proceedings of IEEE Intl. Conf. on Multimedia and Expo (ICME) 2002, 
 *   vol. I, pp. 705-708, available on http://mesh.epfl.ch
 *
 */





#ifndef _MODEL_IN_PLY_PROTO
#define _MODEL_IN_PLY_PROTO


/* To test for little and big endianness of the platform. See
 * 'model_in_ply.c', in function 'read_ply_tmesh'. */
#define TEST_LITTLE_ENDIAN 0x04030201
#define TEST_BIG_ENDIAN    0x01020304

/* Associate the official types defined in the PLY specification to
 * integers, s.t. we can easily access their sizes using the
 * 'plys_sizes' array */
enum ply_types { not_valid=-1, 
                 int8, uint8, 
                 int16, uint16, 
                 int32, uint32, 
                 float32, float64 };

/* Sizes of each data type used in PLY files (in bytes) */
static const int ply_sizes[] = { 1, 1, 2, 2, 4, 4, 4, 8};

/* PLY properties _that are currently supported_. Properties that are
 * not recognized are tagged as 'unsupported' */
enum prop_names { unsupported, /* default */
                  v_x, /* x coord of a vertex */
                  v_y, /* y coord of a vertex */
                  v_z, /* z coord of a vertex */
                  v_idx /* vertex_indices (i.e. face) */
};

/* Structure used to represent a PLY property field. 
   'is_list'   : 1 if the current property is a list, 0 else
   'type_prop' : the type (taken from the 'ply_types' enum)
   'type_list' : the type of the first field (in case of a list)
   'prop'      : the property 'code' taken from the 'prop_names' enum
   */
struct ply_prop {
  int is_list;
  int type_prop;
  int type_list; /* In case it's a list */
  int prop;
};

/* The following unions are used to handle little/big endian
 * differences. The 'bs' fields are filled in the correct order, and
 * you can get the value (hopefully) right in 'bo' (Thks. Diego for
 * the tip :-) */
union sw_uint16 {
  t_uint8  bs[2];
  t_uint16 bo;
};

union sw_int16 {
  t_uint8 bs[2];
  t_int16 bo;
};

union sw_uint32 {
  t_uint8  bs[4];
  t_uint32 bo;
};

union sw_int32 {
  t_uint8 bs[4];
  t_int32 bo;
};

union sw_float32 {
  t_uint8 bs[4];
  float   bo;
};

union sw_float64 {
  t_uint8 bs[8];
  double  bo;
};





#endif
