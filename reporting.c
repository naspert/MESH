/* $Id: reporting.c,v 1.5 2002/08/09 15:26:43 aspert Exp $ */


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
 *   Accepted for publication, ICME 2002, available on http://mesh.epfl.ch
 *
 */



#include <reporting.h>

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <xalloc.h>

/* Minimum amount of free space in buffer */
#define OUTBUF_MIN_FREE (2*OUTBUF_MAX_SZ)

/* --------------------------------------------------------------------------*
 *                          External functions                               *
 * --------------------------------------------------------------------------*/

/* see reporting.h */
struct outbuf *outbuf_new(outbuf_flush_cb_t *flush_cb, void *cb_out)
{
  struct outbuf *ob;
  ob = xa_malloc(sizeof(*ob));
  ob->strbuf = xa_malloc(OUTBUF_MIN_FREE);
  ob->strbuf[0] = '\0';
  ob->pos = ob->strbuf;
  ob->end = ob->strbuf + OUTBUF_MIN_FREE;
  ob->flush = flush_cb;
  ob->cb_out = cb_out;
  return ob;
}

/* see reporting.h */
void outbuf_delete(struct outbuf *ob)
{
  if (ob != NULL) {
    free(ob->strbuf);
    free(ob);
  }
}

/* see reporting.h */
void outbuf_flush(struct outbuf *ob)
{
  assert(*(ob->pos) == '\0');
  if (ob->flush != NULL) {
    ob->flush(ob->cb_out,ob->strbuf);
    ob->pos = ob->strbuf;
    if (ob->end > ob->pos) *(ob->pos) = '\0';
  }
}

/* see reporting.h */
void outbuf_printf(struct outbuf *ob, const char *format, ...)
{
  va_list ap;
  int len;
  int free_sz;
  size_t new_sz,off;

  assert(ob->strbuf != NULL && ob->pos != NULL && ob->end != NULL);

  /* Ensure a minimum amount of free bytes */
  free_sz = ob->end - ob->pos;
  if (free_sz < OUTBUF_MAX_SZ) {
    off = ob->pos-ob->strbuf;
    new_sz = ob->end-ob->strbuf+OUTBUF_MIN_FREE;
    ob->strbuf = xa_realloc(ob->strbuf,new_sz);
    ob->pos = ob->strbuf+off;
    ob->end = ob->strbuf+new_sz;
    free_sz = ob->end - ob->pos;
  }

  /* Print to string buffer */
  va_start(ap,format);
  len = vsprintf(ob->pos,format,ap);
  va_end(ap);
  /* Check that buffer was large enough. Ensuring that we have enough free
   * space requires C99 or some ugly black magic. This should be enough for us
   * anyhow. */
  if (len >= free_sz) {
    fprintf(stderr,"PANIC: String buffer size exceeded. "
            "Memory possibly corrupted.");
  }
  ob->pos += len;
}

/* see reporting.h */
void prog_report(struct prog_reporter *pr, int p)
{
  pr->prog(pr->cb_out,p);
}

/* see reporting.h */
void stdio_puts(void *out, const char *str)
{
  fputs(str,(FILE*)out);
  fflush((FILE*)out);
}

/* see reporting.h */
void stdio_prog(void *out, int p)
{
  FILE *fout;
  fout = (FILE*)out;
  if(p < 0) {
    fprintf(fout,"\r              \r"); /* Remove progress message */
  } else {
    fprintf(fout,"\rProgress %3d %%",p);
  }
  fflush(fout);
}
