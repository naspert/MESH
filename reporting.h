/* $Id: reporting.h,v 1.1 2001/11/06 10:36:11 dsanta Exp $ */

#ifndef _REPORTING_PROTO
#define _REPORTING_PROTO

/* --------------------------------------------------------------------------*
 *                         External includes                                 *
 * --------------------------------------------------------------------------*/

#ifdef __cplusplus
#define BEGIN_DECL extern "C" {
#define END_DECL }
#else
#define BEGIN_DECL
#define END_DECL
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
void outbuf_printf(struct outbuf *ob, const char *format, ...);

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
