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


#include "global.h"

#include <string.h>
#include <errno.h>
#include <unistd.h>


#define CS_CONFIRM  0
#define CS_PROGRESS 1
#define CS_FAILED   2


static struct dir *root, *nextsel, *curdir;
static char noconfirm = 0, ignoreerr = 0, state, seloption;
static int lasterrno;


static void clear_draw_confirm() {
  nccreate(6, 60, "Confirm clear");

  ncprint(1, 2, "Are you sure you want to clear \"%s\"%c",
    cropstr(root->name, 21), root->flags & FF_DIR ? ' ' : '?');

  if(seloption == 0)
    attron(A_REVERSE);
  ncaddstr(4, 15, "yes");
  attroff(A_REVERSE);
  if(seloption == 1)
    attron(A_REVERSE);
  ncaddstr(4, 24, "no");
  attroff(A_REVERSE);
  if(seloption == 2)
    attron(A_REVERSE);
  ncaddstr(4, 31, "don't ask me again");
  attroff(A_REVERSE);

  ncmove(4, seloption == 0 ? 15 : seloption == 1 ? 24 : 31);
}


static void clear_draw_progress() {
  nccreate(6, 60, "Clearing...");

  ncaddstr(1, 2, cropstr(getpath(curdir), 47));
  ncaddstr(4, 41, "Press q to abort");
}


static void clear_draw_error() {
  nccreate(6, 60, "Error!");
  if (root == curdir) 
    ncprint(1, 2, "Can't clear %s:", cropstr(getpath(curdir), 42));
  else
    ncprint(1, 2, "Can't delete %s:", cropstr(getpath(curdir), 42));
  ncaddstr(2, 4, strerror(lasterrno));

  if(seloption == 0)
    attron(A_REVERSE);
  ncaddstr(4, 14, "abort");
  attroff(A_REVERSE);
  if(seloption == 1)
    attron(A_REVERSE);
  ncaddstr(4, 23, "ignore");
  attroff(A_REVERSE);
  if(seloption == 2)
    attron(A_REVERSE);
  ncaddstr(4, 33, "ignore all");
  attroff(A_REVERSE);
}


void clear_draw() {
  browse_draw();
  switch(state) {
    case CS_CONFIRM:  clear_draw_confirm();  break;
    case CS_PROGRESS: clear_draw_progress(); break;
    case CS_FAILED:   clear_draw_error();    break;
  }
}


int clear_key(int ch) {
  /* confirm */
  if(state == CS_CONFIRM)
    switch(ch) {
      case KEY_LEFT:
      case 'h':
        if(--seloption < 0)
          seloption = 0;
        break;
      case KEY_RIGHT:
      case 'l':
        if(++seloption > 2)
          seloption = 2;
        break;
      case '\n':
        if(seloption == 1)
          return 1;
        if(seloption == 2)
          noconfirm++;
        state = CS_PROGRESS;
        break;
      case 'q':
        return 1;
    }
  /* processing deletion */
  else if(state == CS_PROGRESS)
    switch(ch) {
      case 'q':
        return 1;
    }
  /* error */
  else if(state == CS_FAILED)
    switch(ch) {
      case KEY_LEFT:
      case 'h':
        if(--seloption < 0)
          seloption = 0;
        break;
      case KEY_RIGHT:
      case 'l':
        if(++seloption > 2)
          seloption = 2;
        break;
      case 10:
        if(seloption == 0)
          return 1;
        if(seloption == 2)
          ignoreerr++;
        state = CS_PROGRESS;
        break;
      case 'q':
        return 1;
    }

  return 0;
}


static int clear_dir(struct dir *dr) {
  struct dir *nxt, *cur;
  int r;

  /* check for input or screen resizes */
  curdir = dr;
  if(input_handle(1))
    return 1;

  /* do the actual deleting */
  if(dr->flags & FF_DIR) {
    if((r = chdir(dr->name)) < 0)
      goto clear_nxt;
    if(dr->sub != NULL) {
      nxt = dr->sub;
      while(nxt != NULL) {
        cur = nxt;
        nxt = cur->next;
        if(clear_dir(cur))
          return 1;
      }
    }
    if((r = chdir("..")) < 0)
      goto clear_nxt;
    r = dr->sub == NULL && dr != root ? rmdir(dr->name) : 0;
  } else
    r = unlink(dr->name);

clear_nxt:
  /* error occured, ask user what to do */
  if(r == -1 && !ignoreerr) {
    state = CS_FAILED;
    lasterrno = errno;
    curdir = dr;
    while(state == CS_FAILED)
      if(input_handle(0))
        return 1;
  } else if(!(dr->flags & FF_DIR && dr->sub != NULL) && root != dr) {
    freedir(dr);
    return 0;
  }
  return root == dr ? 1 : 0;
}


void clear_process() {

  /* confirm */
  seloption = 1;
  while(state == CS_CONFIRM && !noconfirm)
    if(input_handle(0)) {
      browse_init(root->parent);
      return;
    }

  /* chdir */
  if(path_chdir(getpath(root->parent)) < 0) {
    state = CS_FAILED;
    lasterrno = errno;
    while(state == CS_FAILED)
      if(input_handle(0))
        return;
  }

  /* clear */
  seloption = 0;
  state = CS_PROGRESS;
  clear_dir(root);
  browse_init(root->parent);
}


void clear_init(struct dir *dr) {
  state = CS_CONFIRM;
  root = curdir = dr;
  pstate = ST_CLEAR;
}
