#ifndef MISC_H
#define MISC_H

#include <stdio.h>
#include <errno.h>

typedef enum
{
    FALSE = 0,
    TRUE  = 1,
} boolean_t;

typedef unsigned char byte_t;

#define PANIC(code, reason) \
{ fprintf(stderr, "%s line %d: %s\n", __FILE__, __LINE__, (reason)); \
  exit(code); } \

#define STRDUP(dest, src) \
{ if (((dest) = (char *)strdup((src))) == NULL) \
      PANIC(ENOMEM, "can't strdup"); } \

#define STRNDUP(dest, src, n) \
{ if (((dest) = (char *)strndup((src), (n))) == NULL) \
      PANIC(ENOMEM, "can't strdup"); } \

#define MALLOC(dest, size) \
{ if (((dest) = (void *)malloc((size))) == NULL) \
      PANIC(ENOMEM, "can't malloc"); } \

#define CALLOC(dest, num, size) \
{ if (((dest) = (void *)calloc((num), (size))) == NULL) \
      PANIC(ENOMEM, "can't calloc"); } \

#define REALLOC(dest, size) \
{ if (((dest) = (void *)realloc((dest), (size))) == NULL) \
      PANIC(ENOMEM, "can't realloc"); } \

#define FREE(p) \
{ if ((p)) free((p)); (p) = NULL; }


#endif
