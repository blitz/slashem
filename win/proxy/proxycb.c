/* $Id$ */
/* Copyright (c) Slash'EM Development Team 2001-2002 */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "nhxdr.h"
#include "winproxy.h"
#include "proxycb.h"

extern int proxy_svc_connection;

/*
 * Warning: This uses a gcc extension. The assumption is that we're going to
 * remove the connection number before release anyway, so it's easier not to
 * add a new parameter to every call to next_rpc().
 */

#define nhext_rpc(id, args...) nhext_rpc_c(proxy_svc_connection, id, args)

void
proxy_cb_display_inventory()
{
    nhext_rpc(EXT_CID_DISPLAY_INVENTORY, 0, 0);
}

int
proxy_cb_dlbh_fopen(name, mode)
char *name, *mode;
{
    int retval;
    nhext_rpc(EXT_CID_DLBH_FOPEN, 2, EXT_STRING(name), EXT_STRING(mode),
	1, EXT_INT_P(retval));
    return retval;
}

char *
proxy_cb_dlbh_fgets(buf, len, fh)
char *buf;
int len, fh;
{
    char *retval, *line = (char *)0;
    nhext_rpc(EXT_CID_DLBH_FGETS, 2, EXT_INT(len), EXT_INT(fh),
	1, EXT_STRING_P(line));
    if (*line) {
	strncpy(buf, line, len - 1);
	buf[len - 1] = '\0';
	retval = buf;
    }
    else
	retval = (char *)0;
    free(line);
    return retval;
}

int
proxy_cb_dlbh_fclose(fh)
int fh;
{
    int retval;
    nhext_rpc(EXT_CID_DLBH_FCLOSE, 1, EXT_INT(fh), 1, EXT_INT_P(retval));
    return retval;
}

void
proxy_cb_flush_screen()
{
    nhext_rpc(EXT_CID_FLUSH_SCREEN, 0, 0);
}

void
proxy_cb_doredraw()
{
    nhext_rpc(EXT_CID_DOREDRAW, 0, 0);
}

void
proxy_cb_status_mode(mode)
int mode;
{
    nhext_rpc(EXT_CID_STATUS_MODE, 1, EXT_INT(mode), 0);
}

int
proxy_cb_parse_options(opts)
char *opts;
{
    int retval;
    nhext_rpc(EXT_CID_PARSE_OPTIONS, 1, EXT_STRING(opts), 1, EXT_INT_P(retval));
    return retval;
}

char *
proxy_cb_get_option(opt)
char *opt;
{
    char *retval;
    nhext_rpc(EXT_CID_GET_OPTION, 1, EXT_STRING(opt), 1, EXT_STRING_P(retval));
    return retval;
}
