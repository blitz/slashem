/*
  $Id$
 */
/*
  GTK+ NetHack Copyright (c) Issei Numata 1999-2000
               Copyright (c) Slash'EM Development Team 2000-2002
  GTK+ NetHack may be freely redistributed.  See license for details. 
*/

#include <stdlib.h>
#include <stdio.h>
#include "winGTK.h"
#include "dlb.h"
#include "proxycb.h"
#include "proxysvr.h"

/* #define DEBUG */

int stone_tile = 0;		/* The current tile for stone */

#ifdef GTK_PROXY
int total_tiles_used;
int tiles_per_row;
int tiles_per_col;
#else
/* from tile.c */
extern int total_tiles_used;
extern int tiles_per_row;
extern int tiles_per_col;
#endif

extern GtkWidget	*main_window;

/* The transparancy data for one tile:
 *
 * +-------------+ ^
 * |             | | oy
 * |             | V
 * |     **      | ^
 * |    /__\     | | ny
 * |    *  *     | V
 * |             |
 * |             |
 * +-------------+
 * <---><-->
 *   ox  nx
 *
 * Each pixel inside the nx x ny pixel rectangle has stored within the
 * transparancy data whether it is transparent or opaque. This data is
 * run length encoded on a line-by-line basis (the encoding always breaks
 * at the end of a line). Each length is stored in pixels as one byte.
 * Each line begins with the length of transparent pixels.
 */

static struct tile_transp {
    unsigned char ox;	/* The number of transparent cols at left of tile */
    unsigned char oy;	/* The number of transparent lines at top of tile */
    unsigned char nx;	/* The number of non-transparent cols in tile */
    unsigned char ny;	/* The number of non-transparent lines in tile */
    unsigned char *data;	/* RLE transparency data */
} *tile_transp;

static TileTab		*Tile;
static GdkPixmap	*tile_pixmap;
static GdkImage		*tile_image;
static GdkImage		*tmp_img;
static GdkPixbuf	*tile_pixbuf;
static GdkPixbuf	*tmp_pixbuf;
static unsigned int	tile_bits_per_pixel;

/*
 * Note that while tmp_img always exists, tmp_pixbuf is set to NULL to
 * represent a cleared tmp image. This saves creating a blank pixbuf
 * and then compositing the first tile onto it.
 */

void
x_tile_tmp_clear()
{
    g_return_if_fail(tile_image != NULL || tile_pixbuf != NULL);
    if (tile_image && tmp_img)
	memset(tmp_img->mem, 0, tmp_img->bpl * tmp_img->height);
    else if (tmp_pixbuf) {
	gdk_pixbuf_unref(tmp_pixbuf);
	tmp_pixbuf = NULL;
    }
}

/*
 * This macro returns the address of a specified pixel.
 */

#define PIXBUF_PIXEL(x, y)	(pixels + (y) * rowstride + (x) * n_channels)

/*
 * Test whether a pixel is opaque. Note we only compare RGB.
 * If an alpha channel is present it will be ignored.
 */

#define PIXEL_OPAQUE(x, y)	memcmp(PIXBUF_PIXEL(x, y), nul_pixel, 3)

static void calc_tile_transp(TileTab *t, GdkPixbuf *pixbuf, int i)
{
    int x, y, sx, sy;
    int j, k, n;
    static unsigned char *rle_buf = NULL;
    static guchar *nul_pixel, *pixels;
    static int rowstride;
    static int n_channels;
    static int n_bytes;
    if (i < 0)
    {	/* Clean up */
#ifdef DEBUG
	fprintf(stderr, "%d bytes used for transarency data\n", n_bytes);
#endif
	free(rle_buf);
	rle_buf = NULL;
	return;
    }
    if (!rle_buf)
    {	/* Init */
	n_bytes = 0;
	pixels = gdk_pixbuf_get_pixels(pixbuf);
	rowstride = gdk_pixbuf_get_rowstride(pixbuf);
	n_channels = gdk_pixbuf_get_n_channels(pixbuf);
	nul_pixel = PIXBUF_PIXEL(0, 0);
	rle_buf = (unsigned char *) alloc(t->unit_width * t->unit_height);
	/* This makes the RLE slightly easier at very little cost */
	if (t->unit_width > 255)
	    panic("gtktile: Unit width too large (%d)", t->unit_width);
    }
    sx = (i % tiles_per_row) * Tile->unit_width;
    sy = (i / tiles_per_row) * Tile->unit_height;
    for(y = 0; y < t->unit_height; y++) {
	for(x = 0; x < t->unit_width; x++)
	    if (PIXEL_OPAQUE(sx + x, sy + y))
		break;
	if (x < t->unit_width)
	    break;
    }
    tile_transp[i].oy = y;
    for(y = t->unit_height - 1; y > tile_transp[i].oy; y--) {
	for(x = 0; x < t->unit_width; x++)
	    if (PIXEL_OPAQUE(sx + x, sy + y))
		break;
	if (x < t->unit_width)
	    break;
    }
    tile_transp[i].ny = y - tile_transp[i].oy;
    if (!tile_transp[i].ny) {
	tile_transp[i].oy = 0;
	tile_transp[i].ox = 0;
	tile_transp[i].nx = 0;
	tile_transp[i].data = NULL;
	return;
    }
    for(x = 0; x < t->unit_width; x++) {
	for(y = tile_transp[i].oy; y < tile_transp[i].oy+tile_transp[i].ny; y++)
	    if (PIXEL_OPAQUE(sx + x, sy + y))
		break;
	if (y < tile_transp[i].oy + tile_transp[i].ny)
	    break;
    }
    tile_transp[i].ox = x;
    for(x = t->unit_width - 1; x > tile_transp[i].ox; x--) {
	for(y = tile_transp[i].oy; y < tile_transp[i].oy+tile_transp[i].ny; y++)
	    if (PIXEL_OPAQUE(sx + x, sy + y))
		break;
	if (y < tile_transp[i].oy + tile_transp[i].ny)
	    break;
    }
    tile_transp[i].nx = x - tile_transp[i].ox;
    j = 0;
    for(y = tile_transp[i].oy; y < tile_transp[i].oy + tile_transp[i].ny; y++)
    {
	k = n = 0;
	for(x = tile_transp[i].ox; x < tile_transp[i].ox+tile_transp[i].nx; x++)
	{
	    if (k & 1)
		if (PIXEL_OPAQUE(sx + x, sy + y))
		    n++;
		else {
		    rle_buf[j + k++] = n;
		    n = 1;
		}
	    else
		if (PIXEL_OPAQUE(sx + x, sy + y)) {
		    rle_buf[j + k++] = n;
		    n = 1;
		}
		else
		    n++;
	}
	if (n)
	    rle_buf[j + k++] = n;
	j += k;
    }
    tile_transp[i].data = (unsigned char *) alloc(j);
    memcpy(tile_transp[i].data, rle_buf, j);
    n_bytes += sizeof(*tile_transp) + j;
}

static int
x_tile_load(TileTab *t)
{
    int fh, nb;
    char buf[1024];
    GdkPixbufLoader *loader;
    GError *err = NULL;
    Tile = t;
    tile_pixbuf = NULL;
#ifdef GTK_PROXY
    fh = proxy_cb_dlbh_fopen(t->file, "rb");
#else
# ifdef FILE_AREAS
    fh = dlbh_fopen(FILE_AREA_SHARE, t->file, "rb");
# else
    fh = dlbh_fopen(t->file, "rb");
# endif
#endif /* GTK_PROXY */
    if (fh < 0) {
	pline("Cannot open tile file %s!", t->file);
	return 0;
    }
    loader = gdk_pixbuf_loader_new();
#ifdef GTK_PROXY
    while (nb = proxy_cb_dlbh_fread(buf, 1, sizeof(buf), fh), nb > 0) {
#else
    while (nb = dlbh_fread(buf, 1, sizeof(buf), fh), nb > 0) {
#endif
	if (!gdk_pixbuf_loader_write(loader, buf, nb, &err)) {
	    pline("Error loading %s: %s", t->file, err->message);
	    g_error_free(err);
	    err = NULL;
	    break;
	}
    }
    if (nb < 0) {
	pline("Read error from tile file %s!", t->file);
	(void)gdk_pixbuf_loader_close(loader, NULL);
    } else if (!nb) {
	if (!gdk_pixbuf_loader_close(loader, &err)) {
	    pline("Error loading %s: %s", t->file, err->message);
	    g_error_free(err);
	    err = NULL;
	}
	else {
	    tile_pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);
	    if (!tile_pixbuf)
		pline("No image found in tile file %s!", t->file);
	    else
		g_object_ref(tile_pixbuf);
	}
    } else
	(void)gdk_pixbuf_loader_close(loader, NULL);
#ifdef GTK_PROXY
    proxy_cb_dlbh_fclose(fh);
#else
    dlbh_fclose(fh);
#endif
    g_object_unref(loader);
    if (!tile_pixbuf)
	return 0;
    t->tilemap_width = gdk_pixbuf_get_width(tile_pixbuf);
    t->tilemap_height = gdk_pixbuf_get_height(tile_pixbuf);
    t->unit_width = t->tilemap_width / tiles_per_row;
    t->unit_height = t->tilemap_height / tiles_per_col;
    if (t->spread) {
	t->ofsetx_3d = t->unit_width / 3;
	t->ofsety_3d = t->unit_height / 2;
    } else {
	t->ofsetx_3d = 0;
	t->ofsety_3d = 0;
    }
    return 1;
}

#ifdef GTK_PROXY
static struct proxy_tilemap *
x_tile_load_tilemap(TileTab *t)
{
    int fh;
    struct proxy_tilemap *map;
    fh = proxy_cb_dlbh_fopen(t->mapfile, "rb");
    if (fh < 0) {
	pline("Cannot open tile map file %s!", t->mapfile);
	return NULL;
    }
    map = proxy_load_tilemap(fh);
    proxy_cb_dlbh_fclose(fh);
    return map;
}

/* From tilemap.c */
static int
ceil_sqrt(int c)
{
    int a=c/36,la;      /* Approximation and last approximation */
    /* Compute floor(sqrt(c)) */
    do
    {
	la=a;
	a=(a*a+c)/(2*a);
    } while (a!=la);
    /* Adjust for ceil(sqrt(c)) */
    return a*a==c?a:a+1;
}

static int
x_tile_load_map(TileTab *t)
{
    int retval;
    struct proxycb_get_glyph_mapping_res *glyph_map;
    struct proxy_tilemap *tile_map;
    glyph_map = proxy_cb_get_glyph_mapping();
    if (!glyph_map) {
	pline("Cannot get glyph mapping.");
	return 0;
    }
    tile_map = x_tile_load_tilemap(t);
    if (!tile_map) {
	proxy_cb_free_glyph_mapping(glyph_map);
	return 0;
    }
    total_tiles_used = tile_map->no_tiles;
    tiles_per_row = ceil_sqrt(total_tiles_used);
    tiles_per_col = (total_tiles_used + tiles_per_row - 1) / tiles_per_row;
    retval = proxy_map_glyph2tile(glyph_map, tile_map);
    proxy_cb_free_glyph_mapping(glyph_map);
    proxy_free_tilemap(tile_map);
    return retval;
}
#endif

static enum xshm_map_mode
x_tile_set_map_mode(TileTab *t)
{
    int i;
    enum xshm_map_mode mode;
    GdkPixbuf *copy;
    guchar *pixels;
    GdkGC *gc;
    if (!t->spread && !t->transparent)
	mode = XSHM_MAP_PIXMAP;
    else
	mode = getenv("HACKPIXBUF") ? XSHM_MAP_PIXBUF : XSHM_MAP_IMAGE;
    /*
     * Pixbufs use so much memory that it's not unexpected for us to
     * fail to generate the alpha channel correctly. This is not a
     * great problem since we carefully avoid using it ourselves
     * (preferring to use the transparent colour directly), but might
     * cause gdk_pixbuf_render_to_drawable() to get slightly confused
     * when it dithers the tile (the transparent colour may bleed into
     * the glyph). We therefore issue a warning.
     *
     * Note: It's not clear that gdk_pixbuf_render_to_drawable()
     * actually uses this information, but what more can we do?
     * There's little point using gdk_pixbuf_render_to_drawable_alpha()
     * since all that does is take care not to draw transparent pixels
     * (which we don't care about anyway; we'll never read them).
     */
    if (mode == XSHM_MAP_PIXBUF && gdk_pixbuf_get_has_alpha(tile_pixbuf)) {
	/* We can't trust the XPM alpha channel & it will
	 * overide if we don't get rid of it here */
	copy = gdk_pixbuf_new(gdk_pixbuf_get_colorspace(tile_pixbuf), FALSE,
	  gdk_pixbuf_get_bits_per_sample(tile_pixbuf),
	  t->tilemap_width, t->tilemap_height);
	if (copy) {
	    gdk_pixbuf_copy_area(tile_pixbuf, 0, 0,
	      t->tilemap_width, t->tilemap_height, copy, 0, 0);
	    gdk_pixbuf_unref(tile_pixbuf);
	    tile_pixbuf = copy;
	} else {
	    pline("Warning: Not enough memory: Tiles may be degraded");
	    mode = XSHM_MAP_IMAGE;
	}
    }
    if (mode == XSHM_MAP_PIXBUF && !gdk_pixbuf_get_has_alpha(tile_pixbuf)) {
	pixels = gdk_pixbuf_get_pixels(tile_pixbuf);
	copy = gdk_pixbuf_add_alpha(tile_pixbuf, TRUE,
	  pixels[0], pixels[1], pixels[2]);
	if (copy) {
	    gdk_pixbuf_unref(tile_pixbuf);
	    tile_pixbuf = copy;
	}
	else {
	    pline("Warning: Not enough memory: Tiles may be degraded");
	    mode = XSHM_MAP_IMAGE;
	}
    }
    tmp_pixbuf = NULL;
    if (mode != XSHM_MAP_PIXBUF) {
	tile_pixmap = gdk_pixmap_new(main_window->window,
	  t->tilemap_width, t->tilemap_height, -1);
	if (!tile_pixmap)
	    panic("Not enough memory to load tiles!");
	gc = gdk_gc_new(tile_pixmap);
	gdk_pixbuf_render_to_drawable(tile_pixbuf, tile_pixmap, gc, 0, 0, 0, 0,
	  t->tilemap_width, t->tilemap_height, GDK_RGB_DITHER_NORMAL, 0, 0);
	gdk_gc_unref(gc);
	if (mode == XSHM_MAP_IMAGE) {
	    tile_transp = (struct tile_transp *)
	      alloc(total_tiles_used * sizeof(*tile_transp));
	    for(i = 0; i < total_tiles_used; i++)
		calc_tile_transp(t, tile_pixbuf, i);
	    calc_tile_transp(t, tile_pixbuf, -1);
	    /* TODO: Creating an image via a pixmap is very inefficient;
	     * this should be done directly from pixbuf, even if GTK+ doesn't
	     * provide any easy way to do this.
	     */
	    tile_image = gdk_image_get((GdkWindow *)tile_pixmap,
		0, 0, t->tilemap_width, t->tilemap_height);
	    if (!tile_image)
		panic("Not enough memory to load tiles!");
	    gdk_pixmap_unref(tile_pixmap);
	    tile_pixmap = NULL;
	    tmp_img = gdk_image_new(GDK_IMAGE_NORMAL, gdk_visual_get_system(),
	      t->unit_width, t->unit_height);
	    if (!tmp_img)
		panic("Not enough memory to load tiles!");
#if GTK_CHECK_VERSION(1,3,3)
	    tile_bits_per_pixel = tile_image->bits_per_pixel;
#else
	    /*
	     * Technically, this could give an incorrect result. However, as
	     * long as the bitmap_pad is no more than 32 bits (max. X11 allows);
	     * the bytes/line is not larger than necessary; and the width is
	     * at least 32 pixels, then it will be correct.
	     */
	    tile_bits_per_pixel = tile_image->bpl * 8 / t->tilemap_width;
#endif
	}
	else
	    tmp_img = tile_image = NULL;
	gdk_pixbuf_unref(tile_pixbuf);
	tile_pixbuf = NULL;
    }
    return mode;
}

enum xshm_map_mode
x_tile_init(TileTab *t)
{
    int glyph = cmap_to_glyph(S_stone);
#ifdef GTK_PROXY
    if (!x_tile_load_map(t))
	return XSHM_MAP_NONE;
#endif
    if (!x_tile_load(t))
	return XSHM_MAP_NONE;
#ifdef GTK_PROXY
    stone_tile = proxy_glyph2tile[glyph];
#else
    stone_tile = glyph2tile[glyph];
#endif
    return x_tile_set_map_mode(t);
}

void
x_tile_destroy()
{
    int i;
    if (tmp_img) {
	gdk_image_destroy(tmp_img);
	tmp_img = NULL;
    }
    if (tile_pixmap) {
	gdk_pixmap_unref(tile_pixmap);
	tile_pixmap = NULL;
    }
    if (tmp_pixbuf) {
	gdk_pixbuf_unref(tmp_pixbuf);
	tmp_pixbuf = NULL;
    }
    if (tile_pixbuf) {
	gdk_pixbuf_unref(tile_pixbuf);
	tile_pixbuf = NULL;
    }
    if (tile_transp) {
	for(i = 0; i < total_tiles_used; i++)
	    free(tile_transp[i].data);
	free(tile_transp);
	tile_transp = NULL;
    }
    if (tile_image) {
	gdk_image_destroy(tile_image);
	tile_image = NULL;
    }
}

#define nh_pixbuf_get_pixel(x, y, pixel) \
    				memcpy(pixel, PIXBUF_PIXEL(x, y), n_channels)
#define nh_pixbuf_put_pixel(x, y, pixel) \
    				memcpy(PIXBUF_PIXEL(x, y), pixel, n_channels)

#undef red
#undef green
#undef blue

void
x_tile_tmp_draw_rectangle(int ofsx, int ofsy, int nhc)
{
    int i;
    int width;
    int height;
    guint32 c;
    guchar rgba[4];
    guchar *pixels;
    int rowstride;
    int n_channels;
    g_return_if_fail(tmp_img != NULL || tmp_pixbuf != NULL);
    width = Tile->unit_width;
    height = Tile->unit_height;

    if (tmp_img)
    {
	c = nh_color[nhc].pixel;
	if(ofsy >= 0 && ofsy < height)
	    for(i=0 ; i<width ; ++i)
		if(ofsx + i >= 0 && ofsx + i < width)
		    gdk_image_put_pixel(tmp_img, ofsx + i, ofsy, c);

	if(ofsy + height - 1 >= 0 && ofsy + height - 1 < height)
	    for(i=0 ; i<width ; ++i)
		if(ofsx + i >= 0 && ofsx + i < width)
		    gdk_image_put_pixel(tmp_img, ofsx + i, ofsy + height -1, c);

	if(ofsx >= 0 && ofsx < width)
	    for(i=0 ; i<height ; ++i)
		if(ofsy + i >= 0 && ofsy + i < height)
		    gdk_image_put_pixel(tmp_img, ofsx, ofsy + i, c);

	if(ofsx + width - 1 >= 0 && ofsx + width - 1 < width)
	    for(i=0 ; i<height ; ++i)
		if(ofsy + i >= 0 && ofsy + i < height)
		    gdk_image_put_pixel(tmp_img, ofsx + width - 1, ofsy + i, c);
    }
    else
    {
	g_return_if_fail(gdk_pixbuf_get_bits_per_sample(tmp_pixbuf) == 8 &&
	  gdk_pixbuf_get_colorspace(tmp_pixbuf) == GDK_COLORSPACE_RGB);
	pixels = gdk_pixbuf_get_pixels(tmp_pixbuf);
	rowstride = gdk_pixbuf_get_rowstride(tmp_pixbuf);
	n_channels = gdk_pixbuf_get_n_channels(tmp_pixbuf);
	rgba[0] = nh_color[nhc].red / 257;
	rgba[1] = nh_color[nhc].green / 257;
	rgba[2] = nh_color[nhc].blue / 257;
	rgba[3] = 255;
	if(ofsy >= 0 && ofsy < height)
	    for(i=0 ; i<width ; ++i)
		if(ofsx + i >= 0 && ofsx + i < width)
		    nh_pixbuf_put_pixel(ofsx + i, ofsy, rgba);

	if(ofsy + height - 1 >= 0 && ofsy + height - 1 < height)
	    for(i=0 ; i<width ; ++i)
		if(ofsx + i >= 0 && ofsx + i < width)
		    nh_pixbuf_put_pixel(ofsx + i, ofsy + height -1, rgba);

	if(ofsx >= 0 && ofsx < width)
	    for(i=0 ; i<height ; ++i)
		if(ofsy + i >= 0 && ofsy + i < height)
		    nh_pixbuf_put_pixel(ofsx, ofsy + i, rgba);

	if(ofsx + width - 1 >= 0 && ofsx + width - 1 < width)
	    for(i=0 ; i<height ; ++i)
		if(ofsy + i >= 0 && ofsy + i < height)
		    nh_pixbuf_put_pixel(ofsx + width - 1, ofsy + i, rgba);
    }
}

/*
 * Render the rectangle at (sx, sy) size (width, height) in the raster
 * for tile to the drawable. The rectangle is clipped to the tile boundaries.
 */

int
x_tile_render_to_drawable(GdkDrawable *drawable, GdkGC *gc, int tile,
  int sx, int sy, int dest_x, int dest_y, int width, int height)
{
    int srcx, srcy;

    srcx = (tile % tiles_per_row) * Tile->unit_width;
    srcy = (tile / tiles_per_row) * Tile->unit_height;

    if (sx < 0) {
	dest_x -= sx;
	width += sx;
	sx = 0;
    }
    if (sy < 0) {
	dest_y -= sy;
	height += sy;
	sy = 0;
    }
    if (sx + width > Tile->unit_width)
	width = Tile->unit_width - sx;
    if (sy + height > Tile->unit_height)
	height = Tile->unit_height - sy;

    if (tile_image)
	gdk_draw_image(drawable, gc, tile_image,
	  srcx + sx, srcy + sy, dest_x, dest_y, width, height);
    else if (tile_pixbuf)
	gdk_pixbuf_render_to_drawable(tile_pixbuf, drawable, gc,
	  srcx + sx, srcy + sy, dest_x, dest_y, width, height,
	  GDK_RGB_DITHER_NORMAL, 0, 0);
    else
	gdk_draw_drawable(drawable, gc, tile_pixmap,
	  srcx + sx, srcy + sy, dest_x, dest_y, width, height);
    return 1;
}

/*
 * This version of the function works when transparency is switched on
 * and each pixel is stored in an integer number of bytes.
 */

static void
x_img_tmp_draw_tile_transp_byte(int tile, int ofsx, int ofsy)
{
    int i, j, n;
    int width, height;
    int srcx, srcy;
    struct tile_transp *t = &tile_transp[tile];
    unsigned char *rle = t->data;
    unsigned int bpp = tile_image->bpp;
    unsigned char *src, *dst;
    unsigned int nx = t->nx, ny = t->ny;
#ifdef DEBUG
    if (tmp_img->bpp != bpp)
	impossible("Temp. image in different format");
#endif

    width = tmp_img->width;
    height = tmp_img->height;
#ifdef DEBUG
    if (Tile->unit_width != width || Tile->unit_height != height)
	impossible("Temp. image not unit size");
#endif
    srcx = (tile % tiles_per_row) * width;
    srcy = (tile / tiles_per_row) * height;

    src = tile_image->mem +
      (srcy + t->oy) * tile_image->bpl + (srcx + t->ox) * bpp;
    dst = tmp_img->mem + (ofsy + t->oy) * tmp_img->bpl + (ofsx + t->ox) * bpp;
    if (ofsx < 0) {
	if (ofsx + t->ox + t->nx <= 0)
	    return;		/* Nothing to draw */
	ofsx += t->ox;		/* Ignore those already skipped */
	if (ofsx < 0)
	    ofsx = -ofsx;	/* Remember how many pixels to skip */
	else
	    ofsx = 0;
    } else if (ofsx > 0) {	/* Correct dst pointer & adjust nx */
	unsigned int nxp;
	/* <---- ox ----><---- nx ----><---- ---->
	 * <----------------- width ------------->
	 *               <------ nx' ----><-ofsx->
	 */
	if (t->ox + ofsx >= width)
	    return;		/* Nothing to draw */
	nxp = width - t->ox - ofsx;
	if (nxp < nx)
	    nx = nxp;
	ofsx = 0;
    }
    if (ofsy < 0) {
	if (ofsy + t->oy + t->ny <= 0)
	    return;		/* Nothing to draw */
	ofsy += t->oy;		/* Ignore those already skipped */
	if (ofsy < 0) {
	    src -= ofsy * tile_image->bpl;
	    dst -= ofsy * tmp_img->bpl;
	    ny += ofsy;
	    while(ofsy++ < 0)	/* Skip transp data */
		for(n = 0; n < t->nx; )
		    n += *rle++;
	}
    } else if (ofsy > 0) {	/* Correct dst pointer & adjust ny */
	unsigned int nyp;
	if (t->oy + ofsy >= height)
	    return;		/* Nothing to draw */
	nyp = height - t->oy - ofsy;
	if (nyp < ny)
	    ny = nyp;
    }
    for(j = 0; j < ny; j++) {
	i = *rle++;
	while (i < ofsx) {	/* Skip ofsx pixels */
	    /* <---------------- nx ------------------>
	     * <- t1 ->...<- tn -><- on -><- tn+1 ->...
	     * <------- i ------->
	     * <------ ofsx ------><- n ->
	     */
	    n = *rle - ofsx + i;
	    if (n > 0)
		memcpy(dst + ofsx * bpp, src + ofsx * bpp, n * bpp);
	    i += *rle++;	/* Opaque */
	    if (i < nx)
		i += *rle++;	/* Transparent */
	}
	for(; i < nx; ) {
	    n = *rle;
	    if (n > nx - i)
		n = nx - i;
	    memcpy(dst + i * bpp, src + i * bpp, n * bpp);
	    i += *rle++;
	    if (i < nx)
		i += *rle++;
	}
	for(; i < t->nx; )	/* Skip any remaining transparency data */
	    i += *rle++;
#ifdef DEBUG
	if (i != t->nx) impossible("Transparency data misalign");
#endif
	src += tile_image->bpl;
	dst += tmp_img->bpl;
    }
}

/*
 * This version of the function works whenever transparency is switched on.
 */

static void
x_img_tmp_draw_tile_transp(int tile, int ofsx, int ofsy)
{
    int i, j, n;
    guint32 pixel;
    int width, height;
    int srcx, srcy;
    struct tile_transp *t = &tile_transp[tile];
    unsigned char *rle = t->data;

    width = tmp_img->width;
    height = tmp_img->height;
    srcx = (tile % tiles_per_row) * Tile->unit_width;
    srcy = (tile / tiles_per_row) * Tile->unit_height;

    for(j = t->oy; j < t->oy + t->ny; j++) {
	for(i = t->ox + *rle++; i < t->ox + t->nx; ) {
	    for(n = *rle++; n; i++, n--) {
		if (ofsx + i >= 0 && ofsx + i < width &&
		  ofsy + j >= 0 && ofsy + j < height) {
		    pixel = gdk_image_get_pixel(tile_image, srcx + i, srcy + j);
		    gdk_image_put_pixel(tmp_img, ofsx + i, ofsy + j, pixel);
		}
	    }
	    if (i < t->ox + t->nx)
		i += *rle++;
	}
#ifdef DEBUG
	if (i != t->ox + t->nx) impossible("Transparency data misalign");
#endif
    }
}

/*
 * This version of the function works when transparency is switched off.
 * It isn't used in practice, but is included for completeness.
 */

static void
x_img_tmp_draw_tile_opaque(int tile, int ofsx, int ofsy)
{
    int i, j;
    guint32 pixel;
    int width, height;
    int srcx, srcy;

    width = tmp_img->width;
    height = tmp_img->height;
    srcx = (tile % tiles_per_row) * Tile->unit_width;
    srcy = (tile / tiles_per_row) * Tile->unit_height;

    if (ofsx < 0) {
	srcx -= ofsx;
	width += ofsx;
	ofsx = 0;
    }
    if (ofsy < 0) {
	srcy -= ofsy;
	height += ofsy;
	ofsy = 0;
    }
    if (ofsx > 0)
	width -= ofsx;
    if (ofsy > 0)
	height -= ofsy;

    for(j = 0; j < height; j++)
	for(i = 0; i < width; i++) {
	    pixel = gdk_image_get_pixel(tile_image, srcx + i, srcy + j);
	    gdk_image_put_pixel(tmp_img, ofsx + i, ofsy + j, pixel);
	}
}

/*
 * This version of the function works in pixbuf mode.
 * FIXME: That's a lie! It doesn't work (only a silhouette of the
 * foreground tile is drawn).
 */

static void
x_pixbuf_tmp_draw_tile(int tile, int ofsx, int ofsy)
{
    int i, j, n;
    guint32 pixel;
    int width, height;
    int srcx, srcy;
    int dstx = ofsx, dsty = ofsy;

    width = Tile->unit_width;
    height = Tile->unit_height;
    srcx = (tile % tiles_per_row) * width;
    srcy = (tile / tiles_per_row) * height;

    if (dstx < 0) {
	srcx -= dstx;
	width += dstx;
	dstx = 0;
    }
    if (dsty < 0) {
	srcy -= dsty;
	height += dsty;
	dsty = 0;
    }
    if (dstx > 0)
	width -= dstx;
    if (dsty > 0)
	height -= dsty;

    if (!tmp_pixbuf) {
	tmp_pixbuf = gdk_pixbuf_new(gdk_pixbuf_get_colorspace(tile_pixbuf),
	  gdk_pixbuf_get_has_alpha(tile_pixbuf),
	  gdk_pixbuf_get_bits_per_sample(tile_pixbuf),
	  Tile->unit_width, Tile->unit_height);
	memset(gdk_pixbuf_get_pixels(tmp_pixbuf), 0,
	  Tile->unit_height * gdk_pixbuf_get_rowstride(tmp_pixbuf));
	gdk_pixbuf_copy_area(tile_pixbuf, srcx, srcy, width, height,
	  tmp_pixbuf, dstx, dsty);
	{
	    guchar *pixels;
	    int rowstride;
	    int n_channels;
	    guchar rgba[4];
	    g_return_if_fail(gdk_pixbuf_get_bits_per_sample(tmp_pixbuf) == 8 &&
	      gdk_pixbuf_get_colorspace(tmp_pixbuf) == GDK_COLORSPACE_RGB);
	    pixels = gdk_pixbuf_get_pixels(tmp_pixbuf);
	    rowstride = gdk_pixbuf_get_rowstride(tmp_pixbuf);
	    n_channels = gdk_pixbuf_get_n_channels(tmp_pixbuf);
	    for(j = 0; j < Tile->unit_height; j++)
		for(i = 0; i < Tile->unit_width; i++) {
		    nh_pixbuf_get_pixel(i, j, rgba);
		    rgba[3] = 255;
		    nh_pixbuf_put_pixel(i, j, rgba);
		}
	}
    }
    else
	gdk_pixbuf_composite(tile_pixbuf, tmp_pixbuf, dstx, dsty, width, height,
	  -srcx, -srcy, 1.0, 1.0, GDK_INTERP_NEAREST, 255.0);
}

/*
 * Copy tile to the temporary image at offset (ofsx, ofsy)
 */

void
x_tile_tmp_draw_tile(int tile, int ofsx, int ofsy)
{
    g_return_if_fail(tile_image != NULL || tile_pixbuf != NULL);
    g_return_if_fail(tile_pixbuf || tmp_img != NULL);
#ifdef DEBUG
    fprintf(stderr, "x_tile_tmp_draw_tile(%d, %d, %d)\n", tile, ofsx, ofsy);
#endif

    if (tile_pixbuf)
	x_pixbuf_tmp_draw_tile(tile, ofsx, ofsy);
    else if (Tile->transparent)
	if (tile_bits_per_pixel&7)
	    x_img_tmp_draw_tile_transp(tile, ofsx, ofsy);
	else
	    x_img_tmp_draw_tile_transp_byte(tile, ofsx, ofsy);
    else
	x_img_tmp_draw_tile_opaque(tile, ofsx, ofsy);
}

/*
 * Draw the temporary image to the map at position (dstx, dsty)
 */

void
x_tile_draw_tmp(int dstx, int dsty)
{
    g_return_if_fail(tmp_img != NULL || tmp_pixbuf != NULL);
    if (tmp_img)
	xshm_map_draw_image(tmp_img, 0, 0, dstx, dsty,
	  tmp_img->width, tmp_img->height);
    else
	xshm_map_draw_pixbuf(tmp_pixbuf, 0, 0,
	  dstx, dsty, Tile->unit_width, Tile->unit_height);
}

/*
 * Copy tile to the map at position (dstx, dsty)
 */

void
x_tile_draw_tile(int tile, int dstx, int dsty)
{
    int srcx, srcy;
    g_return_if_fail(tile_image != NULL || tile_pixmap != NULL);
    srcx = (tile % tiles_per_row) * Tile->unit_width;
    srcy = (tile / tiles_per_row) * Tile->unit_height;
    if (tile_image)
	xshm_map_draw_image(tile_image, srcx, srcy, dstx, dsty,
	  Tile->unit_width, Tile->unit_height);
    else
	xshm_map_draw_drawable(tile_pixmap, srcx, srcy, dstx, dsty,
	  Tile->unit_width, Tile->unit_height);
}

/* TODO: These functions need their own module */

void
nhgtk_image_draw_line(GdkImage *img, long c, int x1, int y1, int x2, int y2)
{
    int i;
    unsigned int bpp, bpl;
    g_return_if_fail(x1 == x2 || y1 == y2);
    g_return_if_fail(x1 <= x2 && y1 <= y2);
    g_return_if_fail(img != NULL);
    bpl = img->bpl;
    if (tile_bits_per_pixel&7)
	bpp = 0;
    else
	bpp = img->bpp;
    if (bpp == sizeof(unsigned long))
    {
	unsigned long *dst;
	dst = img->mem + y1 * bpl + x1 * bpp;
	if (y1 == y2)
	    for(i = x2 - x1 + 1; i; i--)
		*dst++ = (unsigned long)c;
	else
	    for(i = y2 - y1 + 1; i; i--) {
		*dst = (unsigned long)c;
		(unsigned char *)dst += bpl;
	    }
    }
    else if (bpp == sizeof(unsigned short))
    {
	unsigned short *dst;
	dst = img->mem + y1 * bpl + x1 * bpp;
	if (y1 == y2)
	    for(i = x2 - x1 + 1; i; i--)
		*dst++ = (unsigned short)c;
	else
	    for(i = y2 - y1 + 1; i; i--) {
		*dst = (unsigned short)c;
		(unsigned char *)dst += bpl;
	    }
    }
    else if (bpp == 1)
    {
	unsigned char *dst;
	dst = img->mem + y1 * bpl + x1 * bpp;
	if (y1 == y2)
	    for(i = x2 - x1 + 1; i; i--)
		*dst++ = (unsigned char)c;
	else
	    for(i = y2 - y1 + 1; i; i--) {
		*dst = (unsigned char)c;
		dst += bpl;
	    }
    }
    else
    {
	if (y1 == y2)
	    for(i = x1; i <= x2; i++)
		gdk_image_put_pixel(img, i, y1, c);
	else
	    for(i = y1; i <= y2; i++)
		gdk_image_put_pixel(img, x1, i, c);
    }
}

void
nhgtk_image_draw_rectangle(GdkImage *img, long c, int dofill, int x, int y, 
  int width, int height)
{
    int i;
    if (dofill) {
	if (width > 0)
	    for(i = 0; i < height; i++)
		nhgtk_image_draw_line(img, c, x, y + i, x + width - 1, y + i);
    }
    else if (width < 1 || height < 1) {
	if (width == 0 && height >= 0)
	    nhgtk_image_draw_line(img, c, x, y, x, y + height);
	else if (height == 0 && width >= 0)
	    nhgtk_image_draw_line(img, c, x, y, x + width, y);
    }
    else {
	nhgtk_image_draw_line(img, c, x, y, x + width, y);
	nhgtk_image_draw_line(img, c, x, y + height, x + width, y + height);
	nhgtk_image_draw_line(img, c, x, y + 1, x, y + height - 1);
	nhgtk_image_draw_line(img, c, x + width, y + 1, x + width, y + height - 1);
    }
}

void
x_tile_draw_rectangle(int dstx, int dsty, GdkColor *c)
{
    GdkRectangle rect;
    g_return_if_fail(c != NULL);
    g_return_if_fail(Tile != NULL);
    g_return_if_fail(xshm_map_pixmap != NULL || xshm_map_image != NULL);
    if (xshm_map_pixmap) {
	GdkGC *gc;
	gc = gdk_gc_new(xshm_map_pixmap);
	gdk_gc_set_foreground(gc, c);
	gdk_draw_rectangle(xshm_map_pixmap, gc, FALSE, dstx, dsty, 
	  Tile->unit_width - 1, Tile->unit_height - 1);
	gdk_gc_unref(gc);
    }
    else
	nhgtk_image_draw_rectangle(xshm_map_image, c->pixel, FALSE, dstx, dsty, 
	  Tile->unit_width, Tile->unit_height);
    rect.x = dstx;
    rect.y = dsty;
    rect.width = Tile->unit_width;
    rect.height = Tile->unit_height;
    xshm_map_draw(&rect);
}

/*
 * [ALI]
 *
 * Create the GTK interface's tileTab[] array from the game's tileset
 * array. We drop any tilesets which we can easily tell aren't supported
 * but we won't be sure until we actually try and load them.
 * Information which depends on the size of the tilemap will be completed
 * when we load the tileset.
 */

int
tile_scan(void)
{
    int i, v, visual, ch;
    int fh;
    const char *tileset = nh_option_cache_get("tileset");
#ifdef GTK_PROXY
    struct proxycb_get_tilesets_res *tilesets_res = proxy_cb_get_tilesets();
    struct proxycb_get_tilesets_res_tileset *tilesets = tilesets_res->tilesets;
    int no_tilesets = tilesets_res->n_tilesets;
#else
    dlb_init();
#endif

    v = 1;	/* Index into tileTab[] array */
    for(i=0 ; i < no_tilesets ; ++i){
	if ((tilesets[i].flags & ~(TILESET_TRANSPARENT | TILESET_PSEUDO3D)) != 0)
	    continue;	/* Unsupported flag set */
#ifdef GTK_PROXY
	if (!tilesets[i].mapfile) {
	    fprintf(stderr, "No map file specified for tileset %s\n",
	      tilesets[i].name);
	    continue;
	}
	fh = proxy_cb_dlbh_fopen(tilesets[i].mapfile, "r");
	if (fh < 0) {
	    perror(tilesets[i].mapfile);
	    continue;	/* Missing or unreadable tile map */
	}
	proxy_cb_dlbh_fclose(fh);
	fh = proxy_cb_dlbh_fopen(tilesets[i].file, "r");
#else
# ifdef FILE_AREAS
	fh = dlbh_fopen(FILE_AREA_SHARE, tilesets[i].file, RDTMODE);
# else
	fh = dlbh_fopen(tilesets[i].file, RDTMODE);
# endif
#endif	/* GTK_PROXY */
	if (fh < 0) {
	    perror(tilesets[i].file);
	    continue;	/* Missing or unreadable tile file */
	}
#ifdef GTK_PROXY
	proxy_cb_dlbh_fclose(fh);
#else
	dlbh_fclose(fh);
#endif
	tileTab[v].ident = strdup(tilesets[i].name);
	tileTab[v].file = strdup(tilesets[i].file);
#ifdef GTK_PROXY
	tileTab[v].mapfile = strdup(tilesets[i].mapfile);
#endif
	tileTab[v].transparent = !!(tilesets[i].flags & TILESET_TRANSPARENT);
	tileTab[v].spread = !!(tilesets[i].flags & TILESET_PSEUDO3D);
	v++;
    }
    no_tileTab = v - 1;
    if (!tileset[0])
	visual = 0;
    else {
	visual = -1;
	for (v = 1; v <= no_tileTab; v++){
	    if (!strcmp(tileset, tileTab[v].ident)){
		visual = v;
		break;
	    }
	}
	if (visual < 0) {
	    pline("Warning: Tile set \"%s\" not supported.", tileset);
	    if (no_tileTab > 0) {
		visual = 1;	/* Default to the first valid tile set */
		nh_option_cache_set("tileset", tileTab[visual].ident);
	    }
	    else {
		visual = 0;	/* Drop down to character mode */
		nh_option_cache_set("tileset", "");
	    }
	}
    }
#ifdef GTK_PROXY
    proxy_cb_free_tilesets(tilesets_res);
#endif
    return visual;
}
