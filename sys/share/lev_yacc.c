#ifndef lint
static char const 
yyrcsid[] = "$FreeBSD: src/usr.bin/yacc/skeleton.c,v 1.28 2000/01/17 02:04:06 bde Exp $";
#endif
#include <stdlib.h>
#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define YYLEX yylex()
#define YYEMPTY -1
#define yyclearin (yychar=(YYEMPTY))
#define yyerrok (yyerrflag=0)
#define YYRECOVERING() (yyerrflag!=0)
static int yygrowstack();
#define YYPREFIX "yy"
/*	SCCS Id: @(#)lev_yacc.c	3.4	2000/01/17	*/
/*	Copyright (c) 1989 by Jean-Christophe Collet */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * This file contains the Level Compiler code
 * It may handle special mazes & special room-levels
 */

/* In case we're using bison in AIX.  This definition must be
 * placed before any other C-language construct in the file
 * excluding comments and preprocessor directives (thanks IBM
 * for this wonderful feature...).
 *
 * Note: some cpps barf on this 'undefined control' (#pragma).
 * Addition of the leading space seems to prevent barfage for now,
 * and AIX will still see the directive.
 */
#ifdef _AIX
 #pragma alloca		/* keep leading space! */
#endif

#include "hack.h"
#include "sp_lev.h"

#define MAX_REGISTERS	10
#define ERR		(-1)
/* many types of things are put in chars for transference to NetHack.
 * since some systems will use signed chars, limit everybody to the
 * same number for portability.
 */
#define MAX_OF_TYPE	128

#define New(type)		\
	(type *) memset((genericptr_t)alloc(sizeof(type)), 0, sizeof(type))
#define NewTab(type, size)	(type **) alloc(sizeof(type *) * size)
#define Free(ptr)		free((genericptr_t)ptr)

extern void FDECL(yyerror, (const char *));
extern void FDECL(yywarning, (const char *));
extern int NDECL(yylex);
int NDECL(yyparse);

extern int FDECL(get_artifact_id, (char *));
extern int FDECL(get_floor_type, (CHAR_P));
extern int FDECL(get_room_type, (char *));
extern int FDECL(get_trap_type, (char *));
extern int FDECL(get_monster_id, (char *,CHAR_P));
extern int FDECL(get_object_id, (char *,CHAR_P));
extern boolean FDECL(check_monster_char, (CHAR_P));
extern boolean FDECL(check_object_char, (CHAR_P));
extern char FDECL(what_map_char, (CHAR_P));
extern void FDECL(scan_map, (char *));
extern void NDECL(wallify_map);
extern boolean NDECL(check_subrooms);
extern void FDECL(check_coord, (int,int,const char *));
extern void NDECL(store_part);
extern void NDECL(store_room);
extern boolean FDECL(write_level_file, (char *,splev *,specialmaze *));
extern void FDECL(free_rooms, (splev *));

static struct reg {
	int x1, y1;
	int x2, y2;
}		current_region;

static struct coord {
	int x;
	int y;
}		current_coord, current_align;

static struct size {
	int height;
	int width;
}		current_size;

char tmpmessage[256];
digpos *tmppass[32];
char *tmpmap[ROWNO];

digpos *tmpdig[MAX_OF_TYPE];
region *tmpreg[MAX_OF_TYPE];
lev_region *tmplreg[MAX_OF_TYPE];
door *tmpdoor[MAX_OF_TYPE];
drawbridge *tmpdb[MAX_OF_TYPE];
walk *tmpwalk[MAX_OF_TYPE];
lev_region *tmprndlreg[MAX_REGISTERS];

room_door *tmprdoor[MAX_OF_TYPE];
trap *tmptrap[MAX_OF_TYPE];
monster *tmpmonst[MAX_OF_TYPE];
object *tmpobj[MAX_OF_TYPE];
altar *tmpaltar[MAX_OF_TYPE];
lad *tmplad[MAX_OF_TYPE];
stair *tmpstair[MAX_OF_TYPE];
gold *tmpgold[MAX_OF_TYPE];
engraving *tmpengraving[MAX_OF_TYPE];
fountain *tmpfountain[MAX_OF_TYPE];
sink *tmpsink[MAX_OF_TYPE];
pool *tmppool[MAX_OF_TYPE];

mazepart *tmppart[10];
room *tmproom[MAXNROFROOMS*2];
corridor *tmpcor[MAX_OF_TYPE];

static specialmaze maze;
static splev special_lev;
static lev_init init_lev;

static char olist[MAX_REGISTERS], mlist[MAX_REGISTERS];
static struct coord plist[MAX_REGISTERS];

int n_olist = 0, n_mlist = 0, n_plist = 0;

unsigned int nrndlreg = 0;
unsigned int nlreg = 0, nreg = 0, ndoor = 0, ntrap = 0, nmons = 0, nobj = 0;
unsigned int ndb = 0, nwalk = 0, npart = 0, ndig = 0, nlad = 0, nstair = 0;
unsigned int naltar = 0, ncorridor = 0, nrooms = 0, ngold = 0, nengraving = 0;
unsigned int nfountain = 0, npool = 0, nsink = 0, npass = 0;

static int lev_flags = 0;

unsigned int max_x_map, max_y_map;

static xchar in_room;

extern int fatal_error;
extern int want_warnings;
extern const char *fname;

typedef union
{
	int	i;
	char*	map;
	struct {
		xchar room;
		xchar wall;
		xchar door;
	} corpos;
} YYSTYPE;
#define YYERRCODE 256
#define CHAR 257
#define INTEGER 258
#define BOOLEAN 259
#define PERCENT 260
#define MESSAGE_ID 261
#define MAZE_ID 262
#define LEVEL_ID 263
#define LEV_INIT_ID 264
#define GEOMETRY_ID 265
#define NOMAP_ID 266
#define OBJECT_ID 267
#define COBJECT_ID 268
#define MONSTER_ID 269
#define TRAP_ID 270
#define DOOR_ID 271
#define DRAWBRIDGE_ID 272
#define MAZEWALK_ID 273
#define WALLIFY_ID 274
#define REGION_ID 275
#define FILLING 276
#define RANDOM_OBJECTS_ID 277
#define RANDOM_MONSTERS_ID 278
#define RANDOM_PLACES_ID 279
#define ALTAR_ID 280
#define LADDER_ID 281
#define STAIR_ID 282
#define NON_DIGGABLE_ID 283
#define NON_PASSWALL_ID 284
#define ROOM_ID 285
#define PORTAL_ID 286
#define TELEPRT_ID 287
#define BRANCH_ID 288
#define LEV 289
#define CHANCE_ID 290
#define CORRIDOR_ID 291
#define GOLD_ID 292
#define ENGRAVING_ID 293
#define FOUNTAIN_ID 294
#define POOL_ID 295
#define SINK_ID 296
#define NONE 297
#define RAND_CORRIDOR_ID 298
#define DOOR_STATE 299
#define LIGHT_STATE 300
#define CURSE_TYPE 301
#define ENGRAVING_TYPE 302
#define DIRECTION 303
#define RANDOM_TYPE 304
#define O_REGISTER 305
#define M_REGISTER 306
#define P_REGISTER 307
#define A_REGISTER 308
#define ALIGNMENT 309
#define LEFT_OR_RIGHT 310
#define CENTER 311
#define TOP_OR_BOT 312
#define ALTAR_TYPE 313
#define UP_OR_DOWN 314
#define SUBROOM_ID 315
#define NAME_ID 316
#define FLAGS_ID 317
#define FLAG_TYPE 318
#define MON_ATTITUDE 319
#define MON_ALERTNESS 320
#define MON_APPEARANCE 321
#define CONTAINED 322
#define OBJFLAG_TYPE 323
#define OBJFLAGS_ID 324
#define RANDOMREGION_ID 325
#define R_REGISTER 326
#define STRING 327
#define MAP_ID 328
const short yylhs[] = {                                        -1,
    0,    0,   37,   37,   38,   38,   39,   40,   33,   23,
   23,   14,   14,   19,   19,   20,   20,   41,   41,   46,
   43,   43,   47,   47,   44,   44,   50,   50,   45,   45,
   52,   53,   53,   54,   54,   36,   51,   51,   57,   55,
   10,   10,   60,   60,   58,   58,   61,   61,   59,   59,
   56,   56,   62,   62,   62,   62,   62,   62,   62,   62,
   62,   62,   62,   62,   62,   62,   63,   64,   78,   65,
   77,   77,   79,   15,   15,   13,   13,   12,   12,   32,
   11,   11,   42,   42,   80,   81,   81,   84,    1,    1,
    2,    2,   82,   82,   85,   85,   85,   85,   48,   48,
   49,   49,   86,   89,   86,   83,   83,   90,   90,   90,
   90,   90,   90,   90,   90,   90,   90,   90,   90,   90,
   90,   90,   90,   90,   90,   90,   90,   90,  105,   66,
  104,  104,  106,  106,  106,  106,  106,   67,   67,  108,
  107,  109,  109,  110,  110,  110,  110,   68,   30,   30,
  111,  111,  112,  113,  113,  114,  114,  114,  116,   91,
  115,  115,  117,   69,   92,   98,   99,  100,   76,  118,
   94,  119,   87,  120,   95,  121,  123,   96,  124,   97,
  122,  122,   22,   22,   71,   72,   73,  101,  102,   93,
   70,   74,   75,   25,   25,   25,   28,   28,   28,   34,
   34,   35,   35,    3,    3,    4,    4,   21,   21,   21,
  103,  103,  103,  103,    5,    5,    6,    6,    7,    7,
    7,    8,    8,  127,  128,   29,   26,    9,   88,   88,
   24,   27,   31,   16,   16,   17,   17,   18,   18,  126,
  125,
};
const short yylen[] = {                                         2,
    0,    1,    1,    2,    1,    1,    5,    7,    3,    0,
   13,    1,    1,    0,    3,    3,    1,    0,    2,    3,
    0,    2,    3,    3,    0,    1,    1,    2,    1,    1,
    1,    0,    2,    5,    5,    7,    2,    2,   12,   12,
    0,    2,    5,    1,    5,    1,    5,    1,    5,    1,
    0,    2,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    3,    3,    0,   11,
    0,    2,    2,    1,    1,    1,    1,    1,    1,    5,
    1,    1,    1,    2,    3,    1,    2,    5,    1,    1,
    1,    1,    0,    2,    3,    3,    3,    1,    1,    3,
    1,    3,    1,    0,    4,    0,    2,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    0,   10,
    0,    2,    2,    2,    2,    2,    3,    2,    2,    0,
    9,    1,    1,    0,    7,    5,    5,    3,    3,    1,
    1,    1,    1,    1,    1,    0,    2,    2,    0,    7,
    0,    2,    2,    6,    7,    5,    1,    5,    5,    0,
    8,    0,    9,    0,    8,    0,    0,    8,    0,    6,
    0,    2,    1,   10,    3,    3,    3,    3,    3,    8,
    7,    5,    7,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    0,    2,    4,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    4,    4,    4,    4,    4,    1,    1,
    1,    1,    1,    1,    1,    0,    1,    1,    1,    5,
    9,
};
const short yydefred[] = {                                      0,
    0,    0,    0,    0,    0,    2,    0,    5,    6,    0,
    0,    0,    0,    0,    4,  233,    0,    9,    0,    0,
    0,    0,    0,    0,   15,    0,    0,    0,    0,   21,
   81,   82,   80,    0,    0,    0,    0,   86,    7,    0,
   93,    0,   19,    0,   16,    0,   20,    0,   84,    0,
   87,    0,    0,    0,    0,    0,   22,   26,    0,   51,
   51,    0,   89,   90,    0,    0,    0,    0,    0,    0,
   94,   98,    0,    0,    0,    0,   31,    8,   29,    0,
   28,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  167,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  108,  109,  110,  112,  119,  120,  125,  126,  124,  107,
  111,  113,  114,  115,  116,  117,  118,  121,  122,  123,
  127,  128,  232,    0,   23,  231,    0,   24,  207,    0,
  206,    0,    0,   33,    0,    0,    0,    0,    0,    0,
   52,   53,   54,   55,   56,   57,   58,   59,   60,   61,
   62,   63,   64,   65,   66,    0,   92,   91,   88,   95,
   97,  230,    0,   96,    0,  229,    0,  237,    0,  138,
  139,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  215,  216,    0,
  214,    0,    0,    0,  211,  212,  213,    0,    0,    0,
    0,    0,    0,    0,  170,    0,  183,  188,  189,  174,
  176,  179,  234,  235,    0,    0,  185,  150,    0,  100,
  102,  217,  218,    0,    0,    0,    0,   74,   75,    0,
   68,  187,  186,   67,    0,    0,    0,    0,  198,    0,
  197,    0,  199,  195,    0,  194,    0,  196,  205,    0,
  204,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  105,  172,    0,
    0,    0,    0,    0,  159,    0,    0,    0,  166,    0,
    0,  221,    0,  219,    0,  220,  168,    0,    0,    0,
  169,    0,    0,    0,  192,  238,  239,    0,  149,   44,
    0,    0,   46,    0,    0,    0,   35,   34,    0,    0,
  240,    0,    0,  203,  202,  140,    0,  201,  200,    0,
  164,  161,  224,  225,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  177,  180,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  226,    0,  227,    0,    0,  165,
    0,    0,    0,  223,  222,  191,    0,    0,    0,    0,
  193,    0,   48,    0,    0,    0,   50,    0,    0,    0,
   76,   77,    0,   12,   13,   11,  173,    0,  129,    0,
  162,    0,    0,  190,  228,    0,  171,  175,    0,  178,
    0,    0,    0,    0,    0,    0,   78,   79,    0,    0,
  143,  142,    0,  131,  163,    0,    0,    0,  182,   43,
    0,    0,   45,    0,    0,   36,   69,    0,  141,    0,
    0,    0,    0,    0,    0,   40,    0,   39,   71,  152,
  151,  153,    0,    0,    0,  132,  241,  210,    0,   47,
   42,   49,    0,    0,    0,  134,  135,    0,  136,  133,
  184,    0,   72,  155,  154,    0,    0,    0,  137,   73,
    0,    0,  146,  147,    0,  157,  158,  145,
};
const short yydgoto[] = {                                       3,
   65,  169,  280,  140,  220,  254,  325,  396,  326,  466,
   33,  439,  413,  416,  260,  245,  179,  338,   13,   25,
  424,  235,   21,  137,  277,  278,  134,  272,  273,  249,
  141,    4,    5,  360,  356,  257,    6,    7,    8,    9,
   28,   39,   44,   56,   78,   29,   57,  135,  138,   58,
   59,   79,   80,  144,   60,   82,   61,  345,  409,  342,
  405,  151,  152,  153,  154,  155,  156,  157,  158,  159,
  160,  161,  162,  163,  164,  165,  483,  469,  493,   40,
   41,   50,   70,   42,   71,  174,   72,  175,  213,  120,
  121,  122,  123,  124,  125,  126,  127,  128,  129,  130,
  131,  132,  236,  460,  444,  476,  180,  386,  443,  459,
  473,  474,  497,  503,  389,  362,  421,  293,  352,  295,
  296,  430,  400,  297,  237,  225,  226,  227,
};
const short yysindex[] = {                                   -145,
  -14,    3,    0, -214, -214,    0, -145,    0,    0, -250,
 -250,   48, -155, -155,    0,    0,   68,    0, -190,   83,
  -97,  -97, -231,  122,    0,  -64,  120, -243,  -97,    0,
    0,    0,    0, -190,  151, -130,  138,    0,    0, -243,
    0, -129,    0, -194,    0,  -59,    0, -128,    0, -240,
    0,  143,  144,  146,  147,  -92,    0,    0, -258,    0,
    0,  164,    0,    0,  165,  154,  155,  156,  124, -113,
    0,    0,  -41,  -40, -268, -268,    0,    0,    0,  -72,
    0, -200, -200,  -39, -127,  -41,  -40,  -37,  -34,  -38,
  -38,  -38,  -38,  163,  167,  168,    0,  169,  170,  171,
  172,  173,  174,  175,  176,  177,  178,  179,  180,  181,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  197,    0,    0,  198,    0,    0,  199,
    0,  200,  187,    0,  188,  189,  190,  191,  192,  193,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  208,    0,    0,    0,    0,
    0,    0,   -4,    0,    0,    0,  162,    0,  201,    0,
    0,  203,  204, -156,   -5,   -5,  216,   -5,   -5,   73,
  216,  216,  -36,  -36,  -36, -226,   -5,   -5,  -66,  -41,
  -40, -160, -160,  223, -206,   -5,    8,   -5,   -5, -250,
   -1,  262,  272,  260, -229, -232, -263,    0,    0,  275,
    0,  231,  232,  280,    0,    0,    0,  281,   72,  311,
  312,  317,  323,  113,    0,  321,    0,    0,    0,    0,
    0,    0,    0,    0,  328,  329,    0,    0,  330,    0,
    0,    0,    0,  332,  335,  125,  338,    0,    0,  340,
    0,    0,    0,    0,  341,  128,  -37,  -36,    0,  296,
    0,  344,    0,    0,  301,    0,  350,    0,    0,  351,
    0,   -5,  142,  148,   98,  101,  363, -160, -162,   94,
  153,  365,  370,  121,  395,  396,  401,   -5, -139,  123,
   -6,    5,  403,  -35, -156, -160,  407,    0,    0,  194,
 -202,  195, -196,   -5,    0,  356,  357,  410,    0,  202,
  411,    0,  360,    0,  412,    0,    0,  413,  205,  -36,
    0,  -36,  -36,  -36,    0,    0,    0,  414,    0,    0,
  206,  415,    0,  209,  417,  159,    0,    0,  422,  424,
    0,  425,  377,    0,    0,    0,  378,    0,    0,  428,
    0,    0,    0,    0, -156,  429, -268,  219, -253,  221,
   21,  450,  451,    0,    0, -250,  452,    6,  453,    7,
  463, -117, -205,  -36,    0,  473,    0,   -5,  474,    0,
  261,  477,  427,    0,    0,    0,  478,  210, -250,  479,
    0,  267,    0, -128,  482,  269,    0,  270,  485, -225,
    0,    0,  486,    0,    0,    0,    0,   71,    0, -250,
    0,  487,  256,    0,    0,  277,    0,    0,  222,    0,
  496,  494,    7,  498,  511, -250,    0,    0,  516, -225,
    0,    0,  512,    0,    0,  309,  531,  533,    0,    0,
 -127,  534,    0,  322,  534,    0,    0, -204,    0,  535,
  540,  324,  326,  541,  327,    0,  544,    0,    0,    0,
    0,    0,  545,  546, -182,    0,    0,    0,  547,    0,
    0,    0,  548, -241, -215,    0,    0, -250,    0,    0,
    0, -250,    0,    0,    0,  551,  552,  552,    0,    0,
 -215, -239,    0,    0,  552,    0,    0,    0,
};
const short yyrindex[] = {                                    591,
    0,    0,    0, -131,  737,    0,  597,    0,    0,    0,
    0,    0, -151,  794,    0,    0,    0,    0,    0,    0,
  -76,  833,    0,  719,    0,    0,    0,    0,  776,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   50,
    0,    0,    0,  105,    0,    0,    0,    0,    0,  441,
    0,    0,    0,    0,    0,   29,    0,    0,  107,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   42,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   52,
    0,  601,  814,    0,    0,    0,    0,    0,    0,  542,
  542,  542,  542,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    2,    0,    0,   66,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  150,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  218,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  476,    0,
    0,  515,    0,    0,    0,    0,    0,    0,    0,  554,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  278,    0,    0,    0,  587,    0,    0,    0,
    0,  639,    0,    0,  639,    0,    0,    0,    0,  342,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  679,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  402,  402,    0,    0,
    0,    0,    0,    0,  402,    0,    0,    0,
};
const short yygindex[] = {                                      0,
  214,  152,    0,  -70, -284, -187,  131,    0,    0,  166,
    0,  158,    0,    0,    0,    0,   99,    0,  594,  585,
    0, -181,  606,  426,    0,    0,  416,    0,    0,    0,
  -10,    0,    0,    0,    0,  337,  636,    0,    0,    0,
   26,  604,    0,    0,    0,    0,    0,  -71,  -69,  586,
    0,    0,    0,    0,    0,  588,    0,    0,  213,    0,
    0,    0,    0,    0,    0,  577,  578,  580,  581,  582,
  583,    0,    0,  584,  589,  590,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  388,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0, -178,    0,    0,    0,  565,    0,    0,    0,
    0,  196, -461, -449,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  -15,  -79,    0,    0,
};
#define YYTABLESIZE 1148
const short yytable[] = {                                      17,
   18,   99,  173,  229,  256,  142,  224,  228,  176,  231,
  232,  240,  241,  242,  170,  255,  494,  171,  246,  247,
  349,   37,   38,  498,  136,   31,   54,  133,   32,  262,
  263,  243,  437,  341,  173,  139,   66,   67,   68,  505,
  279,   85,  494,   10,  344,  404,  408,   30,  504,   83,
  394,   30,  258,  414,   43,  508,   55,  506,   16,  395,
   11,  351,  495,   16,  391,  101,   90,   91,   92,   93,
  145,  274,   32,  275,  269,  270,   16,  244,  438,   99,
  390,  146,   52,   53,   69,  472,  309,  507,  495,  147,
   54,  107,  108,  109,  148,  149,  470,  259,  415,  471,
  321,  354,   12,  315,   25,   19,   27,  358,   20,   10,
  173,   23,  234,   10,   10,  150,    1,    2,  350,  335,
   55,  322,  472,  110,   16,  323,  324,   24,  250,   14,
   16,  251,   14,   14,   14,  361,  486,  487,  488,  252,
   26,  322,  218,  253,   16,  323,  324,  219,  372,  103,
  373,  374,  375,   90,   91,   92,   93,   94,   95,   96,
   97,   98,  336,   27,  337,   34,   99,  100,  101,  102,
  103,  230,  104,  105,  106,  238,  239,   36,  107,  108,
  109,   63,   64,  167,  168,  411,  412,  176,   18,   18,
  182,  183,   35,  104,   46,   48,   47,   62,   51,  264,
   73,   74,  417,   75,   76,   77,  281,   84,   85,  419,
  110,   86,   87,   88,   89,  133,  136,  148,  143,  166,
  184,  178,  347,  177,  185,  186,  187,  188,  189,  190,
  191,  192,  193,  194,  195,  196,  197,  198,  199,  442,
  200,  201,  202,  203,  204,  205,  206,  207,  208,  209,
  210,  211,  233,  212,  214,  229,  248,  265,  215,  172,
  216,  217,  256,   99,   99,  261,   99,   99,   99,   99,
   99,   99,   99,   99,   99,   99,   99,  144,   99,   99,
   99,   99,   99,   99,   99,   99,   99,   99,   99,   99,
   32,   32,   99,   99,   99,   99,  392,  340,  221,   99,
  355,  222,  359,   85,   85,  266,   85,   85,  343,  403,
  407,   83,   83,   30,   30,  267,   99,  268,  282,   32,
  223,  283,  284,  285,  286,   99,   99,  101,  101,  287,
  101,  101,  101,  101,  101,  101,  101,  101,  101,  101,
  101,  130,  101,  101,  101,  101,  101,  101,  101,  101,
  101,  101,  101,  101,  288,  289,  101,  101,  101,  101,
  290,  233,  291,  101,  294,  401,   25,   25,   27,   27,
  292,  298,  299,  300,  221,  301,  221,  222,  302,  222,
  101,  304,  303,  305,  306,  307,  310,  311,  428,  101,
  101,  312,  441,  313,  314,   25,  223,   27,  223,  316,
  318,  156,   25,  319,   27,  317,  320,  327,  329,  445,
  328,  103,  103,  330,  103,  103,  103,  103,  103,  103,
  103,  103,  103,  103,  103,  455,  103,  103,  103,  103,
  103,  103,  103,  103,  331,  103,  103,  103,  332,  333,
  106,  103,  103,  103,  334,  339,  346,  351,  363,  364,
  368,  353,  357,  365,  367,  369,  370,  376,  378,  366,
  380,  381,  371,  377,  490,  382,  379,  383,  384,  385,
  387,  388,  391,  103,  103,  160,  393,  499,  397,  148,
  148,  500,  148,  148,  148,  148,  148,  148,  148,  148,
  148,  148,  148,  398,  399,  402,  406,  148,  148,  148,
  148,  148,  148,  148,  148,  148,  410,  148,  148,  148,
  148,  148,  148,  148,  208,  148,  418,  420,  422,  425,
  423,  426,  429,  427,  431,  433,  434,  435,  436,  440,
  446,  447,  148,  148,  448,  449,  450,  451,  453,  144,
  144,  148,  144,  144,  144,  144,  144,  144,  144,  144,
  144,  144,  144,  181,  454,  458,  456,  144,  144,  144,
  144,  144,  144,  144,  144,  144,  461,  144,  144,  144,
  144,  144,  144,  144,  462,  144,  463,  465,  475,  467,
  477,  480,  478,  479,  482,  481,  209,  491,  484,  485,
    1,  492,  144,  144,  501,  502,    3,  457,   14,  236,
   37,  144,  464,  130,  130,  489,  130,  130,  130,  130,
  130,  130,  130,  130,  130,  130,  130,  432,   45,   22,
  468,  130,  130,  130,  130,  130,  130,  130,  130,  130,
  271,  130,  130,  130,  130,  130,  130,  130,   41,  130,
  348,  276,   15,   49,   81,  452,  111,  112,   83,  113,
  114,  115,  116,  117,  308,  181,  130,  130,  118,  119,
    0,    0,    0,  156,  156,  130,  156,  156,  156,  156,
  156,  156,  156,  156,  156,  156,  156,    0,   70,  496,
    0,  156,  156,  156,  156,  156,  156,  156,  156,  156,
    0,  156,  156,  156,  156,  156,  156,  156,    0,  156,
    0,    0,  106,  106,    0,  106,  106,  106,  106,  106,
  106,  106,  106,  106,  106,  106,  156,  156,   17,    0,
  106,  106,  106,  106,  106,  156,  106,  106,  106,    0,
    0,    0,  106,  106,  106,    0,   14,  160,  160,    0,
  160,  160,  160,  160,  160,  160,  160,  160,  160,  160,
  160,    0,    0,    0,    0,  160,  160,  160,  160,  160,
    0,  160,  160,  160,  106,    0,    0,  160,  160,  160,
    0,    0,    0,    0,    0,   18,  208,  208,    0,  208,
  208,  208,  208,  208,  208,  208,  208,  208,  208,  208,
    0,    0,    0,   10,  208,  208,  208,  208,  208,  160,
  208,  208,  208,    0,    0,    0,  208,  208,  208,    0,
    0,    0,    0,   38,    0,  181,  181,    0,  181,  181,
  181,  181,  181,  181,  181,  181,  181,  181,  181,    0,
    0,    0,   18,  181,  181,  181,  181,  181,  208,  181,
  181,  181,    0,    0,    0,  181,  181,  181,  209,  209,
    0,  209,  209,  209,  209,  209,  209,  209,  209,  209,
  209,  209,   37,   37,    0,    0,  209,  209,  209,  209,
  209,    0,  209,  209,  209,    0,    0,  181,  209,  209,
  209,    0,    0,    0,    0,   37,    0,    0,    0,    0,
    0,   37,    0,    0,    0,    0,    0,    0,   37,    0,
   41,   41,    0,    0,    0,   41,   41,   41,   41,   41,
  209,    0,    0,    0,    0,   37,    0,    0,   41,    0,
   41,    0,    0,   41,    0,    0,    0,    0,   41,   41,
   41,   41,   41,   41,   41,    0,   41,    0,    0,    0,
   70,   70,    0,    0,    0,   70,   70,   70,   70,   70,
    0,    0,    0,   41,   41,    0,    0,    0,   70,    0,
   70,    0,   41,   70,    0,    0,    0,    0,   70,   70,
   70,   70,   70,   70,   70,    0,   70,    0,    0,   17,
   17,   17,   17,   17,   17,    0,    0,    0,    0,    0,
    0,    0,    0,   70,   70,   17,   17,   14,   14,   14,
   14,    0,   70,   17,    0,    0,    0,    0,    0,   17,
    0,    0,    0,   14,   14,    0,   17,    0,    0,    0,
    0,   14,    0,    0,    0,    0,    0,   14,    0,    0,
    0,    0,    0,   17,   14,    0,    0,   18,   18,    0,
   18,   18,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   14,   18,   18,   10,   10,   10,    0,    0,    0,
   18,    0,    0,    0,    0,    0,   18,    0,    0,    0,
   10,   10,    0,   18,    0,   38,   38,    0,   10,    0,
    0,    0,    0,    0,   10,    0,    0,    0,    0,    0,
   18,   10,    0,    0,   18,   18,    0,    0,   38,    0,
    0,    0,    0,    0,   38,    0,    0,    0,   10,   18,
   18,   38,    0,    0,    0,    0,    0,   18,    0,    0,
    0,    0,    0,   18,    0,    0,    0,    0,   38,    0,
   18,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,   18,
};
const short yycheck[] = {                                      10,
   11,    0,   40,   40,   40,   76,  185,  186,   88,  188,
  189,  193,  194,  195,   86,  203,  258,   87,  197,  198,
  305,  265,  266,  485,  257,  257,  285,  257,    0,  208,
  209,  258,  258,   40,   40,  304,  277,  278,  279,  501,
  304,    0,  258,   58,   40,   40,   40,   22,  498,    0,
  304,    0,  259,  259,   29,  505,  315,  297,  327,  313,
   58,   41,  304,  327,   44,    0,  267,  268,  269,  270,
  271,  304,  304,  306,  304,  305,  327,  304,  304,  280,
  365,  282,  277,  278,  325,  327,  268,  327,  304,  290,
  285,  292,  293,  294,  295,  296,  301,  304,  304,  304,
  288,  304,  317,  282,    0,   58,    0,  304,  264,  261,
   40,   44,   40,  265,  266,  316,  262,  263,  306,  298,
  315,  304,  327,  324,  327,  308,  309,  318,  200,  261,
  327,  201,  264,  265,  266,  314,  319,  320,  321,  300,
   58,  304,  299,  304,  327,  308,  309,  304,  330,    0,
  332,  333,  334,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  302,  261,  304,   44,  280,  281,  282,  283,
  284,  187,  286,  287,  288,  191,  192,   58,  292,  293,
  294,  310,  311,  311,  312,  303,  304,  267,  265,  266,
   92,   93,  257,   44,   44,   58,  327,  257,  328,  210,
   58,   58,  384,   58,   58,  298,  217,   44,   44,  388,
  324,   58,   58,   58,   91,  257,  257,    0,  291,  259,
   58,  260,  258,  258,   58,   58,   58,   58,   58,   58,
   58,   58,   58,   58,   58,   58,   58,   58,   58,  418,
   44,   44,   44,   44,   58,   58,   58,   58,   58,   58,
   58,   44,  289,  258,   93,   40,  323,  259,   58,  297,
   58,   58,   40,  262,  263,  258,  265,  266,  267,  268,
  269,  270,  271,  272,  273,  274,  275,    0,  277,  278,
  279,  280,  281,  282,  283,  284,  285,  286,  287,  288,
  262,  263,  291,  292,  293,  294,  367,  304,  304,  298,
  311,  307,  313,  262,  263,   44,  265,  266,  304,  304,
  304,  262,  263,  262,  263,   44,  315,   58,   44,  291,
  326,   91,   91,   44,   44,  324,  325,  262,  263,  258,
  265,  266,  267,  268,  269,  270,  271,  272,  273,  274,
  275,    0,  277,  278,  279,  280,  281,  282,  283,  284,
  285,  286,  287,  288,   44,   44,  291,  292,  293,  294,
   44,  289,   40,  298,   44,  376,  262,  263,  262,  263,
  258,   44,   44,   44,  304,   44,  304,  307,   44,  307,
  315,   44,  258,   44,   44,  258,   91,   44,  399,  324,
  325,   91,  322,   44,   44,  291,  326,  291,  326,  258,
  303,    0,  298,  303,  298,  258,   44,  314,   44,  420,
  258,  262,  263,   44,  265,  266,  267,  268,  269,  270,
  271,  272,  273,  274,  275,  436,  277,  278,  279,  280,
  281,  282,  283,  284,  314,  286,  287,  288,   44,   44,
    0,  292,  293,  294,   44,  323,   44,   41,   93,   93,
   91,  258,  258,   44,   44,   44,   44,   44,   44,  258,
   44,  303,  258,  258,  475,   44,  258,   44,   44,   93,
   93,   44,   44,  324,  325,    0,  258,  488,  258,  262,
  263,  492,  265,  266,  267,  268,  269,  270,  271,  272,
  273,  274,  275,   44,   44,   44,   44,  280,  281,  282,
  283,  284,  285,  286,  287,  288,   44,  290,  291,  292,
  293,  294,  295,  296,    0,  298,   44,   44,  258,   93,
   44,   44,   44,  314,  258,   44,  258,  258,   44,   44,
   44,  276,  315,  316,  258,  314,   41,   44,   41,  262,
  263,  324,  265,  266,  267,  268,  269,  270,  271,  272,
  273,  274,  275,    0,   44,   44,   41,  280,  281,  282,
  283,  284,  285,  286,  287,  288,  258,  290,  291,  292,
  293,  294,  295,  296,   44,  298,   44,   44,   44,  258,
   41,   41,  259,  258,   41,  259,    0,   41,   44,   44,
    0,   44,  315,  316,   44,   44,    0,  440,    5,   58,
    0,  324,  451,  262,  263,  475,  265,  266,  267,  268,
  269,  270,  271,  272,  273,  274,  275,  404,   34,   14,
  455,  280,  281,  282,  283,  284,  285,  286,  287,  288,
  215,  290,  291,  292,  293,  294,  295,  296,    0,  298,
  304,  216,    7,   40,   59,  433,   70,   70,   61,   70,
   70,   70,   70,   70,  267,   91,  315,  316,   70,   70,
   -1,   -1,   -1,  262,  263,  324,  265,  266,  267,  268,
  269,  270,  271,  272,  273,  274,  275,   -1,    0,  484,
   -1,  280,  281,  282,  283,  284,  285,  286,  287,  288,
   -1,  290,  291,  292,  293,  294,  295,  296,   -1,  298,
   -1,   -1,  262,  263,   -1,  265,  266,  267,  268,  269,
  270,  271,  272,  273,  274,  275,  315,  316,    0,   -1,
  280,  281,  282,  283,  284,  324,  286,  287,  288,   -1,
   -1,   -1,  292,  293,  294,   -1,    0,  262,  263,   -1,
  265,  266,  267,  268,  269,  270,  271,  272,  273,  274,
  275,   -1,   -1,   -1,   -1,  280,  281,  282,  283,  284,
   -1,  286,  287,  288,  324,   -1,   -1,  292,  293,  294,
   -1,   -1,   -1,   -1,   -1,    0,  262,  263,   -1,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
   -1,   -1,   -1,    0,  280,  281,  282,  283,  284,  324,
  286,  287,  288,   -1,   -1,   -1,  292,  293,  294,   -1,
   -1,   -1,   -1,    0,   -1,  262,  263,   -1,  265,  266,
  267,  268,  269,  270,  271,  272,  273,  274,  275,   -1,
   -1,   -1,    0,  280,  281,  282,  283,  284,  324,  286,
  287,  288,   -1,   -1,   -1,  292,  293,  294,  262,  263,
   -1,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  262,  263,   -1,   -1,  280,  281,  282,  283,
  284,   -1,  286,  287,  288,   -1,   -1,  324,  292,  293,
  294,   -1,   -1,   -1,   -1,  285,   -1,   -1,   -1,   -1,
   -1,  291,   -1,   -1,   -1,   -1,   -1,   -1,  298,   -1,
  262,  263,   -1,   -1,   -1,  267,  268,  269,  270,  271,
  324,   -1,   -1,   -1,   -1,  315,   -1,   -1,  280,   -1,
  282,   -1,   -1,  285,   -1,   -1,   -1,   -1,  290,  291,
  292,  293,  294,  295,  296,   -1,  298,   -1,   -1,   -1,
  262,  263,   -1,   -1,   -1,  267,  268,  269,  270,  271,
   -1,   -1,   -1,  315,  316,   -1,   -1,   -1,  280,   -1,
  282,   -1,  324,  285,   -1,   -1,   -1,   -1,  290,  291,
  292,  293,  294,  295,  296,   -1,  298,   -1,   -1,  261,
  262,  263,  264,  265,  266,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  315,  316,  277,  278,  261,  262,  263,
  264,   -1,  324,  285,   -1,   -1,   -1,   -1,   -1,  291,
   -1,   -1,   -1,  277,  278,   -1,  298,   -1,   -1,   -1,
   -1,  285,   -1,   -1,   -1,   -1,   -1,  291,   -1,   -1,
   -1,   -1,   -1,  315,  298,   -1,   -1,  262,  263,   -1,
  265,  266,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  315,  277,  278,  261,  262,  263,   -1,   -1,   -1,
  285,   -1,   -1,   -1,   -1,   -1,  291,   -1,   -1,   -1,
  277,  278,   -1,  298,   -1,  262,  263,   -1,  285,   -1,
   -1,   -1,   -1,   -1,  291,   -1,   -1,   -1,   -1,   -1,
  315,  298,   -1,   -1,  262,  263,   -1,   -1,  285,   -1,
   -1,   -1,   -1,   -1,  291,   -1,   -1,   -1,  315,  277,
  278,  298,   -1,   -1,   -1,   -1,   -1,  285,   -1,   -1,
   -1,   -1,   -1,  291,   -1,   -1,   -1,   -1,  315,   -1,
  298,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  315,
};
#define YYFINAL 3
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 328
#if YYDEBUG
const char * const yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,"'('","')'",0,0,"','",0,0,0,0,0,0,0,0,0,0,0,0,0,"':'",0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"'['",0,"']'",0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"CHAR",
"INTEGER","BOOLEAN","PERCENT","MESSAGE_ID","MAZE_ID","LEVEL_ID","LEV_INIT_ID",
"GEOMETRY_ID","NOMAP_ID","OBJECT_ID","COBJECT_ID","MONSTER_ID","TRAP_ID",
"DOOR_ID","DRAWBRIDGE_ID","MAZEWALK_ID","WALLIFY_ID","REGION_ID","FILLING",
"RANDOM_OBJECTS_ID","RANDOM_MONSTERS_ID","RANDOM_PLACES_ID","ALTAR_ID",
"LADDER_ID","STAIR_ID","NON_DIGGABLE_ID","NON_PASSWALL_ID","ROOM_ID",
"PORTAL_ID","TELEPRT_ID","BRANCH_ID","LEV","CHANCE_ID","CORRIDOR_ID","GOLD_ID",
"ENGRAVING_ID","FOUNTAIN_ID","POOL_ID","SINK_ID","NONE","RAND_CORRIDOR_ID",
"DOOR_STATE","LIGHT_STATE","CURSE_TYPE","ENGRAVING_TYPE","DIRECTION",
"RANDOM_TYPE","O_REGISTER","M_REGISTER","P_REGISTER","A_REGISTER","ALIGNMENT",
"LEFT_OR_RIGHT","CENTER","TOP_OR_BOT","ALTAR_TYPE","UP_OR_DOWN","SUBROOM_ID",
"NAME_ID","FLAGS_ID","FLAG_TYPE","MON_ATTITUDE","MON_ALERTNESS",
"MON_APPEARANCE","CONTAINED","OBJFLAG_TYPE","OBJFLAGS_ID","RANDOMREGION_ID",
"R_REGISTER","STRING","MAP_ID",
};
const char * const yyrule[] = {
"$accept : file",
"file :",
"file : levels",
"levels : level",
"levels : level levels",
"level : maze_level",
"level : room_level",
"maze_level : maze_def flags lev_init messages regions",
"room_level : level_def flags lev_init messages rreg_init rooms corridors_def",
"level_def : LEVEL_ID ':' string",
"lev_init :",
"lev_init : LEV_INIT_ID ':' CHAR ',' CHAR ',' BOOLEAN ',' BOOLEAN ',' light_state ',' walled",
"walled : BOOLEAN",
"walled : RANDOM_TYPE",
"flags :",
"flags : FLAGS_ID ':' flag_list",
"flag_list : FLAG_TYPE ',' flag_list",
"flag_list : FLAG_TYPE",
"messages :",
"messages : message messages",
"message : MESSAGE_ID ':' STRING",
"rreg_init :",
"rreg_init : rreg_init init_rreg",
"init_rreg : RANDOM_OBJECTS_ID ':' object_list",
"init_rreg : RANDOM_MONSTERS_ID ':' monster_list",
"rooms :",
"rooms : roomlist",
"roomlist : aroom",
"roomlist : aroom roomlist",
"corridors_def : random_corridors",
"corridors_def : corridors",
"random_corridors : RAND_CORRIDOR_ID",
"corridors :",
"corridors : corridors corridor",
"corridor : CORRIDOR_ID ':' corr_spec ',' corr_spec",
"corridor : CORRIDOR_ID ':' corr_spec ',' INTEGER",
"corr_spec : '(' INTEGER ',' DIRECTION ',' door_pos ')'",
"aroom : room_def room_details",
"aroom : subroom_def room_details",
"subroom_def : SUBROOM_ID ':' room_type ',' light_state ',' subroom_pos ',' room_size ',' string roomfill",
"room_def : ROOM_ID ':' room_type ',' light_state ',' room_pos ',' room_align ',' room_size roomfill",
"roomfill :",
"roomfill : ',' BOOLEAN",
"room_pos : '(' INTEGER ',' INTEGER ')'",
"room_pos : RANDOM_TYPE",
"subroom_pos : '(' INTEGER ',' INTEGER ')'",
"subroom_pos : RANDOM_TYPE",
"room_align : '(' h_justif ',' v_justif ')'",
"room_align : RANDOM_TYPE",
"room_size : '(' INTEGER ',' INTEGER ')'",
"room_size : RANDOM_TYPE",
"room_details :",
"room_details : room_details room_detail",
"room_detail : room_name",
"room_detail : room_chance",
"room_detail : room_door",
"room_detail : monster_detail",
"room_detail : object_detail",
"room_detail : object_flags",
"room_detail : trap_detail",
"room_detail : altar_detail",
"room_detail : fountain_detail",
"room_detail : sink_detail",
"room_detail : pool_detail",
"room_detail : gold_detail",
"room_detail : engraving_detail",
"room_detail : stair_detail",
"room_name : NAME_ID ':' string",
"room_chance : CHANCE_ID ':' INTEGER",
"$$1 :",
"room_door : DOOR_ID ':' secret ',' door_state ',' door_wall ',' door_pos $$1 room_door_infos",
"room_door_infos :",
"room_door_infos : room_door_infos room_door_info",
"room_door_info : ',' string",
"secret : BOOLEAN",
"secret : RANDOM_TYPE",
"door_wall : DIRECTION",
"door_wall : RANDOM_TYPE",
"door_pos : INTEGER",
"door_pos : RANDOM_TYPE",
"maze_def : MAZE_ID ':' string ',' filling",
"filling : CHAR",
"filling : RANDOM_TYPE",
"regions : aregion",
"regions : aregion regions",
"aregion : map_definition reg_init map_details",
"map_definition : NOMAP_ID",
"map_definition : map_geometry MAP_ID",
"map_geometry : GEOMETRY_ID ':' h_justif ',' v_justif",
"h_justif : LEFT_OR_RIGHT",
"h_justif : CENTER",
"v_justif : TOP_OR_BOT",
"v_justif : CENTER",
"reg_init :",
"reg_init : reg_init init_reg",
"init_reg : RANDOM_OBJECTS_ID ':' object_list",
"init_reg : RANDOM_PLACES_ID ':' place_list",
"init_reg : RANDOM_MONSTERS_ID ':' monster_list",
"init_reg : rndlevregion",
"object_list : object",
"object_list : object ',' object_list",
"monster_list : monster",
"monster_list : monster ',' monster_list",
"place_list : place",
"$$2 :",
"place_list : place $$2 ',' place_list",
"map_details :",
"map_details : map_details map_detail",
"map_detail : monster_detail",
"map_detail : object_detail",
"map_detail : object_flags",
"map_detail : door_detail",
"map_detail : trap_detail",
"map_detail : drawbridge_detail",
"map_detail : region_detail",
"map_detail : stair_region",
"map_detail : portal_region",
"map_detail : teleprt_region",
"map_detail : branch_region",
"map_detail : altar_detail",
"map_detail : fountain_detail",
"map_detail : mazewalk_detail",
"map_detail : wallify_detail",
"map_detail : ladder_detail",
"map_detail : stair_detail",
"map_detail : gold_detail",
"map_detail : engraving_detail",
"map_detail : diggable_detail",
"map_detail : passwall_detail",
"$$3 :",
"monster_detail : MONSTER_ID chance ':' monster_c ',' m_name ',' coordinate $$3 monster_infos",
"monster_infos :",
"monster_infos : monster_infos monster_info",
"monster_info : ',' string",
"monster_info : ',' MON_ATTITUDE",
"monster_info : ',' MON_ALERTNESS",
"monster_info : ',' alignment",
"monster_info : ',' MON_APPEARANCE string",
"object_detail : OBJECT_ID object_desc",
"object_detail : COBJECT_ID object_desc",
"$$4 :",
"object_desc : chance ':' object_c ',' o_name $$4 ',' object_where object_infos",
"object_where : coordinate",
"object_where : CONTAINED",
"object_infos :",
"object_infos : ',' curse_state ',' monster_id ',' enchantment optional_name",
"object_infos : ',' curse_state ',' enchantment optional_name",
"object_infos : ',' monster_id ',' enchantment optional_name",
"object_flags : OBJFLAGS_ID ':' obj_flag_list",
"obj_flag_list : obj_flag_list ',' OBJFLAG_TYPE",
"obj_flag_list : OBJFLAG_TYPE",
"curse_state : RANDOM_TYPE",
"curse_state : CURSE_TYPE",
"monster_id : STRING",
"enchantment : RANDOM_TYPE",
"enchantment : INTEGER",
"optional_name :",
"optional_name : ',' NONE",
"optional_name : ',' STRING",
"$$5 :",
"door_detail : DOOR_ID ':' door_state ',' coordinate $$5 door_infos",
"door_infos :",
"door_infos : door_infos door_info",
"door_info : ',' string",
"trap_detail : TRAP_ID chance ':' trap_name ',' coordinate",
"drawbridge_detail : DRAWBRIDGE_ID ':' coordinate ',' DIRECTION ',' door_state",
"mazewalk_detail : MAZEWALK_ID ':' coordinate ',' DIRECTION",
"wallify_detail : WALLIFY_ID",
"ladder_detail : LADDER_ID ':' coordinate ',' UP_OR_DOWN",
"stair_detail : STAIR_ID ':' coordinate ',' UP_OR_DOWN",
"$$6 :",
"stair_region : STAIR_ID ':' lev_region $$6 ',' lev_region ',' UP_OR_DOWN",
"$$7 :",
"rndlevregion : RANDOMREGION_ID '[' INTEGER ']' ':' lev_region $$7 ',' lev_region",
"$$8 :",
"portal_region : PORTAL_ID ':' lev_region $$8 ',' lev_region ',' string",
"$$9 :",
"$$10 :",
"teleprt_region : TELEPRT_ID ':' lev_region $$9 ',' lev_region $$10 teleprt_detail",
"$$11 :",
"branch_region : BRANCH_ID ':' lev_region $$11 ',' lev_region",
"teleprt_detail :",
"teleprt_detail : ',' UP_OR_DOWN",
"lev_region : region",
"lev_region : LEV '(' INTEGER ',' INTEGER ',' INTEGER ',' INTEGER ')'",
"fountain_detail : FOUNTAIN_ID ':' coordinate",
"sink_detail : SINK_ID ':' coordinate",
"pool_detail : POOL_ID ':' coordinate",
"diggable_detail : NON_DIGGABLE_ID ':' region",
"passwall_detail : NON_PASSWALL_ID ':' region",
"region_detail : REGION_ID ':' region ',' light_state ',' room_type prefilled",
"altar_detail : ALTAR_ID ':' coordinate ',' alignment ',' altar_type",
"gold_detail : GOLD_ID ':' amount ',' coordinate",
"engraving_detail : ENGRAVING_ID ':' coordinate ',' engraving_type ',' string",
"monster_c : monster",
"monster_c : RANDOM_TYPE",
"monster_c : m_register",
"object_c : object",
"object_c : RANDOM_TYPE",
"object_c : o_register",
"m_name : string",
"m_name : RANDOM_TYPE",
"o_name : string",
"o_name : RANDOM_TYPE",
"trap_name : string",
"trap_name : RANDOM_TYPE",
"room_type : string",
"room_type : RANDOM_TYPE",
"prefilled :",
"prefilled : ',' FILLING",
"prefilled : ',' FILLING ',' BOOLEAN",
"coordinate : coord",
"coordinate : p_register",
"coordinate : r_register",
"coordinate : RANDOM_TYPE",
"door_state : DOOR_STATE",
"door_state : RANDOM_TYPE",
"light_state : LIGHT_STATE",
"light_state : RANDOM_TYPE",
"alignment : ALIGNMENT",
"alignment : a_register",
"alignment : RANDOM_TYPE",
"altar_type : ALTAR_TYPE",
"altar_type : RANDOM_TYPE",
"p_register : P_REGISTER '[' INTEGER ']'",
"r_register : R_REGISTER '[' INTEGER ']'",
"o_register : O_REGISTER '[' INTEGER ']'",
"m_register : M_REGISTER '[' INTEGER ']'",
"a_register : A_REGISTER '[' INTEGER ']'",
"place : coord",
"place : NONE",
"monster : CHAR",
"object : CHAR",
"string : STRING",
"amount : INTEGER",
"amount : RANDOM_TYPE",
"chance :",
"chance : PERCENT",
"engraving_type : ENGRAVING_TYPE",
"engraving_type : RANDOM_TYPE",
"coord : '(' INTEGER ',' INTEGER ')'",
"region : '(' INTEGER ',' INTEGER ',' INTEGER ',' INTEGER ')'",
};
#endif
#if YYDEBUG
#include <stdio.h>
#endif
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 10000
#define YYMAXDEPTH 10000
#endif
#endif
#define YYINITSTACKSIZE 200
int yydebug;
int yynerrs;
int yyerrflag;
int yychar;
short *yyssp;
YYSTYPE *yyvsp;
YYSTYPE yyval;
YYSTYPE yylval;
short *yyss;
short *yysslim;
YYSTYPE *yyvs;
int yystacksize;

/*lev_comp.y*/
/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int yygrowstack()
{
    int newsize, i;
    short *newss;
    YYSTYPE *newvs;

    if ((newsize = yystacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return -1;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;
    i = yyssp - yyss;
    newss = yyss ? (short *)realloc(yyss, newsize * sizeof *newss) :
      (short *)malloc(newsize * sizeof *newss);
    if (newss == NULL)
        return -1;
    yyss = newss;
    yyssp = newss + i;
    newvs = yyvs ? (YYSTYPE *)realloc(yyvs, newsize * sizeof *newvs) :
      (YYSTYPE *)malloc(newsize * sizeof *newvs);
    if (newvs == NULL)
        return -1;
    yyvs = newvs;
    yyvsp = newvs + i;
    yystacksize = newsize;
    yysslim = yyss + newsize - 1;
    return 0;
}

#define YYABORT goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab

#ifndef YYPARSE_PARAM
#if defined(__cplusplus) || __STDC__
#define YYPARSE_PARAM_ARG void
#define YYPARSE_PARAM_DECL
#else	/* ! ANSI-C/C++ */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif	/* ANSI-C/C++ */
#else	/* YYPARSE_PARAM */
#ifndef YYPARSE_PARAM_TYPE
#define YYPARSE_PARAM_TYPE void *
#endif
#if defined(__cplusplus) || __STDC__
#define YYPARSE_PARAM_ARG YYPARSE_PARAM_TYPE YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else	/* ! ANSI-C/C++ */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL YYPARSE_PARAM_TYPE YYPARSE_PARAM;
#endif	/* ANSI-C/C++ */
#endif	/* ! YYPARSE_PARAM */

int
yyparse (YYPARSE_PARAM_ARG)
    YYPARSE_PARAM_DECL
{
    register int yym, yyn, yystate;
#if YYDEBUG
    register const char *yys;

    if ((yys = getenv("YYDEBUG")))
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = (-1);

    if (yyss == NULL && yygrowstack()) goto yyoverflow;
    yyssp = yyss;
    yyvsp = yyvs;
    *yyssp = yystate = 0;

yyloop:
    if ((yyn = yydefred[yystate])) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yyssp >= yysslim && yygrowstack())
        {
            goto yyoverflow;
        }
        *++yyssp = yystate = yytable[yyn];
        *++yyvsp = yylval;
        yychar = (-1);
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;
#if defined(lint) || defined(__GNUC__)
    goto yynewerror;
#endif
yynewerror:
    yyerror("syntax error");
#if defined(lint) || defined(__GNUC__)
    goto yyerrlab;
#endif
yyerrlab:
    ++yynerrs;
yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yyssp, yytable[yyn]);
#endif
                if (yyssp >= yysslim && yygrowstack())
                {
                    goto yyoverflow;
                }
                *++yyssp = yystate = yytable[yyn];
                *++yyvsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yyssp);
#endif
                if (yyssp <= yyss) goto yyabort;
                --yyssp;
                --yyvsp;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = (-1);
        goto yyloop;
    }
yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    yyval = yyvsp[1-yym];
    switch (yyn)
    {
case 7:
{
			unsigned i;

			if (fatal_error > 0) {
				(void) fprintf(stderr,
				"%s : %d errors detected. No output created!\n",
					fname, fatal_error);
			} else {
				maze.flags = yyvsp[-3].i;
				(void) memcpy((genericptr_t)&(maze.init_lev),
						(genericptr_t)&(init_lev),
						sizeof(lev_init));
				maze.numpart = npart;
				maze.parts = NewTab(mazepart, npart);
				for(i=0;i<npart;i++)
				    maze.parts[i] = tmppart[i];
				if (!write_level_file(yyvsp[-4].map, (splev *)0, &maze)) {
					yyerror("Can't write output file!!");
					exit(EXIT_FAILURE);
				}
				npart = 0;
			}
			Free(yyvsp[-4].map);
		  }
break;
case 8:
{
			unsigned i;

			if (fatal_error > 0) {
			    (void) fprintf(stderr,
			      "%s : %d errors detected. No output created!\n",
					fname, fatal_error);
			} else {
				special_lev.flags = (long) yyvsp[-5].i;
				(void) memcpy(
					(genericptr_t)&(special_lev.init_lev),
					(genericptr_t)&(init_lev),
					sizeof(lev_init));
				special_lev.nroom = nrooms;
				special_lev.rooms = NewTab(room, nrooms);
				for(i=0; i<nrooms; i++)
				    special_lev.rooms[i] = tmproom[i];
				special_lev.ncorr = ncorridor;
				special_lev.corrs = NewTab(corridor, ncorridor);
				for(i=0; i<ncorridor; i++)
				    special_lev.corrs[i] = tmpcor[i];
				if (check_subrooms()) {
				    if (!write_level_file(yyvsp[-6].map, &special_lev,
							  (specialmaze *)0)) {
					yyerror("Can't write output file!!");
					exit(EXIT_FAILURE);
				    }
				}
				free_rooms(&special_lev);
				nrooms = 0;
				ncorridor = 0;
			}
			Free(yyvsp[-6].map);
		  }
break;
case 9:
{
			if (index(yyvsp[0].map, '.'))
			    yyerror("Invalid dot ('.') in level name.");
			if ((int) strlen(yyvsp[0].map) > 8)
			    yyerror("Level names limited to 8 characters.");
			yyval.map = yyvsp[0].map;
			special_lev.nrmonst = special_lev.nrobjects = 0;
			n_mlist = n_olist = 0;
		  }
break;
case 10:
{
			/* in case we're processing multiple files,
			   explicitly clear any stale settings */
			(void) memset((genericptr_t) &init_lev, 0,
					sizeof init_lev);
			init_lev.init_present = FALSE;
			yyval.i = 0;
		  }
break;
case 11:
{
			init_lev.init_present = TRUE;
			init_lev.fg = what_map_char((char) yyvsp[-10].i);
			if (init_lev.fg == INVALID_TYPE)
			    yyerror("Invalid foreground type.");
			init_lev.bg = what_map_char((char) yyvsp[-8].i);
			if (init_lev.bg == INVALID_TYPE)
			    yyerror("Invalid background type.");
			init_lev.smoothed = yyvsp[-6].i;
			init_lev.joined = yyvsp[-4].i;
			init_lev.lit = yyvsp[-2].i;
			init_lev.walled = yyvsp[0].i;
			yyval.i = 1;
		  }
break;
case 14:
{
			yyval.i = 0;
		  }
break;
case 15:
{
			yyval.i = lev_flags;
			lev_flags = 0;	/* clear for next user */
		  }
break;
case 16:
{
			lev_flags |= yyvsp[-2].i;
		  }
break;
case 17:
{
			lev_flags |= yyvsp[0].i;
		  }
break;
case 20:
{
			int i, j;

			i = (int) strlen(yyvsp[0].map) + 1;
			j = (int) strlen(tmpmessage);
			if (i + j > 255) {
			   yyerror("Message string too long (>256 characters)");
			} else {
			    if (j) tmpmessage[j++] = '\n';
			    (void) strncpy(tmpmessage+j, yyvsp[0].map, i - 1);
			    tmpmessage[j + i - 1] = 0;
			}
			Free(yyvsp[0].map);
		  }
break;
case 23:
{
			if(special_lev.nrobjects) {
			    yyerror("Object registers already initialized!");
			} else {
			    special_lev.nrobjects = n_olist;
			    special_lev.robjects = (char *) alloc(n_olist);
			    (void) memcpy((genericptr_t)special_lev.robjects,
					  (genericptr_t)olist, n_olist);
			}
		  }
break;
case 24:
{
			if(special_lev.nrmonst) {
			    yyerror("Monster registers already initialized!");
			} else {
			    special_lev.nrmonst = n_mlist;
			    special_lev.rmonst = (char *) alloc(n_mlist);
			    (void) memcpy((genericptr_t)special_lev.rmonst,
					  (genericptr_t)mlist, n_mlist);
			  }
		  }
break;
case 25:
{
			tmproom[nrooms] = New(room);
			tmproom[nrooms]->name = (char *) 0;
			tmproom[nrooms]->parent = (char *) 0;
			tmproom[nrooms]->rtype = 0;
			tmproom[nrooms]->rlit = 0;
			tmproom[nrooms]->xalign = ERR;
			tmproom[nrooms]->yalign = ERR;
			tmproom[nrooms]->x = 0;
			tmproom[nrooms]->y = 0;
			tmproom[nrooms]->w = 2;
			tmproom[nrooms]->h = 2;
			in_room = 1;
		  }
break;
case 31:
{
			tmpcor[0] = New(corridor);
			tmpcor[0]->src.room = -1;
			ncorridor = 1;
		  }
break;
case 34:
{
			tmpcor[ncorridor] = New(corridor);
			tmpcor[ncorridor]->src.room = yyvsp[-2].corpos.room;
			tmpcor[ncorridor]->src.wall = yyvsp[-2].corpos.wall;
			tmpcor[ncorridor]->src.door = yyvsp[-2].corpos.door;
			tmpcor[ncorridor]->dest.room = yyvsp[0].corpos.room;
			tmpcor[ncorridor]->dest.wall = yyvsp[0].corpos.wall;
			tmpcor[ncorridor]->dest.door = yyvsp[0].corpos.door;
			ncorridor++;
			if (ncorridor >= MAX_OF_TYPE) {
				yyerror("Too many corridors in level!");
				ncorridor--;
			}
		  }
break;
case 35:
{
			tmpcor[ncorridor] = New(corridor);
			tmpcor[ncorridor]->src.room = yyvsp[-2].corpos.room;
			tmpcor[ncorridor]->src.wall = yyvsp[-2].corpos.wall;
			tmpcor[ncorridor]->src.door = yyvsp[-2].corpos.door;
			tmpcor[ncorridor]->dest.room = -1;
			tmpcor[ncorridor]->dest.wall = yyvsp[0].i;
			ncorridor++;
			if (ncorridor >= MAX_OF_TYPE) {
				yyerror("Too many corridors in level!");
				ncorridor--;
			}
		  }
break;
case 36:
{
			if ((unsigned) yyvsp[-5].i >= nrooms)
			    yyerror("Wrong room number!");
			yyval.corpos.room = yyvsp[-5].i;
			yyval.corpos.wall = yyvsp[-3].i;
			yyval.corpos.door = yyvsp[-1].i;
		  }
break;
case 37:
{
			store_room();
		  }
break;
case 38:
{
			store_room();
		  }
break;
case 39:
{
			tmproom[nrooms] = New(room);
			tmproom[nrooms]->parent = yyvsp[-1].map;
			tmproom[nrooms]->name = (char *) 0;
			tmproom[nrooms]->rtype = yyvsp[-9].i;
			tmproom[nrooms]->rlit = yyvsp[-7].i;
			tmproom[nrooms]->filled = yyvsp[0].i;
			tmproom[nrooms]->xalign = ERR;
			tmproom[nrooms]->yalign = ERR;
			tmproom[nrooms]->x = current_coord.x;
			tmproom[nrooms]->y = current_coord.y;
			tmproom[nrooms]->w = current_size.width;
			tmproom[nrooms]->h = current_size.height;
			in_room = 1;
		  }
break;
case 40:
{
			tmproom[nrooms] = New(room);
			tmproom[nrooms]->name = (char *) 0;
			tmproom[nrooms]->parent = (char *) 0;
			tmproom[nrooms]->rtype = yyvsp[-9].i;
			tmproom[nrooms]->rlit = yyvsp[-7].i;
			tmproom[nrooms]->filled = yyvsp[0].i;
			tmproom[nrooms]->xalign = current_align.x;
			tmproom[nrooms]->yalign = current_align.y;
			tmproom[nrooms]->x = current_coord.x;
			tmproom[nrooms]->y = current_coord.y;
			tmproom[nrooms]->w = current_size.width;
			tmproom[nrooms]->h = current_size.height;
			in_room = 1;
		  }
break;
case 41:
{
			yyval.i = 1;
		  }
break;
case 42:
{
			yyval.i = yyvsp[0].i;
		  }
break;
case 43:
{
			if ( yyvsp[-3].i < 1 || yyvsp[-3].i > 5 ||
			    yyvsp[-1].i < 1 || yyvsp[-1].i > 5 ) {
			    yyerror("Room position should be between 1 & 5!");
			} else {
			    current_coord.x = yyvsp[-3].i;
			    current_coord.y = yyvsp[-1].i;
			}
		  }
break;
case 44:
{
			current_coord.x = current_coord.y = ERR;
		  }
break;
case 45:
{
			if ( yyvsp[-3].i < 0 || yyvsp[-1].i < 0) {
			    yyerror("Invalid subroom position !");
			} else {
			    current_coord.x = yyvsp[-3].i;
			    current_coord.y = yyvsp[-1].i;
			}
		  }
break;
case 46:
{
			current_coord.x = current_coord.y = ERR;
		  }
break;
case 47:
{
			current_align.x = yyvsp[-3].i;
			current_align.y = yyvsp[-1].i;
		  }
break;
case 48:
{
			current_align.x = current_align.y = ERR;
		  }
break;
case 49:
{
			current_size.width = yyvsp[-3].i;
			current_size.height = yyvsp[-1].i;
		  }
break;
case 50:
{
			current_size.height = current_size.width = ERR;
		  }
break;
case 67:
{
			if (tmproom[nrooms]->name)
			    yyerror("This room already has a name!");
			else
			    tmproom[nrooms]->name = yyvsp[0].map;
		  }
break;
case 68:
{
			if (tmproom[nrooms]->chance)
			    yyerror("This room already assigned a chance!");
			else if (tmproom[nrooms]->rtype == OROOM)
			    yyerror("Only typed rooms can have a chance!");
			else if (yyvsp[0].i < 1 || yyvsp[0].i > 99)
			    yyerror("The chance is supposed to be percentile.");
			else
			    tmproom[nrooms]->chance = yyvsp[0].i;
		   }
break;
case 69:
{
			/* ERR means random here */
			if (yyvsp[-2].i == ERR && yyvsp[0].i != ERR) {
		     yyerror("If the door wall is random, so must be its pos!");
			    tmprdoor[ndoor] = (struct room_door *)0;
			} else {
			    tmprdoor[ndoor] = New(room_door);
			    tmprdoor[ndoor]->secret = yyvsp[-6].i;
			    tmprdoor[ndoor]->mask = yyvsp[-4].i;
			    tmprdoor[ndoor]->wall = yyvsp[-2].i;
			    tmprdoor[ndoor]->pos = yyvsp[0].i;
			    tmprdoor[ndoor]->arti_key = 0;
			}
		  }
break;
case 70:
{
			if (tmprdoor[ndoor]) {
			    ndoor++;
			    if (ndoor >= MAX_OF_TYPE) {
				    yyerror("Too many doors in room!");
				    ndoor--;
			    }
			}
		  }
break;
case 73:
{
			int token = get_artifact_id(yyvsp[0].map);
			if (token == ERR) {
			    char ebuf[100];
			    Sprintf(ebuf, "Undefined artifact key \"%s\"", yyvsp[0].map);
			    yyerror(ebuf);
			}
			else if (tmprdoor[ndoor])
			    tmprdoor[ndoor]->arti_key = token;
		  }
break;
case 80:
{
			maze.filling = (schar) yyvsp[0].i;
			if (index(yyvsp[-2].map, '.'))
			    yyerror("Invalid dot ('.') in level name.");
			if ((int) strlen(yyvsp[-2].map) > 8)
			    yyerror("Level names limited to 8 characters.");
			yyval.map = yyvsp[-2].map;
			in_room = 0;
			n_plist = n_mlist = n_olist = 0;
		  }
break;
case 81:
{
			yyval.i = get_floor_type((char)yyvsp[0].i);
		  }
break;
case 82:
{
			yyval.i = -1;
		  }
break;
case 85:
{
			store_part();
		  }
break;
case 86:
{
			tmppart[npart] = New(mazepart);
			tmppart[npart]->halign = 1;
			tmppart[npart]->valign = 1;
			tmppart[npart]->nrobjects = 0;
			tmppart[npart]->nloc = 0;
			tmppart[npart]->nrmonst = 0;
			tmppart[npart]->xsize = 1;
			tmppart[npart]->ysize = 1;
			tmppart[npart]->map = (char **) alloc(sizeof(char *));
			tmppart[npart]->map[0] = (char *) alloc(1);
			tmppart[npart]->map[0][0] = STONE;
			max_x_map = COLNO-1;
			max_y_map = ROWNO;
		  }
break;
case 87:
{
			tmppart[npart] = New(mazepart);
			tmppart[npart]->halign = yyvsp[-1].i % 10;
			tmppart[npart]->valign = yyvsp[-1].i / 10;
			tmppart[npart]->nrobjects = 0;
			tmppart[npart]->nloc = 0;
			tmppart[npart]->nrmonst = 0;
			scan_map(yyvsp[0].map);
			Free(yyvsp[0].map);
		  }
break;
case 88:
{
			yyval.i = yyvsp[-2].i + (yyvsp[0].i * 10);
		  }
break;
case 95:
{
			if (tmppart[npart]->nrobjects) {
			    yyerror("Object registers already initialized!");
			} else {
			    tmppart[npart]->robjects = (char *)alloc(n_olist);
			    (void) memcpy((genericptr_t)tmppart[npart]->robjects,
					  (genericptr_t)olist, n_olist);
			    tmppart[npart]->nrobjects = n_olist;
			}
		  }
break;
case 96:
{
			if (tmppart[npart]->nloc) {
			    yyerror("Location registers already initialized!");
			} else {
			    register int i;
			    tmppart[npart]->rloc_x = (char *) alloc(n_plist);
			    tmppart[npart]->rloc_y = (char *) alloc(n_plist);
			    for(i=0;i<n_plist;i++) {
				tmppart[npart]->rloc_x[i] = plist[i].x;
				tmppart[npart]->rloc_y[i] = plist[i].y;
			    }
			    tmppart[npart]->nloc = n_plist;
			}
		  }
break;
case 97:
{
			if (tmppart[npart]->nrmonst) {
			    yyerror("Monster registers already initialized!");
			} else {
			    tmppart[npart]->rmonst = (char *) alloc(n_mlist);
			    (void) memcpy((genericptr_t)tmppart[npart]->rmonst,
					  (genericptr_t)mlist, n_mlist);
			    tmppart[npart]->nrmonst = n_mlist;
			}
		  }
break;
case 98:
{
			/* nothing */
		  }
break;
case 99:
{
			if (n_olist < MAX_REGISTERS)
			    olist[n_olist++] = yyvsp[0].i;
			else
			    yyerror("Object list too long!");
		  }
break;
case 100:
{
			if (n_olist < MAX_REGISTERS)
			    olist[n_olist++] = yyvsp[-2].i;
			else
			    yyerror("Object list too long!");
		  }
break;
case 101:
{
			if (n_mlist < MAX_REGISTERS)
			    mlist[n_mlist++] = yyvsp[0].i;
			else
			    yyerror("Monster list too long!");
		  }
break;
case 102:
{
			if (n_mlist < MAX_REGISTERS)
			    mlist[n_mlist++] = yyvsp[-2].i;
			else
			    yyerror("Monster list too long!");
		  }
break;
case 103:
{
			if (n_plist < MAX_REGISTERS)
			    plist[n_plist++] = current_coord;
			else
			    yyerror("Location list too long!");
		  }
break;
case 104:
{
			if (n_plist < MAX_REGISTERS)
			    plist[n_plist++] = current_coord;
			else
			    yyerror("Location list too long!");
		  }
break;
case 129:
{
			tmpmonst[nmons] = New(monster);
			tmpmonst[nmons]->x = current_coord.x;
			tmpmonst[nmons]->y = current_coord.y;
			tmpmonst[nmons]->class = yyvsp[-4].i;
			tmpmonst[nmons]->peaceful = -1; /* no override */
			tmpmonst[nmons]->asleep = -1;
			tmpmonst[nmons]->align = - MAX_REGISTERS - 2;
			tmpmonst[nmons]->name.str = 0;
			tmpmonst[nmons]->appear = 0;
			tmpmonst[nmons]->appear_as.str = 0;
			tmpmonst[nmons]->chance = yyvsp[-6].i;
			tmpmonst[nmons]->id = NON_PM;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Monster");
			if (yyvsp[-2].map) {
			    int token = get_monster_id(yyvsp[-2].map, (char) yyvsp[-4].i);
			    if (token == ERR)
				yywarning(
			      "Invalid monster name!  Making random monster.");
			    else
				tmpmonst[nmons]->id = token;
			    Free(yyvsp[-2].map);
			}
		  }
break;
case 130:
{
			if (++nmons >= MAX_OF_TYPE) {
			    yyerror("Too many monsters in room or mazepart!");
			    nmons--;
			}
		  }
break;
case 133:
{
			tmpmonst[nmons]->name.str = yyvsp[0].map;
		  }
break;
case 134:
{
			tmpmonst[nmons]->peaceful = yyvsp[0].i;
		  }
break;
case 135:
{
			tmpmonst[nmons]->asleep = yyvsp[0].i;
		  }
break;
case 136:
{
			tmpmonst[nmons]->align = yyvsp[0].i;
		  }
break;
case 137:
{
			tmpmonst[nmons]->appear = yyvsp[-1].i;
			tmpmonst[nmons]->appear_as.str = yyvsp[0].map;
		  }
break;
case 138:
{
		  }
break;
case 139:
{
			/* 1: is contents of next object with 2 */
			/* 2: is a container */
			/* 0: neither */
			tmpobj[nobj-1]->containment = 2;
		  }
break;
case 140:
{
			tmpobj[nobj] = New(object);
			tmpobj[nobj]->class = yyvsp[-2].i;
			tmpobj[nobj]->corpsenm = NON_PM;
			tmpobj[nobj]->curse_state = -1;
			tmpobj[nobj]->name.str = 0;
			tmpobj[nobj]->chance = yyvsp[-4].i;
			tmpobj[nobj]->id = -1;
			tmpobj[nobj]->oflags = 0;
			if (yyvsp[0].map) {
			    int token = get_object_id(yyvsp[0].map, yyvsp[-2].i);
			    if (token == ERR)
				yywarning(
				"Illegal object name!  Making random object.");
			     else
				tmpobj[nobj]->id = token;
			    Free(yyvsp[0].map);
			}
		  }
break;
case 141:
{
			if (++nobj >= MAX_OF_TYPE) {
			    yyerror("Too many objects in room or mazepart!");
			    nobj--;
			}
		  }
break;
case 142:
{
			tmpobj[nobj]->containment = 0;
			tmpobj[nobj]->x = current_coord.x;
			tmpobj[nobj]->y = current_coord.y;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Object");
		  }
break;
case 143:
{
			tmpobj[nobj]->containment = 1;
			/* random coordinate, will be overridden anyway */
			tmpobj[nobj]->x = -MAX_REGISTERS-2;
			tmpobj[nobj]->y = -MAX_REGISTERS-2;
		  }
break;
case 144:
{
			tmpobj[nobj]->spe = -127;
	/* Note below: we're trying to make as many of these optional as
	 * possible.  We clearly can't make curse_state, enchantment, and
	 * monster_id _all_ optional, since ",random" would be ambiguous.
	 * We can't even just make enchantment mandatory, since if we do that
	 * alone, ",random" requires too much lookahead to parse.
	 */
		  }
break;
case 145:
{
		  }
break;
case 146:
{
		  }
break;
case 147:
{
		  }
break;
case 148:
{
		   	if (nobj > 0)
		   	   tmpobj[nobj-1]->oflags = yyvsp[0].i;
			else yyerror("Need an object before object flags!");
		  }
break;
case 149:
{
		     yyval.i = (yyvsp[-2].i | yyvsp[0].i);
		  }
break;
case 150:
{
		     yyval.i = yyvsp[0].i;
		  }
break;
case 151:
{
			tmpobj[nobj]->curse_state = -1;
		  }
break;
case 152:
{
			tmpobj[nobj]->curse_state = yyvsp[0].i;
		  }
break;
case 153:
{
			int token = get_monster_id(yyvsp[0].map, (char)0);
			if (token == ERR)	/* "random" */
			    tmpobj[nobj]->corpsenm = NON_PM - 1;
			else
			    tmpobj[nobj]->corpsenm = token;
			Free(yyvsp[0].map);
		  }
break;
case 154:
{
			tmpobj[nobj]->spe = -127;
		  }
break;
case 155:
{
			tmpobj[nobj]->spe = yyvsp[0].i;
		  }
break;
case 157:
{
		  }
break;
case 158:
{
			tmpobj[nobj]->name.str = yyvsp[0].map;
		  }
break;
case 159:
{
			tmpdoor[ndoor] = New(door);
			tmpdoor[ndoor]->x = current_coord.x;
			tmpdoor[ndoor]->y = current_coord.y;
			tmpdoor[ndoor]->mask = yyvsp[-2].i;
			tmpdoor[ndoor]->arti_key = 0;
			if(current_coord.x >= 0 && current_coord.y >= 0 &&
			   tmpmap[current_coord.y][current_coord.x] != DOOR &&
			   tmpmap[current_coord.y][current_coord.x] != SDOOR)
			    yyerror("Door decl doesn't match the map");
		  }
break;
case 160:
{
			if (++ndoor >= MAX_OF_TYPE) {
			    yyerror("Too many doors in mazepart!");
			    ndoor--;
			}
		  }
break;
case 163:
{
			int token = get_artifact_id(yyvsp[0].map);
			if (token == ERR) {
			    char ebuf[100];
			    Sprintf(ebuf, "Undefined artifact key \"%s\"", yyvsp[0].map);
			    yyerror(ebuf);
			}
			else
			    tmpdoor[ndoor]->arti_key = token;
		  }
break;
case 164:
{
			tmptrap[ntrap] = New(trap);
			tmptrap[ntrap]->x = current_coord.x;
			tmptrap[ntrap]->y = current_coord.y;
			tmptrap[ntrap]->type = yyvsp[-2].i;
			tmptrap[ntrap]->chance = yyvsp[-4].i;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Trap");
			if (++ntrap >= MAX_OF_TYPE) {
				yyerror("Too many traps in room or mazepart!");
				ntrap--;
			}
		  }
break;
case 165:
{
		        int x, y, dir;

			tmpdb[ndb] = New(drawbridge);
			x = tmpdb[ndb]->x = current_coord.x;
			y = tmpdb[ndb]->y = current_coord.y;
			/* convert dir from a DIRECTION to a DB_DIR */
			dir = yyvsp[-2].i;
			switch(dir) {
			case W_NORTH: dir = DB_NORTH; y--; break;
			case W_SOUTH: dir = DB_SOUTH; y++; break;
			case W_EAST:  dir = DB_EAST;  x++; break;
			case W_WEST:  dir = DB_WEST;  x--; break;
			default:
			    yyerror("Invalid drawbridge direction");
			    break;
			}
			tmpdb[ndb]->dir = dir;
			if (current_coord.x >= 0 && current_coord.y >= 0 &&
			    !IS_WALL(tmpmap[y][x])) {
			    char ebuf[60];
			    Sprintf(ebuf,
				    "Wall needed for drawbridge (%02d, %02d)",
				    current_coord.x, current_coord.y);
			    yyerror(ebuf);
			}

			if ( yyvsp[0].i == D_ISOPEN )
			    tmpdb[ndb]->db_open = 1;
			else if ( yyvsp[0].i == D_CLOSED )
			    tmpdb[ndb]->db_open = 0;
			else
			    yyerror("A drawbridge can only be open or closed!");
			ndb++;
			if (ndb >= MAX_OF_TYPE) {
				yyerror("Too many drawbridges in mazepart!");
				ndb--;
			}
		   }
break;
case 166:
{
			tmpwalk[nwalk] = New(walk);
			tmpwalk[nwalk]->x = current_coord.x;
			tmpwalk[nwalk]->y = current_coord.y;
			tmpwalk[nwalk]->dir = yyvsp[0].i;
			nwalk++;
			if (nwalk >= MAX_OF_TYPE) {
				yyerror("Too many mazewalks in mazepart!");
				nwalk--;
			}
		  }
break;
case 167:
{
			wallify_map();
		  }
break;
case 168:
{
			tmplad[nlad] = New(lad);
			tmplad[nlad]->x = current_coord.x;
			tmplad[nlad]->y = current_coord.y;
			tmplad[nlad]->up = yyvsp[0].i;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Ladder");
			nlad++;
			if (nlad >= MAX_OF_TYPE) {
				yyerror("Too many ladders in mazepart!");
				nlad--;
			}
		  }
break;
case 169:
{
			tmpstair[nstair] = New(stair);
			tmpstair[nstair]->x = current_coord.x;
			tmpstair[nstair]->y = current_coord.y;
			tmpstair[nstair]->up = yyvsp[0].i;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Stairway");
			nstair++;
			if (nstair >= MAX_OF_TYPE) {
				yyerror("Too many stairs in room or mazepart!");
				nstair--;
			}
		  }
break;
case 170:
{
			tmplreg[nlreg] = New(lev_region);
			tmplreg[nlreg]->in_islev = yyvsp[0].i;
			tmplreg[nlreg]->inarea.x1 = current_region.x1;
			tmplreg[nlreg]->inarea.y1 = current_region.y1;
			tmplreg[nlreg]->inarea.x2 = current_region.x2;
			tmplreg[nlreg]->inarea.y2 = current_region.y2;
		  }
break;
case 171:
{
			tmplreg[nlreg]->del_islev = yyvsp[-2].i;
			tmplreg[nlreg]->delarea.x1 = current_region.x1;
			tmplreg[nlreg]->delarea.y1 = current_region.y1;
			tmplreg[nlreg]->delarea.x2 = current_region.x2;
			tmplreg[nlreg]->delarea.y2 = current_region.y2;
			if(yyvsp[0].i)
			    tmplreg[nlreg]->rtype = LR_UPSTAIR;
			else
			    tmplreg[nlreg]->rtype = LR_DOWNSTAIR;
			tmplreg[nlreg]->rname.str = 0;
			nlreg++;
			if (nlreg >= MAX_OF_TYPE) {
				yyerror("Too many levregions in mazepart!");
				nlreg--;
			}
		  }
break;
case 172:
{
		   	if ((unsigned) yyvsp[-3].i != nrndlreg)
			    yyerror("Wrong random region number!");
			tmprndlreg[nrndlreg] = New(lev_region);
			tmprndlreg[nrndlreg]->in_islev = yyvsp[0].i;
			tmprndlreg[nrndlreg]->inarea.x1 = current_region.x1;
			tmprndlreg[nrndlreg]->inarea.y1 = current_region.y1;
			tmprndlreg[nrndlreg]->inarea.x2 = current_region.x2;
			tmprndlreg[nrndlreg]->inarea.y2 = current_region.y2;
		  }
break;
case 173:
{
		   	tmprndlreg[nrndlreg]->del_islev = yyvsp[0].i;
			tmprndlreg[nrndlreg]->delarea.x1 = current_region.x1;
			tmprndlreg[nrndlreg]->delarea.y1 = current_region.y1;
			tmprndlreg[nrndlreg]->delarea.x2 = current_region.x2;
			tmprndlreg[nrndlreg]->delarea.y2 = current_region.y2;
			tmprndlreg[nrndlreg]->rtype = 0;
			tmprndlreg[nrndlreg]->rname.str = (char *)0;
			nrndlreg++;
			if (nrndlreg >= MAX_REGISTERS) {
				yyerror("Too many random regions!");
				nrndlreg--;
			}
		  }
break;
case 174:
{
			tmplreg[nlreg] = New(lev_region);
			tmplreg[nlreg]->in_islev = yyvsp[0].i;
			tmplreg[nlreg]->inarea.x1 = current_region.x1;
			tmplreg[nlreg]->inarea.y1 = current_region.y1;
			tmplreg[nlreg]->inarea.x2 = current_region.x2;
			tmplreg[nlreg]->inarea.y2 = current_region.y2;
		  }
break;
case 175:
{
			tmplreg[nlreg]->del_islev = yyvsp[-2].i;
			tmplreg[nlreg]->delarea.x1 = current_region.x1;
			tmplreg[nlreg]->delarea.y1 = current_region.y1;
			tmplreg[nlreg]->delarea.x2 = current_region.x2;
			tmplreg[nlreg]->delarea.y2 = current_region.y2;
			tmplreg[nlreg]->rtype = LR_PORTAL;
			tmplreg[nlreg]->rname.str = yyvsp[0].map;
			nlreg++;
			if (nlreg >= MAX_OF_TYPE) {
				yyerror("Too many levregions in mazepart!");
				nlreg--;
			}
		  }
break;
case 176:
{
			tmplreg[nlreg] = New(lev_region);
			tmplreg[nlreg]->in_islev = yyvsp[0].i;
			tmplreg[nlreg]->inarea.x1 = current_region.x1;
			tmplreg[nlreg]->inarea.y1 = current_region.y1;
			tmplreg[nlreg]->inarea.x2 = current_region.x2;
			tmplreg[nlreg]->inarea.y2 = current_region.y2;
		  }
break;
case 177:
{
			tmplreg[nlreg]->del_islev = yyvsp[0].i;
			tmplreg[nlreg]->delarea.x1 = current_region.x1;
			tmplreg[nlreg]->delarea.y1 = current_region.y1;
			tmplreg[nlreg]->delarea.x2 = current_region.x2;
			tmplreg[nlreg]->delarea.y2 = current_region.y2;
		  }
break;
case 178:
{
			switch(yyvsp[0].i) {
			case -1: tmplreg[nlreg]->rtype = LR_TELE; break;
			case 0: tmplreg[nlreg]->rtype = LR_DOWNTELE; break;
			case 1: tmplreg[nlreg]->rtype = LR_UPTELE; break;
			}
			tmplreg[nlreg]->rname.str = 0;
			nlreg++;
			if (nlreg >= MAX_OF_TYPE) {
				yyerror("Too many levregions in mazepart!");
				nlreg--;
			}
		  }
break;
case 179:
{
			tmplreg[nlreg] = New(lev_region);
			tmplreg[nlreg]->in_islev = yyvsp[0].i;
			tmplreg[nlreg]->inarea.x1 = current_region.x1;
			tmplreg[nlreg]->inarea.y1 = current_region.y1;
			tmplreg[nlreg]->inarea.x2 = current_region.x2;
			tmplreg[nlreg]->inarea.y2 = current_region.y2;
		  }
break;
case 180:
{
			tmplreg[nlreg]->del_islev = yyvsp[0].i;
			tmplreg[nlreg]->delarea.x1 = current_region.x1;
			tmplreg[nlreg]->delarea.y1 = current_region.y1;
			tmplreg[nlreg]->delarea.x2 = current_region.x2;
			tmplreg[nlreg]->delarea.y2 = current_region.y2;
			tmplreg[nlreg]->rtype = LR_BRANCH;
			tmplreg[nlreg]->rname.str = 0;
			nlreg++;
			if (nlreg >= MAX_OF_TYPE) {
				yyerror("Too many levregions in mazepart!");
				nlreg--;
			}
		  }
break;
case 181:
{
			yyval.i = -1;
		  }
break;
case 182:
{
			yyval.i = yyvsp[0].i;
		  }
break;
case 183:
{
			yyval.i = 0;
		  }
break;
case 184:
{
/* This series of if statements is a hack for MSC 5.1.  It seems that its
   tiny little brain cannot compile if these are all one big if statement. */
			if (yyvsp[-7].i <= 0 || yyvsp[-7].i >= COLNO)
				yyerror("Region out of level range!");
			else if (yyvsp[-5].i < 0 || yyvsp[-5].i >= ROWNO)
				yyerror("Region out of level range!");
			else if (yyvsp[-3].i <= 0 || yyvsp[-3].i >= COLNO)
				yyerror("Region out of level range!");
			else if (yyvsp[-1].i < 0 || yyvsp[-1].i >= ROWNO)
				yyerror("Region out of level range!");
			current_region.x1 = yyvsp[-7].i;
			current_region.y1 = yyvsp[-5].i;
			current_region.x2 = yyvsp[-3].i;
			current_region.y2 = yyvsp[-1].i;
			yyval.i = 1;
		  }
break;
case 185:
{
			tmpfountain[nfountain] = New(fountain);
			tmpfountain[nfountain]->x = current_coord.x;
			tmpfountain[nfountain]->y = current_coord.y;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Fountain");
			nfountain++;
			if (nfountain >= MAX_OF_TYPE) {
			    yyerror("Too many fountains in room or mazepart!");
			    nfountain--;
			}
		  }
break;
case 186:
{
			tmpsink[nsink] = New(sink);
			tmpsink[nsink]->x = current_coord.x;
			tmpsink[nsink]->y = current_coord.y;
			nsink++;
			if (nsink >= MAX_OF_TYPE) {
				yyerror("Too many sinks in room!");
				nsink--;
			}
		  }
break;
case 187:
{
			tmppool[npool] = New(pool);
			tmppool[npool]->x = current_coord.x;
			tmppool[npool]->y = current_coord.y;
			npool++;
			if (npool >= MAX_OF_TYPE) {
				yyerror("Too many pools in room!");
				npool--;
			}
		  }
break;
case 188:
{
			tmpdig[ndig] = New(digpos);
			tmpdig[ndig]->x1 = current_region.x1;
			tmpdig[ndig]->y1 = current_region.y1;
			tmpdig[ndig]->x2 = current_region.x2;
			tmpdig[ndig]->y2 = current_region.y2;
			ndig++;
			if (ndig >= MAX_OF_TYPE) {
				yyerror("Too many diggables in mazepart!");
				ndig--;
			}
		  }
break;
case 189:
{
			tmppass[npass] = New(digpos);
			tmppass[npass]->x1 = current_region.x1;
			tmppass[npass]->y1 = current_region.y1;
			tmppass[npass]->x2 = current_region.x2;
			tmppass[npass]->y2 = current_region.y2;
			npass++;
			if (npass >= 32) {
				yyerror("Too many passwalls in mazepart!");
				npass--;
			}
		  }
break;
case 190:
{
			tmpreg[nreg] = New(region);
			tmpreg[nreg]->x1 = current_region.x1;
			tmpreg[nreg]->y1 = current_region.y1;
			tmpreg[nreg]->x2 = current_region.x2;
			tmpreg[nreg]->y2 = current_region.y2;
			tmpreg[nreg]->rlit = yyvsp[-3].i;
			tmpreg[nreg]->rtype = yyvsp[-1].i;
			if(yyvsp[0].i & 1) tmpreg[nreg]->rtype += MAXRTYPE+1;
			tmpreg[nreg]->rirreg = ((yyvsp[0].i & 2) != 0);
			if(current_region.x1 > current_region.x2 ||
			   current_region.y1 > current_region.y2)
			   yyerror("Region start > end!");
			if(tmpreg[nreg]->rtype == VAULT &&
			   (tmpreg[nreg]->rirreg ||
			    (tmpreg[nreg]->x2 - tmpreg[nreg]->x1 != 1) ||
			    (tmpreg[nreg]->y2 - tmpreg[nreg]->y1 != 1)))
				yyerror("Vaults must be exactly 2x2!");
			if(want_warnings && !tmpreg[nreg]->rirreg &&
			   current_region.x1 > 0 && current_region.y1 > 0 &&
			   current_region.x2 < (int)max_x_map &&
			   current_region.y2 < (int)max_y_map) {
			    /* check for walls in the room */
			    char ebuf[60];
			    register int x, y, nrock = 0;

			    for(y=current_region.y1; y<=current_region.y2; y++)
				for(x=current_region.x1;
				    x<=current_region.x2; x++)
				    if(IS_ROCK(tmpmap[y][x]) ||
				       IS_DOOR(tmpmap[y][x])) nrock++;
			    if(nrock) {
				Sprintf(ebuf,
					"Rock in room (%02d,%02d,%02d,%02d)?!",
					current_region.x1, current_region.y1,
					current_region.x2, current_region.y2);
				yywarning(ebuf);
			    }
			    if (
		!IS_ROCK(tmpmap[current_region.y1-1][current_region.x1-1]) ||
		!IS_ROCK(tmpmap[current_region.y2+1][current_region.x1-1]) ||
		!IS_ROCK(tmpmap[current_region.y1-1][current_region.x2+1]) ||
		!IS_ROCK(tmpmap[current_region.y2+1][current_region.x2+1])) {
				Sprintf(ebuf,
				"NonRock edge in room (%02d,%02d,%02d,%02d)?!",
					current_region.x1, current_region.y1,
					current_region.x2, current_region.y2);
				yywarning(ebuf);
			    }
			} else if(tmpreg[nreg]->rirreg &&
		!IS_ROOM(tmpmap[current_region.y1][current_region.x1])) {
			    char ebuf[60];
			    Sprintf(ebuf,
				    "Rock in irregular room (%02d,%02d)?!",
				    current_region.x1, current_region.y1);
			    yyerror(ebuf);
			}
			nreg++;
			if (nreg >= MAX_OF_TYPE) {
				yyerror("Too many regions in mazepart!");
				nreg--;
			}
		  }
break;
case 191:
{
			tmpaltar[naltar] = New(altar);
			tmpaltar[naltar]->x = current_coord.x;
			tmpaltar[naltar]->y = current_coord.y;
			tmpaltar[naltar]->align = yyvsp[-2].i;
			tmpaltar[naltar]->shrine = yyvsp[0].i;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Altar");
			naltar++;
			if (naltar >= MAX_OF_TYPE) {
				yyerror("Too many altars in room or mazepart!");
				naltar--;
			}
		  }
break;
case 192:
{
			tmpgold[ngold] = New(gold);
			tmpgold[ngold]->x = current_coord.x;
			tmpgold[ngold]->y = current_coord.y;
			tmpgold[ngold]->amount = yyvsp[-2].i;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Gold");
			ngold++;
			if (ngold >= MAX_OF_TYPE) {
				yyerror("Too many golds in room or mazepart!");
				ngold--;
			}
		  }
break;
case 193:
{
			tmpengraving[nengraving] = New(engraving);
			tmpengraving[nengraving]->x = current_coord.x;
			tmpengraving[nengraving]->y = current_coord.y;
			tmpengraving[nengraving]->engr.str = yyvsp[0].map;
			tmpengraving[nengraving]->etype = yyvsp[-2].i;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Engraving");
			nengraving++;
			if (nengraving >= MAX_OF_TYPE) {
			    yyerror("Too many engravings in room or mazepart!");
			    nengraving--;
			}
		  }
break;
case 195:
{
			yyval.i = - MAX_REGISTERS - 1;
		  }
break;
case 198:
{
			yyval.i = - MAX_REGISTERS - 1;
		  }
break;
case 201:
{
			yyval.map = (char *) 0;
		  }
break;
case 203:
{
			yyval.map = (char *) 0;
		  }
break;
case 204:
{
			int token = get_trap_type(yyvsp[0].map);
			if (token == ERR)
				yyerror("Unknown trap type!");
			yyval.i = token;
			Free(yyvsp[0].map);
		  }
break;
case 206:
{
			int token = get_room_type(yyvsp[0].map);
			if (token == ERR) {
				yywarning("Unknown room type!  Making ordinary room...");
				yyval.i = OROOM;
			} else
				yyval.i = token;
			Free(yyvsp[0].map);
		  }
break;
case 208:
{
			yyval.i = 0;
		  }
break;
case 209:
{
			yyval.i = yyvsp[0].i;
		  }
break;
case 210:
{
			yyval.i = yyvsp[-2].i + (yyvsp[0].i << 1);
		  }
break;
case 214:
{
			current_coord.x = current_coord.y = -MAX_REGISTERS-2;
		  }
break;
case 221:
{
			yyval.i = - MAX_REGISTERS - 1;
		  }
break;
case 224:
{
			if ( yyvsp[-1].i >= MAX_REGISTERS )
				yyerror("Register Index overflow!");
			else
				current_coord.x = current_coord.y = - yyvsp[-1].i - 1;
		  }
break;
case 225:
{
			if ( yyvsp[-1].i < 0 || yyvsp[-1].i >= nrndlreg )
				yyerror("Register Index overflow!");
			else {
				current_coord.x = -MAX_REGISTERS-1;
				current_coord.y = - yyvsp[-1].i - 1;
			}
		  }
break;
case 226:
{
			if ( yyvsp[-1].i >= MAX_REGISTERS )
				yyerror("Register Index overflow!");
			else
				yyval.i = - yyvsp[-1].i - 1;
		  }
break;
case 227:
{
			if ( yyvsp[-1].i >= MAX_REGISTERS )
				yyerror("Register Index overflow!");
			else
				yyval.i = - yyvsp[-1].i - 1;
		  }
break;
case 228:
{
			if ( yyvsp[-1].i >= 3 )
				yyerror("Register Index overflow!");
			else
				yyval.i = - yyvsp[-1].i - 1;
		  }
break;
case 230:
{
			current_coord.x = (char)-1;
			current_coord.y = (char)-1;
		  }
break;
case 231:
{
			if (check_monster_char((char) yyvsp[0].i))
				yyval.i = yyvsp[0].i ;
			else {
				yyerror("Unknown monster class!");
				yyval.i = ERR;
			}
		  }
break;
case 232:
{
			char c = yyvsp[0].i;
			if (check_object_char(c))
				yyval.i = c;
			else {
				yyerror("Unknown char class!");
				yyval.i = ERR;
			}
		  }
break;
case 236:
{
			yyval.i = 100;	/* default is 100% */
		  }
break;
case 237:
{
			if (yyvsp[0].i <= 0 || yyvsp[0].i > 100)
			    yyerror("Expected percentile chance.");
			yyval.i = yyvsp[0].i;
		  }
break;
case 240:
{
			if (!in_room && !init_lev.init_present &&
			    (yyvsp[-3].i < 0 || yyvsp[-3].i > (int)max_x_map ||
			     yyvsp[-1].i < 0 || yyvsp[-1].i > (int)max_y_map))
			    yyerror("Coordinates out of map range!");
			current_coord.x = yyvsp[-3].i;
			current_coord.y = yyvsp[-1].i;
		  }
break;
case 241:
{
/* This series of if statements is a hack for MSC 5.1.  It seems that its
   tiny little brain cannot compile if these are all one big if statement. */
			if (yyvsp[-7].i < 0 || yyvsp[-7].i > (int)max_x_map)
				yyerror("Region out of map range!");
			else if (yyvsp[-5].i < 0 || yyvsp[-5].i > (int)max_y_map)
				yyerror("Region out of map range!");
			else if (yyvsp[-3].i < 0 || yyvsp[-3].i > (int)max_x_map)
				yyerror("Region out of map range!");
			else if (yyvsp[-1].i < 0 || yyvsp[-1].i > (int)max_y_map)
				yyerror("Region out of map range!");
			current_region.x1 = yyvsp[-7].i;
			current_region.y1 = yyvsp[-5].i;
			current_region.x2 = yyvsp[-3].i;
			current_region.y2 = yyvsp[-1].i;
		  }
break;
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yyssp = YYFINAL;
        *++yyvsp = yyval;
        if (yychar < 0)
        {
            if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yyssp, yystate);
#endif
    if (yyssp >= yysslim && yygrowstack())
    {
        goto yyoverflow;
    }
    *++yyssp = yystate;
    *++yyvsp = yyval;
    goto yyloop;
yyoverflow:
    yyerror("yacc stack overflow");
yyabort:
    return (1);
yyaccept:
    return (0);
}
