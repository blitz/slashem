/* $Id$ */
/* Copyright (c) Slash'EM Development Team 2001-2002 */
/* NetHack may be freely redistributed.  See license for details. */

/* #define DEBUG */

#include <sys/time.h>
#ifdef DEBUG
#include <ctype.h>
#endif
#include "hack.h"
#include "nhxdr.h"
#include "winproxy.h"
#ifdef WIN32
#include "win32api.h"
#endif

/* Window to redirect raw output to, if not WIN_ERR */

int proxy_rawprint_win = WIN_ERR;

/* Current interface mode */

unsigned long proxy_interface_mode = 0;

/* Interface definition for plug-in windowing ports */
struct window_procs proxy_procs = {
    "proxy",
    0xFFFFFFFF,
    proxy_init_nhwindows,
    proxy_player_selection,
    proxy_askname,
    proxy_get_nh_event,
    proxy_exit_nhwindows,
    proxy_suspend_nhwindows,
    proxy_resume_nhwindows,
    proxy_create_nhwindow,
    proxy_clear_nhwindow,
    proxy_display_nhwindow,
    proxy_destroy_nhwindow,
    proxy_curs,
    proxy_putstr,
    proxy_display_file,
    proxy_start_menu,
    proxy_add_menu,
    proxy_end_menu,
    proxy_select_menu,
    proxy_message_menu,
    proxy_update_inventory,
    proxy_mark_synch,
    proxy_wait_synch,
#ifdef CLIPPING
    proxy_cliparound,
#endif
#ifdef POSITIONBAR
    proxy_update_positionbar,
#endif
    proxy_print_glyph,
    proxy_raw_print,
    proxy_raw_print_bold,
    proxy_nhgetch,
    proxy_nh_poskey,
    proxy_nhbell,
    proxy_doprev_message,
    proxy_yn_function,
    proxy_getlin,
    proxy_get_ext_cmd,
    proxy_number_pad,
    proxy_delay_output,
#ifdef CHANGE_COLOR
    proxy_change_color,
#ifdef MAC
    proxy_change_background,
    proxy_set_font_name,
#endif
    proxy_get_color_string,
#endif
    proxy_start_screen,
    proxy_end_screen,
    proxy_outrip
};

static int proxy_connection;

/*
 * Warning: This uses a gcc extension. The assumption is that we're going to
 * remove the connection number before release anyway, so it's easier not to
 * add a new parameter to every call to next_rpc().
 */

#define nhext_rpc(id, args...) nhext_rpc_c(proxy_connection, id, args)

/*
 * The glue functions.
 */

/*
 * Call packet:
 *	ushort ID
 *	int argc
 *	string argv[argc]
 * Reply packet:
 *	ushort ID
 *	boolean inited
 *	int argc
 *	string argv[argc]
 */

void
proxy_init_nhwindows(argcp, argv)
int *argcp;
char **argv;
{
    int i, j, retval;
    struct proxy_init_nhwindow_req req;
    struct proxy_init_nhwindow_res res = {0, 0, (char **)0 };
    req.argc = *argcp;
    req.argv = argv;
    retval = nhext_rpc(EXT_FID_INIT_NHWINDOWS,
      1, EXT_XDRF(proxy_xdr_init_nhwindow_req, &req),
      1, EXT_XDRF(proxy_xdr_init_nhwindow_res, &res));
    if (retval)
    {
	if (res.argc > *argcp)
	    panic("Proxy: Bad argument list from init_nhwindows");
	for(i = 0; i < res.argc; i++)
	{
	    for(j = i; j < *argcp; j++)
		if (!strcmp(res.argv[i], argv[j]))
		{
		    argv[i] = argv[j];
		    break;
		}
	    if (j == *argcp)
		panic("Proxy: Bad argument from init_nhwindows");
	}
	*argcp = res.argc;
	if (res.inited)
	    iflags.window_inited = TRUE;
    }
}

void
proxy_player_selection()
{
    int role, race, gend, align;
    boolean quit;
    nhext_rpc(EXT_FID_PLAYER_SELECTION,
      4, EXT_INT(flags.initrole), EXT_INT(flags.initrace),
         EXT_INT(flags.initgend), EXT_INT(flags.initalign),
      5, EXT_INT_P(role), EXT_INT_P(race), EXT_INT_P(gend), EXT_INT_P(align),
         EXT_BOOLEAN_P(quit));
    if (quit)
    {
	clearlocks();
	proxy_exit_nhwindows((char *)0);
	terminate(0);
    }
    flags.initrole = role;
    flags.initrace = race;
    flags.initgend = gend;
    flags.initalign = align;
}

void
proxy_askname()
{
    char *name = (char *)0;
    nhext_rpc(EXT_FID_ASKNAME, 0, 1, EXT_STRING_P(name));
    strncpy(plname, name, sizeof(plname) - 1);
    plname[sizeof(plname) - 1] = '\0';
    free(name);
}

void
proxy_get_nh_event()
{
    nhext_rpc(EXT_FID_GET_NH_EVENT, 0, 0);
}

void
proxy_exit_nhwindows(str)
const char *str;
{
    nhext_rpc(EXT_FID_EXIT_NHWINDOWS, 1, EXT_STRING(str), 0);
}

void
proxy_suspend_nhwindows(str)
const char *str;
{
    nhext_rpc(EXT_FID_SUSPEND_NHWINDOWS, 1, EXT_STRING(str), 0);
}

void
proxy_resume_nhwindows()
{
    nhext_rpc(EXT_FID_RESUME_NHWINDOWS, 0, 0);
}

winid
proxy_create_nhwindow(type)
int type;
{
    winid id;
    nhext_rpc(EXT_FID_CREATE_NHWINDOW, 1, EXT_INT(type), 1, EXT_WINID_P(id));
    return id;
}

void
proxy_clear_nhwindow(window)
winid window;
{
    nhext_rpc(EXT_FID_CLEAR_NHWINDOW, 1, EXT_WINID(window), 0);
}

void
proxy_display_nhwindow(window, blocking)
winid window;
boolean blocking;
{
    nhext_rpc(EXT_FID_DISPLAY_NHWINDOW,
      2, EXT_WINID(window), EXT_BOOLEAN(blocking), 0);
}

void
proxy_destroy_nhwindow(window)
winid window;
{
    nhext_rpc(EXT_FID_DESTROY_NHWINDOW, 1, EXT_WINID(window), 0);
    mapid_del_winid(window);
}

int proxy_curs_on_u = FALSE;

void
proxy_curs(window, x, y)
winid window;
int x, y;
{
    proxy_curs_on_u = x == u.ux && y == u.uy;
    nhext_rpc(EXT_FID_CURS, 3, EXT_WINID(window), EXT_INT(x), EXT_INT(y), 0);
}

void
proxy_putstr(window, attr, str)
winid window;
int attr;
const char *str;
{
    nhext_rpc(EXT_FID_PUTSTR,
      3, EXT_WINID(window), EXT_INT(attr), EXT_STRING(str), 0);
}

void
#ifdef FILE_AREAS
proxy_display_file(farea, fname, complain)
const char *farea;
#else
proxy_display_file(fname, complain)
#endif
const char *fname;
BOOLEAN_P complain;
{
    int fh;
#ifdef FILE_AREAS
    fh = dlbh_fopen(farea, fname, "r");
#else
    fh = dlbh_fopen(fname, "r");
#endif
    if (fh < 0) {
	if (complain)  pline("Can't open %s.", fname);
	return;
    }
    nhext_rpc(EXT_FID_DISPLAY_FILE,
      1, EXT_INT(fh), 0);
    dlbh_fclose(fh);
}

void
proxy_start_menu(window)
winid window;
{
    nhext_rpc(EXT_FID_START_MENU, 1, EXT_WINID(window), 0);
    mapid_del_identifiers(window);
}

void
proxy_add_menu(window, glyph, identifier, ch, gch, attr, str, preselected)
winid window;
int glyph;
const anything *identifier;
CHAR_P ch, gch;
int attr;
const char *str;
BOOLEAN_P preselected;
{
    int mapping = mapid_map_identifier(window, identifier);
    if (glyph != NO_GLYPH)
	glyph = glyph2proxy[glyph];
    nhext_rpc(EXT_FID_ADD_MENU,
      8, EXT_WINID(window), EXT_INT(glyph), EXT_INT(mapping), EXT_INT(ch),
         EXT_INT(gch), EXT_INT(attr), EXT_STRING(str),
	 EXT_BOOLEAN(preselected),
      0);
}

void
proxy_end_menu(window, prompt)
winid window;
const char *prompt;
{
    nhext_rpc(EXT_FID_END_MENU, 2, EXT_WINID(window), EXT_STRING(prompt), 0);
}

int
proxy_select_menu(window, how, menu_list)
winid window;
int how;
menu_item **menu_list;
{
    int i;
    struct proxy_select_menu_res ret = {0, 0, (struct proxy_mi *)0};
    nhext_rpc(EXT_FID_SELECT_MENU,
      2, EXT_WINID(window), EXT_INT(how),
      1, EXT_XDRF(proxy_xdr_select_menu_res, &ret));
    *menu_list = (menu_item *) alloc(ret.n * sizeof(menu_item));
    for(i = 0; i < ret.n; i++) {
	mapid_unmap_identifier(window, ret.selected[i].item,
	  &(*menu_list)[i].item);
	(*menu_list)[i].count = ret.selected[i].count;
    }
    i = ret.retval;
    nhext_xdr_free(proxy_xdr_select_menu_res, (char *)&ret);
    return i;
}

char
proxy_message_menu(let, how, mesg)
char let;
int how;
const char *mesg;
{
    int ret;
    nhext_rpc(EXT_FID_MESSAGE_MENU,
      3, EXT_INT(let), EXT_INT(how), EXT_STRING(mesg), 1, EXT_INT_P(ret));
    return (char)ret;
}

void
proxy_update_inventory()
{
    nhext_rpc(EXT_FID_UPDATE_INVENTORY, 0, 0);
}

void
proxy_mark_synch()
{
    nhext_rpc(EXT_FID_MARK_SYNC, 0, 0);
}

void
proxy_wait_synch()
{
    nhext_rpc(EXT_FID_WAIT_SYNC, 0, 0);
}

#ifdef CLIPPING
void
proxy_cliparound(x, y)
int x, y;
{
    nhext_rpc(EXT_FID_CLIPAROUND, 2, EXT_INT(x), EXT_INT(y), 0);
}
#endif

#ifdef POSITIONBAR
void
proxy_update_positionbar(posbar)
char *posbar;
{
    nhext_rpc(EXT_FID_UPDATE_POSITIONBAR, 1, EXT_STRING(posbar), 0);
}
#endif

void
proxy_print_glyph(window, x, y, glyph)
winid window;
xchar x, y;
int glyph;
{
#ifdef DISPLAY_LAYERS
    if (proxy_interface_mode & EXT_IM_DISPLAY_LAYERS) {
	struct rm *lev = &levl[x][y];
	int glyphs[5];
	void proxy_print_glyph_layered();

	if (glyph_is_monster(glyph))
	    glyphs[0] = glyph;
	else
	    glyphs[0] = lev->mem_invis ? GLYPH_INVISIBLE : NO_GLYPH;
	if (lev->mem_obj)
	    if (lev->mem_corpse)
		glyphs[1] = body_to_glyph(lev->mem_obj - 1);
	    else
		glyphs[1] = objnum_to_glyph(lev->mem_obj - 1);
	else
	    glyphs[1] = NO_GLYPH;
	glyphs[2] = lev->mem_trap ?
	  cmap_to_glyph(lev->mem_trap - 1 + MAXDCHARS) : NO_GLYPH;
	switch (lev->mem_bg) {
	    case S_room:
	    case S_corr:
	    case S_litcorr:
	    case S_air:
	    case S_water:
		glyphs[3] = NO_GLYPH;
		glyphs[4] = cmap_to_glyph(lev->mem_bg);
		break;
	    default:
		glyphs[3] = cmap_to_glyph(lev->mem_bg);
		if (Is_airlevel(&u.uz))
		    glyphs[4] = cmap_to_glyph(S_air);
		else if (Is_waterlevel(&u.uz))
		    glyphs[4] = cmap_to_glyph(S_water);
		else if (lev->roomno != NO_ROOM)
		    glyphs[4] = cmap_to_glyph(S_room);
		else if (lev->waslit || flags.lit_corridor)
		    glyphs[4] = cmap_to_glyph(S_litcorr);
		else
		    glyphs[4] = cmap_to_glyph(S_corr);
		break;
	}
	proxy_print_glyph_layered(window, x, y, 5, glyphs);
    }
    else
#endif
    nhext_rpc(EXT_FID_PRINT_GLYPH,
      4, EXT_WINID(window), EXT_INT(x), EXT_INT(y), EXT_INT(glyph2proxy[glyph]),
      0);
}

void
proxy_raw_print(str)
const char *str;
{
    static int active = 0;
    if (proxy_rawprint_win != WIN_ERR) {
	proxy_putstr(proxy_rawprint_win, 0, str);
	return;
    }
    if (active++ || !nhext_rpc(EXT_FID_RAW_PRINT, 1, EXT_STRING(str), 0)) {
	puts(str);
	(void) fflush(stdout);
    }
    active--;
}

void
proxy_raw_print_bold(str)
const char *str;
{
    static int active = 0;
    if (proxy_rawprint_win != WIN_ERR) {
	proxy_putstr(proxy_rawprint_win, ATR_BOLD, str);
	return;
    }
    if (active++ || !nhext_rpc(EXT_FID_RAW_PRINT_BOLD, 1, EXT_STRING(str), 0)) {
	puts(str);
	(void) fflush(stdout);
    }
    active--;
}

int
proxy_nhgetch()
{
    int ret;
    nhext_rpc(EXT_FID_NHGETCH, 0, 1, EXT_INT_P(ret));
    return ret;
}

int
proxy_nh_poskey(x, y, mod)
int *x, *y, *mod;
{
    int ret, lx, ly, lmod;
    nhext_rpc(EXT_FID_NH_POSKEY, 0,
      4, EXT_INT_P(ret), EXT_INT_P(lx), EXT_INT_P(ly), EXT_INT_P(lmod));
    *x = lx;
    *y = ly;
    *mod = lmod;
    return ret;
}

void
proxy_nhbell()
{
    nhext_rpc(EXT_FID_NHBELL, 0, 0);
}

int
proxy_doprev_message()
{
    int ret;
    nhext_rpc(EXT_FID_DOPREV_MESSAGE, 0, 1, EXT_INT_P(ret));
    return ret;
}

char
proxy_yn_function(query, resp, def)
const char *query, *resp;
char def;
{
    int ret, count;
    nhext_rpc(EXT_FID_YN_FUNCTION,
      3, EXT_STRING(query), EXT_STRING(resp), EXT_INT(def),
      2, EXT_INT_P(ret), EXT_INT_P(count));
    if (ret == '#')
	yn_number = count;
    return ret;
}

void
proxy_getlin(query, bufp)
const char *query;
char *bufp;
{
    char *reply = (char *)0;
    nhext_rpc(EXT_FID_GETLIN, 1, EXT_STRING(query), 1, EXT_STRING_P(reply));
    strncpy(bufp, reply, BUFSZ - 1);
    bufp[BUFSZ - 1] = '\0';
    free(reply);
}

int
proxy_get_ext_cmd()
{
    int extcmd;
    nhext_rpc(EXT_FID_GET_EXT_CMD, 0, 1, EXT_INT_P(extcmd));
    return extcmd;
}

void
proxy_number_pad(state)
int state;
{
    nhext_rpc(EXT_FID_NUMBER_PAD, 1, EXT_INT(state), 0);
}

void
proxy_delay_output()
{
    nhext_rpc(EXT_FID_DELAY_OUTPUT, 0, 0);
}

#ifdef CHANGE_COLOR
void
proxy_change_color(color, rgb, reverse)
int color;
long rgb;
int reverse;
{
    nhext_rpc(EXT_FID_CHANGE_COLOR,
      3, EXT_INT(color), EXT_LONG(rgb), EXT_BOOL(reverse), 0);
}

#ifdef MAC
void
proxy_change_background(white_or_black)
int white_or_black;
{
    nhext_rpc(EXT_FID_CHANGE_BACKGROUND, 1, EXT_BOOL(white_or_black), 0);
}

short
proxy_set_font_name(window, font)
winid window;
char *font;
{
    int ret;
    nhext_rpc(EXT_FID_SET_FONT_NAME, 2, EXT_WINID(window), EXT_STRING(font),
      1, EXT_INT_P(ret));
    return (short)ret;
}
#endif	/* MAC */

char *
proxy_get_color_string()
{
    char *ret = (char *)0;
    nhext_rpc(EXT_FID_GET_COLOR_STRING, 0, 1, EXT_STRING_P(ret));
    return ret;
}
#endif	/* CHANGE_COLOR */

void
proxy_start_screen()
{
    nhext_rpc(EXT_FID_START_SCREEN, 0, 0);
}

void
proxy_end_screen()
{
    nhext_rpc(EXT_FID_END_SCREEN, 0, 0);
}

void
proxy_outrip(window, how)
winid window;
int how;
{
    boolean handled;
    char *killed_by;
    killed_by = get_killer_string(how);
    nhext_rpc(EXT_FID_OUTRIP, 2, EXT_WINID(window), EXT_STRING(killed_by),
      1, EXT_BOOLEAN_P(handled));
    if (!handled)
	genl_outrip(window, how);
}

void
proxy_status(reconfig, nv, values)
int reconfig, nv;
const char **values;
{
    struct proxy_status_req req;
    req.reconfig = reconfig;
    req.nv = nv;
    req.values = values;
    nhext_rpc(EXT_FID_STATUS, 1, EXT_XDRF(proxy_xdr_status_req, &req), 0);
}

void
proxy_print_glyph_layered(window, x, y, ng, glyphs)
winid window;
xchar x, y;
int ng;
int *glyphs;
{
    int i;
    struct proxy_print_glyph_layered_req req;
    req.window = window;
    req.x = x;
    req.y = y;
    req.ng = ng;
    req.glyphs = (int *)alloc(ng * sizeof(int));
    for(i = 0; i < ng; i++)
	req.glyphs[i] =
	  glyphs[i] == NO_GLYPH ? NO_GLYPH : glyph2proxy[glyphs[i]];
    nhext_rpc(EXT_FID_PRINT_GLYPH_LAYERED,
      1, EXT_XDRF(proxy_xdr_print_glyph_layered_req, &req), 0);
    free(req.glyphs);
}

extern struct nhext_svc proxy_callbacks[];

#ifdef DEBUG
static int FDECL(debug_read, (void *, void *, unsigned int));
static int FDECL(debug_write, (void *, void *, unsigned int));
#endif

/*
 * These counters (which contain the number of unread bytes in each pipe)
 * allow the proxy interface and the server to detect when they need to
 * pass control between each other. This mechanism will not be necessary
 * when they are in seperate processes (and nor would it work).
 */

unsigned long proxy_unread, proxy_svc_unread;

#ifdef WIN32
static HANDLE to_parent[2], to_child[2];

/* Create an anonymous pipe with one end inheritable. */

static int pipe_create(HANDLE *handles, int non_inherit)
{
    HANDLE h;
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;
    if (!CreatePipe(&handles[0], &handles[1], &sa, 0))
	return FALSE;
    if (!DuplicateHandle(GetCurrentProcess(), handles[non_inherit],
      GetCurrentProcess(), &h, 0, FALSE, DUPLICATE_SAME_ACCESS)) {
	CloseHandle(handles[0]);
	CloseHandle(handles[1]);
	return FALSE;
    }
    CloseHandle(handles[non_inherit]);
    handles[non_inherit] = h;
    return TRUE;
}

static void pipe_close(HANDLE *handles)
{
    CloseHandle(handles[0]);
    CloseHandle(handles[1]);
}

int proxy_read(handle, buf, len)
void *handle;
void *buf;
int len;
{
    DWORD nb;
#ifdef PROXY_INTERNAL
    /*
     * Hack for single process operation - let the server handle the pending
     * call; issue any call backs and write a reply.
     */
    if (!proxy_unread)
	win_proxy_svr_iteration();
#endif
    if (!ReadFile((HANDLE)handle, buf, len, &nb, NULL))
	return -1;
    else {
	proxy_unread -= nb;
	return nb;
    }
}

int proxy_write(handle, buf, len)
void *handle;
void *buf;
int len;
{
    DWORD nb;
    if (!WriteFile((HANDLE)handle, buf, len, &nb, NULL))
	return -1;
    else {
	proxy_svc_unread += nb;
	return nb;
    }
}

#ifdef PROXY_INTERNAL
static int
proxy_init()
{
    if (!pipe_create(to_parent, 0))
	return FALSE;
    if (!pipe_create(to_child, 1)) {
	pipe_close(to_parent);
	return FALSE;
    }
    proxy_connection =
#ifndef DEBUG
      nhext_subprotocol1_init(proxy_read, (void *)to_parent[0],
      proxy_write, (void *)to_child[1], proxy_callbacks);
#else
      nhext_subprotocol1_init(debug_read, (void *)to_parent[0],
      debug_write, (void *)to_child[1], proxy_callbacks);
#endif
    if (proxy_connection < 0)
    {
	pipe_close(to_parent);
	pipe_close(to_child);
	return FALSE;
    }
    if (!win_proxy_svr_init(to_child[0], to_parent[1]))
	return FALSE;
    return TRUE;
}
#endif	/* PROXY_INTERNAL */
#else	/* WIN32 */
static int to_parent[2], to_child[2];

static int
proxy_read(handle, buf, len)
void *handle;
void *buf;
unsigned int len;
{
    int fd = (int)handle, nb;
#ifdef PROXY_INTERNAL
    /*
     * Hack for single process operation - let the server handle the pending
     * call; issue any call backs and write a reply.
     */
    if (!proxy_unread)
	win_proxy_svr_iteration();
#endif
    nb = read(fd, buf, len);
    if (nb > 0)
	proxy_unread -= nb;
    return nb;
}

static int
proxy_write(handle, buf, len)
void *handle;
void *buf;
unsigned int len;
{
    int fd = (int)handle, nb;
    fd_set fds;
    struct timeval tv;
    FD_ZERO(&fds);
    FD_SET(fd,&fds);
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    if (select(fd+1, NULL, &fds, NULL, &tv)) {
	nb = write(fd, buf, len);
	if (nb > 0)
	    proxy_svc_unread += nb;
	return nb;
    } else
	return -1;
}

#ifdef PROXY_INTERNAL
static int
proxy_init()
{
    if (pipe(to_child))
	return FALSE;
    if (pipe(to_parent))
    {
	close(to_child[0]);
	close(to_child[1]);
	return FALSE;
    }
    proxy_connection =
#ifndef DEBUG
      nhext_subprotocol1_init(proxy_read, (void *)to_parent[0],
      proxy_write, (void *)to_child[1], proxy_callbacks);
#else
      nhext_subprotocol1_init(debug_read, (void *)to_parent[0],
      debug_write, (void *)to_child[1], proxy_callbacks);
#endif
    if (proxy_connection < 0)
    {
	close(to_child[0]);
	close(to_child[1]);
	close(to_parent[0]);
	close(to_parent[1]);
	return FALSE;
    }
    if (!win_proxy_svr_init(to_child[0], to_parent[1]))
	return FALSE;
    return TRUE;
}
#endif	/* PROXY_INTERNAL */
#endif	/* WIN32 */

#ifndef PROXY_INTERNAL
static int
proxy_init()
{
    proxy_connection =
#ifndef DEBUG
      nhext_subprotocol1_init(proxy_read, (void *)0,
      proxy_write, (void *)1, proxy_callbacks);
#else
      nhext_subprotocol1_init(debug_read, (void *)0,
      debug_write, (void *)1, proxy_callbacks);
#endif
    if (proxy_connection < 0)
	return FALSE;
    return TRUE;
}
#endif	/* !PROXY_INTERNAL */

#ifdef DEBUG
static void
debug_dump(buf, len, arrow)
void *buf;
unsigned int len;
char *arrow;
{
    int i, j, nc;
    long l;
    char cbuf[17];
    unsigned char *bp = buf;
    for(i = 0; i < len; ) {
	if ((i & 15) == 0) {
	    if (!i)
		fputs(arrow, stderr);
	    else {
		cbuf[16] = '\0';
		while(nc++ < 40)
		    fputc(' ', stderr);
		fputs(cbuf, stderr);
		fputs("\n  ", stderr);
	    }
	    nc = 2;
	}
	if (len - i >= 4) {
	    l = (long)bp[i] << 24 | (long)bp[i + 1] << 16 |
	      (long)bp[i + 2] << 8 | bp[i + 3];
	    fprintf(stderr, " %08X", l);
	    nc += 9;
	    for(j = 0; j < 4; j++, i++)
		cbuf[i & 15] = isgraph(bp[i]) || bp[i] == ' ' ?  bp[i] : '.';
	}
	else {
	    fprintf(stderr, " %02X", bp[i]);
	    nc += 3;
	    cbuf[i & 15] = isgraph(bp[i]) || bp[i] == ' ' ? bp[i] : '.';
	    i++;
	}
    }
    if (len) {
	cbuf[i & 15 ? i & 15 : 16] = '\0';
	while(nc++ < 40)
	    fputc(' ', stderr);
	fputs(cbuf, stderr);
    }
    fputc('\n', stderr);
}

static int
debug_read(handle, buf, len)
void *handle;
void *buf;
unsigned int len;
{
    int retval;
    retval = proxy_read(handle, buf, len);
    if (retval < 0)
	fputs("<- ERROR\n", stderr);
    else
	debug_dump(buf, retval, "<-");
    return retval;
}

static int
debug_write(handle, buf, len)
void *handle;
void *buf;
unsigned int len;
{
    int retval;
    retval = proxy_write(handle, buf, len);
    if (retval < 0)
	fputs("-> ERROR\n", stderr);
    else
	debug_dump(buf, retval, "->");
    return retval;
}
#endif	/* DEBUG */

void
win_proxy_init()
{
    set_glyph_mapping();
    if (!proxy_init())
	panic("Proxy: Failed to initialize");
    nhext_rpc(EXT_FID_INIT, 0, 0);
}

int
win_proxy_iteration()
{
    int i;
    i = nhext_svc_c(proxy_connection, proxy_callbacks);
    if (!i)
	fprintf(stderr, "proxy: Ignoring packet with zero ID\n");
    return i;
}
