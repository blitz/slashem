/*
  $Id$
 */
/*
  GTK+ NetHack Copyright (c) Issei Numata 1999-2000
               Copyright (c) Slash'EM Development Team 2001-2002
  GTK+ NetHack may be freely redistributed.  See license for details. 
*/

/* #define DEBUG */			/* Uncomment for debugging */

#include <sys/types.h>
#include <signal.h>
#include "winGTK.h"
#include "wintype.h"
#include "func_tab.h"
#include "dlb.h"
#ifdef SHORT_FILENAMES
#include "patchlev.h"
#else
#include "patchlevel.h"
#endif
#include "proxycb.h"

static int	initialized;
static int	display_inventory_needed;
static int	in_topten;

static void	select_player(GtkWidget *w, guint data);
static void	key_command(GtkWidget *w, gpointer data);
static void	move_command(GtkWidget *w, gpointer data);
static void	fight_command(GtkWidget *w, gpointer data);
static void	ext_command(GtkWidget *w, gpointer data);
static void	game_option(GtkWidget *w, gpointer data);
static void	game_quit(GtkWidget *w, gpointer data);
static void	game_topten(GtkWidget *w, gpointer data);
static void	nh_menu_sensitive(char *menu, boolean f);

static int      GTK_display_file(char *name);
static void	GTK_display_inventory(void);

static void	help_help(GtkWidget *w, gpointer data);
static void	help_shelp(GtkWidget *w, gpointer data);
static void	help_option(GtkWidget *w, gpointer data);
static void	help_je(GtkWidget *w, gpointer data);
static void	help_history(GtkWidget *w, gpointer data);
static void	help_license(GtkWidget *w, gpointer data);

#define GTK_NORTH	0
#define GTK_EAST	1
#define GTK_SOUTH	2
#define GTK_WEST	3
#define GTK_NORTHEAST	4
#define GTK_NORTHWEST	5
#define GTK_SOUTHEAST	6
#define GTK_SOUTHWEST	7

NHWindow gtkWindows[MAXWIN];

static winid		rawprint_win = WIN_ERR;

GtkAccelGroup *accel_group=NULL;

GtkWidget		*main_window, *main_vbox;
#ifdef RADAR
static GtkWidget	*main_radar;
#endif

static GtkItemFactory	*main_item_factory;

int			root_width;
int			root_height;

GdkColor	  nh_color[N_NH_COLORS] = {
    /*
     * Standard NetHack colours (CLR_...)
     * Note: black & white may be remapped depending on map background
     */
    {0, 0*257, 0*257, 0*257,},		/* black */
    {0, 255*257, 0*257, 0*257,},	/* red */
    {0, 152*257, 251*257, 152*257,},	/* pale green */
    {0, 165*257, 42*257, 42*257,},	/* brown */
    {0, 0*257, 0*257, 255*257,},	/* blue */
    {0, 255*257, 0*257, 255*257,}, 	/* magenta */
    {0, 224*257, 255*257, 255*257,}, 	/* light cyan */
    {0, 190*257, 190*257, 190*257,},	/* gray */
    {1, 0*257, 0*257, 0*257,},		/* default  */
    {0, 255*257, 165*257, 0*257,},	/* orange */
    {0, 0*257, 255*257, 0*257,},	/* green */
    {0, 255*257, 255*257, 0*257,}, 	/* yellow */
    {0, 65*257, 105*257, 225*257,},	/* royal blue */
    {0, 238*257, 130*257, 238*257,},	/* violet */
    {0, 0*257, 255*257, 255*257,},	/* cyan */
    {0, 255*257, 255*257, 255*257,},	/* white */
    /*
     * Local additions (MAP_...)
     */
    {0, 0*257, 100*257, 0*257,},	/* dark green */
    {0, 0*257, 0*257, 0*257,},		/* fixed black */
    {0, 255*257, 255*257, 255*257,},	/* fixed white */
};

static struct proxycb_get_player_choices_res *player_choices = NULL;

char *
GTK_ext_askname() {
    int tryct = 0;
    char *buf = (char *)0;
    static char who_are_you[] = "Who are you? ";
    
    do {
	if (tryct > 10) panic("Giving up after 10 tries.\n");
	else tryct++;
    	
	if (buf)
	    free(buf);
	buf = GTK_ext_getlin(who_are_you);
    } while (*buf == '\0');
    return buf;
}

static GtkItemFactoryEntry mainmenu_items[] = {
    {"/Game",			NULL,		NULL,		0,	"<Branch>"},
    {"/Game/Gtear1",		NULL,		NULL,		0,	"<Tearoff>"},
    {"/Game/Play",		NULL,		NULL,		0,	"<Branch>"},
    {"/Game/Gsep1",		NULL,		NULL,		0,	"<Separator>"},
    {"/Game/Save",		"<shift>S",	key_command,	'S',	NULL},
    {"/Game/Option",		"<shift>O",	game_option,	'O',    NULL},
    {"/Game/Score",		NULL,		game_topten,	0,	NULL},
    {"/Game/Gsep2",		NULL,		NULL,		0,	"<Separator>"},
    {"/Game/Quit",		NULL,		game_quit,	0,	NULL},
};

static GtkItemFactoryEntry *menu_items;
static int nmenu_items;

static GtkItemFactoryEntry helpmenu_items[] = {
    {"/Help",			NULL,		NULL,		0,	"<LastBranch>"},
    {"/Help/Htear1",		NULL,		NULL,		0,	"<Tearoff>"},
    {"/Help/Command Help",	NULL,		help_help,	0,	NULL},
    {"/Help/Key Help",		NULL,		help_shelp,	0,	NULL},
    {"/Help/Option Help",	NULL,		help_option,	0,	NULL},
    {"/Help/sep3",		NULL,		NULL,		0,	"<Separator>"},
    {"/Help/History",		NULL,		help_history,	0,	NULL},
    {"/Help/License",		NULL,		help_license,	0,	NULL},
};

static GtkItemFactoryEntry playmenu_items[] = {
    {"/Move",			NULL,		NULL,		0,	"<Branch>"},
    {"/Move/Mtear1",		NULL,		NULL,		0,	"<Tearoff>"},
    {"/Move/North",		NULL,		move_command,	GTK_NORTH,	NULL},
    {"/Move/East",		NULL,		move_command,	GTK_EAST,	NULL},
    {"/Move/South",		NULL,		move_command,	GTK_SOUTH,	NULL},
    {"/Move/West",		NULL,		move_command,	GTK_WEST,	NULL},
    {"/Move/Northeast",		NULL,		move_command,	GTK_NORTHEAST,	NULL},
    {"/Move/Northwest",		NULL,		move_command,	GTK_NORTHWEST,	NULL},
    {"/Move/Southeast",		NULL,		move_command,	GTK_SOUTHEAST,	NULL},
    {"/Move/Southwest",		NULL,		move_command,	GTK_SOUTHWEST,	NULL},
    {"/Move/Down",		"greater",	key_command,	'>',	NULL},
    {"/Move/Up",		"less",		key_command,	'<',	NULL},
    {"/Fight",			NULL,		NULL,		0,	"<Branch>"},
    {"/Fight/Ftear1",		NULL,		NULL,		0,	"<Tearoff>"},
    {"/Fight/North",		NULL,		fight_command,	GTK_NORTH,	NULL},
    {"/Fight/East",		NULL,		fight_command,	GTK_EAST,	NULL},
    {"/Fight/South",		NULL,		fight_command,	GTK_SOUTH,	NULL},
    {"/Fight/West",		NULL,		fight_command,	GTK_WEST,	NULL},
    {"/Fight/Northeast",	NULL,		fight_command,	GTK_NORTHEAST,	NULL},
    {"/Fight/Northwest",	NULL,		fight_command,	GTK_NORTHWEST,	NULL},
    {"/Fight/Southeast",	NULL,		fight_command,	GTK_SOUTHEAST,	NULL},
    {"/Fight/Southwest",	NULL,		fight_command,	GTK_SOUTHWEST,	NULL},
    {"/Check",			NULL,		NULL,		0,	"<Branch>"},
    {"/Check/Ctear1",		NULL,		NULL,		0,	"<Tearoff>"},
    {"/Check/Here",		"colon",	key_command,	':',	NULL},
    {"/Check/There",		"semicolon",	key_command,	';',	NULL},
    {"/Check/Trap",		"asciicircum",	key_command,	'^',	NULL},
    {"/Equip",			NULL,		NULL,		0,	"<Branch>"},
    {"/Equip/Etear1",		NULL,		NULL,		0,	"<Tearoff>"},
    {"/Equip/Wield",		"w",		key_command,	'w',	NULL},	
    {"/Equip/Exchange weapons",	"x",		key_command,	'x',	NULL},	
    {"/Equip/Two-handed",	NULL,		ext_command,	EXT_CMD_TWOWEAPON,	NULL},	
    {"/Equip/Quiver",		"<shift>Q",	key_command,	'Q',	NULL},
    {"/Equip/Wear",		"<shift>w",	key_command,	'W',	NULL},	
    {"/Equip/Take off",		"<shift>T",	key_command,	'T',	NULL},	
    {"/Equip/Puton",		"<shift>p",	key_command,	'P',	NULL},	
    {"/Equip/Remove",		"<shift>r",	key_command,	'R',	NULL},	
    {"/You",			NULL,		NULL,		0,	"<Branch>"},
    {"/You/Ytear1",		NULL,		NULL,		0,	"<Tearoff>"},
    {"/You/Inventory",		"i",		key_command,	'i',	NULL},
    {"/You/Weapon",		"parenright",	key_command,	')',	NULL},	
    {"/You/Armor",		"bracketleft",	key_command,	'[',	NULL},	
    {"/You/Ring",		"equal",	key_command,	'=',	NULL},	
    {"/You/Amulet",		"quotedbl",	key_command,	'"',	NULL},	
    {"/You/Tool",		"parenleft",	key_command,	'(',	NULL},	
    {"/You/Spells",		"plus",		key_command,	'+',	NULL},	
    {"/You/Known Item",		"backslash",	key_command,	'\\',	NULL},	
    {"/You/Conduct",		NULL,		ext_command,	EXT_CMD_CONDUCT,	NULL},	
    {"/Adventure",		NULL,		NULL,		0,	"<Branch>"},
    {"/Adventure/ADtear1",	NULL,		NULL,		0,	"<Tearoff>"},
    {"/Adventure/Name",		"<alt>n",	key_command,	'n' | 0x80,	NULL},
    {"/Adventure/Call",		"<shift>C",	key_command,	'C',	NULL},
    {"/Adventure/Adjust",	"<alt>a",	key_command,	'a' | 0x80,	NULL},
    {"/Action",			NULL,		NULL,		0,	"<Branch>"},
    {"/Action/ACtear1",		NULL,		NULL,		0,	"<Tearoff>"},
    {"/Action/Rest",		"period",	key_command,	'.',	NULL},
    {"/Action/Search",		"s",		key_command,	's',	NULL},
    {"/Action/Eat",		"e",		key_command,	'e',	NULL},
    {"/Action/Asep1",		NULL,		NULL,		0,	"<Separator>"},
    {"/Action/Open",		"o",		key_command,	'o',	NULL},
    {"/Action/Close",		"c",		key_command,	'c',	NULL},
    {"/Action/Asep2",		NULL,		NULL,		0,	"<Separator>"},
    {"/Action/Pickup",		"comma",	key_command,	',',	NULL},
    {"/Action/Drop",		"d",		key_command,	'd',	NULL},
    {"/Action/Loot",		"<alt>l",	key_command,	'l' | 0x80, NULL},
    {"/Action/Apply",		"a",		key_command,	'a',	NULL},
    {"/Action/Kick",		"<control>D",	key_command,	'\04',	NULL},
    {"/Action/Throw",		"t",		key_command,	't',	NULL},
    {"/Action/Fire",		"f",		key_command,	'f',	NULL},
    {"/Action/Asep3",		NULL,		NULL,		0,	"<Separator>"},
    {"/Action/Drink",		"q",		key_command,	'q',	NULL},
    {"/Action/Read",		"r",		key_command,	'r',	NULL},
    {"/Action/Cast Spell",	"<shift>Z",	key_command,	'Z',	NULL},
    {"/Action/Zap",		"z",		key_command,	'z',	NULL},
    {"/Action/Dip",		"<alt>d",	key_command,	'd' | 0x80,	NULL},
    {"/Action/Sit",		"<alt>s",	key_command,	's' | 0x80,	NULL},
    {"/Religion",		NULL,		NULL,		0, 	"<Branch>"},
    {"/Religion/Rtear1",		NULL,		NULL,		0,	"<Tearoff>"},
    {"/Religion/Pray",		"<alt>p",	key_command,	'p' | 0x80,	NULL},
    {"/Religion/Offer",		"<alt>o",	key_command,	'o' | 0x80,	NULL},
    {"/Special",		NULL,		NULL,		0, 	"<Branch>"},
    {"/Special/Stear1",		NULL,		NULL,		0,	"<Tearoff>"},
    {"/Special/Engrave",	"<shift>E",	key_command,	'E', 		NULL},
    {"/Special/Pay",		"p",		key_command,	'p', 		NULL},
    {"/Special/Borrow",		"<alt>b",	key_command,	'b' | 0x80,	NULL},
    {"/Special/Chat",		"<alt>c",	key_command,	'c' | 0x80,	NULL},
    {"/Special/Enhance",	"<alt>e",	key_command,	'e' | 0x80,	NULL},
    {"/Special/Force",		"<alt>f",	key_command,	'f' | 0x80,	NULL},
    {"/Special/Invoke",		"<alt>i",	key_command,	'i' | 0x80,	NULL},
    {"/Special/Monster",	"<alt>m",	key_command,	'm' | 0x80,	NULL},
    {"/Special/Ride",		NULL,		ext_command,	EXT_CMD_RIDE,	NULL},
    {"/Special/Rub",		"<alt>r",	key_command,	'r' | 0x80,	NULL},
    {"/Special/Wipe",		"<alt>w",	key_command,	'w' | 0x80,	NULL},
    {"/Special/Technique",	"<alt>t",	key_command,	't' | 0x80,	NULL},
    {"/Special/Teleport",	"<control>t",	key_command,	'',	NULL},
    {"/Special/Turn",		NULL,		ext_command,	EXT_CMD_TURN,	NULL},
    {"/Special/Untrap",		"<alt>u",	key_command,	'u' | 0x80,	NULL},
    {"/Special/Jump",		"<alt>j",	key_command,	'j' | 0x80,	NULL},
};

static int keysym, keysym1;

void
win_GTK_init()
{
    ;
}

/*
 * Non-modal dialog windows use a partial grab system whereby a number of
 * events which would normally be passed to other widgets are blocked, but
 * still allows scrolling etc.
 */

static gint
nh_dialog_partial_grab(GtkWidget *widget, gpointer data)
{
    boolean enable = !GPOINTER_TO_INT(data);
    nh_menu_sensitive("/Game", enable);
    nh_menu_sensitive("/Move", enable);
    nh_menu_sensitive("/Fight", enable);
    nh_menu_sensitive("/Check", enable);
    nh_menu_sensitive("/Equip", enable);
    nh_menu_sensitive("/You", enable);
    nh_menu_sensitive("/Adventure", enable);
    nh_menu_sensitive("/Action", enable);
    nh_menu_sensitive("/Religion", enable);
    nh_menu_sensitive("/Special", enable);
    nh_menu_sensitive("/Help", enable);
    return 0;
}

GtkWidget *nh_gtk_window_dialog(boolean is_modal)
{
    GtkWidget *w;
    w = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    if (w) {
	gtk_window_set_transient_for(GTK_WINDOW(w), GTK_WINDOW(main_window));
	gtk_window_set_modal(GTK_WINDOW(w), is_modal);
	if (!is_modal) {
	    nh_dialog_partial_grab(w, GINT_TO_POINTER(1));
	    gtk_signal_connect(GTK_OBJECT(w), "destroy",
	      GTK_SIGNAL_FUNC(nh_dialog_partial_grab), 0);
	}
#if GTK_CHECK_VERSION(1,3,2)
	gtk_window_set_destroy_with_parent(GTK_WINDOW(w), TRUE);
#endif
    }
    return w;
}

GtkWidget *
nh_gtk_new(GtkWidget *w, GtkWidget *parent, gchar *lbl)
{
#if 0
    /* [ALI] Removed this code, it doesn't seem to achive anything */
    gtk_widget_ref(w);
    gtk_object_set_data_full(
	GTK_OBJECT(parent), lbl, w,
	(GtkDestroyNotify)gtk_widget_unref);
#endif
    gtk_widget_show(w);

    return w;
}

GtkWidget *
nh_gtk_new_and_add(GtkWidget *w, GtkWidget *parent, gchar *lbl)
{
#if 0
    gtk_widget_ref(w);
    gtk_object_set_data_full(
	GTK_OBJECT(parent), lbl, w,
	(GtkDestroyNotify)gtk_widget_unref);
#endif
    gtk_widget_show(w);

    gtk_container_add(GTK_CONTAINER(parent), w);

    return w;
}

GtkWidget *
nh_gtk_new_and_pack(GtkWidget *w, GtkWidget *parent, gchar *lbl, 
		    gboolean a1, gboolean a2, guint a3)
{
#if 0
    gtk_widget_ref(w);
    gtk_object_set_data_full(
	GTK_OBJECT(parent), lbl, w,
	(GtkDestroyNotify)gtk_widget_unref);
#endif
    gtk_widget_show(w);

    gtk_box_pack_start(GTK_BOX(parent), w, a1, a2, a3);

    return w;
}

GtkWidget *
nh_gtk_new_and_attach(GtkWidget *w, GtkWidget *parent, gchar *lbl, 
		      guint a1, guint a2, guint a3, guint a4)
{
#if 0
    gtk_widget_ref(w);
    gtk_object_set_data_full(
	GTK_OBJECT(parent), lbl, w,
	(GtkDestroyNotify)gtk_widget_unref);
#endif
    gtk_widget_show(w);

    gtk_table_attach_defaults(GTK_TABLE(parent), w, a1, a2, a3, a4);

    return w;
}

GtkWidget *
nh_gtk_new_and_attach2(GtkWidget *w, GtkWidget *parent, gchar *lbl, 
		      guint a1, guint a2, guint a3, guint a4,
		      GtkAttachOptions a5,
		      GtkAttachOptions a6,
		      guint a7, guint  a8)
{
#if 0
    gtk_widget_ref(w);
    gtk_object_set_data_full(
	GTK_OBJECT(parent), lbl, w,
	(GtkDestroyNotify)gtk_widget_unref);
#endif
    gtk_widget_show(w);

    gtk_table_attach(GTK_TABLE(parent), w, a1, a2, a3, a4, a5, a6, a7, a8);

    return w;
}

int
nh_keysym(GdkEventKey *ev)
{
    int ret;

    switch(ev->keyval)
    {
	case GDK_Return:
	case GDK_KP_Enter:
	    ret = '\n';
	    break;
	case GDK_Escape:
	    ret = '\033';
	    break;
	default:
	    if (ev->length)
		ret = ev->string[0];
	    else
		ret = 0;
    }

    return ret;
}

static void
nh_menu_sensitive(char *menu, boolean f)
{
    GtkWidget *p;
    
    p = gtk_item_factory_get_widget(
	main_item_factory, menu);
    gtk_widget_set_sensitive(p, f);
}

/* ALI
 * We need to know the border width and height so that we can correctly
 * clamp dialogs to the screen. Unfortunately, GDK allows us no way to
 * dermine the border sizes before we map the window. We get around this
 * by remembering what they were for the last dialog displayed and
 * checking that they haven't changed after the window is mapped. If
 * we find we're offscreen after mapping we have to re-position the
 * window. Clumsy, but effective.
 *
 * Note: there's no way that I know of to determine the width of the
 * right border or the height of the bottom border. For now, we just
 * assume that these are both equal to the left border. Typically,
 * only the top border is different and this is a valid assumption.
 *
 * This is all further complicated by the fact that, at least under
 * the Enlightenment window manager, gtk_widget_set_uposition() sets
 * the position of the top left pixel of the border before the window
 * is mapped, but the top left pixel of the window after it is mapped.
 *
 * Note: If the window manager is set up such that gtk_widget_set_upostion()
 * always sets the position of the left left pixel of the window, and if
 * gtk_window_get_origin() returns this same position, then this code
 * will fail (gracefully) and dialogs may be positioned with their borders
 * just off-screen.
 */

static gint popup_dialog_bw = 0, popup_dialog_bh = 0;

static void
popup_dialog_mapped(GtkWidget *w, gpointer data)
{
    gint screen_width, screen_height;
    GdkWindow *window = GTK_WIDGET(w)->window;
    GTimer *t;
    gulong ms;
    int abandon = 0;
    gint x, y, nx, ny;
    data = gtk_object_get_user_data(GTK_OBJECT(w));
    if (data) {
	x = GPOINTER_TO_UINT(data) >> 16;
	y = GPOINTER_TO_UINT(data) & 0xffff;
	t = g_timer_new();
	if (!t)
	    return;
	/* Wait for Window Manager to place window and add borders */
	g_timer_start(t);
	do
	{
	    if (abandon) {
		g_timer_destroy(t);
		return;
	    }
	    (void)g_timer_elapsed(t, &ms);
	    if (ms >= 500)
		abandon++;	/* But only after one final check */
	    gdk_window_get_origin(window, &nx, &ny);
	} while (nx == 0 && ny == 0 || nx == x && ny == y);
	g_timer_destroy(t);
	popup_dialog_bw = nx - x;
	popup_dialog_bh = ny - y;
	/* Do we need to re-position this window to stay on-screen? */
	screen_width = gdk_screen_width();
	screen_height = gdk_screen_height();
	if (nx + popup_dialog_bw + w->allocation.width > screen_width)
	    x = screen_width - popup_dialog_bw - w->allocation.width;
	else x = nx;
	if (ny + popup_dialog_bw + w->allocation.height > screen_height)
	    /* popup_dialog_bw because we assume bottom border same as left */
	    y = screen_height - popup_dialog_bw - w->allocation.height;
	else y = ny;
	if (x != nx || y != ny)
	    gtk_widget_set_uposition(w, x, y);
    }
}

static void
popup_dialog_realizing(GtkWidget *w, gpointer data)
{
    GdkWindow *window = GTK_WIDGET(main_window)->window;
    gint x, y, width, height, ox, oy;
    gint screen_width = gdk_screen_width(), screen_height = gdk_screen_height();

    if (w->allocation.width > screen_width ||
      w->allocation.height > screen_height) {
	/* This is going to look bad anyway, we can't centre it because
	 * a position less than 0 is not valid, so the best we can do
	 * is place the pop-up at the top left of the screen and accept
	 * that the right and/or bottom edge will be off-screen.
	 */
	x = y = 0;
    }
    else {
	gdk_window_get_geometry(window, &x, &y, &width, &height, NULL);
	gdk_window_get_origin(window, &ox, &oy);
	x += ox + (width - w->allocation.width) / 2;
	y += oy + (height - w->allocation.height) / 2;
	if (x + w->allocation.width + popup_dialog_bw > screen_width)
	    x = screen_width - popup_dialog_bw - w->allocation.width;
	if (y + w->allocation.height + popup_dialog_bw > screen_height)
	    y = screen_height - popup_dialog_bw - w->allocation.height;
	/* (x,y) is the desired position of the window,
	 * adjust to take into account the expected border. */
	x -= popup_dialog_bw;
	y -= popup_dialog_bh;
	if (x < 0) x = 0;
	if (y < 0) y = 0;
    }
    gtk_widget_set_uposition(w, x, y);
    gtk_object_set_user_data(GTK_OBJECT(w), GUINT_TO_POINTER(x << 16 | y));
}

void
nh_position_popup_dialog(GtkWidget *w)
{
    if (GTK_WIDGET_REALIZED(w))
	popup_dialog_realizing(w, 0);
    if (GTK_WIDGET_MAPPED(w))
	popup_dialog_mapped(w, 0);
    gtk_signal_connect(GTK_OBJECT(w), "realize",
	GTK_SIGNAL_FUNC(popup_dialog_realizing), 0);
    gtk_signal_connect_after(GTK_OBJECT(w), "map",
	GTK_SIGNAL_FUNC(popup_dialog_mapped), 0);
}

void
quit_hook()
{
    gtk_main_quit();
}

void
main_hook()
{
    nh_map_check_visibility();
    if (display_inventory_needed)
	GTK_display_inventory();
#ifdef RADAR
    if (!in_topten)
	nh_radar_update();
#endif

    gtk_main();
}

static void
game_option(GtkWidget *widget, gpointer data)
{
    nh_option_new();
    keysym = '\0';
}

static void
game_topten(GtkWidget *widget, gpointer data)
{
#ifdef GTK_PROXY
    in_topten++;			/* Prevent radar window appearing */
    proxy_cb_display_score();
#else
    winid id;
    char *argv[] = {
	"nethack",
	"-sall",
    };
    
    in_topten++;			/* Prevent radar window appearing */
    id = rawprint_win = create_toptenwin();
    prscore(2, argv);
    GTK_display_nhwindow(id, TRUE);
    destroy_toptenwin();
    dlb_init();				/* Re-initialise DLB */
    rawprint_win = WIN_ERR;
#endif
    keysym = '\0';
    in_topten--;
}

static void
help_license(GtkWidget *widget, gpointer data)
{
#ifdef GTK_PROXY
    if (!GTK_display_file("$(LICENSE)"))
	pline("Cannot display license.  Sorry.");
#else
#ifndef FILE_AREAS    
    display_file(NH_LICENSE, TRUE);
#else
    display_file(NH_LICENSE_AREA, NH_LICENSE, TRUE);
#endif
#endif
    keysym = '\0';
}

static void
help_history(GtkWidget *widget, gpointer data)
{
    dohistory();
    keysym = '\0';
}


static void
help_option(GtkWidget *widget, gpointer data)
{
#ifdef GTK_PROXY
    if (!GTK_display_file("$(OPTIONFILE)"))
	pline("Cannot display option help.  Sorry.");
#else
#ifndef FILE_AREAS    
    display_file(NH_OPTIONFILE, TRUE);
#else
    display_file(NH_OPTIONAREA, NH_OPTIONFILE, TRUE);
#endif
#endif
    keysym = '\0';
}

static void
help_shelp(GtkWidget *widget, gpointer data)
{
#ifdef GTK_PROXY
    if (!GTK_display_file("$(SHELP)"))
	pline("Cannot display short help.  Sorry.");
#else
#ifndef FILE_AREAS    
    display_file(NH_SHELP, TRUE);
#else
    display_file(NH_SHELP_AREA, NH_SHELP, TRUE);
#endif
#endif
    keysym = '\0';
}

static void
help_help(GtkWidget *widget, gpointer data)
{
#ifdef GTK_PROXY
    if (!GTK_display_file("$(HELP)"))
	pline("Cannot display command help.  Sorry.");
#else
#ifndef FILE_AREAS    
    display_file(NH_HELP, TRUE);
#else
    display_file(NH_HELP_AREA, NH_HELP, TRUE);
#endif
#endif
    keysym = '\0';
}

static void
key_command(GtkWidget *widget, gpointer data)
{
    keysym = (int)data;

    quit_hook();
}

static const int dir_keys[8][2] = {
    	{'k','8'},	/* North */
    	{'l','6'},	/* East */
    	{'j','2'},	/* South */
    	{'h','4'},	/* West */
    	{'u','9'},	/* Northeast */
    	{'y','7'},	/* Northwest */
    	{'n','3'},	/* Southeast */
    	{'b','1'},	/* Southwest */
};

static void
move_command(GtkWidget *widget, gpointer data)
{

    keysym = 'm';
    keysym1 = (int) (copts.num_pad ? 
    				dir_keys[(int)data][1] : 
    				dir_keys[(int)data][0]);

    quit_hook();
}

static void
fight_command(GtkWidget *widget, gpointer data)
{
    keysym = 'F';
    keysym1 = (int) (copts.num_pad ? 
    				dir_keys[(int)data][1] : 
    				dir_keys[(int)data][0]);

    quit_hook();
}

static void
ext_command(GtkWidget *widget, gpointer data)
{
    keysym = '#';
    GTK_extcmd_set((int)data);

    quit_hook();
}

static void
quit()
{
#ifdef GTK_PROXY
    proxy_cb_quit_game();
#else
    if (program_state.something_worth_saving)
	done2();
    else {
	clearlocks();
	GTK_exit_nhwindows(NULL);
	terminate(0);
    }
#endif
}

static gint
main_window_destroy(GtkWidget *widget, gpointer data)
{
    quit();

    return TRUE;
}

static void
game_quit(GtkWidget *widget, gpointer data)
{
    quit();
}

static gint
default_destroy(GtkWidget *widget, gpointer data)
{
    GtkWindow **w = (GtkWindow **)data;
    *w = NULL;
    keysym = '\033';
    
    quit_hook();
    return FALSE;
}

int
nh_dir_keysym(GdkEventKey *ev)
{
    int ret;

    switch(ev->keyval)
    {
	case GDK_End:
	case GDK_KP_End:
	    ret = dir_keys[GTK_SOUTHWEST][!!copts.num_pad];
	    break;
	case GDK_Down:
	case GDK_KP_Down:
	    ret = dir_keys[GTK_SOUTH][!!copts.num_pad];
	    break;
	case GDK_Page_Down:
	case GDK_KP_Page_Down:
	    ret = dir_keys[GTK_SOUTHEAST][!!copts.num_pad];
	    break;
	case GDK_Left:
	case GDK_KP_Left:
	    ret = dir_keys[GTK_WEST][!!copts.num_pad];
	    break;
	case GDK_Begin:
	case GDK_KP_Begin:
	    ret = '.';
	    break;
	case GDK_Right:
	case GDK_KP_Right:
	    ret = dir_keys[GTK_EAST][!!copts.num_pad];
	    break;
	case GDK_Home:
	case GDK_KP_Home:
	    ret = dir_keys[GTK_NORTHWEST][!!copts.num_pad];
	    break;
	case GDK_Up:
	case GDK_KP_Up:
	    ret = dir_keys[GTK_NORTH][!!copts.num_pad];
	    break;
	case GDK_Page_Up:
	case GDK_KP_Page_Up:
	    ret = dir_keys[GTK_NORTHEAST][!!copts.num_pad];
	    break;
	default:
	    ret = 0;
    }

    return ret;
}

/*
 * Our model for focus (where key presses should go) is fairly simple.
 * As each window is opened that should receive key presses
 * we start a new layer of the hierarchy. Windows with slave
 * focus don't accept key presses themselves; instead they forward them
 * to another window. Such windows are considered to be at the same
 * level as the window to which they forward key presses (the master).
 *
 * We do not allow the user to select which window should have focus.
 * If an attempt is made we simply over-ride it. On the other hand,
 * users must be able to give the focus to a seperate application and
 * to return focus to the game when they choose.
 *
 * It is important to give the user as much feedback regarding focus
 * as possible. Ideally, we would like to keep our internal concept of
 * focus and the window manager's tied together but this causes a lot
 * of problems with slave windows (if a slave window receives the focus
 * and we attempt to pass it to the master window Gdk will raise the
 * master window at the same time. We can fudge this afterwards but it
 * looks very bad). As a compromise we allow the focus to remain on
 * slave windows and quietly forward the key presses.
 */

static struct focus_hierarchy {
    GtkWindow *master;
    GSList *slaves;
    gint (*handler)(GtkWidget *widget, GdkEventKey *event, gpointer data);
    gpointer data;
    struct focus_hierarchy *next;
} *focus_top = NULL;
static int focus_game = FALSE;	/* Does any game window have focus? */

#ifdef DEBUG
static gint focus_dump(void)
{
    struct focus_hierarchy *fh;
    GSList *list;
    fprintf(stderr,"Focus hierarchy\n");
    fprintf(stderr,"Master\t\tHandler\t\tSlaves\n");
    for(fh = focus_top; fh; fh = fh->next) {
	fprintf(stderr,"%p\t%p\t",fh->master,fh->handler);
	if (fh->slaves)
	    for(list = fh->slaves; list; ) {
		fprintf(stderr,"%p",list->data);
		list = g_slist_next(list);
		if (list)
		    fputs(", ",stderr);
	    }
	else
	    fputs("<none>",stderr);
	fputc('\n',stderr);
    }
}
#endif

static gint
focus_out(GtkWidget *widget, GdkEventFocus *event, gpointer data)
{
    if (focus_game)
	focus_game--;
}

static gint
focus_in(GtkWidget *widget, GdkEventFocus *event, gpointer data)
{
    GtkWindow *w = GTK_WINDOW(widget);
    focus_game++;
    if (!focus_top || w == focus_top->master ||
      g_slist_find(focus_top->slaves, w))
	return 0;
    gtk_window_present(focus_top->master);
    return 1;
}

static gint
focus_map(GtkWidget *widget, gpointer data)
{
    if (focus_game && focus_top && focus_top->master == GTK_WINDOW(widget))
	gdk_window_focus(widget->window, gtk_get_current_event_time());
    return 0;
}

static gint
focus_destroy(GtkWidget *widget, gpointer data)
{
    GtkWindow *w = GTK_WINDOW(widget);
    GSList *list;
    struct focus_hierarchy *fh, *fhl = NULL;
    for(fh = focus_top; fh; fhl = fh, fh = fh->next) {
	if (fh->master == w) {
	    /* Not sure how this could happen in practice, but re-parenting
	     * any slave windows to the next master in the hierarchy seems
	     * a resonable sort of thing to do. If there is no next then
	     * we're probably exiting the game anyway so just drop them.
	     */
	    if (fh->slaves)
		if (fh->next)
		    fh->next->slaves = g_slist_concat(fh->next->slaves,
		      fh->slaves);
		else
		    g_slist_free(fh->slaves);
	    if (fhl)
		fhl->next = fh->next;
	    else
		focus_top = fh->next;
	    free(fh);
	    break;
	}
	else if (list = g_slist_find(fh->slaves, w)) {
	    fh->slaves = g_slist_remove(fh->slaves, w);
	    break;
	}
    }
    if (!fh)
	impossible("Destroying unknown focus window");
    else
	gtk_widget_unref(widget);
}

gint
focus_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    if (focus_top)
	return focus_top->handler(GTK_WIDGET(focus_top->master), event,
	  focus_top->data);
    else
	return GTK_default_key_press(widget, event, data);
}

static void focus_set_events(GtkWindow *w)
{
    gtk_widget_ref(GTK_WIDGET(w));
    gtk_signal_connect(GTK_OBJECT(w), "focus_in_event",
      GTK_SIGNAL_FUNC(focus_in), 0);
    gtk_signal_connect(GTK_OBJECT(w), "focus_out_event",
      GTK_SIGNAL_FUNC(focus_out), 0);
    gtk_signal_connect(GTK_OBJECT(w), "map_event",
      GTK_SIGNAL_FUNC(focus_map), 0);
    gtk_signal_connect(GTK_OBJECT(w), "destroy",
      GTK_SIGNAL_FUNC(focus_destroy), 0);
    gtk_signal_connect(GTK_OBJECT(w), "key_press_event",
      GTK_SIGNAL_FUNC(focus_key_press), 0);
}

void nh_gtk_focus_set_master(GtkWindow *w, GtkSignalFunc func, gpointer data)
{
    struct focus_hierarchy *fh;
    g_return_if_fail(w != NULL);
    fh = (struct focus_hierarchy *)alloc(sizeof(struct focus_hierarchy));
    fh->master = w;
    fh->slaves = NULL;
    fh->handler = (void *)func;
    fh->data = data;
    fh->next = focus_top;
    focus_top = fh;
    focus_set_events(w);
#ifdef DEBUG
    focus_dump();
#endif
}

void nh_gtk_focus_set_slave_for(GtkWindow *w,GtkWindow *slave_for)
{
    struct focus_hierarchy *fh;
    g_return_if_fail(w != NULL);
    for(fh = focus_top; fh; fh = fh->next) {
	if (slave_for == fh->master) {
	    fh->slaves = g_slist_prepend(fh->slaves, w);
	    break;
	}
    }
    if (!fh) {
	impossible("Window focus slave for unknown window?");
	return;
    }
    focus_set_events(w);
#ifdef DEBUG
    focus_dump();
#endif
}

gint
GTK_default_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    keysym = nh_dir_keysym(event);
    if (!keysym)
	switch(event->keyval)
	{
	    case GDK_Insert:
	    case GDK_KP_Insert:
		keysym = 'i';
		break;
	    default:
		keysym = nh_keysym(event);
	}

    if(keysym)
	quit_hook();
    
    return FALSE;
}

static gint
default_button_press(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    if(data)
	keysym = (int)data;
    else
	keysym = '\n';

    quit_hook();

    return FALSE;
}

static gint
credit_expose_event(GtkWidget *widget, GdkEventExpose *event)
{
    gtk_main_quit();

    return FALSE;
}

static void
nh_rc(void)
{
    int i;
    char *rc_file;
    gchar **files, **new_files;
#ifndef FILE_AREAS
    rc_file = strdup("gtkrc");
#else
    rc_file = make_file_name(FILE_AREA_SHARE, "gtkrc");
#endif
    files = gtk_rc_get_default_files();
    for(i = 0; files[i]; i++)
	;
    new_files = g_new(gchar *, i + 2);
    if (!new_files)
	return;
    new_files[0] = (gchar *)rc_file;
    for(i = 0; files[i]; i++)
    {
	new_files[i + 1] = g_strdup(files[i]);
	if (!new_files[i + 1]) {
	    for(; i > 0; i--)
		g_free(new_files[i]);
	    g_free(new_files);
	    return;
	}
    }
    new_files[i + 1] = NULL;
    gtk_rc_set_default_files(new_files);
    for(i = 1; new_files[i]; i++)
	g_free(new_files[i]);
    g_free(new_files);
    free(rc_file);
}

/*
 * ALI
 *
 * Player selection code for 3.3 codebase.
 *
 * This code tries to keep to the spirit of the GTK interface by having
 * all possible selections available as menus for the user (rather than
 * bringing up a dialog). It also keeps the keyboard accelerators (one
 * per role), which are numbered from F1..F12, Shift-F1..Shift-F11 in
 * sequence. Shift-F12 is reserved for a random role.
 *
 * The interaction of the selection code and the flags.initfoo fields
 * (set for example with an OPTIONS=race:gnome line) is complex. These
 * values could either be treated as requirements (so that specifying
 * race:gnome would rule out any possibility of playing a Knight, for
 * example), or as cached answers to questions (so that the answer is
 * ignored until the question is asked and since role is queried before
 * race, the problem does not arise). The TTY interface follows the
 * latter interpretation, so we do too.
 *
 * For our purposes, displaying a menu is equivalent to asking a question
 * and the user choosing one the options (either by actually clicking
 * on an option or simply by moving the mouse) equates to giving an
 * answer. The joy of the GTK interface is that the user can un-answer
 * many questions by simply moving the mouse pointer back to the previous
 * menu.
 */

struct {
    int role;
    int race;
    int gend;
    int align;
} select_player_flags;		/* Used to pass data to/from select_player */

#define SELECT_KEY_ROLESHIFT	    20
#define SELECT_KEY_RACESHIFT	    8
#define SELECT_KEY_GENDSHIFT	    4
#define SELECT_KEY_ALIGNSHIFT	    0

#define SELECT_KEY_NUM(key, shift, mask) \
			((int)((key)>>(shift)&(mask)) - 1)

#define SELECT_KEY_ROLENUM(key)	    \
			SELECT_KEY_NUM(key, SELECT_KEY_ROLESHIFT, 0xfff)
#define SELECT_KEY_RACENUM(key)	    \
			SELECT_KEY_NUM(key, SELECT_KEY_RACESHIFT, 0xfff)
#define SELECT_KEY_GENDNUM(key)	    \
			SELECT_KEY_NUM(key, SELECT_KEY_GENDSHIFT, 0xf)
#define SELECT_KEY_ALIGNNUM(key)    \
			SELECT_KEY_NUM(key, SELECT_KEY_ALIGNSHIFT, 0xf)

/*
 * The call back routine. This extracts the relevant answers from the
 * menu option selected and arranges for a new game to be started.
 */

static void
select_player(GtkWidget *widget, guint data)
{
    select_player_flags.role = SELECT_KEY_ROLENUM(data);
    select_player_flags.race = SELECT_KEY_RACENUM(data);
    select_player_flags.gend = SELECT_KEY_GENDNUM(data);
    select_player_flags.align = SELECT_KEY_ALIGNNUM(data);
    
    quit_hook();
}

/*
 * A node of the temporary tree which is used to generate the menu options.
 *
 * The key field contains the answers which the user would have given in
 * order to reach this point. Note that you need to know the level that the
 * node is in the tree to be able to distinguish an answer of 'random' from
 * `not yet asked'. This information is not stored in the tree itself.
 * Each son represents a possible answer to the next question. Again, you
 * need to know the level to know what the question is.
 *
 * Level	0	1	2		3
 * Question	Role?	Race?	Gender?		Alignment?
 *
 * Tree		root--->Arch--->human------+--->male---------+->lawful
 *                                         |                 \->neutral
 *                                         |                 \->random
 *                                         \--->female-------+->lawful
 *                                                           \...
 */

struct select_node {
    unsigned long key;
    int no_sons;
    struct select_node *sons;
};

#ifdef GTK_PROXY
#define valid_race(role, race)	proxy_cb_is_valid_selection(role, race, -1, -1)

static boolean
valid_gend(int role, int race, int gend)
{
    return proxy_cb_is_valid_selection(role, race, gend, -1);
}

static boolean
valid_align(int role, int race, int align)
{
    return proxy_cb_is_valid_selection(role, race, -1, align);
}
#else	/* GTK_PROXY */
#define valid_race	validrace
#define valid_gend	validgend
#define valid_align	validalign
#endif	/* GTK_PROXY */

/*
 * Return the <indx>th possible answer to the question at level <level>
 * given the answers already given in <key>. Note that indx==0 represents
 * the first possible answer.
 *
 * If indx is out of range, return _exactly_ -1.
 *
 * We use a cached answer if it is a valid answer. If not, we simply ignore it.
 *
 * Note that this function never lists random as a possible answer, the
 * caller must add this to the list if appropriate.
 */

static int
select_node_option(unsigned long key, int level, int indx)
{
    int rolenum, racenum, n, i, j;
    boolean (*valid)(int rolenum, int racenum, int gendalignnum);
    switch(level)
    {
	case 0:
	    /* Role */
	    if (select_player_flags.role >= 0)
		return indx ? -1 : select_player_flags.role;
	    for (i = 0; i < player_choices->n_roles; i++)
		if (!indx--)
		    return i;
	    return -1;
	    break;
	case 1:
	    /* Race */
	    rolenum = SELECT_KEY_ROLENUM(key);
	    if (select_player_flags.race >= 0 &&
	      (rolenum < 0 || valid_race(rolenum, select_player_flags.race)))
		return indx ? -1 : select_player_flags.race;
	    for (i = 0; i < player_choices->n_races; i++)
		if (rolenum < 0 || valid_race(rolenum, i))
		    if (!indx--)
			return i;
	    return -1;
	    break;
	case 3:
	    /* Alignmnent */
	    /* FALL THROUGH */
	case 2:
	    /* Gender */
	    if (level == 2)
	    {
		n = player_choices->n_genders;
		valid = valid_gend;
		i = select_player_flags.gend;
	    }
	    else
	    {
		n = player_choices->n_aligns;
		valid = valid_align;
		i = select_player_flags.align;
	    }
	    rolenum = SELECT_KEY_ROLENUM(key);
	    racenum = SELECT_KEY_RACENUM(key);
	    if (i >= 0)
	    {
		if (rolenum < 0)
		    if (racenum < 0)
			return indx ? -1 : i;
		    else
		    {
			for (j = 0; j < player_choices->n_roles; j++)
			    if (valid(j, racenum, i))
				return indx ? -1 : i;
		    }
		else if (racenum < 0)
		{
		    for (j = 0; j < player_choices->n_races; j++)
			if (valid(rolenum, j, i))
			    return indx ? -1 : i;
		}
		else if (valid(rolenum, racenum, i))
		    return indx ? -1 : i;
	    }
	    if (rolenum < 0)
		if (racenum < 0)
		{
		    if (indx >= 0 && indx < n)
			return indx;
		}
		else
		{
		    for (i = 0; i < n; i++)
			for (j = 0; j < player_choices->n_roles; j++)
			    if (valid(j, racenum, i))
			    {
				if (!indx--)
				    return i;
				break;
			    }
		}
	    else if (racenum < 0)
	    {
		for (i = 0; i < n; i++)
		    for (j = 0; j < player_choices->n_races; j++)
			if (valid(rolenum, j, i))
			{
			    if (!indx--)
				return i;
			    break;
			}
	    }
	    else
		for (i = 0; i < n; i++)
		    if (valid(rolenum, racenum, i))
			if (!indx--)
			    return i;
	    return -1;
	    break;
    }
}

/*
 * Fill a tree starting at <node> which is in level <level>.
 * Return the number of menu items (including titles and seperators)
 * that this tree represents.
 */

static int
select_node_fill(struct select_node *node, int level)
{
    int shift, no_opts, count, option, i;
    for (no_opts = 0; ; no_opts++)
	if (select_node_option(node->key, level, no_opts) < 0)
	    break;
    if (level == 0)
	shift = SELECT_KEY_ROLESHIFT;
    else if (level == 1)
	shift = SELECT_KEY_RACESHIFT;
    else if (level == 2)
	shift = SELECT_KEY_GENDSHIFT;
    else
	shift = SELECT_KEY_ALIGNSHIFT;
    if (no_opts > 1)
    {
	node->no_sons = no_opts + 1;
	count = no_opts + 2;
    }
    else
    {
	node->no_sons = no_opts;
	count = no_opts;
    }
    if (!level)
	count++;
    node->sons = (struct select_node *)alloc(node->no_sons *
      sizeof(struct select_node));
    for (i = 0; i < node->no_sons; i++)
    {
	option = select_node_option(node->key, level, i);
	node->sons[i].key = node->key | (option + 1 << shift);
	if (level<3)
	    count += select_node_fill(node->sons + i, level + 1);
	else
	{
	    node->sons[i].no_sons = 0;
	    node->sons[i].sons = NULL;
	}
    }
    return count;
}

/*
 * Return a menu item path representing the answers given to <level>
 * questions stored in <key>. If <leaf> is not NULL, append this as
 * a leaf to the path.
 */

static char *
select_node_path(unsigned long key, int level, char *leaf)
{
    int rolenum, racenum, gendnum, alignnum, len, i;
    char *path;

    rolenum = SELECT_KEY_ROLENUM(key);
    racenum = SELECT_KEY_RACENUM(key);
    gendnum = SELECT_KEY_GENDNUM(key);
    alignnum = SELECT_KEY_ALIGNNUM(key);
    len = 11 + level;
    if (level > 0)
	if (rolenum >= 0)
	    len += strlen(player_choices->roles[rolenum].male);
	else
	    len += 6;
    if (level > 1)
	if (racenum >= 0)
	    len += strlen(player_choices->races[racenum]);
	else
	    len += 6;
    if (level > 2)
	if (gendnum >= 0)
	    len += strlen(player_choices->genders[gendnum]);
	else
	    len += 6;
    if (level > 3)
	if (alignnum >= 0)
	    len += strlen(player_choices->aligns[alignnum]);
	else
	    len += 6;
    if (leaf)
	len += 1 + strlen(leaf);
    path = (gchar *) alloc(len);
    strcpy(path, "/Game/Play");
#define SELECT_STR(num, str) (((num) >= 0) ? (str) : "Random")
    if (level > 0)
    {
	strcat(path, "/");
	strcat(path, SELECT_STR(rolenum, player_choices->roles[rolenum].male));
    }
    if (level > 1)
    {
	strcat(path, "/");
	strcat(path, SELECT_STR(racenum, player_choices->races[racenum]));
    }
    if (level > 2)
    {
	strcat(path, "/");
	strcat(path, SELECT_STR(gendnum, player_choices->genders[gendnum]));
    }
    if (level > 3)
    {
	strcat(path, "/");
	strcat(path, SELECT_STR(alignnum, player_choices->aligns[alignnum]));
    }
    if (leaf)
    {
	strcat(path, "/");
	strcat(path, leaf);
    }
#undef SELECT_STR
    return path;
}

/*
 * Return a suitable keyboard accelrator to go with this menu item.
 * For most menu items this will be NULL, but we allocate one menu
 * item from each role to a function key. This allows the user to
 * press a function key to start the game in that role. The race,
 * gender and alignment of the character will either be as required
 * by the role, as specified in the select_player_flags.foo fields
 * or randomly chosen.
 */

static char *
select_node_accel(unsigned long key)
{
    int rolenum;
    char *accel;

    rolenum = SELECT_KEY_ROLENUM(key);
    if (rolenum >= 23)
	return NULL;
    if (SELECT_KEY_RACENUM(key) >= 0 && select_node_option(key, 1, 1) >= 0)
	return NULL;
    if (SELECT_KEY_GENDNUM(key) >= 0 && select_node_option(key, 2, 1) >= 0)
	return NULL;
    if (SELECT_KEY_ALIGNNUM(key) >= 0 && select_node_option(key, 3, 1) >= 0)
	return NULL;
    if (rolenum < 0)
    {
	accel = (gchar *) alloc(11);
	sprintf(accel, "<shift>F12");
    }
    else if (rolenum < 12)
    {
	accel = (gchar *) alloc(rolenum > 8 ? 4 : 3);
	sprintf(accel, "F%d", rolenum + 1);
    }
    else
    {
	accel = (gchar *) alloc(rolenum > 20 ? 11 : 10);
	sprintf(accel, "<shift>F%d", rolenum - 11);
    }
    return accel;
}

/*
 * Walk the tree, generating menu items as we go.
 */

static int
select_node_traverse(struct select_node *node, int offset, int level)
{
    int i;
    char *titles[] = { "Role", "Race", "Gender", "Alignment" };
    if (node->no_sons)
    {
	menu_items[offset].path =
	  select_node_path(node->key, level, titles[level]);
	menu_items[offset].accelerator = NULL;
	menu_items[offset].callback = NULL;
	menu_items[offset].callback_action = 0;
	menu_items[offset++].item_type = "<Title>";
	for (i = 0; i < node->no_sons; i++)
	{
	    if (node->sons[i].key == node->key)
	    {
		menu_items[offset].path =
		  select_node_path(node->key, level, "GPSepR");
		menu_items[offset].accelerator = NULL;
		menu_items[offset].callback = NULL;
		menu_items[offset].callback_action = 0;
		menu_items[offset++].item_type = "<Separator>";
	    }
	    offset = select_node_traverse(node->sons + i, offset, level + 1);
	}
    }
    else
    {
	menu_items[offset].path = select_node_path(node->key, level, NULL);
	menu_items[offset].accelerator = select_node_accel(node->key);
	menu_items[offset].callback = select_player;
	menu_items[offset].callback_action = node->key;
	menu_items[offset++].item_type = NULL;
    }
    return offset;
}

static void
select_node_free(struct select_node *node)
{
    int i;
    for (i = 0; i < node->no_sons; i++)
	select_node_free(node->sons + i);
    free(node->sons);
}

#ifdef DEBUG
static void
select_node_dump(struct select_node *node, int level)
{
    int i;
    static int count;
    char buf[BUFSZ];
    char *titles[] = { "Role", "Race", "Gender", "Alignment" };

    if (level) {
	for(i = 0; i < level; i++)
	    fputs("    ", stderr);
	sprintf(buf, "[%d] 0x%lX: ", count++, node->key);
	switch (level)
	{
	case 1:
	    if (SELECT_KEY_ROLENUM(node->key) >= 0)
		fprintf(stderr, "%s%s", buf,
		  player_choices->roles[SELECT_KEY_ROLENUM(node->key)].m);
	    else {
		fprintf(stderr, "[%d] ---\n", count - 1);
		for(i = 0; i < level; i++)
		    fputs("    ", stderr);
		fprintf(stderr, "[%d] 0x%lX: random", count++, node->key);
	    }
	    break;
	case 2:
	    if (SELECT_KEY_RACENUM(node->key) >= 0)
		fprintf(stderr, "%s%s", buf,
		  player_choices->races[SELECT_KEY_RACENUM(node->key)]);
	    else {
		fprintf(stderr, "[%d] ---\n", count - 1);
		for(i = 0; i < level; i++)
		    fputs("    ", stderr);
		fprintf(stderr, "[%d] 0x%lX: random", count++, node->key);
	    }
	    break;
	case 3:
	    if (SELECT_KEY_GENDNUM(node->key) >= 0)
		fprintf(stderr, "%s%s", buf,
		  player_choices->genders[SELECT_KEY_GENDNUM(node->key)]);
	    else {
		fprintf(stderr, "[%d] ---\n", count - 1);
		for(i = 0; i < level; i++)
		    fputs("    ", stderr);
		fprintf(stderr, "[%d] 0x%lX: random", count++, node->key);
	    }
	    break;
	case 4:
	    if (SELECT_KEY_ALIGNNUM(node->key) >= 0)
		fprintf(stderr, "%s%s", buf,
		  player_choices->aligns[SELECT_KEY_ALIGNNUM(node->key)]);
	    else {
		fprintf(stderr, "[%d] ---\n", count - 1);
		for(i = 0; i < level; i++)
		    fputs("    ", stderr);
		fprintf(stderr, "[%d] 0x%lX: random", count++, node->key);
	    }
	    break;
	}
	fputc('\n', stderr);
    }
    else
	count = 0;
    if (node->no_sons) {
	for(i = 0; i < level; i++)
	    fputs("    ", stderr);
	if (level)
	    fprintf(stderr, "%s\n", titles[level]);
	else
	    fprintf(stderr, "[%d] %s\n", count++, titles[0]);
	for(i = 0; i < node->no_sons; i++)
	    select_node_dump(node->sons + i, level + 1);
    }
}
#endif

/*
 * Initialise the player selection code by creating a temporary tree
 * of all the possible options, using it to generate a set of menu
 * items that can be passed to GTK and finally freeing the tree.
 */

static void
init_select_player(boolean init)
{
    int num_opts, i;
    struct select_node *root;

    if (!init) {	/* Exit */
	for (i = 0; i < nmenu_items; i++) {
	    free(menu_items[i].path);
	    free(menu_items[i].accelerator);
	}
	nmenu_items = 0;
	free(menu_items);
	menu_items = NULL;
	proxy_cb_free_player_choices(player_choices);
	player_choices = NULL;
	return;
    }
    player_choices = proxy_cb_get_player_choices();
    root = (struct select_node *)alloc(sizeof(struct select_node));
    root->key = 0;
    num_opts = select_node_fill(root, 0);
#ifdef DEBUG
    select_node_dump(root, 0);
#endif
    menu_items = (GtkItemFactoryEntry *)alloc(sizeof(GtkItemFactoryEntry) *
      num_opts);
    nmenu_items = num_opts;
    nmenu_items = select_node_traverse(root, 0, 0);
    if (nmenu_items != num_opts)
	panic("GTK: init_select_player: Expecting %d options, got %d",
	  num_opts, nmenu_items);
    select_node_free(root);
    free(root);
}

int
GTK_ext_init_nhwindows(int *argc, char **argv)
{
    char *credit_file;
    int i;
    GtkWidget 	*credit_window, *credit_vbox, *credit_credit, *main_hbox;
    GtkWidget	*main_bar;
    GtkStyle	*credit_style;
    GdkPixmap	*credit_pixmap;
    GdkBitmap	*credit_mask;

#ifdef UNIX
    uid_t savuid;
#endif

    gtk_set_locale();
#ifdef GTK_V20
    nh_option_cache_set_bool_addr("color", &copts.use_color);
    nh_option_cache_set_bool_addr("hilite_pet", &copts.hilite_pet);
    nh_option_cache_set_bool_addr("perm_invent", &copts.perm_invent);
#endif
    nh_rc();

    /* Init windows to nothing. */
    for (i = 0; i < MAXWIN; i++)
	gtkWindows[i].type = NHW_NONE;

    /*
     * setuid hack: make sure that if nethack is setuid, to use real uid
     * when opening X11 connections, in case the user is using xauth, since
     * the "games" or whatever user probably doesn't have permission to open
     * a window on the user's display.  This code is harmless if the binary
     * is not installed setuid.  See include/system.h on compilation failures.
     */
#ifdef UNIX
    savuid = geteuid();
    (void) seteuid(getuid());
    hide_privileges(TRUE);
#endif
    
    gtk_init(argc, &argv);

#ifdef UNIX
    hide_privileges(FALSE);
    (void) seteuid(savuid);
#endif
    
/*
  creat credit widget and show
*/
    credit_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(credit_window), GTK_WIN_POS_CENTER);

    gtk_container_border_width(GTK_CONTAINER(credit_window), 2);
/*
  gtk_signal_connect(GTK_OBJECT(credit_window), "check_resize",
  GTK_SIGNAL_FUNC(credit_map_event), NULL);
*/

    gtk_signal_connect(GTK_OBJECT(credit_window), "expose_event",
		       GTK_SIGNAL_FUNC(credit_expose_event), NULL);

    gtk_widget_realize(credit_window);
    root_width = gdk_screen_width();
    root_height = gdk_screen_height();

    credit_vbox = nh_gtk_new_and_add(
	gtk_vbox_new(FALSE, 0), credit_window, "");
     
    credit_style = gtk_widget_get_style(credit_window);
#ifndef FILE_AREAS
    credit_file = "credit.xpm";
#else
    credit_file = make_file_name(FILE_AREA_SHARE, "credit.xpm");
#endif
    credit_pixmap = gdk_pixmap_create_from_xpm(
	credit_window->window,
	&credit_mask,
	&credit_style->bg[GTK_STATE_NORMAL],
	credit_file);
#ifdef FILE_AREAS
    free(credit_file);
#endif
    if (credit_pixmap) {
	credit_credit = nh_gtk_new_and_pack(
	    gtk_pixmap_new(credit_pixmap, credit_mask), credit_vbox, "",
	    FALSE, FALSE, NH_PAD);
	gdk_pixmap_unref(credit_pixmap);
    }
    gtk_widget_show_all(credit_window);

    gtk_main();

/*
  create main widget
*/
    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    nh_gtk_focus_set_master(GTK_WINDOW(main_window),
      GTK_SIGNAL_FUNC(GTK_default_key_press), 0);

    gtk_window_set_policy(GTK_WINDOW(main_window), TRUE, TRUE, TRUE);

    gtk_signal_connect(
	GTK_OBJECT(main_window), "delete_event",
	GTK_SIGNAL_FUNC(main_window_destroy), 0);

    gtk_window_set_title(GTK_WINDOW(main_window), DEF_GAME_NAME);
    gtk_widget_set_name(main_window, DEF_GAME_NAME);
    
    gtk_widget_set_events(main_window, GDK_KEY_PRESS_MASK);
    gtk_widget_realize(main_window);

/*
  allocate color
 */
    {
	int 		i;
	GdkColormap	*cmap;

	cmap = gdk_window_get_colormap(main_window->window);
  
	for(i=0 ; i < N_NH_COLORS ; ++i){
	    if(0 && nh_color[i].pixel){
		nh_color[i] = *(main_window->style->fg);
	    }
	    else if(gdk_colormap_alloc_color(cmap, &nh_color[i], FALSE, TRUE) == TRUE){
		;
	    }
	    else{
		fprintf(stderr, "cannot allocate color\n");
	    }
	}
    }

    main_vbox = nh_gtk_new_and_add(gtk_vbox_new(FALSE, 0), main_window, "");

    accel_group = gtk_accel_group_new();
#if GTK_CHECK_VERSION(1,3,12)
    gtk_window_add_accel_group(GTK_WINDOW(main_window), accel_group);
#else
    gtk_accel_group_attach(accel_group, G_OBJECT(main_window));
#endif

    main_item_factory = gtk_item_factory_new(
	GTK_TYPE_MENU_BAR, "<main>",
	accel_group);

    main_bar = nh_gtk_new_and_pack(
	gtk_item_factory_get_widget(main_item_factory, "<main>"), main_vbox, "",
	FALSE, FALSE, 0);

    main_hbox = nh_gtk_new_and_pack(
	gtk_hbox_new(FALSE, 1), main_vbox, "",
	FALSE, FALSE, 0);
	
    gtk_item_factory_create_items(
	main_item_factory,
	SIZE(mainmenu_items), mainmenu_items,
	NULL);

    gtk_item_factory_create_items(
	main_item_factory,
	SIZE(playmenu_items), playmenu_items,
	NULL);

    gtk_item_factory_create_items(
	main_item_factory,
	SIZE(helpmenu_items), helpmenu_items,
	NULL);

    nh_menu_sensitive("/Game/Play", FALSE);
    nh_option_lock(TRUE);

    (void) nh_gtk_new_and_pack(nh_message_new(), main_hbox, "",
	TRUE, TRUE, 0);
    (void) nh_gtk_new_and_pack(nh_status_new(), main_hbox, "",
	FALSE, FALSE, 0);

#ifdef RADAR
    main_radar = nh_radar_new();
    nh_gtk_focus_set_slave_for(GTK_WINDOW(main_radar), GTK_WINDOW(main_window));
#endif
    
    (void) nh_gtk_new_and_pack(
	nh_map_new(main_window), main_vbox, "",
	TRUE, TRUE, 0);

    /*
     * Initialize standard windows. It used to be the case that window type
     * and id were equivalent (and therefore there could only be one type
     * of each window). We maintain this correlation for the standard
     * windows for compatibility -ALI
     */
    gtkWindows[NHW_MESSAGE].type = NHW_MESSAGE;
    gtkWindows[NHW_MESSAGE].w = main_window;	
    gtkWindows[NHW_STATUS].type = NHW_STATUS;
    gtkWindows[NHW_STATUS].w = main_window;	
    gtkWindows[NHW_MAP].type = NHW_MAP;
    gtkWindows[NHW_MAP].w = main_window;	

    initialized = 1;

    gtk_widget_hide(credit_window);
    gtk_widget_destroy(credit_window);
    gtk_widget_show_all(main_window);
    
    return TRUE;
}

#ifdef MONITOR_XRESOURCES
static int GTK_X11_error_handler(Display *dpy, XErrorEvent *e)
{
    return 0;
}
#endif

void
GTK_exit_nhwindows(const char *str)
{
    int id;

    if(str && *str){
	id = GTK_create_nhwindow(NHW_MENU);
	GTK_putstr(id, 0, str);
	GTK_display_nhwindow(id, TRUE);
	GTK_destroy_nhwindow(id);
    }

    init_select_player(FALSE);

    for(id = 0; id < MAXWIN; id++)
	if (gtkWindows[id].type != NHW_NONE)
	    GTK_destroy_nhwindow(id);

    nh_map_destroy();
    nh_status_destroy();

#ifdef RADAR
    nh_radar_destroy();
#endif
    gtk_widget_destroy(main_window);
    g_blow_chunks();

#ifdef MONITOR_XRESOURCES
    /* ALI: An exceedingly unportable way of checking that
     * there are no X drawables still allocated at this point.
     */
    {
	struct _XDisplay *dpy = (struct _XDisplay *)GDK_DISPLAY();
	XID i;
	Drawable d;
	Window root;
	int x, y;
	unsigned int w, h, bw, dp;
	XWindowAttributes wa;
	char *type;
	fprintf(stderr, "X Drawables not freed:\n");
	fprintf(stderr, "XID       Type    Width   Height\n");
	XSetErrorHandler(GTK_X11_error_handler);
	for(i = 0; i < dpy->resource_id; i++) {
	    d = (Drawable)(dpy->resource_base + (i << dpy->resource_shift));
	    if (XGetGeometry(dpy, d, &root, &x, &y, &w, &h, &bw, &dp)) {
		if (x || y || bw)
		    type = "Window";
		else if (XGetWindowAttributes(dpy, (Window)d, &wa))
		    type = wa.screen ? "Window" : "Pixmap";
		else
		    type = "Pixmap";
		fprintf(stderr, "%-10lX%-8s%-8u%u\n",
		  (unsigned long)d, type, w, h);
	    }
	}
    }
#endif
}

winid
GTK_create_nhwindow(int type)
{
    int retval=-1;
    winid	id;
    NHWindow	*w;

    switch(type) {
/* 
   these windows have already been created
*/
    case NHW_MESSAGE:
    case NHW_STATUS:
    case NHW_MAP:
	if (gtkWindows[type].type != type)
	    panic("GTK_create_nhwindow: standard window (%d) not valid",type);
	retval = type;
	break;
/*
  create new window
*/
    case NHW_MENU:
    case NHW_TEXT:
	for (id = 0; id < MAXWIN; id++) {
	    w = &gtkWindows[id];
	    if (w->type == NHW_NONE)
	    {
		memset(w, 0, sizeof(NHWindow));
		w->type = type;
		if (type == NHW_MENU)
		    GTK_create_menu_window(w);
		retval = id;
		break;
	    }
	}
	if (retval < 0)
	    panic("GTK_create_nhwindow: no free windows!");
	break;
    default:
	panic("GTK_create_nhwindow: Unknown type (%d)!", type);
    }
    return retval;
}

void
GTK_destroy_nhwindow(winid id)
{
    NHWindow *w = &gtkWindows[id]; 

    switch (w->type)
    {
	case NHW_STATUS:
	case NHW_MESSAGE:
	case NHW_MAP:
	    return;
	case NHW_MENU:
	    GTK_destroy_menu_window(w);
	    break;
	case NHW_TEXT:
	    if (w->w) {
		gtk_widget_hide_all(w->w);
		if(w->hid > 0)
		    gtk_signal_disconnect(GTK_OBJECT(w->w), w->hid);
		gtk_widget_destroy(w->w);
	    }
	    break;
    }

    memset(w, 0, sizeof(NHWindow));
    w->type = NHW_NONE;
}

/*
 * Used for close button on blocking text windows.
 */

static gint
blocking_text_clicked(GtkWidget *widget, gpointer data)
{
    keysym = '\033';
    quit_hook();
    return FALSE;
}

/*
 * Used for close button on non-blocking text windows. Don't affect the
 * game state (eg., by setting keysym) and don't call quit_hook. Instead
 * all we do is close the window.
 */

static gint
text_clicked(GtkWidget *widget, gpointer data)
{
    GtkWidget *window;
    window = GTK_WIDGET(data);
    gtk_widget_destroy(window);
    return FALSE;
}

void
GTK_display_nhwindow(winid id, BOOLEAN_P blocking)
{
    NHWindow *w = &gtkWindows[id];
    int type = w->type;
    extern int root_height;

    switch(type) {
	case NHW_STATUS:
	case NHW_MESSAGE:
	    return;	/* We never block on status or message windows */
	case NHW_MAP:	/* flush out */
	    nh_map_flush();
	    break;
	case NHW_TEXT:
	    /*
	     * [ALI] This probably needs some tidying up. Menus will
	     * already have the relevant signals connected. Text windows
	     * are postponed until now. This allows us to treat blocking
	     * and non-blocking windows differently.
	     */
	    if (blocking) {
		w->hid = gtk_signal_connect(GTK_OBJECT(w->w), "destroy",
		  GTK_SIGNAL_FUNC(default_destroy), &w->w);
		gtk_signal_connect(GTK_OBJECT(w->button[0]), "clicked",
		  GTK_SIGNAL_FUNC(blocking_text_clicked), (gpointer)w->w);
	    } else
		w->hid = gtk_signal_connect(GTK_OBJECT(w->button[0]),
		  "clicked", GTK_SIGNAL_FUNC(text_clicked), (gpointer)w->w);
	    /* Fall through */
	case NHW_MENU:
	    if(w->clist &&
	      w->clist->requisition.height >= (2 * root_height) / 3)
		gtk_widget_set_usize(w->clist, -1, (2 * root_height) / 3);
	    if (type == NHW_MENU)
		blocking = TRUE;	/* Menus always block */
#if 0
	    if (blocking)
		gtk_grab_add(w->w);
#endif
	    w->flags |= NHWF_DISPLAYED;
	    gtk_widget_show_all(w->w);
	    if (!blocking)
		w->w = NULL;		/* Gtk window now independant */
	    break;
    }

    if (blocking)
	main_hook();
}

void
GTK_clear_nhwindow(winid id)
{
    if (gtkWindows[id].type == NHW_MAP) {
	nh_map_clear();
    }
}

/*
  ATR_ULINE
  ATR_BOLD
  ATR_BLINK
  ATR_INVERSE
 */
void
GTK_putstr(winid id, int attr, const char *str)
{
    const gchar	*text[1];
    NHWindow *w = &gtkWindows[id]; 

    switch (w->type) {
	case NHW_MESSAGE:
	    nh_message_putstr(str);
	    break;
	case NHW_STATUS:
	case NHW_MAP:
	    panic("bad window");
	    break;
	case NHW_MENU:
	    /* We don't treat windows of type NHW_MENU any differently from
	     * NHW_TEXT windows once putstr() has been called and it makes
	     * it easier if all text windows have type NHW_TEXT.
	     */
	    GTK_destroy_menu_window(w);
	    w->type = NHW_TEXT;
	    /* Fall through */
	case NHW_TEXT:
	    if (!w->w) {
		w->w = nh_gtk_window_dialog(TRUE);
		gtk_widget_set_name(GTK_WIDGET(w->w), "fixed font");
		nh_position_popup_dialog(GTK_WIDGET(w->w));

		nh_gtk_focus_set_slave_for(GTK_WINDOW(w->w),
		  GTK_WINDOW(main_window));

		w->frame = nh_gtk_new_and_add(
		  gtk_frame_new(NULL), w->w, "");

		w->vbox = nh_gtk_new_and_add(
		  gtk_vbox_new(FALSE, 0), w->frame, "");

		w->hbox2 = nh_gtk_new_and_pack(gtk_hbox_new(FALSE, 0), w->vbox,
		  "", FALSE, FALSE, NH_PAD);

		w->clist = nh_gtk_new_and_pack(gtk_clist_new(1), w->hbox2, "",
		  FALSE, FALSE, NH_PAD);
		gtk_clist_set_column_auto_resize(GTK_CLIST(w->clist), 0, TRUE);

		w->adj = (GtkAdjustment *)gtk_adjustment_new(
		  0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
		gtk_clist_set_vadjustment(GTK_CLIST(w->clist), w->adj);

		w->scrolled = nh_gtk_new_and_pack(
		  gtk_vscrollbar_new(GTK_CLIST(w->clist)->vadjustment),
		  w->hbox2, "", FALSE, FALSE, NH_PAD);

		w->hbox3 = nh_gtk_new_and_pack(gtk_hbox_new(FALSE, 0), w->vbox,
		  "", FALSE, FALSE, NH_PAD);

		w->button[0] = nh_gtk_new_and_pack(
		  gtk_button_new_with_label("Close"), w->hbox3, "",
		  TRUE, FALSE, 0);
	    }

	    text[0] = str;
	    gtk_clist_append(GTK_CLIST(w->clist), (gchar **)text);

	    if (attr) {
		gtk_clist_set_foreground(GTK_CLIST(w->clist), 
		  GTK_CLIST(w->clist)->rows - 1,
		  GTK_WIDGET(w->clist)->style->bg); 
		gtk_clist_set_background(GTK_CLIST(w->clist), 
		  GTK_CLIST(w->clist)->rows - 1,
		  GTK_WIDGET(w->clist)->style->fg); 
	    }
    }
}

void
GTK_get_nh_event()
{
    return;
}

int
GTK_nhgetch(void)
{
    int key;
#ifdef RADAR
    nh_radar_update();
#endif

    keysym = keysym1;
    keysym1 = 0;
    while(!keysym)
	main_hook();

    key = keysym;
    keysym = 0;

    return key;
}

int
GTK_nh_poskey(int *x, int *y, int *mod)
{
    int key;

#ifdef RADAR
    nh_radar_update();
#endif

    keysym = keysym1;
    keysym1 = 0;

    while(!keysym)
    {
	nh_map_click(TRUE);
	main_hook();
	nh_map_pos(x, y, mod);
	nh_map_click(FALSE);
    }

    key = keysym;
    keysym = 0;

    return key;
}

#ifdef GTK_PROXY
static int
GTK_display_file(char *name)
{
    int fh;
    fh = proxy_cb_dlbh_fopen(name, "r");
    if (fh < 0)
	return FALSE;
    GTK_ext_display_file(fh);
    proxy_cb_dlbh_fclose(fh);
    return TRUE;
}
#endif

void
GTK_ext_display_file(int fh)
{
    guint hid;
    GtkWidget *w;
    GtkWidget *scrollbar;
    GtkWidget *label;
    GtkWidget *vbox;
    GtkWidget *hbox, *hbox2;
    GtkWidget *text;
    GtkWidget *button;

    char	buf[NH_BUFSIZ];

    w = nh_gtk_window_dialog(TRUE);
    gtk_widget_set_name(GTK_WIDGET(w), "fixed font");

    nh_position_popup_dialog(GTK_WIDGET(w));
    nh_gtk_focus_set_slave_for(GTK_WINDOW(w), GTK_WINDOW(main_window));
    hid = gtk_signal_connect(
	GTK_OBJECT(w), "destroy",
	GTK_SIGNAL_FUNC(default_destroy), &w);

    vbox = nh_gtk_new_and_add(gtk_vbox_new(FALSE, 0), w, "");

    label = nh_gtk_new_and_pack(
	gtk_label_new("HELP"), vbox, "",
	FALSE, FALSE, NH_PAD);

    hbox = nh_gtk_new_and_pack(
	gtk_hbox_new(FALSE, 0), vbox, "",
	FALSE, FALSE, NH_PAD);

    text = nh_gtk_new_and_pack(
	gtk_text_new(NULL, NULL), hbox, "",
	FALSE, FALSE, NH_PAD);

    gtk_widget_set_usize(
	GTK_WIDGET(text), 600, (root_height * 2)/3);

    scrollbar = nh_gtk_new_and_pack(
	gtk_vscrollbar_new(GTK_TEXT(text)->vadj), hbox, "",
	FALSE, FALSE, NH_PAD);

    hbox2 = nh_gtk_new_and_pack(
	gtk_hbox_new(FALSE, 0), vbox, "",
	FALSE, FALSE, NH_PAD);

    button = nh_gtk_new_and_pack(
	gtk_button_new_with_label("Close"), hbox2, "",
	TRUE, FALSE, NH_PAD);
    gtk_signal_connect(
	GTK_OBJECT(button), "clicked",
	GTK_SIGNAL_FUNC(default_button_press), (gpointer)'\033');

#ifndef GTK_PROXY
    while(dlbh_fgets(buf, NH_BUFSIZ, fh)) {
#else
    while(proxy_cb_dlbh_fgets(buf, NH_BUFSIZ, fh)) {
#endif
	char *s;
	if ((s = index(buf, '\r')) != 0)
	    memmove(s, s + 1, strlen(s));
	gtk_text_insert(GTK_TEXT(text), NULL, NULL, NULL, buf, strlen(buf));
    }

    gtk_widget_show_all(w);
    main_hook();

    if (w) {
	gtk_signal_disconnect(GTK_OBJECT(w), hid);
	gtk_widget_destroy(button);
	gtk_widget_destroy(hbox2);
	gtk_widget_destroy(scrollbar);
	gtk_widget_destroy(text);
	gtk_widget_destroy(hbox);
	gtk_widget_destroy(label);
	gtk_widget_destroy(vbox);
	gtk_widget_destroy(w);
    }
}

int
GTK_ext_player_selection(int *role, int *race, int *gend, int *align)
{
    select_player_flags.role = *role;
    select_player_flags.race = *race;
    select_player_flags.gend = *gend;
    select_player_flags.align = *align;

    init_select_player(TRUE);

    gtk_item_factory_create_items(
	main_item_factory,
	nmenu_items, menu_items,
	NULL);

    nh_menu_sensitive("/Game/Play", TRUE);
    nh_menu_sensitive("/Game/Save", FALSE);
    nh_menu_sensitive("/Move", FALSE);
    nh_menu_sensitive("/Fight", FALSE);
    nh_menu_sensitive("/Check", FALSE);
    nh_menu_sensitive("/Equip", FALSE);
    nh_menu_sensitive("/You", FALSE);
    nh_menu_sensitive("/Adventure", FALSE);
    nh_menu_sensitive("/Action", FALSE);
    nh_menu_sensitive("/Religion", FALSE);
    nh_menu_sensitive("/Special", FALSE);
    nh_option_lock(FALSE);

    gtk_main();

    nh_menu_sensitive("/Game/Play", FALSE);
    nh_menu_sensitive("/Game/Save", TRUE);
    nh_menu_sensitive("/Game/Save", TRUE);
    nh_menu_sensitive("/Move", TRUE);
    nh_menu_sensitive("/Fight", TRUE);
    nh_menu_sensitive("/Check", TRUE);
    nh_menu_sensitive("/Equip", TRUE);
    nh_menu_sensitive("/You", TRUE);
    nh_menu_sensitive("/Adventure", TRUE);
    nh_menu_sensitive("/Action", TRUE);
    nh_menu_sensitive("/Religion", TRUE);
    nh_menu_sensitive("/Special", TRUE);
    nh_option_lock(TRUE);

    *role = select_player_flags.role;
    *race = select_player_flags.race;
    *gend = select_player_flags.gend;
    *align = select_player_flags.align;

    return FALSE;		/* User didn't quit */
}

static void
GTK_display_inventory(void)
{
    display_inventory_needed = FALSE;
#ifdef GTK_PROXY
    proxy_cb_display_inventory();
#else
    (void) display_inventory((char *)0, FALSE);
#endif
}

void
GTK_update_inventory(void)
{
    if (copts.perm_invent) {
	if (WIN_INVEN == WIN_ERR ||
		!(gtkWindows[WIN_INVEN].flags & NHWF_DISPLAYED))
	    display_inventory_needed = TRUE;
	else
	    GTK_display_inventory();
    } else if (WIN_INVEN != WIN_ERR)
	GTK_unmap_menu_window(&gtkWindows[WIN_INVEN]);
}

void
GTK_wait_synch()
{
    ;
}

void
GTK_mark_synch()
{
    ;
}

volatile int delay_finished;

static gint delay_timeout(gpointer data)
{
    delay_finished++;
    return FALSE;
}

void
GTK_number_pad(int state)
{
    copts.num_pad = (boolean)state;
}

void
GTK_delay_output()
{
    if (initialized)
    {
	delay_finished = 0;
	gtk_timeout_add(50L, delay_timeout, 0);
	while (!delay_finished)
	    gtk_main_iteration();
    }
}

#define NAME_LINE 0		/* line # for player name */
#define GOLD_LINE 1		/* line # for amount of gold */
#define DEATH_LINE 2		/* line # for death description */
#define YEAR_LINE 6		/* line # for year */

static struct{
    GdkWChar	str[NH_BUFSIZ];
    int		len;
    int		width;
} rip_line[YEAR_LINE + 1];

int
GTK_ext_outrip(winid id, char *str)
{
    int		x, y;
    int		width;
    int		total_len, len, line;
    GtkWidget	*w;
    GtkWidget	*vbox;
    GtkWidget	*rip;
    GdkPixmap 	*rip_pixmap;
    char	mstr[NH_BUFSIZ];
    GdkWChar	*wc;
    GdkWChar	wstr[NH_BUFSIZ];
    extern const char *killed_by_prefix[];
    char	*rip_file;
    GdkFont	*rip_font;

    w = nh_gtk_window_dialog(TRUE);
    gtk_window_set_position(GTK_WINDOW(w), GTK_WIN_POS_CENTER);

    gtk_widget_set_events(
	w, GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK);

    gtk_signal_connect(
	GTK_OBJECT(w), "button_press_event",
	GTK_SIGNAL_FUNC(default_button_press), NULL);
    nh_gtk_focus_set_slave_for(GTK_WINDOW(w), GTK_WINDOW(main_window));
    gtk_signal_connect(
	GTK_OBJECT(w), "destroy",
	GTK_SIGNAL_FUNC(default_destroy), &w);

    gtk_widget_realize(w);

    vbox = nh_gtk_new_and_add(gtk_vbox_new(FALSE, 0), w, "");

#ifndef FILE_AREAS
    rip_file = "rip.xpm";
#else
    rip_file = make_file_name(FILE_AREA_SHARE, "rip.xpm");
#endif
    rip_pixmap = gdk_pixmap_create_from_xpm(
	w->window, 0, 0, rip_file);
#ifdef FILE_AREAS
    free(rip_file);
#endif

    rip = nh_gtk_new_and_pack(
	gtk_pixmap_new(rip_pixmap, 0), vbox, "",
	FALSE, FALSE, NH_PAD);

    rip_font = gtk_style_get_font(rip->style);

    Sprintf(mstr, "%s", plname);
    rip_line[NAME_LINE].len = gdk_mbstowcs(rip_line[NAME_LINE].str, mstr, NH_BUFSIZ);

    Sprintf(mstr, "%ld Au", u.ugold);
    rip_line[GOLD_LINE].len = gdk_mbstowcs(rip_line[GOLD_LINE].str, mstr, NH_BUFSIZ);

    Sprintf(mstr, "%4d", getyear());
    rip_line[YEAR_LINE].len = gdk_mbstowcs(rip_line[YEAR_LINE].str, mstr, NH_BUFSIZ);

    total_len = gdk_mbstowcs(wstr, str, NH_BUFSIZ);
    line = DEATH_LINE;
    wc = wstr;

    while(total_len > 0 && line < YEAR_LINE){
	len = total_len;
	while(1){
	    width = gdk_text_width_wc(rip_font, wc, len);
	    if(width < 96)
		break;
	    --len;
	}
	memcpy(rip_line[line].str, wc, len * sizeof(GdkWChar));
	rip_line[line].len = len;
	wc += len;
	total_len -= len;

	++line;
    }

    x = 155;
    y = 78;

    {
	gint dummy;
	gint height = 0, ascent, descent;

	for(line = 0 ; line <= YEAR_LINE ; ++line){
	    gdk_text_extents_wc(
		rip_font,
		rip_line[line].str, rip_line[line].len,
		&dummy,
		&dummy,
		&rip_line[line].width,
		&ascent,
		&descent);
	    if(height < (ascent + descent))
		height = ascent + descent;
	}

	for(line = 0 ; line <= YEAR_LINE ; ++line){
	    gdk_draw_text_wc(
		rip_pixmap,
		rip_font,
		rip->style->black_gc,
		x - rip_line[line].width / 2, y,
		rip_line[line].str, rip_line[line].len);
	    y += height;
	}
    }

    gtk_widget_show_all(w);
    gtk_main();
    return TRUE;
}

void
GTK_raw_print(const char *str)
{
    if(rawprint_win != WIN_ERR)
	GTK_putstr(rawprint_win, 0, str);
    else {
#ifdef TTY_GRAPHICS
	tty_raw_print(str);
#else
	puts(str); (void) fflush(stdout);
#endif
    }
}

void
GTK_raw_print_bold(const char *str)
{
    if(rawprint_win != WIN_ERR)
	GTK_putstr(rawprint_win, ATR_BOLD, str);
    else {
#ifdef TTY_GRAPHICS
	tty_raw_print_bold(str);
#else
	puts(str); (void) fflush(stdout);
#endif
    }
}
