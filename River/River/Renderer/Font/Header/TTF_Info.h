#pragma once

#include "RiverHead.h"
#include "MathStruct.h"

enum class TTF_HeadType
{
	head,		//字体头，字体的全局信息
	cmap,		//字符代码到图元的映射，将字符代码映射为图元的索引
	glyf,		//图元数据，图元的轮廓定义及网格调整指令
	maxp,		//最大需要求表，字体中所需内存分配情况的汇总数据
	mmtx,		//水平规格，图元水平规格
	loca,		//位置表索引，把图元索引转换为图元的位置
	name,       //命名表，版权说明、字体名、字体族名、风格名等等
	hhea,		//水平布局头,包含布局其字符水平书写的字体所需的信息，如：从左到右或从右到左、基于基线上升/下降、倾斜等
	hmtx,		//水平度量，字体水平布局星系：上高、下高、行间距、最大前进宽度、最小左支撑、最小右支撑
	kerm,		//字距调整表，字距调整对的数组
	post,		//PostScript信息，所有图元的PostScript   FontInfo目录项和PostScript名
};

struct TTF_Table_Directory
{
	uint32 Version;
	uint16 TableNum;
	uint16 SearchRange;
	uint16 EntrySelector;
	uint16 RangeShift;
};

struct TTF_Table
{
	uint32 Tag;
	uint32 CheckSum;
	uint32 Offset;
	uint32 Length;
};

struct TTF_Head
{
	uint32_t					m_Version;
	uint32_t					m_FontRevision;
	uint32_t				m_ChecksumAdjustment;
	uint32_t				m_MagicNumber;
	uint16_t				m_Flags;
	uint16_t				m_UnitsPerEm;
	int64_t	m_Created;
	int64_t	m_Modified;
	int16_t					m_xMin;
	int16_t					m_yMin;
	int16_t					m_xMax;
	int16_t					m_yMax;
	uint16_t				m_macType;
	uint16_t				m_lowestRecPPEM;
	int16_t					m_fondDirectionHint;
	int16_t					m_indexToLocFormat;
	int16_t					m_glyphDataFormat;
};

struct TTF_EncodingRecord
{
	uint16 PlatformId;
	uint16 EncodingId;
	uint32 Offset;
};

struct TTF_CMap
{
	uint16 Format;
	uint16 Length;
	//TTF_EncodingRecord EncodingRecords[Length];
	//Todo
};

struct TTF_Glyph
{
	uint16 ContourNum;		//如果轮廓数为正数或零，则为单个字形； 如果轮廓数小于零，则字形为复合字形
	uint16 MinX;
	uint16 MinY;
	uint16 MaxX;
	uint16 MaxY;
};

//简单字形的数据定义，主要通过 xCoordinates、yCoordinates、endPtsOfContours 和 flags 确定字形的每个轮廓已经矢量方向，
//然后通过 instructionLength、instructions 描述的指令调整字形的最终显示。

struct TTF_GlyphSingle
{
	uint16 EndPtsOfContours;		//[n]个 每个轮廓的最后一个点的数组；n 是轮廓的数量；数组项是每个点的索引
	uint16 InstructionLength;		//指令所需的总字节数
	uint8 instructions;				//此字形的指令数组，长度为InstructionLength
	uint8 Flags;					//m个，标志数组，描述是否在曲线上、是否重复等情况
	uint8 xCoordinates;				//m个，x 坐标数组；第一个是相对于（0,0），其他是相对于前一点
	uint8 yCoordinates;				//m个，y 坐标数组；第一个是相对于（0,0），其他是相对于前一点
};

RIVER_API const int FONT_ATLAS_DEFAULT_TEX_DATA_W = 122; // Actual texture will be 2 times that + 1 spacing.
RIVER_API const int FONT_ATLAS_DEFAULT_TEX_DATA_H = 27;

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
	TTF_Table_Directory HeadDirectory;

	void* userdata;
	uint8* data;              // pointer to .ttf file
	int fontstart;         // offset of start of font

	int numGlyphs;
	int loca, head, glyf, hhea, hmtx, kern, gpos, svg;
	int indexMap;
	int indexToLocFormat;             //0 是 short, 1 是 int

	stbtt__buf cff;                    // cff font data
	stbtt__buf charstrings;            // the charstring index
	stbtt__buf gsubrs;                 // global charstring subroutines index
	stbtt__buf subrs;                  // private charstring subroutines index
	stbtt__buf fontdicts;              // array of font dicts
	stbtt__buf fdselect;               // map from glyph to fontdict
};

//big end byte 
inline int16 GetInt16(const uint8_t* p) { return (p[0] << 8) + p[1]; }
inline uint16 GetUInt16(const uint8* p) { return (p[0] << 8) + p[1]; }
inline int GetInt32(const uint8_t* p) { return (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3]; }
inline uint32 GetUInt32(const uint8* p) { return (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3]; }

inline int UpperPowerOfTwo(int v) { v--; v |= v >> 1; v |= v >> 2; v |= v >> 4; v |= v >> 8; v |= v >> 16; v++; return v; }

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
	int GlyphsHighest;      // 最大Unicode码值 requested codepoint
	int                 GlyphsCount;        // Glyph count (excluding missing glyphs and glyphs already set by an earlier source font)
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

inline float stbtt_ScaleForPixelHeight(const TTF_HeadInfo* info, float height)
{
	int fheight = GetInt16(info->data + info->hhea + 4) - GetInt16(info->data + info->hhea + 6);
	return (float)height / fheight;
}

inline float stbtt_ScaleForMappingEmToPixels(const TTF_HeadInfo* info, float pixels)
{
	int unitsPerEm = GetUInt16(info->data + info->head + 18);
	return pixels / unitsPerEm;
}

inline int TTF_GetGlyfOffset(const TTF_HeadInfo* info, int glyph_index)
{
	int g1, g2;

	if (glyph_index >= info->numGlyphs) return -1; // glyph index out of range
	if (info->indexToLocFormat >= 2)    return -1; // unknown index->glyph map format

	if (info->indexToLocFormat == 0)
	{
		g1 = info->glyf + GetUInt16(info->data + info->loca + glyph_index * 2) * 2;
		g2 = info->glyf + GetUInt16(info->data + info->loca + glyph_index * 2 + 2) * 2;
	}
	else
	{
		g1 = info->glyf + GetUInt32(info->data + info->loca + glyph_index * 4);
		g2 = info->glyf + GetUInt32(info->data + info->loca + glyph_index * 4 + 4);
	}

	return g1 == g2 ? -1 : g1; // if length is 0, return -1
}

inline int stbtt_GetGlyphBox(const TTF_HeadInfo* info, int glyph_index, int* x0, int* y0, int* x1, int* y1)
{
	int g = TTF_GetGlyfOffset(info, glyph_index);
	if (g < 0) return 0;

	if (x0) *x0 = GetInt16(info->data + g + 2);
	if (y0) *y0 = GetInt16(info->data + g + 4);
	if (x1) *x1 = GetInt16(info->data + g + 6);
	if (y1) *y1 = GetInt16(info->data + g + 8);
	
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
		if (ix0) *ix0 = (int)floorf(x0 * scale_x + shift_x);
		if (iy0) *iy0 = (int)floorf(-y1 * scale_y + shift_y);
		if (ix1) *ix1 = (int)ceilf(x1 * scale_x + shift_x);
		if (iy1) *iy1 = (int)ceilf(-y0 * scale_y + shift_y);
	}
}

inline void stbtt_GetGlyphHMetrics(const TTF_HeadInfo* info, int glyph_index, int* advanceWidth, int* leftSideBearing)
{
	uint16 numOfLongHorMetrics = GetUInt16(info->data + info->hhea + 34);
	if (glyph_index < numOfLongHorMetrics)
	{
		if (advanceWidth)     *advanceWidth = GetInt16(info->data + info->hmtx + 4 * glyph_index);
		if (leftSideBearing)  *leftSideBearing = GetInt16(info->data + info->hmtx + 4 * glyph_index + 2);
	}
	else 
	{
		if (advanceWidth)     *advanceWidth = GetInt16(info->data + info->hmtx + 4 * (numOfLongHorMetrics - 1));
		if (leftSideBearing)  *leftSideBearing = GetInt16(info->data + info->hmtx + 4 * numOfLongHorMetrics + 2 * (glyph_index - numOfLongHorMetrics));
	}
}

inline void stbtt_GetGlyphBitmapBox(const TTF_HeadInfo* font, int glyph, float scale_x, float scale_y, int* ix0, int* iy0, int* ix1, int* iy1)
{
	stbtt_GetGlyphBitmapBoxSubpixel(font, glyph, scale_x, scale_y, 0.0f, 0.0f, ix0, iy0, ix1, iy1);
}

inline void stbtt_GetFontVMetrics(const TTF_HeadInfo* info, int* ascent, int* descent, int* lineGap)
{
	if (ascent) *ascent = GetInt16(info->data + info->hhea + 4);
	if (descent) *descent = GetInt16(info->data + info->hhea + 6);
	if (lineGap) *lineGap = GetInt16(info->data + info->hhea + 8);
}

uint32 TTF_FindTable(TTF_HeadInfo* info, const char* tag);

int TTF_InitFont(TTF_HeadInfo* info, uint8* data, int fontstart);

int TTF_FindGlyphIndex(const TTF_HeadInfo* info, int unicode_codepoint);

int TTF_PackBegin(stbtt_pack_context* spc, unsigned char* pixels, int pw, int ph, int stride_in_bytes, int padding, void* alloc_context);

void stbrp_init_target(stbrp_context* context, int width, int height, stbrp_node* nodes, int num_nodes);

void ImFontAtlasBuildPackCustomRects(class FontAtlas* font, void* stbrp_context_opaque);

int TTF_PackRects(stbrp_context* context, stbrp_rect* rects, int num_rects);

int TTF_PackFontRangesRenderIntoRects(stbtt_pack_context* spc, const TTF_HeadInfo* info, stbtt_pack_range* ranges, int num_ranges, stbrp_rect* rects);

void TTF_MakeGlyphBitmapSubpixel(const TTF_HeadInfo* info, unsigned char* output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int glyph);

void TTF_Rasterize(stbtt__bitmap* result, float flatness_in_pixels, stbtt_vertex* vertices, int num_verts, float scale_x, float scale_y, float shift_x, float shift_y, int x_off, int y_off, int invert, void* userdata);

int TTF_GetGlyphShape(const TTF_HeadInfo* info, int glyph_index, stbtt_vertex** pvertices);

void TTF_PackEnd(stbtt_pack_context* spc);

void ImFontAtlasBuildFinish(class FontAtlas* atlas);

void TTF_GetPackedQuad(const stbtt_packedchar* chardata, int pw, int ph, int char_index, float* xpos, float* ypos, stbtt_aligned_quad* q, int align_to_integer);

void ImFontAtlasBuildRender8bppRectFromString(class FontAtlas* atlas, int x, int y, int w, int h, const char* in_str, char in_marker_char, unsigned char in_marker_pixel_value);