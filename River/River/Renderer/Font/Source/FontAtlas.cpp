#include "RiverPch.h"
#include "Renderer/Font/Header/FontAtlas.h"
#include "Renderer/Font/Header/Font.h"

#include <Windows.h>

#define USE_TRUE_TYPE

#ifndef IM_DRAWLIST_TEX_LINES_WIDTH_MAX
	#define IM_DRAWLIST_TEX_LINES_WIDTH_MAX     (63)
#endif

bool    ImFileClose(FILE* f) { return fclose(f) == 0; }
uint64   ImFileGetSize(FILE* f) { long off = 0, sz = 0; return ((off = ftell(f)) != -1 && !fseek(f, 0, SEEK_END) && (sz = ftell(f)) != -1 && !fseek(f, off, SEEK_SET)) ? (uint64)sz : (uint64)-1; }
uint64   ImFileRead(void* data, uint64 sz, uint64 count, FILE* f) { return fread(data, (size_t)sz, (size_t)count, f); }
uint64   ImFileWrite(const void* data, uint64 sz, uint64 count, FILE* f) { return fwrite(data, (size_t)sz, (size_t)count, f); }

FontAtlas::FontAtlas(const char* path, float pixelSize)
{
	const int filename_wsize = ::MultiByteToWideChar(CP_UTF8, 0, path, -1, NULL, 0);
	const int mode_wsize = ::MultiByteToWideChar(CP_UTF8, 0, "rb", -1, NULL, 0);
	V_Array<wchar_t> buf;
	buf.resize(filename_wsize + mode_wsize);
	::MultiByteToWideChar(CP_UTF8, 0, path, -1, (wchar_t*)&buf[0], filename_wsize);
	::MultiByteToWideChar(CP_UTF8, 0, "rb", -1, (wchar_t*)&buf[filename_wsize], mode_wsize);
	auto f =::_wfopen((const wchar_t*)&buf[0], (const wchar_t*)&buf[filename_wsize]);

	size_t file_size = (size_t)ImFileGetSize(f);
	if (file_size == (size_t)-1)
	{
		ImFileClose(f);
	}

	m_Data.resize(file_size);
	if (ImFileRead(m_Data.data(), 1, file_size, f) != file_size)
	{
		ImFileClose(f);
	}

	ImFileClose(f);

	m_Font = new Font(path, this, pixelSize);
}

FontAtlas::~FontAtlas()
{
	if (m_Font)
	{
		delete m_Font;
	}
}

void FontAtlas::GetTextureDataRGBA32(uint8** outPixels)
{
	if (m_PixelRGBA32.empty())
	{
		uint8* alphaPixels = nullptr;
		GetTextureDataAsAlpha8(&alphaPixels);

		if (alphaPixels)
		{
			m_PixelRGBA32.resize(m_TextureWidth * m_TextureHeight);
			for (int i = 0; i < m_PixelRGBA32.size(); i++)
			{
				m_PixelRGBA32[i].r = 255;
				m_PixelRGBA32[i].g = 255;
				m_PixelRGBA32[i].b = 255;
				m_PixelRGBA32[i].a = *alphaPixels++;
			}
		}
	}

	*outPixels = (uint8*)m_PixelRGBA32.data();
}

void FontAtlas::GetTextureDataAsAlpha8(uint8** outPixels)
{
	if (m_PixelAlpha8.empty())
	{
		Build();
	}

	*outPixels = m_PixelAlpha8.data();
}

void FontAtlas::Build()
{
#ifdef USE_TRUE_TYPE
	BuildTrueType();
#endif // USE_TRUE_TYPE

}

void FontAtlas::BuildTrueType()
{
	//Clear data
	m_TextureWidth = m_TextureHeight = 0;
	m_TextureUvScale = { 0.0f, 0.0f };
	m_TextureUvWhitePixel = { 0.0f, 0.0f };
	m_PixelAlpha8.clear();
	m_PixelRGBA32.clear();

	m_PackIdMouseCursors = AddCustomRectRegular(122 * 2 + 1, 27);
	m_PackIdLines = AddCustomRectRegular(IM_DRAWLIST_TEX_LINES_WIDTH_MAX + 2, IM_DRAWLIST_TEX_LINES_WIDTH_MAX + 1);

	V_Array<ImFontBuildSrcData> src_tmp_array;
	V_Array<ImFontBuildDstData> dst_tmp_array;
	src_tmp_array.resize(1);
	dst_tmp_array.resize(1);
	memset(src_tmp_array.data(), 0, sizeof(ImFontBuildSrcData) * src_tmp_array.size());
	memset(dst_tmp_array.data(), 0, sizeof(ImFontBuildDstData) * dst_tmp_array.size());

	static uint16 sRanges[] = {0x20, 0x2eeef, 0 };

	//1.初始化数据
	for (size_t i = 0; i < src_tmp_array.size(); i++)
	{
		auto& src_tmp = src_tmp_array[i];
		stbtt_InitFont(&src_tmp.FontInfo, m_Data.data(), 0);

		src_tmp_array[0].SrcRanges = &sRanges[0];
		for (const uint16* src_range = src_tmp.SrcRanges; src_range[0] && src_range[1]; src_range += 2)
		{
			// Check for valid range. This may also help detect *some* dangling pointers, because a common
			// user error is to setup ImFontConfig::GlyphRanges with a pointer to data that isn't persistent.
			assert(src_range[0] <= src_range[1]);
			src_tmp.GlyphsHighest = max(src_tmp.GlyphsHighest, (int)src_range[1]);
		}
		dst_tmp_array[i].SrcCount++;
		dst_tmp_array[i].GlyphsHighest = max(dst_tmp_array[i].GlyphsHighest, src_tmp.GlyphsHighest);
	}

	//2. 检查codepoint字体数据
	int total_glyphs_count = 0;
	for (int src_i = 0; src_i < src_tmp_array.size(); src_i++)
	{
		ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
		ImFontBuildDstData& dst_tmp = dst_tmp_array[src_tmp.DstIndex];
		src_tmp.GlyphsSet.Create(src_tmp.GlyphsHighest + 1);
		if (dst_tmp.GlyphsSet.Storage.empty())
			dst_tmp.GlyphsSet.Create(dst_tmp.GlyphsHighest + 1);

		for (const uint16* src_range = src_tmp.SrcRanges; src_range[0] && src_range[1]; src_range += 2)
			for (unsigned int codepoint = src_range[0]; codepoint <= src_range[1]; codepoint++)
			{
				if (dst_tmp.GlyphsSet.TestBit(codepoint))    // Don't overwrite existing glyphs. We could make this an option for MergeMode (e.g. MergeOverwrite==true)
					continue;
				if (!stbtt_FindGlyphIndex(&src_tmp.FontInfo, codepoint))    // It is actually in the font?
					continue;

				// Add to avail set/counters
				src_tmp.GlyphsCount++;
				dst_tmp.GlyphsCount++;
				src_tmp.GlyphsSet.SetBit(codepoint);
				dst_tmp.GlyphsSet.SetBit(codepoint);
				total_glyphs_count++;
			}
	}
	
	// 解包bit map 到 list
	for (int src_i = 0; src_i < src_tmp_array.size(); src_i++)
	{
		ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
		src_tmp.GlyphsList.resize(src_tmp.GlyphsCount);
		int index = 0;
		{
			assert(sizeof(src_tmp.GlyphsSet.Storage[0]) == sizeof(int));
			const uint32* it_begin = src_tmp.GlyphsSet.Storage.begin()._Unwrapped();
			const uint32* it_end = src_tmp.GlyphsSet.Storage.end()._Unwrapped();
			for (const uint32* it = it_begin; it < it_end; it++)
				if (uint32 entries_32 = *it)
					for (uint32 bit_n = 0; bit_n < 32; bit_n++)
						if (entries_32 & ((uint32)1 << bit_n))
						{
							int v = (int)(((it - it_begin) << 5) + bit_n);
							src_tmp.GlyphsList[index++] = v;
						}
		}
		src_tmp.GlyphsSet.Clear();
		assert(src_tmp.GlyphsList.size() == src_tmp.GlyphsCount);
	}
	for (int dst_i = 0; dst_i < dst_tmp_array.size(); dst_i++)
		dst_tmp_array[dst_i].GlyphsSet.Clear();
	dst_tmp_array.clear();

	V_Array<stbrp_rect> buf_rects;
	V_Array<stbtt_packedchar> buf_packedchars;
	buf_rects.resize(total_glyphs_count);
	buf_packedchars.resize(total_glyphs_count);
	memset(buf_rects.data(), 0, sizeof(stbrp_rect) * buf_rects.size());
	memset(buf_packedchars.data(), 0, sizeof(stbtt_packedchar) * buf_packedchars.size());

	//收集glyphs ,准备打包
	int total_surface = 0;
	int buf_rects_out_n = 0;
	int buf_packedchars_out_n = 0;
	for (int src_i = 0; src_i < src_tmp_array.size(); src_i++)
	{
		ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
		if (src_tmp.GlyphsCount == 0)
			continue;

		src_tmp.Rects = &buf_rects[buf_rects_out_n];
		src_tmp.PackedChars = &buf_packedchars[buf_packedchars_out_n];
		buf_rects_out_n += src_tmp.GlyphsCount;
		buf_packedchars_out_n += src_tmp.GlyphsCount;

		// Convert our ranges in the format stb_truetype wants
		src_tmp.PackRange.font_size = 20.f;
		src_tmp.PackRange.first_unicode_codepoint_in_range = 0;
		src_tmp.PackRange.array_of_unicode_codepoints = src_tmp.GlyphsList.data();
		src_tmp.PackRange.num_chars = src_tmp.GlyphsList.size();
		src_tmp.PackRange.chardata_for_range = src_tmp.PackedChars;
		src_tmp.PackRange.h_oversample = 2;
		src_tmp.PackRange.v_oversample = 1;

		// Gather the sizes of all rectangles we will need to pack (this loop is based on stbtt_PackFontRangesGatherRects)
		const float scale = (src_tmp.PackRange.font_size > 0) ? stbtt_ScaleForPixelHeight(&src_tmp.FontInfo, src_tmp.PackRange.font_size) :
			stbtt_ScaleForMappingEmToPixels(&src_tmp.FontInfo, -src_tmp.PackRange.font_size);
		const int padding = 1;
		for (int glyph_i = 0; glyph_i < src_tmp.GlyphsList.size(); glyph_i++)
		{
			int x0, y0, x1, y1;
			const int glyph_index_in_font = stbtt_FindGlyphIndex(&src_tmp.FontInfo, src_tmp.GlyphsList[glyph_i]);
			assert(glyph_index_in_font != 0);
			stbtt_GetGlyphBitmapBoxSubpixel(&src_tmp.FontInfo, glyph_index_in_font, scale * src_tmp.PackRange.h_oversample,
				scale * src_tmp.PackRange.v_oversample, 0, 0, &x0, &y0, &x1, &y1);
			src_tmp.Rects[glyph_i].w = (int)(x1 - x0 + padding + src_tmp.PackRange.h_oversample - 1);
			src_tmp.Rects[glyph_i].h = (int)(y1 - y0 + padding + src_tmp.PackRange.v_oversample - 1);
			total_surface += src_tmp.Rects[glyph_i].w * src_tmp.Rects[glyph_i].h;
		}
	}

	// We need a width for the skyline algorithm, any width!
	// The exact width doesn't really matter much, but some API/GPU have texture size limitations and increasing width can decrease height.
	// User can override TexDesiredWidth and TexGlyphPadding if they wish, otherwise we use a simple heuristic to select the width based on expected surface.
	const int surface_sqrt = (int)std::sqrt((float)total_surface) + 1;
	m_TextureHeight = 0;
	/*if (atlas->TexDesiredWidth > 0)
		atlas->TexWidth = atlas->TexDesiredWidth;
	else*/
	{
		m_TextureWidth = (surface_sqrt >= 4096 * 0.7f) ? 4096 : (surface_sqrt >= 2048 * 0.7f) ? 2048 : (surface_sqrt >= 1024 * 0.7f) ? 1024 : 512;
	}

	//开始打包
	const int TEX_HEIGHT_MAX = 1024 * 32;
	stbtt_pack_context spc = {};
	stbtt_PackBegin(&spc, NULL, m_TextureWidth, TEX_HEIGHT_MAX, 0, 1, NULL);
	ImFontAtlasBuildPackCustomRects(this, spc.pack_info);

	//打包每个字体
	for (int src_i = 0; src_i < src_tmp_array.size(); src_i++)
	{
		ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
		if (src_tmp.GlyphsCount == 0)
			continue;

		stbrp_pack_rects((stbrp_context*)spc.pack_info, src_tmp.Rects, src_tmp.GlyphsCount);

		// Extend texture height and mark missing glyphs as non-packed so we won't render them.
		// FIXME: We are not handling packing failure here (would happen if we got off TEX_HEIGHT_MAX or if a single if larger than TexWidth?)
		for (int glyph_i = 0; glyph_i < src_tmp.GlyphsCount; glyph_i++)
			if (src_tmp.Rects[glyph_i].was_packed)
				m_TextureHeight = max(m_TextureHeight, src_tmp.Rects[glyph_i].y + src_tmp.Rects[glyph_i].h);
	}

	//分配texture
	m_TextureHeight = ImUpperPowerOfTwo(m_TextureHeight);
	m_TextureUvScale = FLOAT_2(1.0f / m_TextureWidth, 1.0f / m_TextureHeight);
	m_PixelAlpha8.resize(m_TextureWidth* m_TextureHeight);
	memset(m_PixelAlpha8.data(), 0, m_PixelAlpha8.size());
	spc.pixels = m_PixelAlpha8.data();
	spc.height = m_TextureHeight;

	// 8. Render/rasterize font characters into the texture
	for (int src_i = 0; src_i < src_tmp_array.size(); src_i++)
	{
		ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
		if (src_tmp.GlyphsCount == 0)
			continue;

		stbtt_PackFontRangesRenderIntoRects(&spc, &src_tmp.FontInfo, &src_tmp.PackRange, 1, src_tmp.Rects);

		// Apply multiply operator
		/*if (cfg.RasterizerMultiply != 1.0f)
		{
			unsigned char multiply_table[256];
			ImFontAtlasBuildMultiplyCalcLookupTable(multiply_table, cfg.RasterizerMultiply);
			stbrp_rect* r = &src_tmp.Rects[0];
			for (int glyph_i = 0; glyph_i < src_tmp.GlyphsCount; glyph_i++, r++)
				if (r->was_packed)
					ImFontAtlasBuildMultiplyRectAlpha8(multiply_table, atlas->TexPixelsAlpha8, r->x, r->y, r->w, r->h, atlas->TexWidth * 1);
		}*/
		src_tmp.Rects = NULL;
	}

	// End packing
	stbtt_PackEnd(&spc);
	buf_rects.clear();

	// 9. Setup ImFont and glyphs for runtime
	for (int src_i = 0; src_i < src_tmp_array.size(); src_i++)
	{
		// When merging fonts with MergeMode=true:
		// - We can have multiple input fonts writing into a same destination font.
		// - dst_font->ConfigData is != from cfg which is our source configuration.
		ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];

		const float font_scale = stbtt_ScaleForPixelHeight(&src_tmp.FontInfo, 20.f);
		int unscaled_ascent, unscaled_descent, unscaled_line_gap;
		stbtt_GetFontVMetrics(&src_tmp.FontInfo, &unscaled_ascent, &unscaled_descent, &unscaled_line_gap);

		const float ascent = std::floorf(unscaled_ascent * font_scale + ((unscaled_ascent > 0.0f) ? +1 : -1));
		const float descent = std::floorf(unscaled_descent * font_scale + ((unscaled_descent > 0.0f) ? +1 : -1));
		//ImFontAtlasBuildSetupFont(this, dst_font, &cfg, ascent, descent);
		const float font_off_x = 0;
		const float font_off_y = 0 + std::round(13.0f);

		for (int glyph_i = 0; glyph_i < src_tmp.GlyphsCount; glyph_i++)
		{
			// Register glyph
			const int codepoint = src_tmp.GlyphsList[glyph_i];
			const stbtt_packedchar& pc = src_tmp.PackedChars[glyph_i];
			stbtt_aligned_quad q;
			float unused_x = 0.0f, unused_y = 0.0f;
			stbtt_GetPackedQuad(src_tmp.PackedChars, m_TextureWidth, m_TextureHeight, glyph_i, &unused_x, &unused_y, &q, 0);
			m_Font->AddGlyph((uint16)codepoint, q.x0 + font_off_x, q.y0 + font_off_y, q.x1 + font_off_x, q.y1 + font_off_y, q.s0, q.t0, q.s1, q.t1, pc.xadvance);
		}
	}

	ImFontAtlasBuildFinish(this);
	return;
}

int FontAtlas::AddCustomRectRegular(int width, int height)
{
	assert(width > 0 && width <= 0xFFFF);
	assert(height > 0 && height <= 0xFFFF);
	FontAtlasCustomRect r;
	r.Width = (unsigned short)width;
	r.Height = (unsigned short)height;
	m_CustomRects.push_back(r);
	return m_CustomRects.size() - 1; // Return index
}
