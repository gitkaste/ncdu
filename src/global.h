/* ncdu - NCurses Disk Usage

  Copyright (c) 2007-2014 Yoran Heling

  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files (the
  "Software"), to deal in the Software without restriction, including
  without limitation the rights to use, copy, modify, merge, publish,
  distribute, sublicense, and/or sell copies of the Software, and to
  permit persons to whom the Software is furnished to do so, subject to
  the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#ifndef _global_h
#define _global_h

#include "config.h"
#include <stdio.h>
#include <stddef.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef HAVE_INTTYPES_H
# include <inttypes.h>
#endif
#ifdef HAVE_STDINT_H
# include <stdint.h>
#endif

/* File Flags (struct dir -> flags) */
#define FF_DIR    0x01
#define FF_FILE   0x02
#define FF_ERR    0x04 /* error while reading this item */
#define FF_OTHFS  0x08 /* excluded because it was an other filesystem */
#define FF_EXL    0x10 /* excluded using exlude patterns */
#define FF_SERR   0x20 /* error in subdirectory */
#define FF_HLNKC  0x40 /* hard link candidate (file with st_nlink > 1) */
#define FF_BSEL   0x80 /* selected */

/* Program states */
#define ST_CALC   0
#define ST_BROWSE 1
#define ST_DEL    2
#define ST_HELP   3
#define ST_SHELL  4


/* structure representing a file or directory */
struct dir {
  int64_t size, asize;
  uint64_t ino, dev;
  struct dir *parent, *next, *prev, *sub, *hlnk;
  int items;
  unsigned char flags;
  char name[3]; /* must be large enough to hold ".." */
};
/* sizeof(total dir) = SDIRSIZE + strlen(name) = offsetof(struct dir, name) + strlen(name) + 1 */
#define SDIRSIZE (offsetof(struct dir, name)+1)

/* A note on the ino and dev fields above: ino is usually represented as ino_t,
 * which POSIX specifies to be an unsigned integer.  dev is usually represented
 * as dev_t, which may be either a signed or unsigned integer, and in practice
 * both are used. dev represents an index / identifier of a device or
 * filesystem, and I'm unsure whether a negative value has any meaning in that
 * context. Hence my choice of using an unsigned integer. Negative values, if
 * we encounter them, will just get typecasted into a positive value. No
 * information is lost in this conversion, and the semantics remain the same.
 */


/* program state */
extern int pstate;
/* read-only flag */
extern int read_only;
/* minimum screen update interval when calculating, in ms */
extern long update_delay;
/* filter directories with CACHEDIR.TAG */
extern int cachedir_tags;

/* handle input from keyboard and update display */
int input_handle(int);


/* import all other global functions and variables */
#include "browser.h"
#include "delete.h"
#include "dir.h"
#include "dirlist.h"
#include "exclude.h"
#include "help.h"
#include "path.h"
#include "util.h"
#include "shell.h"

#endif
