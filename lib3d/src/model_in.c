/* $Id: model_in.c,v 1.41 2003/06/09 12:43:49 aspert Exp $ */


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
 * Functions to read 3D model data from files
 *
 * Author: Diego Santa Cruz
 *
 * N. Aspert is guilty for all the stuff that are zlib/block-read
 * related + the Inventor, PLY & SMF parsers.
 */

#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#ifdef READ_TIME
# include <time.h>
#endif
#include <string.h>
#include <ctype.h>
#include <model_in.h>
#ifdef DEBUG
# include <debug_print.h>
#endif

/* --------------------------------------------------------------------------
   LOCAL PARAMETERS
   -------------------------------------------------------------------------- */


/* Buffer size (in bytes) for gzread  */
#define GZ_BUF_SZ   16384
/* Real number of bytes read by gzread = 95% of GZ_BUF_SZ. Some supplementary 
   bytes are read from the file until we reach a valid separator */
#define GZ_RBYTES   16000
/* Increment for the size of the buffer, just in case ... */
#define GZ_BUF_INCR 512

/* Converts argument into string, without replacing defines in argument */
#define STRING_Q(N) #N
/* Converts argument into string, replacing defines in argument */
#define STRING(N) STRING_Q(N)



/* --------------------------------------------------------------------------
   LOCAL FUNCTIONS
   -------------------------------------------------------------------------- */
static int refill_buffer(struct file_data*);
/* 
   In order to be able to use zlib to read gzipped files directly, we
   have to use our own versions of most of the IO functions. The data
   read from the file is buffered in the 'file_data' structure, and
   accessed *only* using this method, except for the low-level stuff 
   (refill_buffer). The 'scanf' has been torn apart into smaller
   pieces because the use of sscanf on a buffer>512 bytes is _slow_ 
   (probably because of a hidden memcpy ...).
   
   Summary : 
   - '[un]getc' is redefined through a macro to 'buf_[un]getc'
   - '*scanf' are specific parsers that are designed to avoid at all
   cost the calls to 'sscanf'
   - 'buf_fscanf_1arg' is a wrapper for 'sscanf' when called with 1
   return value. It is only called when there is no alternate solution
   - all the 'loc_*' functions point either to the 'gz*' calls or to
   the 'f*' calls, depending whether DONT_USE_ZLIB is defined or not
   (see model_in.h for more info)

*/


int buf_getc_func(struct file_data* data) 
{

  if(!refill_buffer(data))
    return EOF;

  return ((int)data->block[data->pos++]);
}

/* This function refills the data block from the file. It reads a 15KB
 * block but adds supplementary chars until a valid separator (space
 * ...) is encountered (this is why the size of the block is =
 * 16KB. The function returns 0 if nothing has been read and 1 if the
 * block has been refilled. The function also takes care of the case
 * where an ungetc is done just after a refill (of course, in that
 * case, 2 unget's would just generate an error ;-) */
static int refill_buffer(struct file_data* data) 
{
  int rbytes, tmp;
  int rsz;

  if (data->eof_reached) /* we cannot read anything from the buffer */
    return 0;

  /* backup last byte for silly ungetc's */
  if (data->pos > 1) {
    data->block[0] = data->block[data->pos-1];
    data->pos = 1;
  }

  /* now fill da buffer w. at most GZ_RBYTES of data */
  rsz = (GZ_RBYTES < data->size-1) ? GZ_RBYTES : data->size-1;
  assert(rsz > 255);
  rbytes = loc_fread(&(data->block[1]), sizeof(unsigned char), rsz, data->f);
  data->nbytes = rbytes+1;


  if (rbytes < ((int)(rsz*sizeof(unsigned char)))) { 
    /* if we read less, this means that an EOF has been encoutered */
    data->eof_reached = 1;
    memset(&(data->block[data->nbytes]), 0, 
           (data->size-data->nbytes)*sizeof(unsigned char));
#ifdef DEBUG
    DEBUG_PRINT("refill_buffer %d bytes\n", data->nbytes);
#endif
    return 1;
  }

  /* if data is binary we're done */
  if (data->is_binary) return 1;
  
  /* now let's fill the buffer s.t. a valid separator ends it */
  while (strchr(VRML_WS_CHARS, data->block[data->nbytes-1]) == NULL) {
    tmp = loc_getc(data->f);
    if (tmp == EOF) {
      data->eof_reached = 1;
      memset(&(data->block[data->nbytes]), 0, 
             (data->size-data->nbytes)*sizeof(unsigned char));

#ifdef DEBUG
      DEBUG_PRINT("refill_buffer %d bytes\n", data->nbytes);
#endif
      return 1; /* kinda OK... */
    }
    data->block[data->nbytes++] = (unsigned char)tmp;
    if (data->nbytes == data->size-1) { /* we need more space */
      data->block = grow_array(data->block, sizeof(unsigned char), 
                               &(data->size), GZ_BUF_INCR);
#ifdef DEBUG 
      DEBUG_PRINT("New block size = %d bytes\n", data->size);
#endif 
      if (data->block == NULL)
        return MESH_NO_MEM;
    }
  }
  assert(data->size >= data->nbytes);
  memset(&(data->block[data->nbytes]), 0, 
         (data->size-data->nbytes)*sizeof(unsigned char));

#ifdef DEBUG
  DEBUG_PRINT("refill_buffer %d bytes\n", data->nbytes);
#endif
  return 1;
}

/* equivalent of fread() */
size_t bin_read(void *ptr, size_t size, size_t nmemb, struct file_data *data) 
{
  size_t len,i;
  char *cptr;
  int c;

  len = size*nmemb;
  i = 0;
  cptr = (char*)ptr;
  do {
    c = getc(data);
    if (c != EOF) *(cptr++) = c;
  } while (c != EOF && (++i) < len);
  return i/size;
}

/* This function is an equivalent for 'sscanf(data->block, "%d", out)'
 * except that it uses 'strtod' that is faster... */
int int_scanf(struct file_data *data, int *out) 
{
  char *endptr=NULL;
  int tmp, c;

  do {
    c = getc(data);
  } while ((c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '#' ||
           c == '\"' || c ==',') && c != EOF);
  if (c != EOF)
    ungetc(c, data);
  
  tmp = (int)strtol((char*)&(data->block[data->pos]), &endptr, 10);
  if (endptr == (char*)&(data->block[data->pos]) || endptr == NULL) {
#ifdef DEBUG
    DEBUG_PRINT("pos=%d block= %s\n", data->pos, &(data->block[data->pos]));
#endif
    return 0;
  }  
#ifdef DEBUG
  DEBUG_PRINT("tmp = %d\n", tmp);
#endif
  data->pos += (endptr - (char*)&(data->block[data->pos]))*sizeof(char);

  if (data->pos == data->nbytes-1) {
#ifdef DEBUG
    DEBUG_PRINT("calling refill_buffer\n");
#endif
    refill_buffer(data);
  }

  *out = tmp;
  return 1;
}

/* This function is an equivalent for 'sscanf(data->block, "%f", out)'
 * except that it uses 'strtol' that is faster... 
 * Two versions: the one for Window$ makes a copy of the number into a
 * buffer, s.t. strtol does not waste more time in strlen calls. 
 * It is still _much_ slower than the glibc implementation, but it 
 * improves things.
 */
#ifdef _WIN32
int float_scanf(struct file_data *data, float *out) 
{
  char buf[512];
  float tmp;
  int c, i=-1;

  do {
    c = getc(data);
  } while ((c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '#' ||
            c == '\"' || c ==',') && c != EOF);
  if (c != EOF)
    ungetc(c, data);

  do {
    buf[++i] = getc(data);
  } while ((buf[i] >= '0' && buf[i] <= '9') || buf[i] == 'e' || 
           buf[i] == 'E' || buf[i] == '+' || buf[i] == '-' || 
           buf[i] == '.');
  buf[++i] = '\0';

  if (i == 1) /* screwed up ! */
    return 0;

  tmp = (float)atof(buf);

#ifdef DEBUG
  DEBUG_PRINT("tmp = %f\n", tmp);
#endif
  if (data->pos == data->nbytes-1) {
    refill_buffer(data);
  }
  *out = tmp;
  return 1;
}
#else
int float_scanf(struct file_data *data, float *out) 
{
  float tmp;
  char *endptr = NULL;
  int c;

  do {
    c = getc(data);
  } while ((c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '#' ||
            c == '\"' || c ==',') && c != EOF);
  if (c != EOF)
    ungetc(c, data);

  tmp = strtod((char*)&(data->block[data->pos]), &endptr);
  if (endptr == (char*)&(data->block[data->pos]) || endptr==NULL) 
    return 0;
#ifdef DEBUG
  DEBUG_PRINT("tmp = %f\n", tmp);
#endif
  data->pos += (endptr - (char*)&(data->block[data->pos]))*sizeof(char);

  if (data->pos == data->nbytes-1) {
    refill_buffer(data);
  }
  *out = tmp;
  return 1;
}
#endif

/* Reads a word until a separator is encountered. This is the
 * equivalent of doing a [sf]canf(data, "%60[^ \t,\n\r#\"]", out)
 */
int string_scanf(struct file_data *data, char *out) 
{
  int nb_read=0;
  char stmp[MAX_WORD_LEN+1];
  int c;

  do {
    c = getc(data);
    stmp[nb_read++] = (char)c;
  } while (c != ' ' && c != '\t' && c != '\n' && c != '\r' && c != '#' &&
           c != '\"' && c!=',' && c != '[' && c != '{' &&
           c != EOF && nb_read < MAX_WORD_LEN+1);
  
  if (nb_read > 1 && nb_read < MAX_WORD_LEN) {
    if (c != EOF)
      ungetc(c, data);
    stmp[--nb_read] = '\0';
#ifdef DEBUG
    DEBUG_PRINT("stmp=%s\n", stmp);
#endif
    strcpy(out, stmp);
    return 1;
  }
  return 0;
}


/* sscanf wrapper. This is relatively slow (apperently due to a memcpy
 * of the data block done by sscanf).  */
int buf_fscanf_1arg(struct file_data *data, const char *fmt, void *out)
{
  int count, n;
  char _fmt[64];


  /* tweak the format */
  strcpy(_fmt, fmt);
  strcat(_fmt, "%n");

  count = sscanf((char*)&(data->block[data->pos]), _fmt, out, &n);
  if (count > 0) {
    data->pos += n;
    return count;
  }

  /* otherwise try to read another block and do sscanf once more...*/
  if (refill_buffer(data)) {
    count = sscanf((char*)&(data->block[data->pos]), _fmt, out, &n);
    if (count > 0) {
      data->pos += n;
    }
    return count;
  } else /* fail shamelessly ... */
    return count;

}



/* Grows the array pointed to by 'p'. The current array size (in elements) is
 * given by '*len'. The array is doubled in size, if the size increment is
 * less than 'max_incr' elements, or augmented by 'max_incr' elements
 * otherwise. The new array size is returned in '*len'. The element size, in
 * bytes, is given by 'elem_sz'. If p is NULL a new array is allocated of size
 * '*len', if '*len' is not zero, or SZ_INIT_DEF otherwise. Otherwise, if
 * '*len' is zero, the new length is SZ_INIT_DEF. Otherwise the size is
 * doubled, up to a maximum increment of 'max_incr'. If 'max_incr' is zero, it
 * is taken to be infinity (i.e. the array size is always doubled). The
 * pointer to the new storage allocation is returned. If there is not enough
 * memory NULL is returned and the current array, if any, is freed, and zero
 * is returned in '*len'.  */
void* grow_array(void *p, size_t elem_sz, int *len, int max_incr)
{
  void *new_p;
  int cur_len,new_len;

  assert(elem_sz != 0);
  assert(*len >= 0 && max_incr >= 0);
  cur_len = *len;
  if (p == NULL) { /* no array */
    new_len = (cur_len > 0) ? cur_len : SZ_INIT_DEF;
  } else if (cur_len <= 0) { /* empty array */
    new_len = SZ_INIT_DEF;
  } else {
    new_len = (max_incr <= 0 || cur_len < max_incr) ? 2*cur_len :
      cur_len+max_incr;
  }
  new_p = realloc(p,elem_sz*new_len);
  if (new_p != NULL) {
    *len = new_len;
  } else {
    free(p);
    *len = 0;
  }
  return new_p;
}



/* Advances the data stream to the next non-comment and non-whitespace
 * character in the '*data' stream. If the end-of-file is reached or an I/O
 * error occurs, EOF is returned. Otherwise the next character to be read from
 * '*data' is returned. Comments and whitespace are interpreted as in VRML
 * (note that a comma is a whitespace in VRML!).
 */
int skip_ws_comm(struct file_data *data)
{
  int c;

  c = getc(data);
  do {
    if (c == '#' /* hash */ ) { /* Comment line, skip whole line */
      do {
        c = getc(data);
      } while (c != '\n' && c != '\r' && c != EOF);
      if (c != EOF) c = getc(data); /* Get first character of next line */
    } else if (c == ' ' || c == '\t' || c == ',' || c == '\n' || c == '\r') {
      /* VRML whitespace char, get next character */
      c = getc(data);
    } else if (c != EOF) { /* a meaningful character, put it back */
      c = ungetc(c,data);
      break;
    }
  } while (c != EOF);
  return c;
}

/* Like skip_ws_comm(), but also skips over VRML quoted strings. Returns the
 * next non-whitespace, non-comment and non-string character, or EOF if
 * end-of-file or I/O error is encountered. */
int skip_ws_comm_str(struct file_data *data)
{
  int c;
  int in_escape;

  c = skip_ws_comm(data);
  while (c == '"') { /* skip all consecutive strings */
    getc(data); /* skip '"' starting string */
    in_escape = 0;
    do {
      c = getc(data);
      if (!in_escape) {
        if (c == '\\') in_escape = 1;
      } else {
        in_escape = 0;
        if (c == '"') c = '\0'; /* escaped double quotes, not interesting */
      }
    } while (c != '"' && c != EOF);
    if (c != EOF) {
      c = skip_ws_comm(data);
    }
  }
  return c;
}

/* Advances the '*data' stream until one of the characters in 'chars' (null
 * terminated string) is the next character to be read. Returns the matched
 * character or EOF if end-of-file or an I/O error is encountered. Characters
 * in quoted strings and comments (excluding line-termination character) of
 * the '*data' stream are never matched. */
int find_chars(struct file_data *data, const char *chars)
{
  int c;
  int in_escape;

  c = getc(data);
  while (c != EOF && strchr(chars,c) == NULL) {
    if (c == '"') { /* start of quoted string, skip it */
      in_escape = 0;
      do {
        c = getc(data);
        if (!in_escape) {
          if (c == '\\') in_escape = 1;
        } else {
          in_escape = 0;
          if (c == '"') c = '\0'; /* escaped double quotes, not interesting */
        }
      } while (c != '"' && c != EOF);
      if (c != EOF) c = getc(data); /* next char */
    } else if (c == '#') { /* start comment, skip it */
      do {
        c = getc(data);
      } while (c != '\n' && c != '\r' && c != EOF);
    } else { /* unquoted char, just get next */
      c = getc(data);
    }
  }
  if (c != EOF) c = ungetc(c,data); /* put matched char back */
  return c;
}

/* Advances the '*data' stream until the 'string' is found as a whole
 * word. Returns the next character to be read from the '*data' stream, which
 * is the first character after the word 'string'. EOF is returned if
 * end-of-file is reached or an I/O error occurs. Comments, quoted strings and
 * whitespace in the '*data' stream are ignored. The 'string' argument shall
 * not contain whitespace, comments or quoted strings. */
int find_string(struct file_data *data, const char *string)
{
  int c,i;
  
  do {
    c = getc(data);
    if (strchr(VRML_WSCOMMSTR_CHARS,c)) { /* characters we need to skip */
      if (ungetc(c,data) == EOF) return EOF;
      skip_ws_comm_str(data);
      c = getc(data);
    }
    i = 0;
    while (c != EOF && string[i] != '\0' && string[i] == c) {
      c = getc(data);
      i++;
    }
    if (string[i] == '\0' &&
        (c == EOF || strchr(VRML_WSCOMMSTR_CHARS,c) != NULL)) {
#ifdef DEBUG
      DEBUG_PRINT("%s matched\n", string);
#endif
      break; /* string matched and is whole word */
    }
  } while (c != EOF);
  if (c != EOF) c = ungetc(c,data);
  return c;
}





/* Detect the file format of the '*data' stream, returning the detected
 * type. The header identifying the file format, if any, is stripped out and
 * the '*data' stream is positioned just after it. If an I/O error occurs
 * MESH_CORRUPTED is returned. If the file format can not be detected
 * MESH_BAD_FF is returned. The detected file formats are: MESH_FF_RAW,
 * MESH_FF_VRML, MESH_FF_IV, MESH_FF_PLY and MESH_FF_SMF. */
static int detect_file_format(struct file_data *data)
{
  char stmp[MAX_WORD_LEN+1];
  const char swfmt[] = "%" STRING(MAX_WORD_LEN) "s";
  const char svfmt[] = "%" STRING(MAX_WORD_LEN) "[0-9V.]";
  int c;
  int rcode;
  char *eptr;
  double ver;

  c = getc(data);
  if (c == '#') { /* Probably VRML or Inventor but can also be a SMF comment */
    if (buf_fscanf_1arg(data,swfmt,stmp) == 1) {
      if (strcmp(stmp,"VRML") == 0) {
        if (getc(data) == ' ' && buf_fscanf_1arg(data,swfmt,stmp) == 1 &&
            strcmp(stmp,"V2.0") == 0 && getc(data) == ' ' &&
            buf_fscanf_1arg(data,swfmt,stmp) == 1 && strcmp(stmp,"utf8") == 0 &&
            ((c = getc(data)) == '\n' || c == '\r' || c == ' ' || 
             c == '\t')) {
          while (c != EOF && c != '\n' && c != '\r') { /* skip rest of header */
            c = getc(data);
          }
          rcode = (c != EOF) ? MESH_FF_VRML : MESH_CORRUPTED;
        } else {
          rcode = ferror((FILE*)data->f) ? MESH_CORRUPTED : MESH_BAD_FF;
        }
      } else if (strcmp(stmp,"Inventor") == 0) {
        if (getc(data) == ' ' && buf_fscanf_1arg(data,svfmt,stmp) == 1 &&
            stmp[0] == 'V' && (ver = strtod(stmp+1,&eptr)) >= 2 &&
            ver < 3 && *eptr == '\0' && getc(data) == ' ' &&
            buf_fscanf_1arg(data,swfmt,stmp) == 1 && strcmp(stmp,"ascii") == 0 &&
            ((c = getc(data)) == '\n' || c == '\r' || c == ' ' || c == '\t')) {
          while (c != EOF && c != '\n' && c != '\r') { /* skip rest of header */
            c = getc(data);
          }
          rcode = (c != EOF) ? MESH_FF_IV : MESH_CORRUPTED;
        } else {
          rcode = ferror((FILE*)data->f) ? MESH_CORRUPTED : MESH_BAD_FF;
        }
      } else {
        /* Is is a comment line of a SMF file ? */
        data->pos = 1; /* rewind file */
        if ((c = skip_ws_comm(data)) == EOF) rcode = MESH_BAD_FF;
        if (c == 'v' || c == 'b' || c == 'f' || c == 'c')
          rcode = MESH_FF_SMF;
        else 
          rcode = ferror((FILE*)data->f) ? MESH_CORRUPTED : MESH_BAD_FF;
      }
    } else {
      /* We need to test for SMF files here also, maybe a comment line
       * */
      data->pos = 1; /* rewind file */
      if((c = skip_ws_comm(data)) == EOF) rcode = MESH_BAD_FF;
      if (c == 'v' || c == 'b' || c == 'f' || c == 'c')
        rcode = MESH_FF_SMF;
      else 
        rcode = ferror((FILE*)data->f) ? MESH_CORRUPTED : MESH_BAD_FF;
    }
  } else if (c == 'p') { /* Probably ply */
    c = ungetc(c,data);
    if (c != EOF) {
      if (string_scanf(data, stmp) == 1 && strcmp(stmp, "ply") == 0) {
        rcode = MESH_FF_PLY;
      } else {
        rcode = ferror((FILE*)data->f) ? MESH_CORRUPTED : MESH_BAD_FF;
      }
    } else {
      rcode = MESH_CORRUPTED;
    }
  } else if (c >= '0' && c <= '9') { /* probably raw */
    c = ungetc(c,data);
    rcode = (c != EOF) ? MESH_FF_RAW : MESH_CORRUPTED;
  } else { 
    /* test for SMF also here before returning */
    data->pos=1; /* rewind file */
    if ((c = skip_ws_comm(data)) == EOF) rcode = MESH_BAD_FF;

    if (c == 'v' || c == 'b' || c == 'f' || c == 'c')
      rcode = MESH_FF_SMF;
    else 
      rcode = ferror((FILE*)data->f) ? MESH_CORRUPTED : MESH_BAD_FF;

  }
  return rcode;
}

/* see model_in.h */
int read_model(struct model **models_ref, struct file_data *data, 
               int fformat, int concat)
{
  int rcode;
  struct model *models;

  if (fformat == MESH_FF_AUTO) {
    fformat = detect_file_format(data);
    if (fformat < 0) return fformat; /* error or unrecognized file format */
  }
  rcode = 0;
  models = NULL;

  switch (fformat) {
  case MESH_FF_RAW:
  case MESH_FF_RAWBIN:
    rcode = read_raw_tmesh(&models, data);
    break;
  case MESH_FF_VRML:
    rcode = read_vrml_tmesh(&models, data, concat);
    break;
  case MESH_FF_IV:
    rcode = read_iv_tmesh(&models, data);
    break;
  case MESH_FF_SMF:
    rcode = read_smf_tmesh(&models, data);
    break;
  case MESH_FF_PLY:
    rcode = read_ply_tmesh(&models, data);
    break;
  default:
    rcode = MESH_BAD_FF;
  }
  

  if (rcode >= 0) {
    *models_ref = models;
  }
  return rcode;
}

/* see model_in.h */
int read_fmodel(struct model **models_ref, const char *fname,
                int fformat, int concat)
{
  int rcode;
  struct file_data *data;
#ifdef READ_TIME
  clock_t stime;
#endif

  data = (struct file_data*)malloc(sizeof(struct file_data));

  data->f = loc_fopen(fname, "rb");
  data->block = (unsigned char*)malloc(GZ_BUF_SZ*sizeof(unsigned char));

  if (data->f == NULL) return MESH_BAD_FNAME;
  /* initialize file_data structure */
  data->size = GZ_BUF_SZ;
  data->eof_reached = 0;
  data->nbytes = 0;
  data->pos = 1;
  data->is_binary = 0;

#ifdef READ_TIME
  stime = clock();
#endif

  rcode = read_model(models_ref, data, fformat, concat);

#ifdef READ_TIME
  printf("Model read in %f sec.\n", (double)(clock()-stime)/CLOCKS_PER_SEC);
#endif

  loc_fclose(data->f);
  free(data->block);
  free(data);
  return rcode;
}
