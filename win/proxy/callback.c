/* $Id$ */
/* Copyright (c) Slash'EM Development Team 2001-2002 */
/* NetHack may be freely redistributed.  See license for details. */

/* #define DEBUG */

#include "hack.h"
#include "func_tab.h"
#include "nhxdr.h"
#include "proxycom.h"
#include "winproxy.h"

static void FDECL(callback_display_inventory, \
			(unsigned short, NhExtXdr *, NhExtXdr *));
static void FDECL(callback_dlbh_fopen, \
			(unsigned short, NhExtXdr *, NhExtXdr *));
static void FDECL(callback_dlbh_fgets, \
			(unsigned short, NhExtXdr *, NhExtXdr *));
static void FDECL(callback_dlbh_fread, \
			(unsigned short, NhExtXdr *, NhExtXdr *));
static void FDECL(callback_dlbh_fclose, \
			(unsigned short, NhExtXdr *, NhExtXdr *));
static void FDECL(callback_flush_screen, \
			(unsigned short, NhExtXdr *, NhExtXdr *));
static void FDECL(callback_doredraw, \
			(unsigned short, NhExtXdr *, NhExtXdr *));
static void FDECL(callback_interface_mode, \
			(unsigned short, NhExtXdr *, NhExtXdr *));
static void FDECL(callback_parse_options, \
			(unsigned short, NhExtXdr *, NhExtXdr *));
static void FDECL(callback_get_option, \
			(unsigned short, NhExtXdr *, NhExtXdr *));
static void FDECL(callback_get_player_choices, \
			(unsigned short, NhExtXdr *, NhExtXdr *));
static void FDECL(callback_is_valid_selection, \
			(unsigned short, NhExtXdr *, NhExtXdr *));
static void FDECL(callback_quit_game, \
			(unsigned short, NhExtXdr *, NhExtXdr *));
static void FDECL(callback_display_score, \
			(unsigned short, NhExtXdr *, NhExtXdr *));
static void FDECL(callback_doset, \
			(unsigned short, NhExtXdr *, NhExtXdr *));
static void FDECL(callback_get_extended_commands, \
			(unsigned short, NhExtXdr *, NhExtXdr *));
static void FDECL(callback_map_menu_cmd, \
			(unsigned short, NhExtXdr *, NhExtXdr *));
static void FDECL(callback_get_standard_winid, \
			(unsigned short, NhExtXdr *, NhExtXdr *));
static void FDECL(callback_get_tilesets, \
			(unsigned short, NhExtXdr *, NhExtXdr *));
static void FDECL(callback_get_glyph_mapping, \
			(unsigned short, NhExtXdr *, NhExtXdr *));

static void
callback_display_inventory(id, request, reply)
unsigned short id;
NhExtXdr *request, *reply;
{
    display_inventory((char *)0, FALSE);
    nhext_rpc_params(reply, 0);
}

#ifndef FILE_AREAS
#define SET_FILE(f, a) if (1) { file = f; } else
#else
#define SET_FILE(f, a) if (1) { file = f; area = a; } else
#endif

static void
callback_dlbh_fopen(id, request, reply)
unsigned short id;
NhExtXdr *request, *reply;
{
    char *name, *mode, *file, *subname;
#ifdef FILE_AREAS
    char *area;
#endif
    char *s;
    char *buf = NULL;
    int i, retval = 0;
    nhext_rpc_params(request, 2, EXT_STRING_P(name), EXT_STRING_P(mode));
    if (name[0] != '$' || name[1] != '(')
	retval = -1;
    else {
	for(i = 0; name[i] && name[i] != ')'; i++)
	    ;
	if (!name[i])
	    retval = -1;
	else {
	    name[i] = '\0';
	    subname = name + i + 1;
	    if (*subname=='/')
		subname++;
	    else if (*subname)
		retval = -1;
	}
    }
    if (strcmp(mode, "r") && strcmp(mode, "rb"))
	retval = -1;
    if (!retval && *subname) {
	if (!strcmp(name + 2, "TILEDIR") || !strcmp(name + 2, "TILEMAPDIR")) {
#ifdef MAXNOTILESETS
	    for(i = 0; i < no_tilesets; i++)
		if (!strcmp(tilesets[i].file, subname))
		    break;
	    if (i == no_tilesets)
		retval = -1;
	    else
#else
	    if (strcmp(tile_file, subname))
		retval = -1;
	    else
#endif
	    if (!strcmp(name + 2, "TILEDIR"))
		SET_FILE(subname, FILE_AREA_SHARE);
	    else {
		s = strrchr(subname, '.');
		if (s) {
		    buf = (char *)alloc(s - subname + 5);
		    (void)strncpy(buf, subname, s - subname);
		} else {
		    buf = (char *)alloc(strlen(subname) + 5);
		    Strcpy(buf, subname);
		}
		Strcat(buf, ".map");
		SET_FILE(buf, FILE_AREA_SHARE);
	    }
	} else
	    retval = -1;
    } else if (!retval) {
	if (!strcmp(name + 2, "RECORD"))
	    SET_FILE(NH_RECORD, NH_RECORD_AREA);
	else if (!strcmp(name + 2, "HELP"))
	    SET_FILE(NH_HELP, NH_HELP_AREA);
	else if (!strcmp(name + 2, "SHELP"))
	    SET_FILE(NH_SHELP, NH_SHELP_AREA);
	else if (!strcmp(name + 2, "DEBUGHELP"))
	    SET_FILE(NH_DEBUGHELP, NH_DEBUGHELP_AREA);
#if 0
	else if (!strcmp(name + 2, "RUMORFILE"))
	    SET_FILE(NH_RUMORFILE, NH_RUMORAREA);
	else if (!strcmp(name + 2, "ORACLEFILE"))
	    SET_FILE(NH_ORACLEFILE, NH_ORACLEAREA);
#endif
	else if (!strcmp(name + 2, "DATAFILE"))
	    SET_FILE(NH_DATAFILE, NH_DATAAREA);
	else if (!strcmp(name + 2, "CMDHELPFILE"))
	    SET_FILE(NH_CMDHELPFILE, NH_CMDHELPAREA);
	else if (!strcmp(name + 2, "HISTORY"))
	    SET_FILE(NH_HISTORY, NH_HISTORY_AREA);
	else if (!strcmp(name + 2, "LICENSE"))
	    SET_FILE(NH_LICENSE, NH_LICENSE_AREA);
	else if (!strcmp(name + 2, "OPTIONFILE"))
	    SET_FILE(NH_OPTIONFILE, NH_OPTIONAREA);
	else if (!strcmp(name + 2, "OPTIONS_USED"))
	    SET_FILE(NH_OPTIONS_USED, NH_OPTIONS_USED_AREA);
	else if (!strcmp(name + 2, "GUIDEBOOK"))
	    SET_FILE(NH_GUIDEBOOK, NH_GUIDEBOOK_AREA);
	else
	    retval = -1;
    }
    if (!retval) {
	dlb_init();
#ifndef FILE_AREAS
	retval = dlbh_fopen(file, mode);
#else
	retval = dlbh_fopen_area(area, file, mode);
#endif
    }
    nhext_rpc_params(reply, 1, EXT_INT(retval));
    free(name);
    free(mode);
    if (buf)
	free(buf);
}

#undef SET_FILE

static void
callback_dlbh_fgets(id, request, reply)
unsigned short id;
NhExtXdr *request, *reply;
{
    int fh, len;
    char *retval;
    char buf[512];	/* Must be small enough that packet fits into 1Kb */
    extern char *dlbh_fgets();
    nhext_rpc_params(request, 2, EXT_INT_P(len), EXT_INT_P(fh));
    retval = dlbh_fgets(buf, len < 512 ? len : 512, fh);
    nhext_rpc_params(reply, 1, EXT_STRING(retval ? retval : ""));
}

static void
callback_dlbh_fread(id, request, reply)
unsigned short id;
NhExtXdr *request, *reply;
{
    int fh, len, nb;
    char buf[512];	/* Must be small enough that packet fits into 1Kb */
    nhext_rpc_params(request, 2, EXT_INT_P(len), EXT_INT_P(fh));
    if (len > 512)
	len = 512;
    nb = dlbh_fread(buf, 1, len, fh);
    nhext_rpc_params(reply, 2,
      EXT_INT(nb < 0), EXT_BYTES(buf, nb >= 0 ? nb : 0));
}

static void
callback_dlbh_fclose(id, request, reply)
unsigned short id;
NhExtXdr *request, *reply;
{
    int fh, retval;
    nhext_rpc_params(request, 1, EXT_INT_P(fh));
    retval = dlbh_fclose(fh);
    nhext_rpc_params(reply, 1, EXT_INT(retval));
}

static void
callback_flush_screen(id, request, reply)
unsigned short id;
NhExtXdr *request, *reply;
{
    extern int proxy_curs_on_u;
    flush_screen(proxy_curs_on_u);
    nhext_rpc_params(reply, 0);
}

static void
callback_doredraw(id, request, reply)
unsigned short id;
NhExtXdr *request, *reply;
{
    (void)doredraw();
    nhext_rpc_params(reply, 0);
}

static void
callback_interface_mode(id, request, reply)
unsigned short id;
NhExtXdr *request, *reply;
{
    nhext_rpc_params(request, 1, EXT_LONG_P(proxy_interface_mode));
    bot_set_handler(proxy_interface_mode & EXT_IM_STATUS ?
      proxy_status : (void (*)())0L);
    nhext_rpc_params(reply, 0);
}

static void
callback_parse_options(id, request, reply)
unsigned short id;
NhExtXdr *request, *reply;
{
    char *opts;
    nhext_rpc_params(request, 1, EXT_STRING_P(opts));
    parseoptions(opts, FALSE, FALSE);
    free(opts);
    nhext_rpc_params(reply, 1, EXT_INT(0));
}

static void
callback_get_option(id, request, reply)
unsigned short id;
NhExtXdr *request, *reply;
{
    char *opt, *value;
    nhext_rpc_params(request, 1, EXT_STRING_P(opt));
    value = get_option(opt);
    free(opt);
    nhext_rpc_params(reply, 1, EXT_STRING(value));
}

static void
callback_get_player_choices(id, request, reply)
unsigned short id;
NhExtXdr *request, *reply;
{
    int i;
    const char *aligns_adj[ROLE_ALIGNS], *genders_adj[ROLE_GENDERS];
    struct proxycb_get_player_choices_res choices;
    choices.n_aligns = ROLE_ALIGNS;
    choices.aligns = aligns_adj;
    for(i = 0; i < ROLE_ALIGNS; i++)
	aligns_adj[i] = aligns[i].adj;
    choices.n_genders = ROLE_GENDERS;
    choices.genders = genders_adj;
    for(i = 0; i < ROLE_GENDERS; i++)
	genders_adj[i] = genders[i].adj;
    for(i = 0; races[i].noun; i++)
	;
    choices.n_races = i;
    choices.races = (const char **)alloc(i * sizeof(const char *));
    for(i = 0; i < choices.n_races; i++)
	choices.races[i] = races[i].noun;
    for(i = 0; roles[i].name.m; i++)
	;
    choices.n_roles = i;
    choices.roles = (struct proxycb_get_player_choices_res_role *)
      alloc(i * sizeof(struct proxycb_get_player_choices_res_role));
    for(i = 0; i < choices.n_roles; i++) {
	choices.roles[i].male = roles[i].name.m;
	choices.roles[i].female = roles[i].name.f ? roles[i].name.f : "";
    }
    nhext_rpc_params(reply, 1,
      EXT_XDRF(proxycb_xdr_get_player_choices_res, &choices));
    free(choices.races);
    free(choices.roles);
}

/*
 * If s is negative, then iterate i over 0 <= i < n, otherwise do just one
 * iteration at i == s. Early exit as soon as we find a valid combination.
 */

#define ITERATE(i, s, n) for((i) = (s) >= 0 ? (s) : 0; \
			     !valid && ((s) >= 0 ? (i) == (s) : (i) < (n)); \
			     (i)++)

static void
callback_is_valid_selection(id, request, reply)
unsigned short id;
NhExtXdr *request, *reply;
{
    int valid = 0;
    int role, race, gend, align;
    int irole, irace, igend, ialign;
    int nrole, nrace;
    nhext_rpc_params(request, 4, EXT_INT_P(role), EXT_INT_P(race),
      EXT_INT_P(gend), EXT_INT_P(align));
    if (role < 0)
	for(nrole = 0; roles[nrole].name.m; nrole++)
	    ;
    if (race < 0)
	for(nrace = 0; races[nrace].noun; nrace++)
	    ;
    ITERATE(irole, role, nrole)
	if (validrole(irole))
	    ITERATE(irace, race, nrace)
		if (validrace(irole, irace)) {
		    ITERATE(igend, gend, ROLE_GENDERS)
			if (validgend(irole, irace, igend))
			    valid = 1;
		    ITERATE(ialign, align, ROLE_ALIGNS)
			if (validalign(irole, irace, ialign))
			    valid = 1;
		}
    nhext_rpc_params(reply, 1, EXT_INT(valid));
}

#undef ITERATE

static void
callback_quit_game(id, request, reply)
unsigned short id;
NhExtXdr *request, *reply;
{
    if (program_state.something_worth_saving)
	done2();
    else {
	clearlocks();
	exit_nhwindows(NULL);
	terminate(EXIT_SUCCESS);
    }
    /* Not reached */
    nhext_rpc_params(reply, 0);
}

static void
callback_display_score(id, request, reply)
unsigned short id;
NhExtXdr *request, *reply;
{
    extern int proxy_rawprint_win;
    char *argv[] = {
	"nethack",
	"-sall",
    };

    proxy_rawprint_win = create_toptenwin();
    prscore(2, argv);
    display_nhwindow(proxy_rawprint_win, TRUE);
    destroy_toptenwin();
    dlb_init();                         /* Re-initialise DLB */
    proxy_rawprint_win = WIN_ERR;
    nhext_rpc_params(reply, 0);
}

static void
callback_doset(id, request, reply)
unsigned short id;
NhExtXdr *request, *reply;
{
    doset();
    nhext_rpc_params(reply, 0);
}

static void
callback_get_extended_commands(id, request, reply)
unsigned short id;
NhExtXdr *request, *reply;
{
    int i;
    extern struct ext_func_tab extcmdlist[];
    struct proxycb_get_extended_commands_res list;
    for(i = 0; extcmdlist[i].ef_txt; i++)
	;
    list.n_commands = i;
    list.commands = (const char **)alloc(i * sizeof(const char *));
    for(i = 0; i < list.n_commands; i++)
	list.commands[i] = extcmdlist[i].ef_txt;
    nhext_rpc_params(reply, 1,
      EXT_XDRF(proxycb_xdr_get_extended_commands_res, &list));
    free(list.commands);
}

static void
callback_map_menu_cmd(id, request, reply)
unsigned short id;
NhExtXdr *request, *reply;
{
    int ch, retval;
    nhext_rpc_params(request, 1, EXT_INT_P(ch));
    retval = map_menu_cmd(ch);
    nhext_rpc_params(reply, 1, EXT_INT(retval));
}

static void
callback_get_standard_winid(id, request, reply)
unsigned short id;
NhExtXdr *request, *reply;
{
    char *window;
    int retval;
    nhext_rpc_params(request, 1, EXT_STRING_P(window));
    if (!strcmp(window,"MESSAGE"))
	retval = WIN_MESSAGE;
    else if (!strcmp(window,"STATUS"))
	retval = WIN_STATUS;
    else if (!strcmp(window,"MAP"))
	retval = WIN_MAP;
    else if (!strcmp(window,"INVEN"))
	retval = WIN_INVEN;
    else
	retval = -1;
    nhext_rpc_params(reply, 1, EXT_INT(retval));
}

static void
callback_get_tilesets(id, request, reply)
unsigned short id;
NhExtXdr *request, *reply;
{
    int i;
    char *file;
    struct proxycb_get_tilesets_res list;
#ifdef MAXNOTILESETS
    list.n_tilesets = no_tilesets;
    list.tilesets = (struct proxycb_get_tilesets_res_tileset *)
      alloc(no_tilesets * sizeof(*list.tilesets));
    for(i = 0; i < list.n_tilesets; i++) {
	list.tilesets[i].name = tilesets[i].name;
	file = (char *)alloc(strlen(tilesets[i].file) + 12);
	sprintf(file, "$(TILEDIR)/%s", tilesets[i].file);
	list.tilesets[i].file = file;
	file = (char *)alloc(strlen(tilesets[i].file) + 16);
	sprintf(file, "$(TILEMAPDIR)/%s", tilesets[i].file);
	list.tilesets[i].mapfile = file;
	list.tilesets[i].flags = tilesets[i].flags;
    }
#else
    list.n_tilesets = 1;
    list.tilesets = (struct proxycb_get_tilesets_res_tileset *)
      alloc(1 * sizeof(*list.tilesets));
    list.tilesets[0].name = "Default";
    file = (char *)alloc(strlen(tile_file) + 12);
    sprintf(file, "$(TILEDIR)/%s", tile_file);
    list.tilesets[0].file = file;
    file = (char *)alloc(strlen(tile_file) + 16);
    sprintf(file, "$(TILEMAPDIR)/%s", tile_file);
    list.tilesets[0].mapfile = file;
    list.tilesets[0].flags = 0;
#endif
    nhext_rpc_params(reply, 1, EXT_XDRF(proxycb_xdr_get_tilesets_res, &list));
    for(i = 0; i < list.n_tilesets; i++)
	free((char *)list.tilesets[i].file);
    free(list.tilesets);
}

static void
callback_get_glyph_mapping(id, request, reply)
unsigned short id;
NhExtXdr *request, *reply;
{
    struct proxycb_get_glyph_mapping_res *mapping;
    mapping = get_glyph_mapping();
    nhext_rpc_params(reply, 1,
      EXT_XDRF(proxycb_xdr_get_glyph_mapping_res, mapping));
    free_glyph_mapping(mapping);
}

struct nhext_svc proxy_callbacks[] = {
    EXT_CID_DISPLAY_INVENTORY,		callback_display_inventory,
    EXT_CID_DLBH_FOPEN,			callback_dlbh_fopen,
    EXT_CID_DLBH_FGETS,			callback_dlbh_fgets,
    EXT_CID_DLBH_FREAD,			callback_dlbh_fread,
    EXT_CID_DLBH_FCLOSE,		callback_dlbh_fclose,
    EXT_CID_FLUSH_SCREEN,		callback_flush_screen,
    EXT_CID_DOREDRAW,			callback_doredraw,
    EXT_CID_INTERFACE_MODE,		callback_interface_mode,
    EXT_CID_PARSE_OPTIONS,		callback_parse_options,
    EXT_CID_GET_OPTION,			callback_get_option,
    EXT_CID_GET_PLAYER_CHOICES,		callback_get_player_choices,
    EXT_CID_IS_VALID_SELECTION,		callback_is_valid_selection,
    EXT_CID_QUIT_GAME,			callback_quit_game,
    EXT_CID_DISPLAY_SCORE,		callback_display_score,
    EXT_CID_DOSET,			callback_doset,
    EXT_CID_GET_EXTENDED_COMMANDS,	callback_get_extended_commands,
    EXT_CID_MAP_MENU_CMD,		callback_map_menu_cmd,
    EXT_CID_GET_STANDARD_WINID,		callback_get_standard_winid,
    EXT_CID_GET_TILESETS,		callback_get_tilesets,
    EXT_CID_GET_GLYPH_MAPPING,		callback_get_glyph_mapping,
    0, NULL,
};
