/* $Id: reporting.h,v 1.4 2002/03/29 17:20:30 dsanta Exp $ */


/*
 *
 *  Copyright (C) 2001-2002 EPFL (Swiss Federal Institute of Technology,
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
 *   Submitted to ICME 2002, available on http://mesh.epfl.ch
 *
 */

#ifndef _REPORTING_PROTO
#define _REPORTING_PROTO

/* --------------------------------------------------------------------------*
 *                         External includes                                 *
 * --------------------------------------------------------------------------*
 */


#ifdef __cplusplus
#define BEGIN_DECL extern "C" {
#define END_DECL }
#else
#define BEGIN_DECL
#define END_DECL
#endif

#ifdef __GNUC__
#define REPORTING_PRINTF_ATTR(m,n) \
  __attribute__ ((format (__printf__, (m) , (n))))
#else
#define REPORTING_PRINTF_ATTR(m,n)
#endif

BEGIN_DECL
#undef BEGIN_DECL

/* --------------------------------------------------------------------------*
 *                       Exported data types                                 *
 * --------------------------------------------------------------------------*/

/* The maximum string size that can be printed to the buffer in one call,
 * including the terminating nul. */
#define OUTBUF_MAX_SZ 256

/* The callback to flush the output buffer. The out argument points to some
 * callback private data, while str is the string to output. */
typedef void outbuf_flush_cb_t(void *out, const char* str);

/* The log output buffer */
struct outbuf {
  char *strbuf;  /* The string buffer. Always terminated by '\0'. */
  char *pos;     /* Current position for the next character */
  char *end;     /* The (exclusive) end of the string buffer */
  outbuf_flush_cb_t *flush; /* The flush callback. If NULL none. */
  void *cb_out;  /* The callback out parameter */
};

/* The type for the progress indicator callback. The argument out is a pointer
 * to callback private data, while p is the current progress as a percentage
 * value. If p is negative the progress indicator should be removed (if
 * applicable). The progress must start with a call with p as zero, and end
 * with a call with p as negative (after calls with values 0..100). */
typedef void prog_func_cb_t(void *out, int p);

/* The progress reporter */
struct prog_reporter {
  prog_func_cb_t *prog; /* The function used to report the progress */
  void *cb_out;         /* Data private to the callback function */
};

/* --------------------------------------------------------------------------*
 *                       Exported functions                                  *
 * --------------------------------------------------------------------------*/

/* Returns a new output buffer using the flush_cb callback with the cb_out
 * value as out parameter. An initial buffer with a default size is set up. */
struct outbuf *outbuf_new(outbuf_flush_cb_t *flush_cb, void *cb_out);

/* Frees all storage associated with the output buffer ob */
void outbuf_delete(struct outbuf *ob);

/* Flushes the buffered output of ob using its callback. If no callback
 * nothing is done. */
void outbuf_flush(struct outbuf *ob);

/* Performs printf using the given format and stores the output in the ob
 * output buffer. The string produced by this call should not exceed
 * OUTBUF_MAX_SZ characters in length, including the terminating nul. */
void outbuf_printf(struct outbuf *ob, const char *format, ...)
  REPORTING_PRINTF_ATTR(2,3); /* allow GCC to check format string */

/* Reports the progress p to pr. */
void prog_report(struct prog_reporter *pr, int p);

/* Writes the string to stdio stream out (really a FILE *). To use with
 * outbuf. */
void stdio_puts(void *out, const char *str);

/* Prints progress percentage to stdout. If negative it erases a previous
 * progress message. */
void stdio_prog(void *out, int p);

END_DECL
#undef END_DECL

#endif /* _REPORTING_PROTO */
