#include "RiverPch.h"
#include "Utils/Header/FileUtils.h"
#include "Renderer/Font/Header/FontAtlas.h"
#include "Renderer/Font/Header/Font.h"


#define USE_TRUE_TYPE

#ifndef IM_DRAWLIST_TEX_LINES_WIDTH_MAX
	#define IM_DRAWLIST_TEX_LINES_WIDTH_MAX     (63)
#endif

FontAtlas::FontAtlas(const char* path, float pixelSize)
{
	auto f = W_OpenFile(path, "rb");
	uint64 file_size = FileGetSize(f);
	if (file_size == (uint64)-1) fclose(f);

	m_Data.resize(file_size);
	if (fread(m_Data.data(), 1, file_size, f) != file_size) fclose(f);

	fclose(f);
	m_Font = MakeUnique<Font>(path, this, pixelSize);
}

FontAtlas::~FontAtlas()
{
}

const uint8* FontAtlas::GetTextureDataRGBA32()
{
	if (m_PixelRGBA32.empty())
	{
		 const uint8* alphaPixels = GetTextureDataAsAlpha8();

		if (alphaPixels)
		{
			m_PixelRGBA32.resize(m_TextureWidth * m_TextureHeight);
			for (int i = 0; i < m_PixelRGBA32.size(); i++)
			{
				m_PixelRGBA32[i].r = *alphaPixels++;
				m_PixelRGBA32[i].g = 0;
				m_PixelRGBA32[i].b = 0;
				m_PixelRGBA32[i].a = m_PixelRGBA32[i].r;

				/*line = i / 4096;
				if (line < 1024 && i % 1024 < 512)
				{
					m_PixelRGBA32[i].g = 255;
				}*/
			}
		}
	}

	return (uint8*)m_PixelRGBA32.data();
}

const uint8* FontAtlas::GetTextureDataAsAlpha8()
{
	if (m_PixelAlpha8.empty())
	{
		Build();
	}

	return m_PixelAlpha8.data();
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

	m_PackIdMouseCursors = AddCustomRectRegular(FONT_ATLAS_DEFAULT_TEX_DATA_W * 2 + 1, FONT_ATLAS_DEFAULT_TEX_DATA_H);
	m_PackIdLines = AddCustomRectRegular(IM_DRAWLIST_TEX_LINES_WIDTH_MAX + 2, IM_DRAWLIST_TEX_LINES_WIDTH_MAX + 1);

	ImFontBuildSrcData srcData;
	memset(&srcData, 0, sizeof(ImFontBuildSrcData));

	static uint16 sRanges[] = { 0x20, std::numeric_limits<uint16>::max(), 0};
	srcData.SrcRanges = &sRanges[0];

	//1.初始化数据
	TTF_InitFont(&srcData.FontInfo, m_Data.data(), 0);
	for (const uint16* range = srcData.SrcRanges; range[0] && range[1]; range += 2)
	{
		assert(range[0] <= range[1]);
		srcData.GlyphsHighest = std::max(srcData.GlyphsHighest, (int)range[1]);
	}

	//2. 检查codepoint字体数据
	int total_glyphs_count = 0;
	BitArray tempBitArray;
	{
		tempBitArray.resize(srcData.GlyphsHighest + 1);
		for (const uint16* src_range = srcData.SrcRanges; src_range[0] && src_range[1]; src_range += 2)
		{
			for (unsigned int codepoint = src_range[0]; codepoint <= src_range[1]; codepoint++)
			{
				if (!TTF_FindGlyphIndex(&srcData.FontInfo, codepoint))
					continue;

				srcData.GlyphsCount++;
				tempBitArray[codepoint] = 1;
				total_glyphs_count++;
			}
		}
	}
	
	//3. 解包bit array 到 list
	{
		srcData.GlyphsList.resize(srcData.GlyphsCount);
		int index = 0;
		{
			for (int i = 0; i < tempBitArray.size(); i++)
			{
				if (tempBitArray[i])
				{
					srcData.GlyphsList[index++] = i;
				}
			}
		}

		assert(srcData.GlyphsList.size() == srcData.GlyphsCount);
	}

	V_Array<stbrp_rect> buf_rects;
	V_Array<stbtt_packedchar> buf_packedchars;
	buf_rects.resize(total_glyphs_count);
	buf_packedchars.resize(total_glyphs_count);
	memset(buf_rects.data(), 0, sizeof(stbrp_rect) * buf_rects.size());
	memset(buf_packedchars.data(), 0, sizeof(stbtt_packedchar) * buf_packedchars.size());

	//收集glyphs ,准备打包
	int total_surface = 0;
	{
		assert(srcData.GlyphsCount);

		srcData.Rects = &buf_rects[0];
		srcData.PackedChars = &buf_packedchars[0];

		// Convert our ranges in the format stb_truetype wants
		srcData.PackRange.font_size = m_Font->GetPixelSize();
		srcData.PackRange.first_unicode_codepoint_in_range = 0;
		srcData.PackRange.array_of_unicode_codepoints = srcData.GlyphsList.data();
		srcData.PackRange.num_chars = (int)srcData.GlyphsList.size();
		srcData.PackRange.chardata_for_range = srcData.PackedChars;
		srcData.PackRange.h_oversample = 2;
		srcData.PackRange.v_oversample = 1;

		const float scale = (srcData.PackRange.font_size > 0) ? stbtt_ScaleForPixelHeight(&srcData.FontInfo, srcData.PackRange.font_size) :
			stbtt_ScaleForMappingEmToPixels(&srcData.FontInfo, -srcData.PackRange.font_size);
		const int padding = 1;
		for (int glyph_i = 0; glyph_i < srcData.GlyphsList.size(); glyph_i++)
		{
			int x0, y0, x1, y1;
			const int glyph_index_in_font = TTF_FindGlyphIndex(&srcData.FontInfo, srcData.GlyphsList[glyph_i]);
			assert(glyph_index_in_font != 0);
			stbtt_GetGlyphBitmapBoxSubpixel(&srcData.FontInfo, glyph_index_in_font, scale * srcData.PackRange.h_oversample,
				scale * srcData.PackRange.v_oversample, 0, 0, &x0, &y0, &x1, &y1);
			srcData.Rects[glyph_i].w = (int)(x1 - x0 + padding + srcData.PackRange.h_oversample - 1);
			srcData.Rects[glyph_i].h = (int)(y1 - y0 + padding + srcData.PackRange.v_oversample - 1);
			total_surface += srcData.Rects[glyph_i].w * srcData.Rects[glyph_i].h;
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
	TTF_PackBegin(&spc, NULL, m_TextureWidth, TEX_HEIGHT_MAX, 0, 1, NULL);
	ImFontAtlasBuildPackCustomRects(this, spc.pack_info);

	//包裹每个字体，计算出每个字体的左下角位置与宽高
	{
		if (srcData.GlyphsCount == 0)
			return;//continue;

		TTF_PackRects((stbrp_context*)spc.pack_info, srcData.Rects, srcData.GlyphsCount);

		// Extend texture height and mark missing glyphs as non-packed so we won't render them.
		// FIXME: We are not handling packing failure here (would happen if we got off TEX_HEIGHT_MAX or if a single if larger than TexWidth?)
		for (int glyph_i = 0; glyph_i < srcData.GlyphsCount; glyph_i++)
			if (srcData.Rects[glyph_i].was_packed)
				m_TextureHeight = std::max(m_TextureHeight, srcData.Rects[glyph_i].y + srcData.Rects[glyph_i].h);
	}

	//分配texture
	m_TextureHeight = UpperPowerOfTwo(m_TextureHeight);
	m_TextureUvScale = FLOAT_2(1.0f / m_TextureWidth, 1.0f / m_TextureHeight);
	m_PixelAlpha8.resize(m_TextureWidth* m_TextureHeight);
	memset(m_PixelAlpha8.data(), 0, m_PixelAlpha8.size());
	spc.pixels = m_PixelAlpha8.data();
	spc.height = m_TextureHeight;

	// 8. Render/rasterize font characters into the texture
	if (srcData.GlyphsCount == 0)
		return;//continue;

	//将每个字体光栅化到alpha8上
	TTF_PackFontRangesRenderIntoRects(&spc, &srcData.FontInfo, &srcData.PackRange, 1, srcData.Rects);
	srcData.Rects = NULL;

	// End packing
	TTF_PackEnd(&spc);
	buf_rects.clear();

	// 9. Setup ImFont and glyphs for runtime
	//for (int src_i = 0; src_i < src_tmp_array.size(); src_i++)
	{
		// When merging fonts with MergeMode=true:
		// - We can have multiple input fonts writing into a same destination font.
		// - dst_font->ConfigData is != from cfg which is our source configuration.

		const float font_scale = stbtt_ScaleForPixelHeight(&srcData.FontInfo, 20.f);
		int unscaled_ascent, unscaled_descent, unscaled_line_gap;
		stbtt_GetFontVMetrics(&srcData.FontInfo, &unscaled_ascent, &unscaled_descent, &unscaled_line_gap);

		const float ascent = std::floorf(unscaled_ascent * font_scale + ((unscaled_ascent > 0.0f) ? +1 : -1));
		const float descent = std::floorf(unscaled_descent * font_scale + ((unscaled_descent > 0.0f) ? +1 : -1));
		//ImFontAtlasBuildSetupFont(this, dst_font, &cfg, ascent, descent);
		const float font_off_x = 0;
		const float font_off_y = 0 + std::round(13.0f);

		for (int glyph_i = 0; glyph_i < srcData.GlyphsCount; glyph_i++)
		{
			// Register glyph
			const int codepoint = srcData.GlyphsList[glyph_i];
			const stbtt_packedchar& pc = srcData.PackedChars[glyph_i];
			stbtt_aligned_quad q;
			float unused_x = 0.0f, unused_y = 0.0f;
			TTF_GetPackedQuad(srcData.PackedChars, m_TextureWidth, m_TextureHeight, glyph_i, &unused_x, &unused_y, &q, 0);
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
	return (int)m_CustomRects.size() - 1; // Return index
}
