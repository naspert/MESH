/* $Id: types.h,v 1.4 2003/01/13 12:46:09 aspert Exp $ */


/*
 *
 *  Copyright (C) 2001-2003 EPFL (Swiss Federal Institute of Technology,
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






/*
 * General type definitions:
 *
 * Created by Diego Santa Cruz
 *
 */



/* Defines types with known numbers of bits and the number of bits of each
 * type. The type definitions are redundant with the ones from the C99
 * standard that are found in <stdint.h>, but the names are slightly
 * different, in purpose. The intent is mainly for use in ANSI C or C89 since
 * only ANSI C features are used.
 *
 * The defined integral types are:
 *
 * - t_byte:            Unsigned integral type for 8 bit bytes.
 * - t_int8, t_uint8:   Signed and unsigned integral 8 bit types.
 * - t_int16, t_uint16: Signed and unsigned integral 16 bit types.
 * - t_int32, t_uint32: Signed and unsigned integral 32 bit types.
 * - t_int64, t_uint64: Signed and unsigned integral 64 bit types. Only
 *                      defined if T_DEFINE_64_BIT is defined.
 *
 * The #defines are:
 *
 * - T_BYTE_BITS:      Number of bits in the 'byte' type. Always 8.
 * - T_CHAR_BITS:      Number of bits in the 'char' 'signed char' and 'unsigned
 *                     char' types.
 * - T_SHRT_BITS:      Number of bits in the 'short int' and 'unsigned short
 *                     int' types. 
 * - T_INT_BITS:       Number of bits in the 'int' and 'unsigned int' types.
 * - T_LONG_BITS:      Number of bits in the 'long int' and 'unsigned long int'
 *                     types.
 * - T_LONG_LONG_BITS: Number of bits in the 'long long int' and 'unsigned long 
 *                     long int' types. Only defined if T_USE_LONG_LONG is
 *                     defined.
 * - T_INT8_SIGN_BIT:  Sign bit for 't_int8' type.
 * - T_INT16_SIGN_BIT: Sign bit for 't_int16' type.
 * - T_INT32_SIGN_BIT: Sign bit for 't_int32' type.
 * - T_INT64_SIGN_BIT: Sign bit for 't_int64' type. Only defined if
 *                     T_DEFINE_64_BIT is defined.
 *
 * It is checked that signed and unsigned versions of a type have the same
 * number of bits.
 *
 * Note that not all compilers support the 'long long int' type, although it
 * is required by the C99 standard. Under some machines (typically 32 bit
 * ones) 'long long int' is required for 64 bit types.
 * */

#ifndef _TYPES_H_ /* Protect against double inclusion */
#define _TYPES_H_

#include <limits.h>

/* Do we have 8 bits bytes ? */
#if (UCHAR_MAX == 255)
#  define T_BYTE_BITS 8
#else
#  error "Unsigned char is not an 8 bit byte"
#endif

/* Get number of bits in char types */
#if (SCHAR_MAX == 127)
#  define T_CHAR_BITS 8
#else
#  error "Type char is not 8 bits"
#endif
#if (UCHAR_MAX != (SCHAR_MAX * 2U + 1))
#  error "Number of bits of unsigned short and short types differ"
#endif

/* Get number of bits in short int types */
#if (SHRT_MAX == 32767)
#  define T_SHRT_BITS 16
#elif (SHRT_MAX == 2147483647)
#  define T_SHRT_BITS 32
#else
#  error "Could not determine number of bits of short type"
#endif
#if (USHRT_MAX != (SHRT_MAX * 2U + 1))
#  error "Number of bits of unsigned short and short types differ"
#endif

/* Get number of bits in int types */
#if (INT_MAX == 32767)
#  define T_INT_BITS 16
#elif (INT_MAX == 2147483647)
#  define T_INT_BITS 32
#elif (INT_MAX == 9223372036854775807)
#  define T_INT_BITS 64
#else
#  error "Could not determine number of bits of int type"
#endif
#if (UINT_MAX != (INT_MAX * 2U + 1))
#  error "Number of bits of unsigned int and int types differ"
#endif

/* Get number of bits in long int types */
#if (LONG_MAX == 32767L)
#  define T_LONG_BITS 16
#elif (LONG_MAX == 2147483647L)
#  define T_LONG_BITS 32
#elif (LONG_MAX == 9223372036854775807L)
#  define T_LONG_BITS 64
#else
#  error "Could not determine number of bits of long type"
#endif
#if (ULONG_MAX != (LONG_MAX * 2UL + 1))
#  error "Number of bits of unsigned long and long types differ"
#endif

/* Get number of bits in long long int types. Only if T_USE_LONG_LONG is
 * defined */
#ifdef T_USE_LONG_LONG
#  if (LONG_LONG_MAX == 2147483647L)
#    define T_LONG_LONG_BITS 32
#  elif (LONG_MAX == 9223372036854775807L)
#    define T_LONG_LONG_BITS 64
#  else
#    error "Could not determine number of bits of long long type"
#  endif
#  if (ULONG_LONG_MAX != (LONG_LONG_MAX * 2ULL + 1))
#    error "Number of bits of unsigned long long and long long types differ"
#  endif
#endif

/* Byte type (unsigned, 8 bits) */
typedef unsigned char t_byte;

/* Define the 8 bit types */
/* We only support 8 bit chars */
typedef signed char t_int8;
typedef unsigned char t_uint8;
#define T_INT8_SIGN_BIT (1<<7);

/* Define 16 bit types */
#if (T_SHRT_BITS == 16)
typedef signed short int t_int16;
typedef unsigned short int t_uint16;
#elif (T_INT_BITS == 16)
typedef signed int t_int16;
typedef unsigned int t_uint16;
#elif (T_LONG_BITS == 16)
typedef signed long int t_int16;
typedef unsigned long int t_uint16;
#else
#  error "Could not determine 16 bit integral types"
#endif
#define T_INT16_SIGN_BIT (1<<15);

/* Define 32 bit types */
#if (T_SHRT_BITS == 32)
typedef signed short int t_int32;
typedef unsigned short int t_uint32;
#elif (T_INT_BITS == 32)
typedef signed int t_int32;
typedef unsigned int t_uint32;
#elif (T_LONG_BITS == 32)
typedef signed long int t_int32;
typedef unsigned long int t_uint32;
#elif (defined(T_USE_LONG_LONG) && T_LONG_LONG_BITS == 32)
typedef signed long long int t_int32;
typedef unsigned long long int t_uint32;
#else
#  error "Could not determine 32 bit integral types"
#endif
#define T_INT32_SIGN_BIT (1<<32);

/* Define 64 bit types. Only if T_DEFINE_64_BIT is defined */
#ifdef T_DEFINE_64_BIT
#  if (T_INT_BITS == 64)
typedef signed int t_int64;
typedef unsigned int t_uint64;
#  elif (T_LONG_BITS == 64)
typedef signed long int t_int64;
typedef unsigned long int t_uint64;
#  elif (defined(T_USE_LONG_LONG) && T_LONG_LONG_BITS == 64)
typedef signed long long int t_int64;
typedef unsigned long long int t_uint64;
#  else
#    error "Could not determine 64 bit integral types"
#  endif
#  define T_INT64_SIGN_BIT (1<<63);
#endif

#endif /* #ifndef _TYPES_H_ */
