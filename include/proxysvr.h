/* $Id$ */
/* Copyright (c) Slash'EM Development Team 2002 */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef PROXYSVR_H
#define PROXYSVR_H

extern short *proxy_glyph2tile;

struct proxy_tilemap_entry {
    unsigned int tile;
    unsigned int refs;
    unsigned int no_descs;
    char **descs;
};

struct proxy_tilemap {
    unsigned int no_entries;		/* One tile may have several entries */
    unsigned int no_tiles;
    unsigned int max_entries;		/* No. allocted entries */
    struct proxy_tilemap_entry *entries;
};

#define PROXY_LEVEL_MAPPING		0
#define PROXY_LEVEL_FLAGS		1
#define PROXY_LEVEL_SUBMAPPING		2
#define PROXY_LEVEL_GLYPH		3
#define PROXY_LEVEL_BASED_MAPPING	4
#define PROXY_LEVEL_BASED_SUBMAPPING	5
#define PROXY_LEVEL_BASED_GLYPH		6
#define PROXY_MAX_NO_LEVELS		7

struct proxy_glyph_mapping {
    unsigned long rgbsym;
    int alt_glyph;
    unsigned int no_descs;
    const char **descs;
};

struct proxy_glyph_map_info {
    struct proxy_glyph_mapping current;
    struct proxycb_get_glyph_mapping_res *glyph_map;
    int mi, smi, gi, bsmi, bgi;
    const char *descs[PROXY_MAX_NO_LEVELS];
};

struct window_ext_procs {
    const char *name;
    int FDECL((*winext_init_nhwindows), (int *, char **));
    int FDECL((*winext_player_selection), (int *, int *, int *, int *));
    char *NDECL((*winext_askname));
    void NDECL((*winext_get_nh_event)) ;
    void FDECL((*winext_exit_nhwindows), (const char *));
    void FDECL((*winext_suspend_nhwindows), (const char *));
    void NDECL((*winext_resume_nhwindows));
    int FDECL((*winext_create_nhwindow), (int));
    void FDECL((*winext_clear_nhwindow), (int));
    void FDECL((*winext_display_nhwindow), (int, BOOLEAN_P));
    void FDECL((*winext_destroy_nhwindow), (int));
    void FDECL((*winext_curs), (int,int,int));
    void FDECL((*winext_putstr), (int, int, const char *));
    void FDECL((*winext_display_file), (int));
    void FDECL((*winext_start_menu), (int));
    void FDECL((*winext_add_menu), (int,int,int,
		CHAR_P,CHAR_P,int,const char *, BOOLEAN_P));
    void FDECL((*winext_end_menu), (int, const char *));
    int FDECL((*winext_select_menu), (int, int, struct proxy_mi **));
    int FDECL((*winext_message_menu), (int,int,const char *));
    void NDECL((*winext_update_inventory));
    void NDECL((*winext_mark_synch));
    void NDECL((*winext_wait_synch));
    void FDECL((*winext_cliparound), (int, int));
    void FDECL((*winext_update_positionbar), (char *));
    void FDECL((*winext_print_glyph), (int,int,int,int));
    void FDECL((*winext_raw_print), (const char *));
    void FDECL((*winext_raw_print_bold), (const char *));
    int NDECL((*winext_nhgetch));
    int FDECL((*winext_nh_poskey), (int *, int *, int *));
    void NDECL((*winext_nhbell));
    int NDECL((*winext_doprev_message));
    char FDECL((*winext_yn_function), (const char *, const char *,
		CHAR_P, int *));
    char *FDECL((*winext_getlin), (const char *));
    int NDECL((*winext_get_ext_cmd));
    void FDECL((*winext_number_pad), (int));
    void NDECL((*winext_delay_output));
    void FDECL((*winext_change_color), (int, long, int));
    void FDECL((*winext_change_background), (int));
    int FDECL((*winext_set_font_name), (int, char *));
    char *NDECL((*winext_get_color_string));
    void NDECL((*winext_start_screen));
    void NDECL((*winext_end_screen));
    int FDECL((*winext_outrip), (int, char *));
    void FDECL((*winext_status), (int, int, const char **));
    void FDECL((*winext_print_glyph_layered), (int, int, int, int, int *));
};

/* ### proxysvc.c ### */

extern void proxy_svc_set_ext_procs(void (*)(void), struct window_ext_procs *);
extern int win_proxy_svr_init(void *, void *);
extern char *win_proxy_svr_get_failed_packet(int *);

/* ### prxymap.c ### */

extern struct proxy_glyph_mapping *proxy_glyph_map_first(
	struct proxy_glyph_map_info *, struct proxycb_get_glyph_mapping_res *);
extern struct proxy_glyph_mapping *proxy_glyph_map_next(
	struct proxy_glyph_map_info *);
extern void proxy_glyph_map_close(struct proxy_glyph_map_info *);

/* ### prxytile.c ### */

extern struct proxy_tilemap *proxy_load_tilemap(int);
extern void proxy_free_tilemap(struct proxy_tilemap *);
extern short * proxy_map_glyph2tile(
	struct proxycb_get_glyph_mapping_res *glyph_map,
	struct proxy_tilemap *);

/* ### prxychar.c ### */

extern long *proxy_map_glyph2char(struct proxycb_get_glyph_mapping_res *);

#endif /* PROXYSVR_H */
