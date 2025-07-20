#ifndef _LIB_LOG_H
#define _LIB_LOG_H

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "macros.h"

M_INLINE void _log(const char* file, int line, const char* function,
                   const char* prefix, const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  FILE* fp = !strcmp(prefix, "LOG") ? stdout : stderr;
  fprintf(fp, "[%s][%s:%d][%s] ", prefix, file, line, function);

  const int len = vsnprintf(NULL, 0, fmt, ap);
  char buf[len + 1];
  vsnprintf(buf, len + 1, fmt, ap);
  fprintf(fp, "%s%s", buf, buf[len] == '\n' ? "" : "\n");
  va_end(ap);
}

#define LOG(_fmt, ...) _log(__FILE__, __LINE__, __FUNCTION__, "LOG", (_fmt), ##__VA_ARGS__)
#define WARN(_fmt, ...) _log( __FILE__, __LINE__, __FUNCTION__, "WRN", (_fmt), ##__VA_ARGS__)
#define ERROR(_fmt, ...) _log(__FILE__, __LINE__, __FUNCTION__, "ERR", (_fmt), ##__VA_ARGS__)

#endif
