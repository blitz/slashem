/* Copyright (C) 2002 Andrew Apted <ajapted@users.sourceforge.net> */
/* NetHack may be freely redistributed.  See license for details.  */

/*
 * SDL/GL window port for NetHack & Slash'EM.
 *
 * This file contains all the window handling code.  Basically all of
 * the interface routines which have a `window' argument.  We attempt
 * to behave as closelu as possible to the TTY port, with a few
 * enhancements.  Map handling is in gl_map.c.
 */

#include "hack.h"
#include "dlb.h"
#include "patchlevel.h"

#if defined(GL_GRAPHICS) || defined(SDL_GRAPHICS)

#define WINGL_INTERNAL
#include "winGL.h"

#include <string.h>
#include <ctype.h>
#include <signal.h>


struct TextWindow *text_wins[MAXWIN] = { NULL, };
static int textwin_num = 0;

int sdlgl_map_win = WIN_ERR;
int sdlgl_top_win = WIN_ERR;

#define MORE_STR  "[More]"
#define MORE_LEN  6

#define DEPTH_MAP      2
#define DEPTH_STATUS   3
#define DEPTH_MESSAGE  4
#define DEPTH_SC_BACK  5

#define HEIGHT_STATUS  3

#define WIN_PADDING    8


void sdlgl_win_startup(void)
{
  /* tell the main nethack code: we're ready to Rock 'n' Roll */
  iflags.window_inited = 1;
}

void sdlgl_win_shutdown(void)
{
  winid i;

  /* Just forget any windows existed, since we're about to exit anyway.
   * Disable windows to avoid calls to window routines.
   */
  for(i=0; i < MAXWIN; i++)
  {
    if (text_wins[i])
      Sdlgl_destroy_nhwindow(i);
  }

  iflags.window_inited = 0;
}

winid Sdlgl_create_nhwindow(int type)
{
  winid newid;
  struct TextWindow *newwin;

  /* no more window numbers ? */
  if (textwin_num == MAXWIN)
    return WIN_ERR;

  for (newid=0; newid <= textwin_num; newid++)
    if (!text_wins[newid])
      break;

  assert(newid <= textwin_num);

  newwin = text_wins[newid] = sdlgl_new_textwin(type);
  textwin_num++;

  return newid;
}

void Sdlgl_destroy_nhwindow(winid window)
{
  struct TextWindow *win;
  
  if (window == WIN_ERR)
    return;

  assert (0 <= window && window < MAXWIN && text_wins[window]);
  win = text_wins[window];

  if (win->base)
    Sdlgl_dismiss_nhwindow(window);

  if (win->items)
    sdlgl_clear_menu_items(win);

  if (win->lines)
    sdlgl_clear_text_items(win);

  sdlgl_free_textwin(win);

  text_wins[window] = NULL;
  textwin_num--;
}

void Sdlgl_clear_nhwindow(winid window)
{
  struct TextWindow *win;
  int x, y;
  
  if (window == WIN_ERR)
    return;

  assert (0 <= window && window < MAXWIN && text_wins[window]);
  win = text_wins[window];
    
  if (! win->base)
    return;

  if (win->type == NHW_MESSAGE)
  {
    sdlgl_putc(win, 12);  /* ^L: clear */
    sdlgl_home(win);

    win->fresh_lines = 0;
    return;
  }

  sdlgl_blank_area(win->base, 0, 0, win->base->total_w,
      win->base->total_h);
   
  for (y=0; y < win->base->total_h; y++)
  for (x=0; x < win->base->total_w; x++)
  {
    int offset = y * win->base->total_w + x;

    if (win->glyphs)
    {
      win->glyphs[offset].bg = NO_GLYPH;
      win->glyphs[offset].fg = NO_GLYPH;

      sdlgl_remove_extrashapes(win->base, x, y);
    }
  }

  sdlgl_home(win);
}

/*
 * Pan the text/menu window by the given offset.  The dx/dy values
 * have the following meanings (in absolute value) :
 *
 *    0  -  no change (but compute anyway).
 *    1  -  move by a single character.
 *    2  -  move by a bunch of characters (5 or so).
 *    3  -  move by a "page" (the window size).
 *    4  -  move as far as possible.
 */
void sdlgl_pan_window(int window, int dx, int dy)
{
  struct TextWindow *win;
  struct TileWindow *base;

  int x, y;
  
  assert(0 <= window && window < MAXWIN);

  win = text_wins[window];
  assert(win);

  base = win->base;
  assert(base);

  x = win->focus_x;
  y = win->focus_y;
  
  switch (abs(dx))
  {
    case 2:
      dx = sgn(dx) * 10;
      break;

    case 3:
      dx = sgn(dx) * (win->show_w - 1);
      break;

    case 4:
      dx = sgn(dx) * win->calc_w;
      break;
  }

  switch (abs(dy))
  {
    case 2:
      dy = sgn(dy) * 5;
      break;

    case 3:
      dy = sgn(dy) * (win->show_h - 1);
      break;
        
    case 4:
      dy = sgn(dy) * win->calc_h;
      break;
  }

  x += dx;
  y += dy;
  
  /* limit new focus value */
  win->focus_x = max(0, min(win->calc_w - win->show_w, x));
  win->focus_y = max(0, min(win->calc_h - win->show_h, y));

  /* compute panning for tile window */
  sdlgl_set_pan(base, 
      win->focus_x * base->scale_w,
      win->focus_y * base->scale_h);

  sdlgl_flush();
}

static int compute_window_base_y(int type, int *h)
{
  int pad = WIN_PADDING;

  int mesg_h = pad + iflags.wc_vary_msgcount * sdlgl_font_message->tile_h;
  int stat_h = pad + HEIGHT_STATUS * sdlgl_font_status->tile_h;
  int map_h  = sdlgl_height - mesg_h - stat_h;

  int stat_pos = 0;  /* 0 for bottom, 1 for middle, 2 for top */
  int mesg_pos = 2;

  int a_mesg = iflags.wc_align_message;
  int a_stat = iflags.wc_align_status;
  
  /* we treat ALIGN_LEFT the same as ALIGN_TOP, and ALIGN_RIGHT the
   * same as ALIGN_BOTTOM.
   */
  a_mesg = (a_mesg == ALIGN_LEFT || a_mesg == ALIGN_TOP) ? 0 : 1;
  a_stat = (a_stat == ALIGN_LEFT || a_stat == ALIGN_TOP) ? 0 : 1;

  switch (a_stat * 2 + a_mesg)
  {
    case 0: stat_pos = 1, mesg_pos = 2; break;
    case 1: stat_pos = 2, mesg_pos = 0; break;
    case 2: stat_pos = 0, mesg_pos = 2; break;
    case 3: stat_pos = 0, mesg_pos = 1; break;
  }

  switch (type)
  {
    case NHW_MESSAGE:
      (*h) = mesg_h - pad;
      switch (mesg_pos)
      {
        case 0: return 0;
        case 1: return (stat_pos < 1) ? stat_h : (map_h + pad);
        case 2: return sdlgl_height - (*h);
      }
      break;
      
    case NHW_STATUS:
      (*h) = stat_h - pad;
      switch (stat_pos)
      {
        case 0: return 0;
        case 1: return (mesg_pos < 1) ? mesg_h : (map_h + pad);
        case 2: return sdlgl_height - (*h);
      }
      break;

    case NHW_MAP:
      (*h) = map_h;
      if (mesg_pos > 0 && stat_pos > 0)
        return 0;

      if (mesg_pos < 2 && stat_pos < 2)
        return sdlgl_height - (*h);
      
      return (mesg_pos < 1) ? mesg_h : stat_h;
  }

  impossible("compute_window_base_y: bad type or pos !\n");
  return 0;
}

void Sdlgl_display_nhwindow(winid window, BOOLEAN_P blocking)
{
  struct TextWindow *win;
  int win_y, win_h;
  
  if (window == WIN_ERR)
    return;

  assert (0 <= window && window < MAXWIN && text_wins[window]);
  win = text_wins[window];

  /* already displayed ? */
  if (win->base)
  {
    sdlgl_flush();
    
    /* handle `blocking' special cases -- this fixes the bug with
     * detection potions only flashing very briefly.
     */
    if (blocking && window == WIN_MAP)
    {
      sdlgl_get_key(0);
    }
    
    return;
  }

  switch (win->type)
  {
    case NHW_MESSAGE:
      win->show_w = sdlgl_width / sdlgl_font_message->tile_w;
      win->show_h = iflags.wc_vary_msgcount;

      win_y = compute_window_base_y(NHW_MESSAGE, &win_h);

      win->base = sdlgl_new_tilewin(sdlgl_font_message, win->show_w,
          win->show_h, 1);
      
      sdlgl_map_tilewin(win->base, 0, win_y, sdlgl_width, win_h,
          DEPTH_MESSAGE);

      sdlgl_home(win);

      /* create scroll-back window */
      assert(! win->scrollback);
      assert(iflags.msg_history >= MIN_HISTORY);
          
      win->scrollback = sdlgl_new_tilewin(sdlgl_font_message,
          win->show_w, iflags.msg_history, 1);
      win->scrollback->background = PREV_BACK_COL;
      win->scrollback_pos  = 0;
      win->scrollback_size = 0;
      win->scrollback_enable = 1;
      break;

    case NHW_STATUS:
      win->show_w = sdlgl_width / sdlgl_font_status->tile_w;
      win->show_h = HEIGHT_STATUS;

      win_y = compute_window_base_y(NHW_STATUS, &win_h);

      win->base = sdlgl_new_tilewin(sdlgl_font_status, win->show_w,
          win->show_h, 1);
      
      sdlgl_map_tilewin(win->base, 0, win_y, sdlgl_width, win_h,
          DEPTH_STATUS);

      break;

    case NHW_MAP:
      sdlgl_map_win = window;
      win_y = compute_window_base_y(NHW_MAP, &win_h);

      win->base = sdlgl_new_tilewin(sdlgl_tiles, 80, 22, 0);
      sdlgl_map_tilewin(win->base, 0, win_y, sdlgl_width, win_h,
          DEPTH_MAP);

      sdlgl_create_map(win, 80, 22);
      break;

    case NHW_MENU:
      if (win->is_menu == 0)
        sdlgl_process_text_window(window, win);
      break;
     
    case NHW_TEXT:
      sdlgl_process_text_window(window, win);
      break;

    default:
      break;
  }

  sdlgl_flush();
}

void Sdlgl_dismiss_nhwindow(winid window)
{
  struct TextWindow *win;
  
  if (window == WIN_ERR)
    return;

  assert (0 <= window && window < MAXWIN && text_wins[window]);
  win = text_wins[window];

  /* already dismissed ? */
  if (! win->base)
    return;

  if (win->type == NHW_MAP)
  {
    sdlgl_map_win = WIN_ERR;

    assert(win->glyphs);
    free(win->glyphs);
    win->glyphs = NULL;
  }

  switch (win->type)
  {
    case NHW_MESSAGE:
      /* remove scroll-back window */
      if (sdlgl_alt_prev)
        sdlgl_remove_scrollback();

      assert(win->scrollback);
      sdlgl_free_tilewin(win->scrollback);
      win->scrollback = NULL;

      /* FALL THROUGH */

    case NHW_STATUS:
    case NHW_MAP:
      sdlgl_unmap_tilewin(win->base);
      sdlgl_free_tilewin(win->base);
      win->base = NULL;

    default:
      break;
  }

  sdlgl_flush();
}

void Sdlgl_curs(winid window, int x, int y)
{
  /* does nothing */
}

void sdlgl_more(struct TextWindow *win)
{
  const char *prompt = MORE_STR;
  struct TilePair old_buf[16];

  int len = strlen(prompt);
  int start_x, start_y;
  int ch;

  assert(win->base);
  assert(win->base->is_text);
  assert(0 < win->fresh_lines && win->fresh_lines <= win->show_h);

  start_x = 0;
  start_y = win->write_y;

  /* when the [MORE] does not fit, put it at the start of the next
   * line.
   */
  if (start_y > 0 && win->write_x + 1 > win->show_w - len)
    start_y--;
  else
    start_x = min(win->show_w - len, win->write_x + 1);

  /* save contents where the [MORE] goes, and restore it later */
  sdlgl_transfer_line(win->base, start_x, start_y, len, old_buf, 1);

  sdlgl_store_str(win->base, start_x, start_y, prompt, len, B_RED);

  for (;;)
  {
    sdlgl_flush();
    ch = sdlgl_get_key(0);

    if (ch == 16 /* ^P */)
    {
      Sdlgl_doprev_message();
      continue;
    }

    if (sdlgl_alt_prev)
      sdlgl_remove_scrollback();

    if (ch && ch == win->dismiss_more)
    {
      win->more_ch = ch;
      break;
    }

    if (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\033' ||
        ch == 'q' || ch == 'n' || ch == 'y')
      break;
  }

  sdlgl_transfer_line(win->base, start_x, start_y, len, old_buf, 0);

  win->fresh_lines = 0;
}

static void message_inc_fresh(struct TextWindow *win)
{
  if (win->fresh_lines >= win->show_h)
  {
    sdlgl_more(win);
    sdlgl_putc(win, '\n');
    return;
  }

  if (win->write_x > 0)
    sdlgl_putc(win, '\n');

  win->fresh_lines += 1;
}

static void do_message_putstr(struct TextWindow *win,
    int attr, const char *str)
{
  int len = strlen(str);
  int use_len;

  /* factor in the size of the [More] */
  int is_bottom = win->fresh_lines == win->show_h;
  int width = win->show_w - (is_bottom ? MORE_LEN : 0);

  if (sdlgl_alt_prev)
    sdlgl_remove_scrollback();

  /* If there is room on the line, print message on same line */
  /* But messages like "You die..." deserve their own line */

  if (win->fresh_lines > 0 && !win->want_new_line &&
      (win->write_x + 2 + len + 1 < width) &&
      strncmp(str, "You die", 7) != 0)
  {
    /* Note: fresh_lines field stays the same */
    sdlgl_puts_nolf(win, "  ");
    sdlgl_puts_nolf(win, str);
    return;
  }

  win->want_new_line = 0;

  while (len > 0)
  {
    message_inc_fresh(win);

    assert(win->write_x == 0);

    is_bottom = win->fresh_lines == win->show_h;
    width = win->show_w - (is_bottom ? MORE_LEN : 0);
    
    /* does it fit yet ? */
    if (len < width)
    {
      sdlgl_puts_nolf(win, str);
      return;
    }

    /* choose a good place to break the line */
    for (use_len = width-1; use_len > width/2; use_len--)
      if (str[use_len] == ' ')
        break;

    /* no space in latter half of string ?  Must be a very long token,
     * so we might as well split it at the screen edge.
     */
    if (str[use_len] != ' ')
      use_len = width-1;

    for (; use_len > 0; use_len--, str++, len--)
      if (*str != '\n' && *str != '\r')
        sdlgl_putc(win, *str);

    /* remove leading spaces */
    for (; *str == ' '; str++, len--) { }
  }
}

void Sdlgl_putstr(winid window, int attr, const char *str)
{
  struct TextWindow *win;
  
  if (window == WIN_ERR)
    return;

  assert (0 <= window && window < MAXWIN && text_wins[window]);
  win = text_wins[window];

  switch (win->type)
  {
    case NHW_MESSAGE:
      strncpy(toplines, str, TBUFSZ);   /* for Norep() */
      toplines[TBUFSZ - 1] = 0;

      do_message_putstr(win, attr, str);
      break;

    case NHW_STATUS:
      /* does nothing -- it is drawn elsewhere */
      break;

    case NHW_MENU:
      if (win->is_menu == 1)
        return;

      if (win->is_menu < 0)
      {
        /* mark this menu as a text window */
        win->is_menu = 0;
        win->calc_w = win->calc_h = 0;
      }
      /*FALL THROUGH*/

    case NHW_TEXT:
    {
      sdlgl_insert_text_item(win, NULL, 0, attr, str);
      break;
    }
     
    default:
      break;
  }
}

tilecol_t sdlgl_attr_to_tilecol(int attr)
{
  switch (attr)
  {
    case ATR_BOLD:    return WHITE;
    case ATR_DIM:     return GREY;
    case ATR_INVERSE: return B_GREEN;
    case ATR_ULINE:   return B_YELLOW;
    case ATR_BLINK:   return B_RED;

    default:
      break;
  }

  return L_GREY;
}

int Sdlgl_doprev_message(void)
{
  struct TextWindow *win;
  int pixel_h;
  int ch;

  int dy, max_foc;

  if (WIN_MESSAGE == WIN_ERR)
    return 0;

  assert(0 <= WIN_MESSAGE && WIN_MESSAGE < MAXWIN);

  win = text_wins[WIN_MESSAGE];
  assert(win);

  if (win->type != NHW_MESSAGE || !win->scrollback)
    return 0;

  if (sdlgl_alt_prev)
  {
    assert(win->show_h > 0);
    assert(win->base);

    /* In the alternate mode, we keep the scrollback tile window
     * mapped, pan it and return.  The next keypress will either be ^P
     * or ^O and sdlgl_adjust_scrollback will be called. or something
     * else and sdlgl_remove_scrollback will be called.
     */
    if (win->scrollback_pos == 0)
    {
      int top_y = win->base->scr_y + win->base->scr_h;
       
      pixel_h = win->scrollback->scale_h;
      sdlgl_map_tilewin(win->scrollback, 0, top_y - pixel_h,
          sdlgl_width, pixel_h, DEPTH_SC_BACK);
    }

    sdlgl_adjust_scrollback(win, +1);
    return 0;
  }
      
  /* NOTE: we don't set sdlgl_top_win here, since this does a
   * different style of panning.
   */
  dy = sdlgl_prev_step;
  win->focus_y = dy - 1;

  pixel_h = win->scrollback->scale_h * (1 + win->focus_y);
  sdlgl_map_tilewin(win->scrollback, 0, sdlgl_height - pixel_h,
      sdlgl_width, pixel_h, DEPTH_SC_BACK);

  max_foc = min(win->scrollback->total_h - 1,
      max(win->scrollback_size - 1, 2));
   
  for (;;)
  {
    sdlgl_flush();
    ch = sdlgl_get_key(POSKEY_ALLOW_REPEAT);

    if (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\033' ||
        ch == 'q' || ch == 'n' || ch == 'y')
      break;

    if (ch == 16 /* ^P */)
    {
      win->focus_y = min(max_foc, win->focus_y + dy);
    }
    else if (ch == 15 /* ^O */ || ch == 14 /* ^N */)
    {
      win->focus_y -= dy;

      if (win->focus_y < 0)
        break;
    }
    else
      continue;

    /* we rely on the bottom-up nature of tilewins, otherwise we'd
     * need to update the pan_y field too.
     */
    pixel_h = win->scrollback->scale_h * (1 + win->focus_y);
    sdlgl_set_new_pos(win->scrollback, 0, sdlgl_height - pixel_h,
        sdlgl_width, pixel_h);
  }

  sdlgl_unmap_tilewin(win->scrollback);
  win->scrollback_pos = 0;

  return 0;
}

/* dy is +1 it scroll back, -1 to scroll forward.
 */
void sdlgl_adjust_scrollback(struct TextWindow *win, int dy)
{
  int old_pos;
  int new_pos;
 
  assert(win && win->scrollback);
  assert(win->base);

  old_pos = win->scrollback_pos;
 
  dy = sgn(dy) * sdlgl_prev_step;

  new_pos = old_pos + dy;

  if (new_pos <= 0)
  {
    sdlgl_remove_scrollback();
    return;
  }

  /* limit scrollback */
  if (new_pos > max(win->show_h, win->scrollback_size + abs(dy) - 1))
    return;
   
  win->scrollback_pos = new_pos;

  if (old_pos < win->show_h || new_pos < win->show_h)
  {
    int top_y = win->base->scr_y + win->base->scr_h;

    int pixel_h = win->scrollback->scale_h * min(win->show_h, new_pos);

    sdlgl_set_new_pos(win->scrollback, 0, top_y - pixel_h,
        sdlgl_width, pixel_h);
  }

  sdlgl_set_pan(win->scrollback, win->scrollback->pan_x,
      max(0, new_pos - win->show_h) * win->scrollback->scale_h);

  sdlgl_flush();
}

void sdlgl_remove_scrollback(void)
{
  struct TextWindow *win;

  if (WIN_MESSAGE == WIN_ERR)
    return;

  assert(0 <= WIN_MESSAGE && WIN_MESSAGE < MAXWIN);

  win = text_wins[WIN_MESSAGE];
  assert(win);

  if (win->type != NHW_MESSAGE || !win->scrollback)
    return;

  /* check if mapped */
  if (win->scrollback_pos == 0)
    return;

  win->scrollback_pos = 0;

  sdlgl_unmap_tilewin(win->scrollback);
  sdlgl_flush();
}

/* returns 1 if key was eaten */
int sdlgl_internal_key_handler(SDL_keysym *key, int repeat)
{
  int shift, ctrl, step;

  assert(key);

  shift = (key->mod & KMOD_SHIFT);
  ctrl  = (key->mod & KMOD_CTRL);

  step = ctrl ? 4 : shift ? 2 : 1;
  
  if (key->sym == SDLK_F5 && !repeat)
  {
    /* FIXME: /tmp is unix specific ! */
    sdlgl_make_screenshot("/tmp/shot_");
    return 1;
  }

  if (sdlgl_top_win != WIN_ERR)
  {
    switch (key->sym)
    {
      case SDLK_LEFT:
        sdlgl_pan_window(sdlgl_top_win, -step, 0);
        return 1;
          
      case SDLK_RIGHT:
        sdlgl_pan_window(sdlgl_top_win, +step, 0);
        return 1;
          
      case SDLK_UP:
        sdlgl_pan_window(sdlgl_top_win, 0, +step);
        return 1;
          
      case SDLK_DOWN:
        sdlgl_pan_window(sdlgl_top_win, 0, -step);
        return 1;
          
      case SDLK_PAGEUP:
        sdlgl_pan_window(sdlgl_top_win, 0, +3);
        return 1;

      case SDLK_PAGEDOWN:
        sdlgl_pan_window(sdlgl_top_win, 0, -3);
        return 1;
      
      case SDLK_HOME:
        sdlgl_pan_window(sdlgl_top_win, -4, +4);
        return 1;
      
      case SDLK_END:
        sdlgl_pan_window(sdlgl_top_win, -4, -4);
        return 1;

      default:
        return 0;
    }
  }

  if (sdlgl_map_win != WIN_ERR)
  {
    /* none of these need auto-repeat */
    if (repeat)
      return 0;

    switch (key->sym)
    {
      case SDLK_PAGEUP:
        sdlgl_zoom_map(ctrl ? +2 : +1);
        sdlgl_flush();
        return 1;

      case SDLK_PAGEDOWN:
        sdlgl_zoom_map(ctrl ? -2 : -1);
        sdlgl_flush();
        return 1;
      
      case SDLK_HOME:
        sdlgl_center_screen_on_player();
        sdlgl_flush();
        return 1;

      case SDLK_END:
        sdlgl_toggle_text_view();
        sdlgl_flush();
        return 1;

      default:
        return 0;
    }
  }

  return 0;
}

void Sdlgl_outrip(winid window, int how)
{
  struct TextWindow *win;
  
  if (window == WIN_ERR)
    return;

  assert (0 <= window && window < MAXWIN && text_wins[window]);
  win = text_wins[window];

  if (win->type == NHW_TEXT)
  {
    /* actual tombstone image is put up at display_nhwindow() time */
    win->is_rip = 1;
    win->how_died = how;
  }
}

#endif /* GL_GRAPHICS */
/*gl_win.c*/
