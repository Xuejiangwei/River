#include "RiverPch.h"
#include "Renderer/Font/Header/TTF_Info.h"
#include "Renderer/Font/Header/Font.h"
#include "Renderer/Font/Header/FontAtlas.h"

#ifdef _WIN32
    #include <Windows.h>
#endif // _WIN32


RIVER_API const int FONT_ATLAS_DEFAULT_TEX_DATA_W = 122; // Actual texture will be 2 times that + 1 spacing.
RIVER_API const int FONT_ATLAS_DEFAULT_TEX_DATA_H = 27;
RIVER_API const char FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS[FONT_ATLAS_DEFAULT_TEX_DATA_W * FONT_ATLAS_DEFAULT_TEX_DATA_H + 1] =
{
    "..-         -XXXXXXX-    X    -           X           -XXXXXXX          -          XXXXXXX-     XX          - XX       XX "
    "..-         -X.....X-   X.X   -          X.X          -X.....X          -          X.....X-    X..X         -X..X     X..X"
    "---         -XXX.XXX-  X...X  -         X...X         -X....X           -           X....X-    X..X         -X...X   X...X"
    "X           -  X.X  - X.....X -        X.....X        -X...X            -            X...X-    X..X         - X...X X...X "
    "XX          -  X.X  -X.......X-       X.......X       -X..X.X           -           X.X..X-    X..X         -  X...X...X  "
    "X.X         -  X.X  -XXXX.XXXX-       XXXX.XXXX       -X.X X.X          -          X.X X.X-    X..XXX       -   X.....X   "
    "X..X        -  X.X  -   X.X   -          X.X          -XX   X.X         -         X.X   XX-    X..X..XXX    -    X...X    "
    "X...X       -  X.X  -   X.X   -    XX    X.X    XX    -      X.X        -        X.X      -    X..X..X..XX  -     X.X     "
    "X....X      -  X.X  -   X.X   -   X.X    X.X    X.X   -       X.X       -       X.X       -    X..X..X..X.X -    X...X    "
    "X.....X     -  X.X  -   X.X   -  X..X    X.X    X..X  -        X.X      -      X.X        -XXX X..X..X..X..X-   X.....X   "
    "X......X    -  X.X  -   X.X   - X...XXXXXX.XXXXXX...X -         X.X   XX-XX   X.X         -X..XX........X..X-  X...X...X  "
    "X.......X   -  X.X  -   X.X   -X.....................X-          X.X X.X-X.X X.X          -X...X...........X- X...X X...X "
    "X........X  -  X.X  -   X.X   - X...XXXXXX.XXXXXX...X -           X.X..X-X..X.X           - X..............X-X...X   X...X"
    "X.........X -XXX.XXX-   X.X   -  X..X    X.X    X..X  -            X...X-X...X            -  X.............X-X..X     X..X"
    "X..........X-X.....X-   X.X   -   X.X    X.X    X.X   -           X....X-X....X           -  X.............X- XX       XX "
    "X......XXXXX-XXXXXXX-   X.X   -    XX    X.X    XX    -          X.....X-X.....X          -   X............X--------------"
    "X...X..X    ---------   X.X   -          X.X          -          XXXXXXX-XXXXXXX          -   X...........X -             "
    "X..X X..X   -       -XXXX.XXXX-       XXXX.XXXX       -------------------------------------    X..........X -             "
    "X.X  X..X   -       -X.......X-       X.......X       -    XX           XX    -           -    X..........X -             "
    "XX    X..X  -       - X.....X -        X.....X        -   X.X           X.X   -           -     X........X  -             "
    "      X..X  -       -  X...X  -         X...X         -  X..X           X..X  -           -     X........X  -             "
    "       XX   -       -   X.X   -          X.X          - X...XXXXXXXXXXXXX...X -           -     XXXXXXXXXX  -             "
    "-------------       -    X    -           X           -X.....................X-           -------------------             "
    "                    ----------------------------------- X...XXXXXXXXXXXXX...X -                                           "
    "                                                      -  X..X           X..X  -                                           "
    "                                                      -   X.X           X.X   -                                           "
    "                                                      -    XX           XX    -                                           "
};

#define ttCHAR(p)     (* (int8 *) (p))
#define STBTT__NOTUSED(v)  (void)(v)
#define STBRP__NOTUSED(v)  (void)(v)
#define STBRP_ASSERT(v) assert(v)
#define STBRP__MAXVAL  0x7fffffff
#define STBTT__CSCTX_INIT(bounds) {bounds,0, 0,0, 0,0, 0,0,0,0, NULL, 0}
#define STBTT__OVER_MASK  (STBTT_MAX_OVERSAMPLE-1)
#define STBTT_MAX_OVERSAMPLE   8
#define STBTT_fabs fabs

#define IM_ALLOC(_SIZE)                     malloc(_SIZE)
#define IM_FREE(_PTR)                      free(_PTR)
#define STBTT_malloc(x,u)   ((void)(u), IM_ALLOC(x))
#define STBTT_free(x,u)     ((void)(u), IM_FREE(x))
#define IM_ASSERT(_EXPR)            assert(_EXPR) 
#define STBTT_assert(x)  do { IM_ASSERT(x); } while(0)
#define STBTT_memset       memset
#define STBTT__COMPARE(a,b)  ((a)->y0 < (b)->y0)


static uint8 stbtt__buf_get8(stbtt__buf* b)
{
    if (b->cursor >= b->size)
        return 0;
    return b->data[b->cursor++];
}
static uint32 stbtt__buf_get(stbtt__buf* b, int n)
{
    uint32 v = 0;
    int i;
    assert(n >= 1 && n <= 4);
    for (i = 0; i < n; i++)
        v = (v << 8) | stbtt__buf_get8(b);
    return v;
}

static void stbtt__buf_seek(stbtt__buf* b, int o)
{
    assert(!(o > b->size || o < 0));
    b->cursor = (o > b->size || o < 0) ? b->size : o;
}


static void stbtt__buf_skip(stbtt__buf* b, int o)
{
    stbtt__buf_seek(b, b->cursor + o);
}

#define stbtt__buf_get16(b)  stbtt__buf_get((b), 2)
#define stbtt__buf_get32(b)  stbtt__buf_get((b), 4)

enum 
{ // platformID
    STBTT_PLATFORM_ID_UNICODE = 0,
    STBTT_PLATFORM_ID_MAC = 1,
    STBTT_PLATFORM_ID_ISO = 2,
    STBTT_PLATFORM_ID_MICROSOFT = 3
};

enum
{ // encodingID for STBTT_PLATFORM_ID_MICROSOFT
    STBTT_MS_EID_SYMBOL = 0,
    STBTT_MS_EID_UNICODE_BMP = 1,
    STBTT_MS_EID_SHIFTJIS = 2,
    STBTT_MS_EID_UNICODE_FULL = 10
};

enum
{
    STBRP_HEURISTIC_Skyline_default = 0,
    STBRP_HEURISTIC_Skyline_BL_sortHeight = STBRP_HEURISTIC_Skyline_default,
    STBRP_HEURISTIC_Skyline_BF_sortHeight
};

enum {
    STBTT_vmove = 1,
    STBTT_vline,
    STBTT_vcurve,
    STBTT_vcubic
};

#define ttBYTE(p)     (* (uint8_t *) (p))
#define stbtt_tag4(p,c0,c1,c2,c3) ((p)[0] == (c0) && (p)[1] == (c1) && (p)[2] == (c2) && (p)[3] == (c3))
#define stbtt_tag(p,str) stbtt_tag4(p,str[0],str[1],str[2],str[3])

static int rect_height_compare(const void* a, const void* b)
{
    const stbrp_rect* p = (const stbrp_rect*)a;
    const stbrp_rect* q = (const stbrp_rect*)b;
    if (p->h > q->h)
        return -1;
    if (p->h < q->h)
        return  1;
    return (p->w > q->w) ? -1 : (p->w < q->w);
}

static int rect_original_order(const void* a, const void* b)
{
    const stbrp_rect* p = (const stbrp_rect*)a;
    const stbrp_rect* q = (const stbrp_rect*)b;
    return (p->was_packed < q->was_packed) ? -1 : (p->was_packed > q->was_packed);
}

static stbtt__buf stbtt__new_buf(const void* p, size_t size)
{
    stbtt__buf r;
    assert(size < 0x40000000);
    r.data = (uint8*)p;
    r.size = (int)size;
    r.cursor = 0;
    return r;
}

static stbtt__buf stbtt__buf_range(const stbtt__buf* b, int o, int s)
{
    stbtt__buf r = stbtt__new_buf(NULL, 0);
    if (o < 0 || s < 0 || o > b->size || s > b->size - o) return r;
    r.data = b->data + o;
    r.size = s;
    return r;
}

static stbtt__buf stbtt__cff_index_get(stbtt__buf b, int i)
{
    int count, offsize, start, end;
    stbtt__buf_seek(&b, 0);
    count = stbtt__buf_get16(&b);
    offsize = stbtt__buf_get8(&b);
    assert(i >= 0 && i < count);
    assert(offsize >= 1 && offsize <= 4);
    stbtt__buf_skip(&b, i * offsize);
    start = stbtt__buf_get(&b, offsize);
    end = stbtt__buf_get(&b, offsize);
    return stbtt__buf_range(&b, 2 + (count + 1) * offsize + start, end - start);
}

static void stbtt__track_vertex(stbtt__csctx* c, int x, int y)
{
    if (x > c->max_x || !c->started) c->max_x = x;
    if (y > c->max_y || !c->started) c->max_y = y;
    if (x < c->min_x || !c->started) c->min_x = x;
    if (y < c->min_y || !c->started) c->min_y = y;
    c->started = 1;
}

static void stbtt_setvertex(stbtt_vertex* v, uint8 type, int x, int y, int cx, int cy)
{
    v->type = type;
    v->x = (int16)x;
    v->y = (int16)y;
    v->cx = (int16)cx;
    v->cy = (int16)cy;
}

static void stbtt__csctx_v(stbtt__csctx* c, uint8 type, int x, int y, int cx, int cy, int cx1, int cy1)
{
    if (c->bounds) {
        stbtt__track_vertex(c, x, y);
        if (type == STBTT_vcubic) {
            stbtt__track_vertex(c, cx, cy);
            stbtt__track_vertex(c, cx1, cy1);
        }
    }
    else {
        stbtt_setvertex(&c->pvertices[c->num_vertices], type, x, y, cx, cy);
        c->pvertices[c->num_vertices].cx1 = (int16)cx1;
        c->pvertices[c->num_vertices].cy1 = (int16)cy1;
    }
    c->num_vertices++;
}

static void stbtt__csctx_close_shape(stbtt__csctx* ctx)
{
    if (ctx->first_x != ctx->x || ctx->first_y != ctx->y)
        stbtt__csctx_v(ctx, STBTT_vline, (int)ctx->first_x, (int)ctx->first_y, 0, 0, 0, 0);
}

static void stbtt__csctx_rmove_to(stbtt__csctx* ctx, float dx, float dy)
{
    stbtt__csctx_close_shape(ctx);
    ctx->first_x = ctx->x = ctx->x + dx;
    ctx->first_y = ctx->y = ctx->y + dy;
    stbtt__csctx_v(ctx, STBTT_vmove, (int)ctx->x, (int)ctx->y, 0, 0, 0, 0);
}

static void stbtt__csctx_rline_to(stbtt__csctx* ctx, float dx, float dy)
{
    ctx->x += dx;
    ctx->y += dy;
    stbtt__csctx_v(ctx, STBTT_vline, (int)ctx->x, (int)ctx->y, 0, 0, 0, 0);
}

static uint32 stbtt__cff_int(stbtt__buf* b)
{
    int b0 = stbtt__buf_get8(b);
    if (b0 >= 32 && b0 <= 246)       return b0 - 139;
    else if (b0 >= 247 && b0 <= 250) return (b0 - 247) * 256 + stbtt__buf_get8(b) + 108;
    else if (b0 >= 251 && b0 <= 254) return -(b0 - 251) * 256 - stbtt__buf_get8(b) - 108;
    else if (b0 == 28)               return stbtt__buf_get16(b);
    else if (b0 == 29)               return stbtt__buf_get32(b);
    assert(0);
    return 0;
}

static int stbtt__cff_index_count(stbtt__buf* b)
{
    stbtt__buf_seek(b, 0);
    return stbtt__buf_get16(b);
}

static stbtt__buf stbtt__get_subr(stbtt__buf idx, int n)
{
    int count = stbtt__cff_index_count(&idx);
    int bias = 107;
    if (count >= 33900)
        bias = 32768;
    else if (count >= 1240)
        bias = 1131;
    n += bias;
    if (n < 0 || n >= count)
        return stbtt__new_buf(NULL, 0);
    return stbtt__cff_index_get(idx, n);
}


uint8 stbtt__buf_peek8(stbtt__buf* b)
{
    if (b->cursor >= b->size)
        return 0;
    return b->data[b->cursor];
}

static void stbtt__cff_skip_operand(stbtt__buf* b) {
    int v, b0 = stbtt__buf_peek8(b);
    assert(b0 >= 28);
    if (b0 == 30) {
        stbtt__buf_skip(b, 1);
        while (b->cursor < b->size) {
            v = stbtt__buf_get8(b);
            if ((v & 0xF) == 0xF || (v >> 4) == 0xF)
                break;
        }
    }
    else {
        stbtt__cff_int(b);
    }
}

static stbtt__buf stbtt__dict_get(stbtt__buf* b, int key)
{
    stbtt__buf_seek(b, 0);
    while (b->cursor < b->size) {
        int start = b->cursor, end, op;
        while (stbtt__buf_peek8(b) >= 28)
            stbtt__cff_skip_operand(b);
        end = b->cursor;
        op = stbtt__buf_get8(b);
        if (op == 12)  op = stbtt__buf_get8(b) | 0x100;
        if (op == key) return stbtt__buf_range(b, start, end - start);
    }
    return stbtt__buf_range(b, 0, 0);
}

static void stbtt__dict_get_ints(stbtt__buf* b, int key, int outcount, uint32* out)
{
    int i;
    stbtt__buf operands = stbtt__dict_get(b, key);
    for (i = 0; i < outcount && operands.cursor < operands.size; i++)
        out[i] = stbtt__cff_int(&operands);
}

static stbtt__buf stbtt__cff_get_index(stbtt__buf* b)
{
    int count, start, offsize;
    start = b->cursor;
    count = stbtt__buf_get16(b);
    if (count) {
        offsize = stbtt__buf_get8(b);
        assert(offsize >= 1 && offsize <= 4);
        stbtt__buf_skip(b, offsize * count);
        stbtt__buf_skip(b, stbtt__buf_get(b, offsize) - 1);
    }
    return stbtt__buf_range(b, start, b->cursor - start);
}

static stbtt__buf stbtt__get_subrs(stbtt__buf cff, stbtt__buf fontdict)
{
    uint32 subrsoff = 0, private_loc[2] = { 0, 0 };
    stbtt__buf pdict;
    stbtt__dict_get_ints(&fontdict, 18, 2, private_loc);
    if (!private_loc[1] || !private_loc[0]) return stbtt__new_buf(NULL, 0);
    pdict = stbtt__buf_range(&cff, private_loc[1], private_loc[0]);
    stbtt__dict_get_ints(&pdict, 19, 1, &subrsoff);
    if (!subrsoff) return stbtt__new_buf(NULL, 0);
    stbtt__buf_seek(&cff, private_loc[1] + subrsoff);
    return stbtt__cff_get_index(&cff);
}

static void stbtt__csctx_rccurve_to(stbtt__csctx* ctx, float dx1, float dy1, float dx2, float dy2, float dx3, float dy3)
{
    float cx1 = ctx->x + dx1;
    float cy1 = ctx->y + dy1;
    float cx2 = cx1 + dx2;
    float cy2 = cy1 + dy2;
    ctx->x = cx2 + dx3;
    ctx->y = cy2 + dy3;
    stbtt__csctx_v(ctx, STBTT_vcubic, (int)ctx->x, (int)ctx->y, (int)cx1, (int)cy1, (int)cx2, (int)cy2);
}

static float stbtt__sized_triangle_area(float height, float width)
{
    return height * width / 2;
}

static float stbtt__sized_trapezoid_area(float height, float top_width, float bottom_width)
{
    STBTT_assert(top_width >= 0);
    STBTT_assert(bottom_width >= 0);
    return (top_width + bottom_width) / 2.0f * height;
}

static float stbtt__position_trapezoid_area(float height, float tx0, float tx1, float bx0, float bx1)
{
    return stbtt__sized_trapezoid_area(height, tx1 - tx0, bx1 - bx0);
}


static void stbtt__v_prefilter(unsigned char* pixels, int w, int h, int stride_in_bytes, unsigned int kernel_width)
{
    unsigned char buffer[STBTT_MAX_OVERSAMPLE];
    int safe_h = h - kernel_width;
    int j;
    STBTT_memset(buffer, 0, STBTT_MAX_OVERSAMPLE); // suppress bogus warning from VS2013 -analyze
    for (j = 0; j < w; ++j) {
        int i;
        unsigned int total;
        STBTT_memset(buffer, 0, kernel_width);

        total = 0;

        // make kernel_width a constant in common cases so compiler can optimize out the divide
        switch (kernel_width) {
        case 2:
            for (i = 0; i <= safe_h; ++i) {
                total += pixels[i * stride_in_bytes] - buffer[i & STBTT__OVER_MASK];
                buffer[(i + kernel_width) & STBTT__OVER_MASK] = pixels[i * stride_in_bytes];
                pixels[i * stride_in_bytes] = (unsigned char)(total / 2);
            }
            break;
        case 3:
            for (i = 0; i <= safe_h; ++i) {
                total += pixels[i * stride_in_bytes] - buffer[i & STBTT__OVER_MASK];
                buffer[(i + kernel_width) & STBTT__OVER_MASK] = pixels[i * stride_in_bytes];
                pixels[i * stride_in_bytes] = (unsigned char)(total / 3);
            }
            break;
        case 4:
            for (i = 0; i <= safe_h; ++i) {
                total += pixels[i * stride_in_bytes] - buffer[i & STBTT__OVER_MASK];
                buffer[(i + kernel_width) & STBTT__OVER_MASK] = pixels[i * stride_in_bytes];
                pixels[i * stride_in_bytes] = (unsigned char)(total / 4);
            }
            break;
        case 5:
            for (i = 0; i <= safe_h; ++i) {
                total += pixels[i * stride_in_bytes] - buffer[i & STBTT__OVER_MASK];
                buffer[(i + kernel_width) & STBTT__OVER_MASK] = pixels[i * stride_in_bytes];
                pixels[i * stride_in_bytes] = (unsigned char)(total / 5);
            }
            break;
        default:
            for (i = 0; i <= safe_h; ++i) {
                total += pixels[i * stride_in_bytes] - buffer[i & STBTT__OVER_MASK];
                buffer[(i + kernel_width) & STBTT__OVER_MASK] = pixels[i * stride_in_bytes];
                pixels[i * stride_in_bytes] = (unsigned char)(total / kernel_width);
            }
            break;
        }

        for (; i < h; ++i) {
            STBTT_assert(pixels[i * stride_in_bytes] == 0);
            total -= buffer[i & STBTT__OVER_MASK];
            pixels[i * stride_in_bytes] = (unsigned char)(total / kernel_width);
        }

        pixels += 1;
    }
}

static void stbtt__hheap_cleanup(stbtt__hheap* hh, void* userdata)
{
    stbtt__hheap_chunk* c = hh->head;
    while (c) {
        stbtt__hheap_chunk* n = c->next;
        STBTT_free(c, userdata);
        c = n;
    }
}

static stbtt__buf stbtt__cid_get_glyph_subrs(const TTF_HeadInfo* info, int glyph_index)
{
    stbtt__buf fdselect = info->fdselect;
    int nranges, start, end, v, fmt, fdselector = -1, i;

    stbtt__buf_seek(&fdselect, 0);
    fmt = stbtt__buf_get8(&fdselect);
    if (fmt == 0) {
        // untested
        stbtt__buf_skip(&fdselect, glyph_index);
        fdselector = stbtt__buf_get8(&fdselect);
    }
    else if (fmt == 3) {
        nranges = stbtt__buf_get16(&fdselect);
        start = stbtt__buf_get16(&fdselect);
        for (i = 0; i < nranges; i++) {
            v = stbtt__buf_get8(&fdselect);
            end = stbtt__buf_get16(&fdselect);
            if (glyph_index >= start && glyph_index < end) {
                fdselector = v;
                break;
            }
            start = end;
        }
    }
    if (fdselector == -1) return stbtt__new_buf(NULL, 0); // [DEAR IMGUI] fixed, see #6007 and nothings/stb#1422
    return stbtt__get_subrs(info->cff, stbtt__cff_index_get(info->fontdicts, fdselector));
}

static void stbtt__h_prefilter(unsigned char* pixels, int w, int h, int stride_in_bytes, unsigned int kernel_width)
{
    unsigned char buffer[STBTT_MAX_OVERSAMPLE];
    int safe_w = w - kernel_width;
    int j;
    STBTT_memset(buffer, 0, STBTT_MAX_OVERSAMPLE); // suppress bogus warning from VS2013 -analyze
    for (j = 0; j < h; ++j) {
        int i;
        unsigned int total;
        STBTT_memset(buffer, 0, kernel_width);

        total = 0;

        // make kernel_width a constant in common cases so compiler can optimize out the divide
        switch (kernel_width) {
        case 2:
            for (i = 0; i <= safe_w; ++i) {
                total += pixels[i] - buffer[i & STBTT__OVER_MASK];
                buffer[(i + kernel_width) & STBTT__OVER_MASK] = pixels[i];
                pixels[i] = (unsigned char)(total / 2);
            }
            break;
        case 3:
            for (i = 0; i <= safe_w; ++i) {
                total += pixels[i] - buffer[i & STBTT__OVER_MASK];
                buffer[(i + kernel_width) & STBTT__OVER_MASK] = pixels[i];
                pixels[i] = (unsigned char)(total / 3);
            }
            break;
        case 4:
            for (i = 0; i <= safe_w; ++i) {
                total += pixels[i] - buffer[i & STBTT__OVER_MASK];
                buffer[(i + kernel_width) & STBTT__OVER_MASK] = pixels[i];
                pixels[i] = (unsigned char)(total / 4);
            }
            break;
        case 5:
            for (i = 0; i <= safe_w; ++i) {
                total += pixels[i] - buffer[i & STBTT__OVER_MASK];
                buffer[(i + kernel_width) & STBTT__OVER_MASK] = pixels[i];
                pixels[i] = (unsigned char)(total / 5);
            }
            break;
        default:
            for (i = 0; i <= safe_w; ++i) {
                total += pixels[i] - buffer[i & STBTT__OVER_MASK];
                buffer[(i + kernel_width) & STBTT__OVER_MASK] = pixels[i];
                pixels[i] = (unsigned char)(total / kernel_width);
            }
            break;
        }

        for (; i < w; ++i) {
            STBTT_assert(pixels[i] == 0);
            total -= buffer[i & STBTT__OVER_MASK];
            pixels[i] = (unsigned char)(total / kernel_width);
        }

        pixels += stride_in_bytes;
    }
}

static int stbtt__run_charstring(const TTF_HeadInfo* info, int glyph_index, stbtt__csctx* c)
{
    int in_header = 1, maskbits = 0, subr_stack_height = 0, sp = 0, v, i, b0;
    int has_subrs = 0, clear_stack;
    float s[48];
    stbtt__buf subr_stack[10], subrs = info->subrs, b;
    float f;

#define STBTT__CSERR(s) (0)

    // this currently ignores the initial width value, which isn't needed if we have hmtx
    b = stbtt__cff_index_get(info->charstrings, glyph_index);
    while (b.cursor < b.size) {
        i = 0;
        clear_stack = 1;
        b0 = stbtt__buf_get8(&b);
        switch (b0) {
            // @TODO implement hinting
        case 0x13: // hintmask
        case 0x14: // cntrmask
            if (in_header)
                maskbits += (sp / 2); // implicit "vstem"
            in_header = 0;
            stbtt__buf_skip(&b, (maskbits + 7) / 8);
            break;

        case 0x01: // hstem
        case 0x03: // vstem
        case 0x12: // hstemhm
        case 0x17: // vstemhm
            maskbits += (sp / 2);
            break;

        case 0x15: // rmoveto
            in_header = 0;
            if (sp < 2) return STBTT__CSERR("rmoveto stack");
            stbtt__csctx_rmove_to(c, s[sp - 2], s[sp - 1]);
            break;
        case 0x04: // vmoveto
            in_header = 0;
            if (sp < 1) return STBTT__CSERR("vmoveto stack");
            stbtt__csctx_rmove_to(c, 0, s[sp - 1]);
            break;
        case 0x16: // hmoveto
            in_header = 0;
            if (sp < 1) return STBTT__CSERR("hmoveto stack");
            stbtt__csctx_rmove_to(c, s[sp - 1], 0);
            break;

        case 0x05: // rlineto
            if (sp < 2) return STBTT__CSERR("rlineto stack");
            for (; i + 1 < sp; i += 2)
                stbtt__csctx_rline_to(c, s[i], s[i + 1]);
            break;

            // hlineto/vlineto and vhcurveto/hvcurveto alternate horizontal and vertical
            // starting from a different place.

        case 0x07: // vlineto
            if (sp < 1) return STBTT__CSERR("vlineto stack");
            goto vlineto;
        case 0x06: // hlineto
            if (sp < 1) return STBTT__CSERR("hlineto stack");
            for (;;) {
                if (i >= sp) break;
                stbtt__csctx_rline_to(c, s[i], 0);
                i++;
            vlineto:
                if (i >= sp) break;
                stbtt__csctx_rline_to(c, 0, s[i]);
                i++;
            }
            break;

        case 0x1F: // hvcurveto
            if (sp < 4) return STBTT__CSERR("hvcurveto stack");
            goto hvcurveto;
        case 0x1E: // vhcurveto
            if (sp < 4) return STBTT__CSERR("vhcurveto stack");
            for (;;) {
                if (i + 3 >= sp) break;
                stbtt__csctx_rccurve_to(c, 0, s[i], s[i + 1], s[i + 2], s[i + 3], (sp - i == 5) ? s[i + 4] : 0.0f);
                i += 4;
            hvcurveto:
                if (i + 3 >= sp) break;
                stbtt__csctx_rccurve_to(c, s[i], 0, s[i + 1], s[i + 2], (sp - i == 5) ? s[i + 4] : 0.0f, s[i + 3]);
                i += 4;
            }
            break;

        case 0x08: // rrcurveto
            if (sp < 6) return STBTT__CSERR("rcurveline stack");
            for (; i + 5 < sp; i += 6)
                stbtt__csctx_rccurve_to(c, s[i], s[i + 1], s[i + 2], s[i + 3], s[i + 4], s[i + 5]);
            break;

        case 0x18: // rcurveline
            if (sp < 8) return STBTT__CSERR("rcurveline stack");
            for (; i + 5 < sp - 2; i += 6)
                stbtt__csctx_rccurve_to(c, s[i], s[i + 1], s[i + 2], s[i + 3], s[i + 4], s[i + 5]);
            if (i + 1 >= sp) return STBTT__CSERR("rcurveline stack");
            stbtt__csctx_rline_to(c, s[i], s[i + 1]);
            break;

        case 0x19: // rlinecurve
            if (sp < 8) return STBTT__CSERR("rlinecurve stack");
            for (; i + 1 < sp - 6; i += 2)
                stbtt__csctx_rline_to(c, s[i], s[i + 1]);
            if (i + 5 >= sp) return STBTT__CSERR("rlinecurve stack");
            stbtt__csctx_rccurve_to(c, s[i], s[i + 1], s[i + 2], s[i + 3], s[i + 4], s[i + 5]);
            break;

        case 0x1A: // vvcurveto
        case 0x1B: // hhcurveto
            if (sp < 4) return STBTT__CSERR("(vv|hh)curveto stack");
            f = 0.0;
            if (sp & 1) { f = s[i]; i++; }
            for (; i + 3 < sp; i += 4) {
                if (b0 == 0x1B)
                    stbtt__csctx_rccurve_to(c, s[i], f, s[i + 1], s[i + 2], s[i + 3], 0.0);
                else
                    stbtt__csctx_rccurve_to(c, f, s[i], s[i + 1], s[i + 2], 0.0, s[i + 3]);
                f = 0.0;
            }
            break;

        case 0x0A: // callsubr
            if (!has_subrs) {
                if (info->fdselect.size)
                    subrs = stbtt__cid_get_glyph_subrs(info, glyph_index);
                has_subrs = 1;
            }
            // FALLTHROUGH
        case 0x1D: // callgsubr
            if (sp < 1) return STBTT__CSERR("call(g|)subr stack");
            v = (int)s[--sp];
            if (subr_stack_height >= 10) return STBTT__CSERR("recursion limit");
            subr_stack[subr_stack_height++] = b;
            b = stbtt__get_subr(b0 == 0x0A ? subrs : info->gsubrs, v);
            if (b.size == 0) return STBTT__CSERR("subr not found");
            b.cursor = 0;
            clear_stack = 0;
            break;

        case 0x0B: // return
            if (subr_stack_height <= 0) return STBTT__CSERR("return outside subr");
            b = subr_stack[--subr_stack_height];
            clear_stack = 0;
            break;

        case 0x0E: // endchar
            stbtt__csctx_close_shape(c);
            return 1;

        case 0x0C: { // two-byte escape
            float dx1, dx2, dx3, dx4, dx5, dx6, dy1, dy2, dy3, dy4, dy5, dy6;
            float dx, dy;
            int b1 = stbtt__buf_get8(&b);
            switch (b1) {
                // @TODO These "flex" implementations ignore the flex-depth and resolution,
                // and always draw beziers.
            case 0x22: // hflex
                if (sp < 7) return STBTT__CSERR("hflex stack");
                dx1 = s[0];
                dx2 = s[1];
                dy2 = s[2];
                dx3 = s[3];
                dx4 = s[4];
                dx5 = s[5];
                dx6 = s[6];
                stbtt__csctx_rccurve_to(c, dx1, 0, dx2, dy2, dx3, 0);
                stbtt__csctx_rccurve_to(c, dx4, 0, dx5, -dy2, dx6, 0);
                break;

            case 0x23: // flex
                if (sp < 13) return STBTT__CSERR("flex stack");
                dx1 = s[0];
                dy1 = s[1];
                dx2 = s[2];
                dy2 = s[3];
                dx3 = s[4];
                dy3 = s[5];
                dx4 = s[6];
                dy4 = s[7];
                dx5 = s[8];
                dy5 = s[9];
                dx6 = s[10];
                dy6 = s[11];
                //fd is s[12]
                stbtt__csctx_rccurve_to(c, dx1, dy1, dx2, dy2, dx3, dy3);
                stbtt__csctx_rccurve_to(c, dx4, dy4, dx5, dy5, dx6, dy6);
                break;

            case 0x24: // hflex1
                if (sp < 9) return STBTT__CSERR("hflex1 stack");
                dx1 = s[0];
                dy1 = s[1];
                dx2 = s[2];
                dy2 = s[3];
                dx3 = s[4];
                dx4 = s[5];
                dx5 = s[6];
                dy5 = s[7];
                dx6 = s[8];
                stbtt__csctx_rccurve_to(c, dx1, dy1, dx2, dy2, dx3, 0);
                stbtt__csctx_rccurve_to(c, dx4, 0, dx5, dy5, dx6, -(dy1 + dy2 + dy5));
                break;

            case 0x25: // flex1
                if (sp < 11) return STBTT__CSERR("flex1 stack");
                dx1 = s[0];
                dy1 = s[1];
                dx2 = s[2];
                dy2 = s[3];
                dx3 = s[4];
                dy3 = s[5];
                dx4 = s[6];
                dy4 = s[7];
                dx5 = s[8];
                dy5 = s[9];
                dx6 = dy6 = s[10];
                dx = dx1 + dx2 + dx3 + dx4 + dx5;
                dy = dy1 + dy2 + dy3 + dy4 + dy5;
                if (fabs(dx) > fabs(dy))
                    dy6 = -dy;
                else
                    dx6 = -dx;
                stbtt__csctx_rccurve_to(c, dx1, dy1, dx2, dy2, dx3, dy3);
                stbtt__csctx_rccurve_to(c, dx4, dy4, dx5, dy5, dx6, dy6);
                break;

            default:
                return STBTT__CSERR("unimplemented");
            }
        } break;

        default:
            if (b0 != 255 && b0 != 28 && b0 < 32)
                return STBTT__CSERR("reserved operator");

            // push immediate
            if (b0 == 255) {
                f = (float)(int)stbtt__buf_get32(&b) / 0x10000;
            }
            else {
                stbtt__buf_skip(&b, -1);
                f = (float)(int16)stbtt__cff_int(&b);
            }
            if (sp >= 48) return STBTT__CSERR("push stack overflow");
            s[sp++] = f;
            clear_stack = 0;
            break;
        }
        if (clear_stack) sp = 0;
    }
    return STBTT__CSERR("no endchar");

#undef STBTT__CSERR
}

static void stbtt__sort_edges_quicksort(stbtt__edge* p, int n)
{
    /* threshold for transitioning to insertion sort */
    while (n > 12) {
        stbtt__edge t;
        int c01, c12, c, m, i, j;

        /* compute median of three */
        m = n >> 1;
        c01 = STBTT__COMPARE(&p[0], &p[m]);
        c12 = STBTT__COMPARE(&p[m], &p[n - 1]);
        /* if 0 >= mid >= end, or 0 < mid < end, then use mid */
        if (c01 != c12) {
            /* otherwise, we'll need to swap something else to middle */
            int z;
            c = STBTT__COMPARE(&p[0], &p[n - 1]);
            /* 0>mid && mid<n:  0>n => n; 0<n => 0 */
            /* 0<mid && mid>n:  0>n => 0; 0<n => n */
            z = (c == c12) ? 0 : n - 1;
            t = p[z];
            p[z] = p[m];
            p[m] = t;
        }
        /* now p[m] is the median-of-three */
        /* swap it to the beginning so it won't move around */
        t = p[0];
        p[0] = p[m];
        p[m] = t;

        /* partition loop */
        i = 1;
        j = n - 1;
        for (;;) {
            /* handling of equality is crucial here */
            /* for sentinels & efficiency with duplicates */
            for (;; ++i) {
                if (!STBTT__COMPARE(&p[i], &p[0])) break;
            }
            for (;; --j) {
                if (!STBTT__COMPARE(&p[0], &p[j])) break;
            }
            /* make sure we haven't crossed */
            if (i >= j) break;
            t = p[i];
            p[i] = p[j];
            p[j] = t;

            ++i;
            --j;
        }
        /* recurse on smaller side, iterate on larger */
        if (j < (n - i)) {
            stbtt__sort_edges_quicksort(p, j);
            p = p + i;
            n = n - i;
        }
        else {
            stbtt__sort_edges_quicksort(p + i, n - i);
            n = j;
        }
    }
}

static void stbtt__sort_edges_ins_sort(stbtt__edge* p, int n)
{
    int i, j;
    for (i = 1; i < n; ++i) {
        stbtt__edge t = p[i], * a = &t;
        j = i;
        while (j > 0) {
            stbtt__edge* b = &p[j - 1];
            int c = STBTT__COMPARE(a, b);
            if (!c) break;
            p[j] = p[j - 1];
            --j;
        }
        if (i != j)
            p[j] = t;
    }
}

static void stbtt__sort_edges(stbtt__edge* p, int n)
{
    stbtt__sort_edges_quicksort(p, n);
    stbtt__sort_edges_ins_sort(p, n);
}

static void stbtt__hheap_free(stbtt__hheap* hh, void* p)
{
    *(void**)p = hh->first_free;
    hh->first_free = p;
}

static void* stbtt__hheap_alloc(stbtt__hheap* hh, size_t size, void* userdata)
{
    if (hh->first_free) {
        void* p = hh->first_free;
        hh->first_free = *(void**)p;
        return p;
    }
    else {
        if (hh->num_remaining_in_head_chunk == 0) {
            int count = (size < 32 ? 2000 : size < 128 ? 800 : 100);
            stbtt__hheap_chunk* c = (stbtt__hheap_chunk*)STBTT_malloc(sizeof(stbtt__hheap_chunk) + size * count, userdata);
            if (c == NULL)
                return NULL;
            c->next = hh->head;
            hh->head = c;
            hh->num_remaining_in_head_chunk = count;
        }
        --hh->num_remaining_in_head_chunk;
        return (char*)(hh->head) + sizeof(stbtt__hheap_chunk) + size * hh->num_remaining_in_head_chunk;
    }
}

static stbtt__active_edge* stbtt__new_active(stbtt__hheap* hh, stbtt__edge* e, int off_x, float start_point, void* userdata)
{
    stbtt__active_edge* z = (stbtt__active_edge*)stbtt__hheap_alloc(hh, sizeof(*z), userdata);
    float dxdy = (e->x1 - e->x0) / (e->y1 - e->y0);
    STBTT_assert(z != NULL);
    //STBTT_assert(e->y0 <= start_point);
    if (!z) return z;
    z->fdx = dxdy;
    z->fdy = dxdy != 0.0f ? (1.0f / dxdy) : 0.0f;
    z->fx = e->x0 + dxdy * (start_point - e->y0);
    z->fx -= off_x;
    z->direction = e->invert ? 1.0f : -1.0f;
    z->sy = e->y0;
    z->ey = e->y1;
    z->next = 0;
    return z;
}

static void stbtt__handle_clipped_edge(float* scanline, int x, stbtt__active_edge* e, float x0, float y0, float x1, float y1)
{
    if (y0 == y1) return;
    STBTT_assert(y0 < y1);
    STBTT_assert(e->sy <= e->ey);
    if (y0 > e->ey) return;
    if (y1 < e->sy) return;
    if (y0 < e->sy) {
        x0 += (x1 - x0) * (e->sy - y0) / (y1 - y0);
        y0 = e->sy;
    }
    if (y1 > e->ey) {
        x1 += (x1 - x0) * (e->ey - y1) / (y1 - y0);
        y1 = e->ey;
    }

    if (x0 == x)
        STBTT_assert(x1 <= x + 1);
    else if (x0 == x + 1)
        STBTT_assert(x1 >= x);
    else if (x0 <= x)
        STBTT_assert(x1 <= x);
    else if (x0 >= x + 1)
        STBTT_assert(x1 >= x + 1);
    else
        STBTT_assert(x1 >= x && x1 <= x + 1);

    if (x0 <= x && x1 <= x)
        scanline[x] += e->direction * (y1 - y0);
    else if (x0 >= x + 1 && x1 >= x + 1)
        ;
    else {
        STBTT_assert(x0 >= x && x0 <= x + 1 && x1 >= x && x1 <= x + 1);
        scanline[x] += e->direction * (y1 - y0) * (1 - ((x0 - x) + (x1 - x)) / 2); // coverage = 1 - average x position
    }
}

static void stbtt__fill_active_edges_new(float* scanline, float* scanline_fill, int len, stbtt__active_edge* e, float y_top)
{
    float y_bottom = y_top + 1;

    while (e) {
        // brute force every pixel

        // compute intersection points with top & bottom
        STBTT_assert(e->ey >= y_top);

        if (e->fdx == 0) {
            float x0 = e->fx;
            if (x0 < len) {
                if (x0 >= 0) {
                    stbtt__handle_clipped_edge(scanline, (int)x0, e, x0, y_top, x0, y_bottom);
                    stbtt__handle_clipped_edge(scanline_fill - 1, (int)x0 + 1, e, x0, y_top, x0, y_bottom);
                }
                else {
                    stbtt__handle_clipped_edge(scanline_fill - 1, 0, e, x0, y_top, x0, y_bottom);
                }
            }
        }
        else {
            float x0 = e->fx;
            float dx = e->fdx;
            float xb = x0 + dx;
            float x_top, x_bottom;
            float sy0, sy1;
            float dy = e->fdy;
            STBTT_assert(e->sy <= y_bottom && e->ey >= y_top);

            // compute endpoints of line segment clipped to this scanline (if the
            // line segment starts on this scanline. x0 is the intersection of the
            // line with y_top, but that may be off the line segment.
            if (e->sy > y_top) {
                x_top = x0 + dx * (e->sy - y_top);
                sy0 = e->sy;
            }
            else {
                x_top = x0;
                sy0 = y_top;
            }
            if (e->ey < y_bottom) {
                x_bottom = x0 + dx * (e->ey - y_top);
                sy1 = e->ey;
            }
            else {
                x_bottom = xb;
                sy1 = y_bottom;
            }

            if (x_top >= 0 && x_bottom >= 0 && x_top < len && x_bottom < len) {
                // from here on, we don't have to range check x values

                if ((int)x_top == (int)x_bottom) {
                    float height;
                    // simple case, only spans one pixel
                    int x = (int)x_top;
                    height = (sy1 - sy0) * e->direction;
                    STBTT_assert(x >= 0 && x < len);
                    scanline[x] += stbtt__position_trapezoid_area(height, x_top, x + 1.0f, x_bottom, x + 1.0f);
                    scanline_fill[x] += height; // everything right of this pixel is filled
                }
                else {
                    int x, x1, x2;
                    float y_crossing, y_final, step, sign, area;
                    // covers 2+ pixels
                    if (x_top > x_bottom) {
                        // flip scanline vertically; signed area is the same
                        float t;
                        sy0 = y_bottom - (sy0 - y_top);
                        sy1 = y_bottom - (sy1 - y_top);
                        t = sy0, sy0 = sy1, sy1 = t;
                        t = x_bottom, x_bottom = x_top, x_top = t;
                        dx = -dx;
                        dy = -dy;
                        t = x0, x0 = xb, xb = t;
                    }
                    STBTT_assert(dy >= 0);
                    STBTT_assert(dx >= 0);

                    x1 = (int)x_top;
                    x2 = (int)x_bottom;
                    // compute intersection with y axis at x1+1
                    y_crossing = y_top + dy * (x1 + 1 - x0);

                    // compute intersection with y axis at x2
                    y_final = y_top + dy * (x2 - x0);

                    //           x1    x_top                            x2    x_bottom
                    //     y_top  +------|-----+------------+------------+--------|---+------------+
                    //            |            |            |            |            |            |
                    //            |            |            |            |            |            |
                    //       sy0  |      Txxxxx|............|............|............|............|
                    // y_crossing |            *xxxxx.......|............|............|............|
                    //            |            |     xxxxx..|............|............|............|
                    //            |            |     /-   xx*xxxx........|............|............|
                    //            |            | dy <       |    xxxxxx..|............|............|
                    //   y_final  |            |     \-     |          xx*xxx.........|............|
                    //       sy1  |            |            |            |   xxxxxB...|............|
                    //            |            |            |            |            |            |
                    //            |            |            |            |            |            |
                    //  y_bottom  +------------+------------+------------+------------+------------+
                    //
                    // goal is to measure the area covered by '.' in each pixel

                    // if x2 is right at the right edge of x1, y_crossing can blow up, github #1057
                    // @TODO: maybe test against sy1 rather than y_bottom?
                    if (y_crossing > y_bottom)
                        y_crossing = y_bottom;

                    sign = e->direction;

                    // area of the rectangle covered from sy0..y_crossing
                    area = sign * (y_crossing - sy0);

                    // area of the triangle (x_top,sy0), (x1+1,sy0), (x1+1,y_crossing)
                    scanline[x1] += stbtt__sized_triangle_area(area, x1 + 1 - x_top);

                    // check if final y_crossing is blown up; no test case for this
                    if (y_final > y_bottom) {
                        int denom = (x2 - (x1 + 1));
                        y_final = y_bottom;
                        if (denom != 0) { // [DEAR IMGUI] Avoid div by zero (https://github.com/nothings/stb/issues/1316)
                            dy = (y_final - y_crossing) / denom; // if denom=0, y_final = y_crossing, so y_final <= y_bottom
                        }
                    }

                    // in second pixel, area covered by line segment found in first pixel
                    // is always a rectangle 1 wide * the height of that line segment; this
                    // is exactly what the variable 'area' stores. it also gets a contribution
                    // from the line segment within it. the THIRD pixel will get the first
                    // pixel's rectangle contribution, the second pixel's rectangle contribution,
                    // and its own contribution. the 'own contribution' is the same in every pixel except
                    // the leftmost and rightmost, a trapezoid that slides down in each pixel.
                    // the second pixel's contribution to the third pixel will be the
                    // rectangle 1 wide times the height change in the second pixel, which is dy.

                    step = sign * dy * 1; // dy is dy/dx, change in y for every 1 change in x,
                    // which multiplied by 1-pixel-width is how much pixel area changes for each step in x
                    // so the area advances by 'step' every time

                    for (x = x1 + 1; x < x2; ++x) {
                        scanline[x] += area + step / 2; // area of trapezoid is 1*step/2
                        area += step;
                    }
                    STBTT_assert(STBTT_fabs(area) <= 1.01f); // accumulated error from area += step unless we round step down
                    STBTT_assert(sy1 > y_final - 0.01f);

                    // area covered in the last pixel is the rectangle from all the pixels to the left,
                    // plus the trapezoid filled by the line segment in this pixel all the way to the right edge
                    scanline[x2] += area + sign * stbtt__position_trapezoid_area(sy1 - y_final, (float)x2, x2 + 1.0f, x_bottom, x2 + 1.0f);

                    // the rest of the line is filled based on the total height of the line segment in this pixel
                    scanline_fill[x2] += sign * (sy1 - sy0);
                }
            }
            else {
                // if edge goes outside of box we're drawing, we require
                // clipping logic. since this does not match the intended use
                // of this library, we use a different, very slow brute
                // force implementation
                // note though that this does happen some of the time because
                // x_top and x_bottom can be extrapolated at the top & bottom of
                // the shape and actually lie outside the bounding box
                int x;
                for (x = 0; x < len; ++x) {
                    // cases:
                    //
                    // there can be up to two intersections with the pixel. any intersection
                    // with left or right edges can be handled by splitting into two (or three)
                    // regions. intersections with top & bottom do not necessitate case-wise logic.
                    //
                    // the old way of doing this found the intersections with the left & right edges,
                    // then used some simple logic to produce up to three segments in sorted order
                    // from top-to-bottom. however, this had a problem: if an x edge was epsilon
                    // across the x border, then the corresponding y position might not be distinct
                    // from the other y segment, and it might ignored as an empty segment. to avoid
                    // that, we need to explicitly produce segments based on x positions.

                    // rename variables to clearly-defined pairs
                    float y0 = y_top;
                    float x1 = (float)(x);
                    float x2 = (float)(x + 1);
                    float x3 = xb;
                    float y3 = y_bottom;

                    // x = e->x + e->dx * (y-y_top)
                    // (y-y_top) = (x - e->x) / e->dx
                    // y = (x - e->x) / e->dx + y_top
                    float y1 = (x - x0) / dx + y_top;
                    float y2 = (x + 1 - x0) / dx + y_top;

                    if (x0 < x1 && x3 > x2) {         // three segments descending down-right
                        stbtt__handle_clipped_edge(scanline, x, e, x0, y0, x1, y1);
                        stbtt__handle_clipped_edge(scanline, x, e, x1, y1, x2, y2);
                        stbtt__handle_clipped_edge(scanline, x, e, x2, y2, x3, y3);
                    }
                    else if (x3 < x1 && x0 > x2) {  // three segments descending down-left
                        stbtt__handle_clipped_edge(scanline, x, e, x0, y0, x2, y2);
                        stbtt__handle_clipped_edge(scanline, x, e, x2, y2, x1, y1);
                        stbtt__handle_clipped_edge(scanline, x, e, x1, y1, x3, y3);
                    }
                    else if (x0 < x1 && x3 > x1) {  // two segments across x, down-right
                        stbtt__handle_clipped_edge(scanline, x, e, x0, y0, x1, y1);
                        stbtt__handle_clipped_edge(scanline, x, e, x1, y1, x3, y3);
                    }
                    else if (x3 < x1 && x0 > x1) {  // two segments across x, down-left
                        stbtt__handle_clipped_edge(scanline, x, e, x0, y0, x1, y1);
                        stbtt__handle_clipped_edge(scanline, x, e, x1, y1, x3, y3);
                    }
                    else if (x0 < x2 && x3 > x2) {  // two segments across x+1, down-right
                        stbtt__handle_clipped_edge(scanline, x, e, x0, y0, x2, y2);
                        stbtt__handle_clipped_edge(scanline, x, e, x2, y2, x3, y3);
                    }
                    else if (x3 < x2 && x0 > x2) {  // two segments across x+1, down-left
                        stbtt__handle_clipped_edge(scanline, x, e, x0, y0, x2, y2);
                        stbtt__handle_clipped_edge(scanline, x, e, x2, y2, x3, y3);
                    }
                    else {  // one segment
                        stbtt__handle_clipped_edge(scanline, x, e, x0, y0, x3, y3);
                    }
                }
            }
        }
        e = e->next;
    }
}

static void stbtt__rasterize_sorted_edges(stbtt__bitmap* result, stbtt__edge* e, int n, int vsubsample, int off_x, int off_y, void* userdata)
{
    stbtt__hheap hh = { 0, 0, 0 };
    stbtt__active_edge* active = NULL;
    int y, j = 0, i;
    float scanline_data[129], * scanline, * scanline2;

    STBTT__NOTUSED(vsubsample);

    if (result->w > 64)
        scanline = (float*)STBTT_malloc((result->w * 2 + 1) * sizeof(float), userdata);
    else
        scanline = scanline_data;

    scanline2 = scanline + result->w;

    y = off_y;
    e[n].y0 = (float)(off_y + result->h) + 1;

    while (j < result->h) {
        // find center of pixel for this scanline
        float scan_y_top = y + 0.0f;
        float scan_y_bottom = y + 1.0f;
        stbtt__active_edge** step = &active;

        STBTT_memset(scanline, 0, result->w * sizeof(scanline[0]));
        STBTT_memset(scanline2, 0, (result->w + 1) * sizeof(scanline[0]));

        // update all active edges;
        // remove all active edges that terminate before the top of this scanline
        while (*step) {
            stbtt__active_edge* z = *step;
            if (z->ey <= scan_y_top) {
                *step = z->next; // delete from list
                STBTT_assert(z->direction);
                z->direction = 0;
                stbtt__hheap_free(&hh, z);
            }
            else {
                step = &((*step)->next); // advance through list
            }
        }

        // insert all edges that start before the bottom of this scanline
        while (e->y0 <= scan_y_bottom) {
            if (e->y0 != e->y1) {
                stbtt__active_edge* z = stbtt__new_active(&hh, e, off_x, scan_y_top, userdata);
                if (z != NULL) {
                    if (j == 0 && off_y != 0) {
                        if (z->ey < scan_y_top) {
                            // this can happen due to subpixel positioning and some kind of fp rounding error i think
                            z->ey = scan_y_top;
                        }
                    }
                    STBTT_assert(z->ey >= scan_y_top); // if we get really unlucky a tiny bit of an edge can be out of bounds
                    // insert at front
                    z->next = active;
                    active = z;
                }
            }
            ++e;
        }

        // now process all active edges
        if (active)
            stbtt__fill_active_edges_new(scanline, scanline2 + 1, result->w, active, scan_y_top);

        {
            float sum = 0;
            for (i = 0; i < result->w; ++i) {
                float k;
                int m;
                sum += scanline2[i];
                k = scanline[i] + sum;
                k = (float)fabs(k) * 255 + 0.5f;
                m = (int)k;
                if (m > 255) m = 255;
                result->pixels[j * result->stride + i] = (unsigned char)m;
            }
        }
        // advance all the edges
        step = &active;
        while (*step) {
            stbtt__active_edge* z = *step;
            z->fx += z->fdx; // advance to position for current scanline
            step = &((*step)->next); // advance through list
        }

        ++y;
        ++j;
    }

    stbtt__hheap_cleanup(&hh, userdata);

    if (scanline != scanline_data)
        STBTT_free(scanline, userdata);
}


static void stbtt__rasterize(stbtt__bitmap* result, stbtt__point* pts, int* wcount, int windings, float scale_x, float scale_y, float shift_x, float shift_y, int off_x, int off_y, int invert, void* userdata)
{
    float y_scale_inv = invert ? -scale_y : scale_y;
    stbtt__edge* e;
    int n, i, j, k, m;
    int vsubsample = 1;
    // vsubsample should divide 255 evenly; otherwise we won't reach full opacity

    // now we have to blow out the windings into explicit edge lists
    n = 0;
    for (i = 0; i < windings; ++i)
        n += wcount[i];

    e = (stbtt__edge*)STBTT_malloc(sizeof(*e) * (n + 1), userdata); // add an extra one as a sentinel
    if (e == 0) return;
    n = 0;

    m = 0;
    for (i = 0; i < windings; ++i) {
        stbtt__point* p = pts + m;
        m += wcount[i];
        j = wcount[i] - 1;
        for (k = 0; k < wcount[i]; j = k++) {
            int a = k, b = j;
            // skip the edge if horizontal
            if (p[j].y == p[k].y)
                continue;
            // add edge from j to k to the list
            e[n].invert = 0;
            if (invert ? p[j].y > p[k].y : p[j].y < p[k].y) {
                e[n].invert = 1;
                a = j, b = k;
            }
            e[n].x0 = p[a].x * scale_x + shift_x;
            e[n].y0 = (p[a].y * y_scale_inv + shift_y) * vsubsample;
            e[n].x1 = p[b].x * scale_x + shift_x;
            e[n].y1 = (p[b].y * y_scale_inv + shift_y) * vsubsample;
            ++n;
        }
    }

    // now sort the edges by their highest point (should snap to integer, and then by x)
    //STBTT_sort(e, n, sizeof(e[0]), stbtt__edge_compare);
    stbtt__sort_edges(e, n);

    // now, traverse the scanlines and find the intersections on each scanline, use xor winding rule
    stbtt__rasterize_sorted_edges(result, e, n, vsubsample, off_x, off_y, userdata);

    STBTT_free(e, userdata);
}

static int stbrp__skyline_find_min_y(stbrp_context* c, stbrp_node* first, int x0, int width, int* pwaste)
{
    stbrp_node* node = first;
    int x1 = x0 + width;
    int min_y, visited_width, waste_area;

    STBRP__NOTUSED(c);

    STBRP_ASSERT(first->x <= x0);

#if 0
    // skip in case we're past the node
    while (node->next->x <= x0)
        ++node;
#else
    STBRP_ASSERT(node->next->x > x0); // we ended up handling this in the caller for efficiency
#endif

    STBRP_ASSERT(node->x <= x0);

    min_y = 0;
    waste_area = 0;
    visited_width = 0;
    while (node->x < x1) {
        if (node->y > min_y) {
            // raise min_y higher.
            // we've accounted for all waste up to min_y,
            // but we'll now add more waste for everything we've visted
            waste_area += visited_width * (node->y - min_y);
            min_y = node->y;
            // the first time through, visited_width might be reduced
            if (node->x < x0)
                visited_width += node->next->x - x0;
            else
                visited_width += node->next->x - node->x;
        }
        else {
            // add waste area
            int under_width = node->next->x - node->x;
            if (under_width + visited_width > width)
                under_width = width - visited_width;
            waste_area += under_width * (min_y - node->y);
            visited_width += under_width;
        }
        node = node->next;
    }

    *pwaste = waste_area;
    return min_y;
}

static void stbtt__add_point(stbtt__point* points, int n, float x, float y)
{
    if (!points) return; // during first pass, it's unallocated
    points[n].x = x;
    points[n].y = y;
}

static int stbtt__tesselate_curve(stbtt__point* points, int* num_points, float x0, float y0, float x1, float y1, float x2, float y2, float objspace_flatness_squared, int n)
{
    // midpoint
    float mx = (x0 + 2 * x1 + x2) / 4;
    float my = (y0 + 2 * y1 + y2) / 4;
    // versus directly drawn line
    float dx = (x0 + x2) / 2 - mx;
    float dy = (y0 + y2) / 2 - my;
    if (n > 16) // 65536 segments on one curve better be enough!
        return 1;
    if (dx * dx + dy * dy > objspace_flatness_squared) { // half-pixel error allowed... need to be smaller if AA
        stbtt__tesselate_curve(points, num_points, x0, y0, (x0 + x1) / 2.0f, (y0 + y1) / 2.0f, mx, my, objspace_flatness_squared, n + 1);
        stbtt__tesselate_curve(points, num_points, mx, my, (x1 + x2) / 2.0f, (y1 + y2) / 2.0f, x2, y2, objspace_flatness_squared, n + 1);
    }
    else {
        stbtt__add_point(points, *num_points, x2, y2);
        *num_points = *num_points + 1;
    }
    return 1;
}

static void stbtt__tesselate_cubic(stbtt__point* points, int* num_points, float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, float objspace_flatness_squared, int n)
{
    // @TODO this "flatness" calculation is just made-up nonsense that seems to work well enough
    float dx0 = x1 - x0;
    float dy0 = y1 - y0;
    float dx1 = x2 - x1;
    float dy1 = y2 - y1;
    float dx2 = x3 - x2;
    float dy2 = y3 - y2;
    float dx = x3 - x0;
    float dy = y3 - y0;
    float longlen = (float)(sqrt(dx0 * dx0 + dy0 * dy0) + sqrt(dx1 * dx1 + dy1 * dy1) + sqrt(dx2 * dx2 + dy2 * dy2));
    float shortlen = (float)sqrt(dx * dx + dy * dy);
    float flatness_squared = longlen * longlen - shortlen * shortlen;

    if (n > 16) // 65536 segments on one curve better be enough!
        return;

    if (flatness_squared > objspace_flatness_squared) {
        float x01 = (x0 + x1) / 2;
        float y01 = (y0 + y1) / 2;
        float x12 = (x1 + x2) / 2;
        float y12 = (y1 + y2) / 2;
        float x23 = (x2 + x3) / 2;
        float y23 = (y2 + y3) / 2;

        float xa = (x01 + x12) / 2;
        float ya = (y01 + y12) / 2;
        float xb = (x12 + x23) / 2;
        float yb = (y12 + y23) / 2;

        float mx = (xa + xb) / 2;
        float my = (ya + yb) / 2;

        stbtt__tesselate_cubic(points, num_points, x0, y0, x01, y01, xa, ya, mx, my, objspace_flatness_squared, n + 1);
        stbtt__tesselate_cubic(points, num_points, mx, my, xb, yb, x23, y23, x3, y3, objspace_flatness_squared, n + 1);
    }
    else {
        stbtt__add_point(points, *num_points, x3, y3);
        *num_points = *num_points + 1;
    }
}

static stbtt__point* stbtt_FlattenCurves(stbtt_vertex* vertices, int num_verts, float objspace_flatness, int** contour_lengths, int* num_contours, void* userdata)
{
    stbtt__point* points = 0;
    int num_points = 0;

    float objspace_flatness_squared = objspace_flatness * objspace_flatness;
    int i, n = 0, start = 0, pass;

    // count how many "moves" there are to get the contour count
    for (i = 0; i < num_verts; ++i)
        if (vertices[i].type == STBTT_vmove)
            ++n;

    *num_contours = n;
    if (n == 0) return 0;

    *contour_lengths = (int*)STBTT_malloc(sizeof(**contour_lengths) * n, userdata);

    if (*contour_lengths == 0) {
        *num_contours = 0;
        return 0;
    }

    // make two passes through the points so we don't need to realloc
    for (pass = 0; pass < 2; ++pass) {
        float x = 0, y = 0;
        if (pass == 1) {
            points = (stbtt__point*)STBTT_malloc(num_points * sizeof(points[0]), userdata);
            if (points == NULL) goto error;
        }
        num_points = 0;
        n = -1;
        for (i = 0; i < num_verts; ++i) {
            switch (vertices[i].type) {
            case STBTT_vmove:
                // start the next contour
                if (n >= 0)
                    (*contour_lengths)[n] = num_points - start;
                ++n;
                start = num_points;

                x = vertices[i].x, y = vertices[i].y;
                stbtt__add_point(points, num_points++, x, y);
                break;
            case STBTT_vline:
                x = vertices[i].x, y = vertices[i].y;
                stbtt__add_point(points, num_points++, x, y);
                break;
            case STBTT_vcurve:
                stbtt__tesselate_curve(points, &num_points, x, y,
                    vertices[i].cx, vertices[i].cy,
                    vertices[i].x, vertices[i].y,
                    objspace_flatness_squared, 0);
                x = vertices[i].x, y = vertices[i].y;
                break;
            case STBTT_vcubic:
                stbtt__tesselate_cubic(points, &num_points, x, y,
                    vertices[i].cx, vertices[i].cy,
                    vertices[i].cx1, vertices[i].cy1,
                    vertices[i].x, vertices[i].y,
                    objspace_flatness_squared, 0);
                x = vertices[i].x, y = vertices[i].y;
                break;
            }
        }
        (*contour_lengths)[n] = num_points - start;
    }

    return points;
error:
    STBTT_free(points, userdata);
    STBTT_free(*contour_lengths, userdata);
    *contour_lengths = 0;
    *num_contours = 0;
    return NULL;
}

static stbrp__findresult stbrp__skyline_find_best_pos(stbrp_context* c, int width, int height)
{
    int best_waste = (1 << 30), best_x, best_y = (1 << 30);
    stbrp__findresult fr;
    stbrp_node** prev, * node, * tail, ** best = NULL;

    // align to multiple of c->align
    width = (width + c->align - 1);
    width -= width % c->align;

    assert(width % c->align == 0);

    // if it can't possibly fit, bail immediately
    if (width > c->width || height > c->height) {
        fr.prev_link = NULL;
        fr.x = fr.y = 0;
        return fr;
    }

    node = c->active_head;
    prev = &c->active_head;
    while (node->x + width <= c->width) {
        int y, waste;
        y = stbrp__skyline_find_min_y(c, node, node->x, width, &waste);
        if (c->heuristic == STBRP_HEURISTIC_Skyline_BL_sortHeight) { // actually just want to test BL
            // bottom left
            if (y < best_y) {
                best_y = y;
                best = prev;
            }
        }
        else {
            // best-fit
            if (y + height <= c->height) {
                // can only use it if it first vertically
                if (y < best_y || (y == best_y && waste < best_waste)) {
                    best_y = y;
                    best_waste = waste;
                    best = prev;
                }
            }
        }
        prev = &node->next;
        node = node->next;
    }

    best_x = (best == NULL) ? 0 : (*best)->x;

    // if doing best-fit (BF), we also have to try aligning right edge to each node position
    //
    // e.g, if fitting
    //
    //     ____________________
    //    |____________________|
    //
    //            into
    //
    //   |                         |
    //   |             ____________|
    //   |____________|
    //
    // then right-aligned reduces waste, but bottom-left BL is always chooses left-aligned
    //
    // This makes BF take about 2x the time

    if (c->heuristic == STBRP_HEURISTIC_Skyline_BF_sortHeight) {
        tail = c->active_head;
        node = c->active_head;
        prev = &c->active_head;
        // find first node that's admissible
        while (tail->x < width)
            tail = tail->next;
        while (tail) {
            int xpos = tail->x - width;
            int y, waste;
            STBRP_ASSERT(xpos >= 0);
            // find the left position that matches this
            while (node->next->x <= xpos) {
                prev = &node->next;
                node = node->next;
            }
            STBRP_ASSERT(node->next->x > xpos && node->x <= xpos);
            y = stbrp__skyline_find_min_y(c, node, xpos, width, &waste);
            if (y + height <= c->height) {
                if (y <= best_y) {
                    if (y < best_y || waste < best_waste || (waste == best_waste && xpos < best_x)) {
                        best_x = xpos;
                        //STBRP_ASSERT(y <= best_y); [DEAR IMGUI]
                        best_y = y;
                        best_waste = waste;
                        best = prev;
                    }
                }
            }
            tail = tail->next;
        }
    }

    fr.prev_link = best;
    fr.x = best_x;
    fr.y = best_y;
    return fr;
}

static stbrp__findresult stbrp__skyline_pack_rectangle(stbrp_context* context, int width, int height)
{
    // find best position according to heuristic
    stbrp__findresult res = stbrp__skyline_find_best_pos(context, width, height);
    stbrp_node* node, * cur;

    // bail if:
    //    1. it failed
    //    2. the best node doesn't fit (we don't always check this)
    //    3. we're out of memory
    if (res.prev_link == NULL || res.y + height > context->height || context->free_head == NULL) {
        res.prev_link = NULL;
        return res;
    }

    // on success, create new node
    node = context->free_head;
    node->x = (int)res.x;
    node->y = (int)(res.y + height);

    context->free_head = node->next;

    // insert the new node into the right starting point, and
    // let 'cur' point to the remaining nodes needing to be
    // stiched back in

    cur = *res.prev_link;
    if (cur->x < res.x) {
        // preserve the existing one, so start testing with the next one
        stbrp_node* next = cur->next;
        cur->next = node;
        cur = next;
    }
    else {
        *res.prev_link = node;
    }

    // from here, traverse cur and free the nodes, until we get to one
    // that shouldn't be freed
    while (cur->next && cur->next->x <= res.x + width) {
        stbrp_node* next = cur->next;
        // move the current node to the free list
        cur->next = context->free_head;
        context->free_head = cur;
        cur = next;
    }

    // stitch the list back in
    node->next = cur;

    if (cur->x < res.x + width)
        cur->x = (int)(res.x + width);

#ifdef _DEBUG
    cur = context->active_head;
    while (cur->x < context->width) {
        STBRP_ASSERT(cur->x < cur->next->x);
        cur = cur->next;
    }
    STBRP_ASSERT(cur->next == NULL);

    {
        int count = 0;
        cur = context->active_head;
        while (cur) {
            cur = cur->next;
            ++count;
        }
        cur = context->free_head;
        while (cur) {
            cur = cur->next;
            ++count;
        }
        STBRP_ASSERT(count == context->num_nodes + 2);
    }
#endif

    return res;
}

static float stbtt__oversample_shift(int oversample)
{
    if (!oversample)
        return 0.0f;

    // The prefilter is a box filter of width "oversample",
    // which shifts phase by (oversample - 1)/2 pixels in
    // oversampled space. We want to shift in the opposite
    // direction to counter this.
    return (float)-(oversample - 1) / (2.0f * (float)oversample);
}

static int stbtt__close_shape(stbtt_vertex* vertices, int num_vertices, int was_off, int start_off,
    int sx, int sy, int scx, int scy, int cx, int cy)
{
    if (start_off) {
        if (was_off)
            stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve, (cx + scx) >> 1, (cy + scy) >> 1, cx, cy);
        stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve, sx, sy, scx, scy);
    }
    else {
        if (was_off)
            stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve, sx, sy, cx, cy);
        else
            stbtt_setvertex(&vertices[num_vertices++], STBTT_vline, sx, sy, 0, 0);
    }
    return num_vertices;
}

static int stbtt__GetGlyphShapeTT(const TTF_HeadInfo* info, int glyph_index, stbtt_vertex** pvertices)
{
    int16 numberOfContours;
    uint8* endPtsOfContours;
    uint8* data = info->data;
    stbtt_vertex* vertices = 0;
    int num_vertices = 0;
    int g = stbtt__GetGlyfOffset(info, glyph_index);

    *pvertices = NULL;

    if (g < 0) return 0;

    numberOfContours = ttSHORT(data + g);

    if (numberOfContours > 0) {
        uint8 flags = 0, flagcount;
        int ins, i, j = 0, m, n, next_move, was_off = 0, off, start_off = 0;
        int x, y, cx, cy, sx, sy, scx, scy;
        uint8* points;
        endPtsOfContours = (data + g + 10);
        ins = ttUSHORT(data + g + 10 + numberOfContours * 2);
        points = data + g + 10 + numberOfContours * 2 + 2 + ins;

        n = 1 + ttUSHORT(endPtsOfContours + numberOfContours * 2 - 2);

        m = n + 2 * numberOfContours;  // a loose bound on how many vertices we might need
        vertices = (stbtt_vertex*)STBTT_malloc(m * sizeof(vertices[0]), info->userdata);
        if (vertices == 0)
            return 0;

        next_move = 0;
        flagcount = 0;

        // in first pass, we load uninterpreted data into the allocated array
        // above, shifted to the end of the array so we won't overwrite it when
        // we create our final data starting from the front

        off = m - n; // starting offset for uninterpreted data, regardless of how m ends up being calculated

        // first load flags

        for (i = 0; i < n; ++i) {
            if (flagcount == 0) {
                flags = *points++;
                if (flags & 8)
                    flagcount = *points++;
            }
            else
                --flagcount;
            vertices[off + i].type = flags;
        }

        // now load x coordinates
        x = 0;
        for (i = 0; i < n; ++i) {
            flags = vertices[off + i].type;
            if (flags & 2) {
                int16 dx = *points++;
                x += (flags & 16) ? dx : -dx; // ???
            }
            else {
                if (!(flags & 16)) {
                    x = x + (int16)(points[0] * 256 + points[1]);
                    points += 2;
                }
            }
            vertices[off + i].x = (int16)x;
        }

        // now load y coordinates
        y = 0;
        for (i = 0; i < n; ++i) {
            flags = vertices[off + i].type;
            if (flags & 4) {
                int16 dy = *points++;
                y += (flags & 32) ? dy : -dy; // ???
            }
            else {
                if (!(flags & 32)) {
                    y = y + (int16)(points[0] * 256 + points[1]);
                    points += 2;
                }
            }
            vertices[off + i].y = (int16)y;
        }

        // now convert them to our format
        num_vertices = 0;
        sx = sy = cx = cy = scx = scy = 0;
        for (i = 0; i < n; ++i) {
            flags = vertices[off + i].type;
            x = (int16)vertices[off + i].x;
            y = (int16)vertices[off + i].y;

            if (next_move == i) {
                if (i != 0)
                    num_vertices = stbtt__close_shape(vertices, num_vertices, was_off, start_off, sx, sy, scx, scy, cx, cy);

                // now start the new one
                start_off = !(flags & 1);
                if (start_off) {
                    // if we start off with an off-curve point, then when we need to find a point on the curve
                    // where we can start, and we need to save some state for when we wraparound.
                    scx = x;
                    scy = y;
                    if (!(vertices[off + i + 1].type & 1)) {
                        // next point is also a curve point, so interpolate an on-point curve
                        sx = (x + (int)vertices[off + i + 1].x) >> 1;
                        sy = (y + (int)vertices[off + i + 1].y) >> 1;
                    }
                    else {
                        // otherwise just use the next point as our start point
                        sx = (int)vertices[off + i + 1].x;
                        sy = (int)vertices[off + i + 1].y;
                        ++i; // we're using point i+1 as the starting point, so skip it
                    }
                }
                else {
                    sx = x;
                    sy = y;
                }
                stbtt_setvertex(&vertices[num_vertices++], STBTT_vmove, sx, sy, 0, 0);
                was_off = 0;
                next_move = 1 + ttUSHORT(endPtsOfContours + j * 2);
                ++j;
            }
            else {
                if (!(flags & 1)) { // if it's a curve
                    if (was_off) // two off-curve control points in a row means interpolate an on-curve midpoint
                        stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve, (cx + x) >> 1, (cy + y) >> 1, cx, cy);
                    cx = x;
                    cy = y;
                    was_off = 1;
                }
                else {
                    if (was_off)
                        stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve, x, y, cx, cy);
                    else
                        stbtt_setvertex(&vertices[num_vertices++], STBTT_vline, x, y, 0, 0);
                    was_off = 0;
                }
            }
        }
        num_vertices = stbtt__close_shape(vertices, num_vertices, was_off, start_off, sx, sy, scx, scy, cx, cy);
    }
    else if (numberOfContours < 0) {
        // Compound shapes.
        int more = 1;
        uint8* comp = data + g + 10;
        num_vertices = 0;
        vertices = 0;
        while (more) {
            uint16 flags, gidx;
            int comp_num_verts = 0, i;
            stbtt_vertex* comp_verts = 0, * tmp = 0;
            float mtx[6] = { 1,0,0,1,0,0 }, m, n;

            flags = ttSHORT(comp); comp += 2;
            gidx = ttSHORT(comp); comp += 2;

            if (flags & 2) { // XY values
                if (flags & 1) { // shorts
                    mtx[4] = ttSHORT(comp); comp += 2;
                    mtx[5] = ttSHORT(comp); comp += 2;
                }
                else {
                    mtx[4] = ttCHAR(comp); comp += 1;
                    mtx[5] = ttCHAR(comp); comp += 1;
                }
            }
            else {
                // @TODO handle matching point
                STBTT_assert(0);
            }
            if (flags & (1 << 3)) { // WE_HAVE_A_SCALE
                mtx[0] = mtx[3] = ttSHORT(comp) / 16384.0f; comp += 2;
                mtx[1] = mtx[2] = 0;
            }
            else if (flags & (1 << 6)) { // WE_HAVE_AN_X_AND_YSCALE
                mtx[0] = ttSHORT(comp) / 16384.0f; comp += 2;
                mtx[1] = mtx[2] = 0;
                mtx[3] = ttSHORT(comp) / 16384.0f; comp += 2;
            }
            else if (flags & (1 << 7)) { // WE_HAVE_A_TWO_BY_TWO
                mtx[0] = ttSHORT(comp) / 16384.0f; comp += 2;
                mtx[1] = ttSHORT(comp) / 16384.0f; comp += 2;
                mtx[2] = ttSHORT(comp) / 16384.0f; comp += 2;
                mtx[3] = ttSHORT(comp) / 16384.0f; comp += 2;
            }

            // Find transformation scales.
            m = (float)std::sqrt(mtx[0] * mtx[0] + mtx[1] * mtx[1]);
            n = (float)std::sqrt(mtx[2] * mtx[2] + mtx[3] * mtx[3]);

            // Get indexed glyph.
            comp_num_verts = stbtt_GetGlyphShape(info, gidx, &comp_verts);
            if (comp_num_verts > 0) {
                // Transform vertices.
                for (i = 0; i < comp_num_verts; ++i) {
                    stbtt_vertex* v = &comp_verts[i];
                    short x, y;
                    x = v->x; y = v->y;
                    v->x = (short)(m * (mtx[0] * x + mtx[2] * y + mtx[4]));
                    v->y = (short)(n * (mtx[1] * x + mtx[3] * y + mtx[5]));
                    x = v->cx; y = v->cy;
                    v->cx = (short)(m * (mtx[0] * x + mtx[2] * y + mtx[4]));
                    v->cy = (short)(n * (mtx[1] * x + mtx[3] * y + mtx[5]));
                }
                // Append vertices.
                tmp = (stbtt_vertex*)STBTT_malloc((num_vertices + comp_num_verts) * sizeof(stbtt_vertex), info->userdata);
                if (!tmp) {
                    if (vertices) STBTT_free(vertices, info->userdata);
                    if (comp_verts) STBTT_free(comp_verts, info->userdata);
                    return 0;
                }
                if (num_vertices > 0 && vertices) memcpy(tmp, vertices, num_vertices * sizeof(stbtt_vertex));
                memcpy(tmp + num_vertices, comp_verts, comp_num_verts * sizeof(stbtt_vertex));
                if (vertices) STBTT_free(vertices, info->userdata);
                vertices = tmp;
                STBTT_free(comp_verts, info->userdata);
                num_vertices += comp_num_verts;
            }
            // More components ?
            more = flags & (1 << 5);
        }
    }
    else {
        // numberOfCounters == 0, do nothing
    }

    *pvertices = vertices;
    return num_vertices;
}

static int stbtt__GetGlyphShapeT2(const TTF_HeadInfo* info, int glyph_index, stbtt_vertex** pvertices)
{
    // runs the charstring twice, once to count and once to output (to avoid realloc)
    stbtt__csctx count_ctx = STBTT__CSCTX_INIT(1);
    stbtt__csctx output_ctx = STBTT__CSCTX_INIT(0);
    if (stbtt__run_charstring(info, glyph_index, &count_ctx)) {
        *pvertices = (stbtt_vertex*)STBTT_malloc(count_ctx.num_vertices * sizeof(stbtt_vertex), info->userdata);
        output_ctx.pvertices = *pvertices;
        if (stbtt__run_charstring(info, glyph_index, &output_ctx)) {
            assert(output_ctx.num_vertices == count_ctx.num_vertices);
            return output_ctx.num_vertices;
        }
    }
    *pvertices = NULL;
    return 0;
}











uint32 stbtt__find_table(uint8* data, uint32 fontstart, const char* tag)
{
    int num_tables = ttUSHORT(data + fontstart + 4);
    uint32_t tabledir = fontstart + 12;
    for (uint32_t i = 0; i < num_tables; ++i)
    {
        uint32_t loc = tabledir + 16 * i;
        if (stbtt_tag(data + loc + 0, tag))
        {
            return ttULONG(data + loc + 8);
        }
    }

    return 0;
}

int stbtt_InitFont(TTF_HeadInfo* info, uint8* data, int fontstart)
{
    info->data = data;
    info->fontstart = 0;
    auto cmap = stbtt__find_table(info->data, fontstart, "cmap");             // required
    info->loca = stbtt__find_table(info->data, fontstart, "loca");             // required
    info->head = stbtt__find_table(info->data, fontstart, "head");             // required
    info->glyf = stbtt__find_table(info->data, fontstart, "glyf");             // required
    info->hhea = stbtt__find_table(info->data, fontstart, "hhea");             // required
    info->hmtx = stbtt__find_table(info->data, fontstart, "hmtx");             // required
    info->kern = stbtt__find_table(info->data, fontstart, "kern");             // not required
    info->gpos = stbtt__find_table(info->data, fontstart, "GPOS");             // not required

    if (!cmap || !info->head || !info->hhea || !info->hmtx) return 0;

    if (info->glyf)
    {
        if (!info->loca) return 0;
    }
    else
    {
        // initialization for CFF / Type2 fonts (OTF)
    }

    auto t = stbtt__find_table(info->data, fontstart, "maxp");
    if (t)
    {
        info->numGlyphs = ttUSHORT(info->data + t + 4);
    }
    else
    {
        info->numGlyphs = 0xffff;
    }

    info->svg = -1;

    // find a cmap encoding table we understand *now* to avoid searching
    // later. (todo: could make this installable)
    // the same regardless of glyph.
    auto numTables = ttUSHORT(info->data + cmap + 2);
    info->indexMap = 0;

    for (int i = 0; i < numTables; ++i)
    {
        uint32_t encoding_record = cmap + 4 + 8 * i;
        // find an encoding we understand:
        switch (ttUSHORT(info->data + encoding_record)) {
        case STBTT_PLATFORM_ID_MICROSOFT:
            switch (ttUSHORT(info->data + encoding_record + 2)) {
            case STBTT_MS_EID_UNICODE_BMP:
            case STBTT_MS_EID_UNICODE_FULL:
                // MS/Unicode
                info->indexMap= cmap + ttULONG(info->data + encoding_record + 4);
                break;
            }
            break;
        case STBTT_PLATFORM_ID_UNICODE:
            // Mac/iOS has these
            // all the encodingIDs are unicode, so we don't bother to check it
            info->indexMap = cmap + ttULONG(info->data + encoding_record + 4);
            break;
        }
    }
    if (info->indexMap == 0) return 0;

    info->indexToLocFormat = ttUSHORT(info->data + info->head + 50);

    return 1;
}

int AddCustomRectRegular(V_Array<FontAtlasCustomRect>& rects, int width, int height)
{
    assert(width > 0 && width <= 0xFFFF);
    assert(height > 0 && height <= 0xFFFF);
    FontAtlasCustomRect r;
    r.Width = (unsigned short)width;
    r.Height = (unsigned short)height;
    rects.push_back(r);
    return rects.size() - 1; // Return index
}


int stbtt_FindGlyphIndex(const TTF_HeadInfo* info, int unicode_codepoint)
{
    uint8_t* data = const_cast<uint8_t*>(info->data);
    uint32_t index_map = info->indexMap;

    uint16_t format = ttUSHORT(data + index_map + 0);
    if (format == 0) { // apple byte encoding
        int32_t bytes = ttUSHORT(data + index_map + 2);
        if (unicode_codepoint < bytes - 6)
            return ttBYTE(data + index_map + 6 + unicode_codepoint);
        return 0;
    }
    else if (format == 6) {
        uint32_t first = ttUSHORT(data + index_map + 6);
        uint32_t count = ttUSHORT(data + index_map + 8);
        if ((uint32_t)unicode_codepoint >= first && (uint32_t)unicode_codepoint < first + count)
            return ttUSHORT(data + index_map + 10 + (unicode_codepoint - first) * 2);
        return 0;
    }
    else if (format == 2) {
        // STBTT_assert(0); // @TODO: high-byte mapping for japanese/chinese/korean
        return 0;
    }
    else if (format == 4) { // standard mapping for windows fonts: binary search collection of ranges
        uint16_t segcount = ttUSHORT(data + index_map + 6) >> 1;
        uint16_t searchRange = ttUSHORT(data + index_map + 8) >> 1;
        uint16_t entrySelector = ttUSHORT(data + index_map + 10);
        uint16_t rangeShift = ttUSHORT(data + index_map + 12) >> 1;

        // do a binary search of the segments
        uint32_t endCount = index_map + 14;
        uint32_t search = endCount;

        if (unicode_codepoint > 0xffff)
            return 0;

        // they lie from endCount .. endCount + segCount
        // but searchRange is the nearest power of two, so...
        if (unicode_codepoint >= ttUSHORT(data + search + rangeShift * 2))
            search += rangeShift * 2;

        // now decrement to bias correctly to find smallest
        search -= 2;
        while (entrySelector) {
            uint16_t end;
            searchRange >>= 1;
            end = ttUSHORT(data + search + searchRange * 2);
            if (unicode_codepoint > end)
                search += searchRange * 2;
            --entrySelector;
        }
        search += 2;

        {
            uint16_t offset, start, last;
            uint16_t item = (uint16_t)((search - endCount) >> 1);

            start = ttUSHORT(data + index_map + 14 + segcount * 2 + 2 + 2 * item);
            last = ttUSHORT(data + endCount + 2 * item);
            if (unicode_codepoint < start || unicode_codepoint > last)
                return 0;

            offset = ttUSHORT(data + index_map + 14 + segcount * 6 + 2 + 2 * item);
            if (offset == 0)
                return (uint16_t)(unicode_codepoint + ttSHORT(data + index_map + 14 + segcount * 4 + 2 + 2 * item));

            return ttUSHORT(data + offset + (unicode_codepoint - start) * 2 + index_map + 14 + segcount * 6 + 2 + 2 * item);
        }
    }
    else if (format == 12 || format == 13) {
        uint32_t ngroups = ttULONG(data + index_map + 12);
        int32_t low, high;
        low = 0; high = (int32_t)ngroups;
        // Binary search the right group.
        while (low < high) {
            int32_t mid = low + ((high - low) >> 1); // rounds down, so low <= mid < high
            uint32_t start_char = ttULONG(data + index_map + 16 + mid * 12);
            uint32_t end_char = ttULONG(data + index_map + 16 + mid * 12 + 4);
            if ((uint32_t)unicode_codepoint < start_char)
                high = mid;
            else if ((uint32_t)unicode_codepoint > end_char)
                low = mid + 1;
            else {
                uint32_t start_glyph = ttULONG(data + index_map + 16 + mid * 12 + 8);
                if (format == 12)
                    return start_glyph + unicode_codepoint - start_char;
                else // format == 13
                    return start_glyph;
            }
        }
        return 0; // not found
    }
    // @TODO
    //STBTT_assert(0);
    return 0;
}

int stbtt_PackBegin(stbtt_pack_context* spc, unsigned char* pixels, int pw, int ph, int stride_in_bytes, int padding, void* alloc_context)
{
    stbrp_context* context = (stbrp_context*)malloc(sizeof(*context));
    int            num_nodes = pw - padding;
    stbrp_node* nodes = (stbrp_node*)malloc(sizeof(*nodes) * num_nodes);

    if (context == NULL || nodes == NULL) {
        if (context != NULL) free(context);
        if (nodes != NULL) free(nodes);
        return 0;
    }

    spc->user_allocator_context = alloc_context;
    spc->width = pw;
    spc->height = ph;
    spc->pixels = pixels;
    spc->pack_info = context;
    spc->nodes = nodes;
    spc->padding = padding;
    spc->stride_in_bytes = stride_in_bytes != 0 ? stride_in_bytes : pw;
    spc->h_oversample = 1;
    spc->v_oversample = 1;
    spc->skip_missing = 0;

    stbrp_init_target(context, pw - padding, ph - padding, nodes, num_nodes);

    if (pixels)
        memset(pixels, 0, pw * ph); // background of 0 around pixels

    return 1;
}

void stbrp_init_target(stbrp_context* context, int width, int height, stbrp_node* nodes, int num_nodes)
{
    int i;

    for (i = 0; i < num_nodes - 1; ++i)
        nodes[i].next = &nodes[i + 1];
    nodes[i].next = NULL;
    context->init_mode = 1;
    context->heuristic = 0;
    context->free_head = &nodes[0];
    context->active_head = &context->extra[0];
    context->width = width;
    context->height = height;
    context->num_nodes = num_nodes;
    context->align = (context->width + context->num_nodes - 1) / context->num_nodes;

    // node 0 is the full width, node 1 is the sentinel (lets us not store width explicitly)
    context->extra[0].x = 0;
    context->extra[0].y = 0;
    context->extra[0].next = &context->extra[1];
    context->extra[1].x = width;
    context->extra[1].y = (1 << 30);
    context->extra[1].next = NULL;
}

void ImFontAtlasBuildPackCustomRects(class FontAtlas* atlas, void* stbrp_context_opaque)
{
    stbrp_context* pack_context = (stbrp_context*)stbrp_context_opaque;

    V_Array<FontAtlasCustomRect>& user_rects = atlas->m_CustomRects;

    V_Array<stbrp_rect> pack_rects;
    pack_rects.resize(user_rects.size());
    memset(pack_rects.data(), 0, (size_t)pack_rects.size() * sizeof(stbrp_rect));
    for (int i = 0; i < user_rects.size(); i++)
    {
        pack_rects[i].w = user_rects[i].Width;
        pack_rects[i].h = user_rects[i].Height;
    }
    stbrp_pack_rects(pack_context, &pack_rects[0], pack_rects.size());
    for (int i = 0; i < pack_rects.size(); i++)
    {
        if (pack_rects[i].was_packed)
        {
            user_rects[i].X = (unsigned short)pack_rects[i].x;
            user_rects[i].Y = (unsigned short)pack_rects[i].y;
            assert(pack_rects[i].w == user_rects[i].Width && pack_rects[i].h == user_rects[i].Height);
            atlas->m_TextureHeight  = (max(atlas->m_TextureHeight, pack_rects[i].y + pack_rects[i].h));
        }
    }
}

int stbrp_pack_rects(stbrp_context* context, stbrp_rect* rects, int num_rects)
{
    int i, all_rects_packed = 1;

    // we use the 'was_packed' field internally to allow sorting/unsorting
    for (i = 0; i < num_rects; ++i) {
        rects[i].was_packed = i;
    }

    // sort according to heuristic
    std::qsort(rects, num_rects, sizeof(rects[0]), rect_height_compare);

    for (i = 0; i < num_rects; ++i) {
        if (rects[i].w == 0 || rects[i].h == 0) {
            rects[i].x = rects[i].y = 0;  // empty rect needs no space
        }
        else {
            stbrp__findresult fr = stbrp__skyline_pack_rectangle(context, rects[i].w, rects[i].h);
            if (fr.prev_link) {
                rects[i].x = (int)fr.x;
                rects[i].y = (int)fr.y;
            }
            else {
                rects[i].x = rects[i].y = STBRP__MAXVAL;
            }
        }
    }

    // unsort
    std::qsort(rects, num_rects, sizeof(rects[0]), rect_original_order);

    // set was_packed flags and all_rects_packed status
    for (i = 0; i < num_rects; ++i) {
        rects[i].was_packed = !(rects[i].x == STBRP__MAXVAL && rects[i].y == STBRP__MAXVAL);
        if (!rects[i].was_packed)
            all_rects_packed = 0;
    }

    // return the all_rects_packed status
    return all_rects_packed;
}

int stbtt_PackFontRangesRenderIntoRects(stbtt_pack_context* spc, const TTF_HeadInfo* info, stbtt_pack_range* ranges, int num_ranges, stbrp_rect* rects)
{
    int i, j, k, missing_glyph = -1, return_value = 1;

    // save current values
    int old_h_over = spc->h_oversample;
    int old_v_over = spc->v_oversample;

    k = 0;
    for (i = 0; i < num_ranges; ++i) {
        float fh = ranges[i].font_size;
        float scale = fh > 0 ? stbtt_ScaleForPixelHeight(info, fh) : stbtt_ScaleForMappingEmToPixels(info, -fh);
        float recip_h, recip_v, sub_x, sub_y;
        spc->h_oversample = ranges[i].h_oversample;
        spc->v_oversample = ranges[i].v_oversample;
        recip_h = 1.0f / spc->h_oversample;
        recip_v = 1.0f / spc->v_oversample;
        sub_x = stbtt__oversample_shift(spc->h_oversample);
        sub_y = stbtt__oversample_shift(spc->v_oversample);
        for (j = 0; j < ranges[i].num_chars; ++j) {
            stbrp_rect* r = &rects[k];
            if (r->was_packed && r->w != 0 && r->h != 0) {
                stbtt_packedchar* bc = &ranges[i].chardata_for_range[j];
                int advance, lsb, x0, y0, x1, y1;
                int codepoint = ranges[i].array_of_unicode_codepoints == NULL ? ranges[i].first_unicode_codepoint_in_range + j : ranges[i].array_of_unicode_codepoints[j];
                int glyph = stbtt_FindGlyphIndex(info, codepoint);
                int pad = (int)spc->padding;

                // pad on left and top
                r->x += pad;
                r->y += pad;
                r->w -= pad;
                r->h -= pad;
                stbtt_GetGlyphHMetrics(info, glyph, &advance, &lsb);
                stbtt_GetGlyphBitmapBox(info, glyph,
                    scale * spc->h_oversample,
                    scale * spc->v_oversample,
                    &x0, &y0, &x1, &y1);
                stbtt_MakeGlyphBitmapSubpixel(info,
                    spc->pixels + r->x + r->y * spc->stride_in_bytes,
                    r->w - spc->h_oversample + 1,
                    r->h - spc->v_oversample + 1,
                    spc->stride_in_bytes,
                    scale * spc->h_oversample,
                    scale * spc->v_oversample,
                    0, 0,
                    glyph);

                if (spc->h_oversample > 1)
                    stbtt__h_prefilter(spc->pixels + r->x + r->y * spc->stride_in_bytes,
                        r->w, r->h, spc->stride_in_bytes,
                        spc->h_oversample);

                if (spc->v_oversample > 1)
                    stbtt__v_prefilter(spc->pixels + r->x + r->y * spc->stride_in_bytes,
                        r->w, r->h, spc->stride_in_bytes,
                        spc->v_oversample);

                bc->x0 = (int16)r->x;
                bc->y0 = (int16)r->y;
                bc->x1 = (int16)(r->x + r->w);
                bc->y1 = (int16)(r->y + r->h);
                bc->xadvance = scale * advance;
                bc->xoff = (float)x0 * recip_h + sub_x;
                bc->yoff = (float)y0 * recip_v + sub_y;
                bc->xoff2 = (x0 + r->w) * recip_h + sub_x;
                bc->yoff2 = (y0 + r->h) * recip_v + sub_y;

                if (glyph == 0)
                    missing_glyph = j;
            }
            else if (spc->skip_missing) {
                return_value = 0;
            }
            else if (r->was_packed && r->w == 0 && r->h == 0 && missing_glyph >= 0) {
                ranges[i].chardata_for_range[j] = ranges[i].chardata_for_range[missing_glyph];
            }
            else {
                return_value = 0; // if any fail, report failure
            }

            ++k;
        }
    }

    // restore original values
    spc->h_oversample = old_h_over;
    spc->v_oversample = old_v_over;

    return return_value;
}

void stbtt_MakeGlyphBitmapSubpixel(const TTF_HeadInfo* info, unsigned char* output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int glyph)
{
    int ix0, iy0;
    stbtt_vertex* vertices;
    int num_verts = stbtt_GetGlyphShape(info, glyph, &vertices);
    stbtt__bitmap gbm;

    stbtt_GetGlyphBitmapBoxSubpixel(info, glyph, scale_x, scale_y, shift_x, shift_y, &ix0, &iy0, 0, 0);
    gbm.pixels = output;
    gbm.w = out_w;
    gbm.h = out_h;
    gbm.stride = out_stride;

    if (gbm.w && gbm.h)
        stbtt_Rasterize(&gbm, 0.35f, vertices, num_verts, scale_x, scale_y, shift_x, shift_y, ix0, iy0, 1, info->data);

    STBTT_free(vertices, info->userdata);
}

void stbtt_Rasterize(stbtt__bitmap* result, float flatness_in_pixels, stbtt_vertex* vertices, int num_verts, float scale_x, float scale_y, float shift_x, float shift_y, int x_off, int y_off, int invert, void* userdata)
{
    float scale = scale_x > scale_y ? scale_y : scale_x;
    int winding_count = 0;
    int* winding_lengths = NULL;
    stbtt__point* windings = stbtt_FlattenCurves(vertices, num_verts, flatness_in_pixels / scale, &winding_lengths, &winding_count, userdata);
    if (windings) {
        stbtt__rasterize(result, windings, winding_lengths, winding_count, scale_x, scale_y, shift_x, shift_y, x_off, y_off, invert, userdata);
        STBTT_free(winding_lengths, userdata);
        STBTT_free(windings, userdata);
    }
}

int stbtt_GetGlyphShape(const TTF_HeadInfo* info, int glyph_index, stbtt_vertex** pvertices)
{
    return stbtt__GetGlyphShapeTT(info, glyph_index, pvertices);
}

static void ImFontAtlasBuildRenderDefaultTexData(FontAtlas* atlas)
{
    auto r = atlas->m_CustomRects[atlas->m_PackIdMouseCursors];
    assert(r.IsPacked());

    const int w = atlas->m_TextureWidth;
    if (!(atlas->m_Flags & ImFontAtlasFlags_NoMouseCursors))
    {
        // Render/copy pixels
        assert(r.Width == FONT_ATLAS_DEFAULT_TEX_DATA_W * 2 + 1 && r.Height == FONT_ATLAS_DEFAULT_TEX_DATA_H);
        const int x_for_white = r.X;
        const int x_for_black = r.X + FONT_ATLAS_DEFAULT_TEX_DATA_W + 1;
        if (atlas->m_PixelAlpha8.size())
        {
            ImFontAtlasBuildRender8bppRectFromString(atlas, x_for_white, r.Y, FONT_ATLAS_DEFAULT_TEX_DATA_W, FONT_ATLAS_DEFAULT_TEX_DATA_H, FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS, '.', 0xFF);
            ImFontAtlasBuildRender8bppRectFromString(atlas, x_for_black, r.Y, FONT_ATLAS_DEFAULT_TEX_DATA_W, FONT_ATLAS_DEFAULT_TEX_DATA_H, FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS, 'X', 0xFF);
        }
        /*else
        {
            ImFontAtlasBuildRender32bppRectFromString(atlas, x_for_white, r->Y, FONT_ATLAS_DEFAULT_TEX_DATA_W, FONT_ATLAS_DEFAULT_TEX_DATA_H, FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS, '.', IM_COL32_WHITE);
            ImFontAtlasBuildRender32bppRectFromString(atlas, x_for_black, r->Y, FONT_ATLAS_DEFAULT_TEX_DATA_W, FONT_ATLAS_DEFAULT_TEX_DATA_H, FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS, 'X', IM_COL32_WHITE);
        }*/
    }
    else
    {
        // Render 4 white pixels
        IM_ASSERT(r.Width == 2 && r.Height == 2);
        const int offset = (int)r.X + (int)r.Y * w;
        if (atlas->m_PixelAlpha8.size())
        {
            atlas->m_PixelAlpha8[offset] = atlas->m_PixelAlpha8[offset + 1] = atlas->m_PixelAlpha8[offset + w] = atlas->m_PixelAlpha8[offset + w + 1] = 0xFF;
        }
        else
        {
            atlas->m_PixelRGBA32[offset] = atlas->m_PixelRGBA32[offset + 1] = atlas->m_PixelRGBA32[offset + w] = atlas->m_PixelRGBA32[offset + w + 1] = UINT8_4(255, 255, 255, 255);
        }
    }
    atlas->m_TextureUvWhitePixel = River::Float2((r.X + 0.5f) * atlas->m_TextureUvScale.x, (r.Y + 0.5f) * atlas->m_TextureUvScale.y);
}

static void ImFontAtlasBuildRenderLinesTexData(FontAtlas* atlas)
{
    if (atlas->m_Flags & ImFontAtlasFlags_NoBakedLines)
        return;

    // This generates a triangular shape in the texture, with the various line widths stacked on top of each other to allow interpolation between them
    auto r = &atlas->m_CustomRects[atlas->m_PackIdLines];
    IM_ASSERT(r->IsPacked());
    for (unsigned int n = 0; n < 63 + 1; n++) // +1 because of the zero-width row
    {
        // Each line consists of at least two empty pixels at the ends, with a line of solid pixels in the middle
        unsigned int y = n;
        unsigned int line_width = n;
        unsigned int pad_left = (r->Width - line_width) / 2;
        unsigned int pad_right = r->Width - (pad_left + line_width);

        // Write each slice
        IM_ASSERT(pad_left + line_width + pad_right == r->Width && y < r->Height); // Make sure we're inside the texture bounds before we start writing pixels
        if (atlas->m_PixelAlpha8.size())
        {
            unsigned char* write_ptr = &atlas->m_PixelAlpha8[r->X + ((r->Y + y) * atlas->m_TextureWidth)];
            for (unsigned int i = 0; i < pad_left; i++)
                *(write_ptr + i) = 0x00;

            for (unsigned int i = 0; i < line_width; i++)
                *(write_ptr + pad_left + i) = 0xFF;

            for (unsigned int i = 0; i < pad_right; i++)
                *(write_ptr + pad_left + line_width + i) = 0x00;
        }
        else
        {
            unsigned int* write_ptr = (uint32*)(&atlas->m_PixelRGBA32[r->X + ((r->Y + y) * atlas->m_TextureWidth)]);
            for (unsigned int i = 0; i < pad_left; i++)
                *(write_ptr + i) = UINT8_4(255, 255, 255, 0);

            for (unsigned int i = 0; i < line_width; i++)
                *(write_ptr + pad_left + i) = UINT8_4(255, 255, 255, 255);

            for (unsigned int i = 0; i < pad_right; i++)
                *(write_ptr + pad_left + line_width + i) = UINT8_4(255, 255, 255, 0);
        }

        // Calculate UVs for this line
        FLOAT_2 uv0 = FLOAT_2((float)(r->X + pad_left - 1), (float)(r->Y + y)) * atlas->m_TextureUvScale;
        FLOAT_2 uv1 = FLOAT_2((float)(r->X + pad_left + line_width + 1), (float)(r->Y + y + 1)) * atlas->m_TextureUvScale;
        float half_v = (uv0.y + uv1.y) * 0.5f; // Calculate a constant V in the middle of the row to avoid sampling artifacts
        atlas->m_TexUvLines[n] = FLOAT_4(uv0.x, half_v, uv1.x, half_v);
    }
}

void ImFontAtlasBuildFinish(FontAtlas* atlas)
{
    // Render into our custom data blocks
    assert(atlas->m_PixelAlpha8.size());
    ImFontAtlasBuildRenderDefaultTexData(atlas);
    ImFontAtlasBuildRenderLinesTexData(atlas);

    // Register custom rectangle glyphs
    for (int i = 0; i < atlas->m_CustomRects.size(); i++)
    {
        auto r = &atlas->m_CustomRects[i];
        if (r->Font == NULL || r->GlyphID == 0)
            continue;

        // Will ignore ImFontConfig settings: GlyphMinAdvanceX, GlyphMinAdvanceY, GlyphExtraSpacing, PixelSnapH
      /*  IM_ASSERT(r->Font->ContainerAtlas == atlas);
        ImVec2 uv0, uv1;
        atlas->CalcCustomRectUV(r, &uv0, &uv1);
        r->Font->AddGlyph(NULL, (ImWchar)r->GlyphID, r->GlyphOffset.x, r->GlyphOffset.y, r->GlyphOffset.x + r->Width, r->GlyphOffset.y + r->Height, uv0.x, uv0.y, uv1.x, uv1.y, r->GlyphAdvanceX);*/
    }

    // Build all fonts lookup tables
    atlas->m_Font->BuildLookupTable();

    //atlas->TexReady = true;
}

void stbtt_GetPackedQuad(const stbtt_packedchar* chardata, int pw, int ph, int char_index, float* xpos, float* ypos, stbtt_aligned_quad* q, int align_to_integer)
{
    float ipw = 1.0f / pw, iph = 1.0f / ph;
    const stbtt_packedchar* b = chardata + char_index;

    if (align_to_integer) {
        float x = (float)std::floorf((*xpos + b->xoff) + 0.5f);
        float y = (float)std::floorf((*ypos + b->yoff) + 0.5f);
        q->x0 = x;
        q->y0 = y;
        q->x1 = x + b->xoff2 - b->xoff;
        q->y1 = y + b->yoff2 - b->yoff;
    }
    else {
        q->x0 = *xpos + b->xoff;
        q->y0 = *ypos + b->yoff;
        q->x1 = *xpos + b->xoff2;
        q->y1 = *ypos + b->yoff2;
    }

    q->s0 = b->x0 * ipw;
    q->t0 = b->y0 * iph;
    q->s1 = b->x1 * ipw;
    q->t1 = b->y1 * iph;

    *xpos += b->xadvance;
}

void stbtt_PackEnd(stbtt_pack_context* spc)
{
    STBTT_free(spc->nodes, spc->user_allocator_context);
    STBTT_free(spc->pack_info, spc->user_allocator_context);
}

void ImFontAtlasBuildRender8bppRectFromString(class FontAtlas* atlas, int x, int y, int w, int h, const char* in_str, char in_marker_char, unsigned char in_marker_pixel_value)
{
    assert(x >= 0 && x + w <= atlas->m_TextureWidth);
    assert(y >= 0 && y + h <= atlas->m_TextureHeight);
    unsigned char* out_pixel = atlas->m_PixelAlpha8.data() + x + (y * atlas->m_TextureWidth);
    for (int off_y = 0; off_y < h; off_y++, out_pixel += atlas->m_TextureWidth, in_str += w)
        for (int off_x = 0; off_x < w; off_x++)
            out_pixel[off_x] = (in_str[off_x] == in_marker_char) ? in_marker_pixel_value : 0x00;
}