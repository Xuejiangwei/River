#pragma once

#include "RiverHead.h"
#include "MathStruct.h"

//https://www.cnblogs.com/sjhrun2001/archive/2010/01/19/1651274.html
template<typename T> static inline T ImClamp(T v, T mn, T mx) { return (v < mn) ? mn : (v > mx) ? mx : v; }
struct ImFontGlyph
{
	unsigned int    Colored : 1;        // Flag to indicate glyph is colored and should generally ignore tinting (make it usable with no shift on little-endian as this is used in loops)
	unsigned int    Visible : 1;        // Flag to indicate glyph has no visible pixels (e.g. space). Allow early out when rendering.
	unsigned int    Codepoint : 30;     // 0x0000..0x10FFFF
	float           AdvanceX;           // Distance to next character (= data from font + ImFontConfig::GlyphExtraSpacing.x baked in)
	float           X0, Y0, X1, Y1;     // Glyph corners
	float           U0, V0, U1, V1;     // Texture coordinates
};

typedef struct
{
	unsigned char* data;
	int cursor;
	int size;
} stbtt__buf;

typedef struct stbtt__edge {
	float x0, y0, x1, y1;
	int invert;
} stbtt__edge;

typedef struct stbtt__hheap_chunk
{
	struct stbtt__hheap_chunk* next;
} stbtt__hheap_chunk;

typedef struct
{
	float x0, y0, s0, t0; // top-left
	float x1, y1, s1, t1; // bottom-right
} stbtt_aligned_quad;

typedef struct stbtt__hheap
{
	struct stbtt__hheap_chunk* head;
	void* first_free;
	int    num_remaining_in_head_chunk;
} stbtt__hheap;

typedef struct stbtt__active_edge
{
	struct stbtt__active_edge* next;
	float fx, fdx, fdy;
	float direction;
	float sy;
	float ey;
} stbtt__active_edge;

typedef struct
{
	int w, h, stride;
	unsigned char* pixels;
} stbtt__bitmap;

typedef struct
{
	float x, y;
} stbtt__point;

typedef struct
{
	unsigned short x0, y0, x1, y1; // coordinates of bbox in bitmap
	float xoff, yoff, xadvance;
	float xoff2, yoff2;
} stbtt_packedchar;

typedef struct
{
	float font_size;
	int first_unicode_codepoint_in_range;  // if non-zero, then the chars are continuous, and this is the first codepoint
	int* array_of_unicode_codepoints;       // if non-zero, then this is an array of unicode codepoints
	int num_chars;
	stbtt_packedchar* chardata_for_range; // output
	unsigned char h_oversample, v_oversample; // don't set these, they're used internally
} stbtt_pack_range;

typedef struct
{
	short x, y, cx, cy, cx1, cy1;
	unsigned char type, padding;
} stbtt_vertex;

struct TTF_HeadInfo
{
	void* userdata;
	unsigned char* data;              // pointer to .ttf file
	int fontstart;         // offset of start of font

	int numGlyphs;
	int loca, head, glyf, hhea, hmtx, kern, gpos, svg;
	int indexMap;
	int indexToLocFormat;              // format needed to map from glyph index to glyph

	stbtt__buf cff;                    // cff font data
	stbtt__buf charstrings;            // the charstring index
	stbtt__buf gsubrs;                 // global charstring subroutines index
	stbtt__buf subrs;                  // private charstring subroutines index
	stbtt__buf fontdicts;              // array of font dicts
	stbtt__buf fdselect;               // map from glyph to fontdict
};

inline void ImBitArraySetBit(uint32* arr, int n) { uint32 mask = (uint32)1 << (n & 31); arr[n >> 5] |= mask; }
inline void ImBitArrayClearBit(uint32* arr, int n) { uint32 mask = (uint32)1 << (n & 31); arr[n >> 5] &= ~mask; }

inline int16 ttSHORT(const uint8_t* p) { return p[0] * 256 + p[1]; }
inline uint16 ttUSHORT(const uint8* p) { return p[0] * 256 + p[1]; }
inline int ttLONG(const uint8_t* p) { return (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3]; }
inline uint32 ttULONG(const uint8* p) { return (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3]; }
inline int ImUpperPowerOfTwo(int v) { v--; v |= v >> 1; v |= v >> 2; v |= v >> 4; v |= v >> 8; v |= v >> 16; v++; return v; }

#define         IM_BITARRAY_TESTBIT(_ARRAY, _N)                 ((_ARRAY[(_N) >> 5] & ((uint32)1 << ((_N) & 31))) != 0) // Macro version of ImBitArrayTestBit(): ensure args have side-effect or are costly!
struct ImBitVector
{
	V_Array<uint32> Storage;
	void            Create(int sz) { Storage.resize((sz + 31) >> 5); memset(Storage.data(), 0, (size_t)Storage.size() * sizeof(Storage[0])); }
	void            Clear() { Storage.clear(); }
	bool            TestBit(int n) const { assert(n < (Storage.size() << 5)); return IM_BITARRAY_TESTBIT(Storage.data(), n); }
	void            SetBit(int n) { assert(n < (Storage.size() << 5)); ImBitArraySetBit(Storage.data(), n); }
	void            ClearBit(int n) { assert(n < (Storage.size() << 5)); ImBitArrayClearBit(Storage.data(), n); }
};

struct FontAtlasCustomRect
{
	unsigned short  Width, Height;  // Input    // Desired rectangle dimension
	unsigned short  X, Y;           // Output   // Packed position in Atlas
	unsigned int    GlyphID;        // Input    // For custom font glyphs only (ID < 0x110000)
	float           GlyphAdvanceX;  // Input    // For custom font glyphs only: glyph xadvance
	FLOAT_2          GlyphOffset;    // Input    // For custom font glyphs only: glyph display offset
	class Font* Font;           // Input    // For custom font glyphs only: target font
	FontAtlasCustomRect() { Width = Height = 0; X = Y = 0xFFFF; GlyphID = 0; GlyphAdvanceX = 0.0f; GlyphOffset = FLOAT_2(0.0f, 0.0f); Font = nullptr; }
	bool IsPacked() const { return X != 0xFFFF; }
};

struct stbrp_rect
{
	// reserved for your use:
	int            id;

	// input:
	int    w, h;

	// output:
	int    x, y;
	int            was_packed;  // non-zero if valid packing

}; // 16 bytes, nominally

struct stbtt_pack_context 
{
	void* user_allocator_context;
	void* pack_info;
	int   width;
	int   height;
	int   stride_in_bytes;
	int   padding;
	int   skip_missing;
	unsigned int   h_oversample, v_oversample;
	unsigned char* pixels;
	void* nodes;
};

struct stbrp_node
{
	int  x, y;
	stbrp_node* next;
};

struct stbrp_context
{
	int width;
	int height;
	int align;
	int init_mode;
	int heuristic;
	int num_nodes;
	stbrp_node* active_head;
	stbrp_node* free_head;
	stbrp_node extra[2]; // we allocate two extra nodes so optimal user-node-count is 'width' not 'width+2'
};

struct ImFontBuildSrcData
{
	TTF_HeadInfo      FontInfo;
	stbtt_pack_range    PackRange;          // Hold the list of codepoints to pack (essentially points to Codepoints.Data)
	stbrp_rect* Rects;              // Rectangle to pack. We first fill in their size and the packer will give us their position.
	stbtt_packedchar* PackedChars;        // Output glyphs
	uint16* SrcRanges;          // Ranges as requested by user (user is allowed to request too much, e.g. 0x0020..0xFFFF)
	int                 DstIndex;           // Index into atlas->Fonts[] and dst_tmp_array[]
	int                 GlyphsHighest;      // Highest requested codepoint
	int                 GlyphsCount;        // Glyph count (excluding missing glyphs and glyphs already set by an earlier source font)
	ImBitVector         GlyphsSet;          // Glyph bit map (random access, 1-bit per codepoint. This will be a maximum of 8KB)
	V_Array<int>       GlyphsList;         // Glyph codepoints list (flattened version of GlyphsSet)
};

typedef struct
{
	int x, y;
	stbrp_node** prev_link;
} stbrp__findresult;

typedef struct
{
	int bounds;
	int started;
	float first_x, first_y;
	float x, y;
	int min_x, max_x, min_y, max_y;

	stbtt_vertex* pvertices;
	int num_vertices;
} stbtt__csctx;

inline uint64_t GetSize(FILE* f)
{
	uint64_t off = 0, sz = 0;
	return ((off = ftell(f)) != -1 && !fseek(f, 0, SEEK_END) && (sz = ftell(f)) != -1 &&
		!fseek(f, off, SEEK_SET)) ? sz : -1;
}

inline float stbtt_ScaleForPixelHeight(const TTF_HeadInfo* info, float height)
{
	int fheight = ttSHORT(info->data + info->hhea + 4) - ttSHORT(info->data + info->hhea + 6);
	return (float)height / fheight;
}

inline float stbtt_ScaleForMappingEmToPixels(const TTF_HeadInfo* info, float pixels)
{
	int unitsPerEm = ttUSHORT(info->data + info->head + 18);
	return pixels / unitsPerEm;
}

inline int stbtt__GetGlyfOffset(const TTF_HeadInfo* info, int glyph_index)
{
	int g1, g2;

	if (glyph_index >= info->numGlyphs) return -1; // glyph index out of range
	if (info->indexToLocFormat >= 2)    return -1; // unknown index->glyph map format

	if (info->indexToLocFormat == 0)
	{
		g1 = info->glyf + ttUSHORT(info->data + info->loca + glyph_index * 2) * 2;
		g2 = info->glyf + ttUSHORT(info->data + info->loca + glyph_index * 2 + 2) * 2;
	}
	else
	{
		g1 = info->glyf + ttULONG(info->data + info->loca + glyph_index * 4);
		g2 = info->glyf + ttULONG(info->data + info->loca + glyph_index * 4 + 4);
	}

	return g1 == g2 ? -1 : g1; // if length is 0, return -1
}

inline int stbtt_GetGlyphBox(const TTF_HeadInfo* info, int glyph_index, int* x0, int* y0, int* x1, int* y1)
{
	int g = stbtt__GetGlyfOffset(info, glyph_index);
	if (g < 0) return 0;

	if (x0) *x0 = ttSHORT(info->data + g + 2);
	if (y0) *y0 = ttSHORT(info->data + g + 4);
	if (x1) *x1 = ttSHORT(info->data + g + 6);
	if (y1) *y1 = ttSHORT(info->data + g + 8);
	
	return 1;
}

inline void stbtt_GetGlyphBitmapBoxSubpixel(const TTF_HeadInfo* font, int glyph, float scale_x, float scale_y, float shift_x, float shift_y, int* ix0, int* iy0, int* ix1, int* iy1)
{
	int x0 = 0, y0 = 0, x1, y1; // =0 suppresses compiler warning
	if (!stbtt_GetGlyphBox(font, glyph, &x0, &y0, &x1, &y1)) 
	{
		// e.g. space character
		if (ix0) *ix0 = 0;
		if (iy0) *iy0 = 0;
		if (ix1) *ix1 = 0;
		if (iy1) *iy1 = 0;
	}
	else 
	{
		// move to integral bboxes (treating pixels as little squares, what pixels get touched)?
		if (ix0) *ix0 = floorf(x0 * scale_x + shift_x);
		if (iy0) *iy0 = floorf(-y1 * scale_y + shift_y);
		if (ix1) *ix1 = ceilf(x1 * scale_x + shift_x);
		if (iy1) *iy1 = ceilf(-y0 * scale_y + shift_y);
	}
}

inline void stbtt_GetGlyphHMetrics(const TTF_HeadInfo* info, int glyph_index, int* advanceWidth, int* leftSideBearing)
{
	uint16 numOfLongHorMetrics = ttUSHORT(info->data + info->hhea + 34);
	if (glyph_index < numOfLongHorMetrics)
	{
		if (advanceWidth)     *advanceWidth = ttSHORT(info->data + info->hmtx + 4 * glyph_index);
		if (leftSideBearing)  *leftSideBearing = ttSHORT(info->data + info->hmtx + 4 * glyph_index + 2);
	}
	else 
	{
		if (advanceWidth)     *advanceWidth = ttSHORT(info->data + info->hmtx + 4 * (numOfLongHorMetrics - 1));
		if (leftSideBearing)  *leftSideBearing = ttSHORT(info->data + info->hmtx + 4 * numOfLongHorMetrics + 2 * (glyph_index - numOfLongHorMetrics));
	}
}

inline void stbtt_GetGlyphBitmapBox(const TTF_HeadInfo* font, int glyph, float scale_x, float scale_y, int* ix0, int* iy0, int* ix1, int* iy1)
{
	stbtt_GetGlyphBitmapBoxSubpixel(font, glyph, scale_x, scale_y, 0.0f, 0.0f, ix0, iy0, ix1, iy1);
}

inline void stbtt_GetFontVMetrics(const TTF_HeadInfo* info, int* ascent, int* descent, int* lineGap)
{
	if (ascent) *ascent = ttSHORT(info->data + info->hhea + 4);
	if (descent) *descent = ttSHORT(info->data + info->hhea + 6);
	if (lineGap) *lineGap = ttSHORT(info->data + info->hhea + 8);
}

int AddCustomRectRegular(V_Array<FontAtlasCustomRect>& rects, int width, int height);

uint32 stbtt__find_table(uint8* data, uint32 fontstart, const char* tag);

int stbtt_InitFont(TTF_HeadInfo* info, uint8* data, int fontstart);

int stbtt_FindGlyphIndex(const TTF_HeadInfo* info, int unicode_codepoint);

int stbtt_PackBegin(stbtt_pack_context* spc, unsigned char* pixels, int pw, int ph, int stride_in_bytes, int padding, void* alloc_context);

void stbrp_init_target(stbrp_context* context, int width, int height, stbrp_node* nodes, int num_nodes);

void ImFontAtlasBuildPackCustomRects(class FontAtlas* font, void* stbrp_context_opaque);

int stbrp_pack_rects(stbrp_context* context, stbrp_rect* rects, int num_rects);

int stbtt_PackFontRangesRenderIntoRects(stbtt_pack_context* spc, const TTF_HeadInfo* info, stbtt_pack_range* ranges, int num_ranges, stbrp_rect* rects);

void stbtt_MakeGlyphBitmapSubpixel(const TTF_HeadInfo* info, unsigned char* output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int glyph);

void stbtt_Rasterize(stbtt__bitmap* result, float flatness_in_pixels, stbtt_vertex* vertices, int num_verts, float scale_x, float scale_y, float shift_x, float shift_y, int x_off, int y_off, int invert, void* userdata);

int stbtt_GetGlyphShape(const TTF_HeadInfo* info, int glyph_index, stbtt_vertex** pvertices);

void stbtt_PackEnd(stbtt_pack_context* spc);

void ImFontAtlasBuildFinish(class FontAtlas* atlas);

void stbtt_GetPackedQuad(const stbtt_packedchar* chardata, int pw, int ph, int char_index, float* xpos, float* ypos, stbtt_aligned_quad* q, int align_to_integer);

void ImFontAtlasBuildRender8bppRectFromString(class FontAtlas* atlas, int x, int y, int w, int h, const char* in_str, char in_marker_char, unsigned char in_marker_pixel_value);