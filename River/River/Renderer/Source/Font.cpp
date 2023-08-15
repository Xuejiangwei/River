#include "RiverPch.h"
#include "Font.h"

#ifdef _WIN32
#include <Windows.h>
#endif // _WIN32

#define stbtt_tag4(p,c0,c1,c2,c3) ((p)[0] == (c0) && (p)[1] == (c1) && (p)[2] == (c2) && (p)[3] == (c3))
#define stbtt_tag(p,str)           stbtt_tag4(p,str[0],str[1],str[2],str[3])

static uint32_t ttULONG(unsigned char* p) { return (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3]; }
static unsigned short ttUSHORT(unsigned char* p) { return p[0] * 256 + p[1]; }

static uint32_t stbtt__find_table(unsigned char* data, uint32_t fontstart, const char* tag)
{
    int num_tables = ttUSHORT(data + fontstart + 4);
    uint32_t tabledir = fontstart + 12;
    uint32_t i;
    for (i = 0; i < num_tables; ++i) 
    {
        uint32_t loc = tabledir + 16 * i;
        if (stbtt_tag(data + loc + 0, tag))
            return ttULONG(data + loc + 8);
    }
    return 0;
}

uint64_t GetSize(FILE* f)
{
    uint64_t off = 0, sz = 0;
    return ((off = ftell(f)) != -1 && !fseek(f, 0, SEEK_END) && (sz = ftell(f)) != -1 && 
        !fseek(f, off, SEEK_SET)) ? sz : -1;
}

Font::Font(const char* path)
{
#if defined(_WIN32) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS) && !defined(__CYGWIN__) && !defined(__GNUC__)
    const int filename_wsize = ::MultiByteToWideChar(CP_UTF8, 0, path, -1, NULL, 0);
    const int mode_wsize = ::MultiByteToWideChar(CP_UTF8, 0, "rb", -1, NULL, 0);
    V_Array<wchar_t> buf;
    buf.resize(filename_wsize + mode_wsize);
    ::MultiByteToWideChar(CP_UTF8, 0, path, -1, (wchar_t*)&buf[0], filename_wsize);
    ::MultiByteToWideChar(CP_UTF8, 0, "rb", -1, (wchar_t*)&buf[filename_wsize], mode_wsize);
    
    auto file = ::_wfopen((const wchar_t*)&buf[0], (const wchar_t*)&buf[filename_wsize]);
    auto fileSize = GetSize(file);
    m_Data.resize(fileSize);
    fread(m_Data.begin()._Unwrapped(), 1, fileSize, file);
    fclose(file);

#else
    auto file = fopen(filename, mode);
#endif
}

Font::~Font()
{

}

//https://www.cnblogs.com/sjhrun2001/archive/2010/01/19/1651274.html
void Font::LoadTtfFileData(const char* path)
{
    auto data = m_Data.data();
    auto fontstart = 0;
    TtfHead info;
    auto cmap = stbtt__find_table(data, fontstart, "cmap");       // required
    info.loca = stbtt__find_table(data, fontstart, "loca");             // required
    info.head = stbtt__find_table(data, fontstart, "head");             // required
    info.glyf = stbtt__find_table(data, fontstart, "glyf");             // required
    info.hhea = stbtt__find_table(data, fontstart, "hhea");             // required
    info.hmtx = stbtt__find_table(data, fontstart, "hmtx");             // required
    info.kern = stbtt__find_table(data, fontstart, "kern");             // not required
    info.gpos = stbtt__find_table(data, fontstart, "GPOS");             // not required

    if (!cmap || !info.head || !info.hhea || !info.hmtx)
        return;
    if (info.glyf)
    {
        // required for truetype
        if (!info.loca) return;
    }
    else {
        // initialization for CFF / Type2 fonts (OTF)

    //    typedef struct
    //    {
    //        unsigned char* data;
    //        int cursor;
    //        int size;
    //    } stbtt__buf;
    //    stbtt__buf b, topdict, topdictidx;
    //    uint32_t cstype = 2, charstrings = 0, fdarrayoff = 0, fdselectoff = 0;
    //    uint32_t cff;

    //    cff = stbtt__find_table(data, fontstart, "CFF ");
    //    if (!cff) return;

    //    info.fontdicts = stbtt__new_buf(NULL, 0);
    //    info.fdselect = stbtt__new_buf(NULL, 0);

    //    // @TODO this should use size from table (not 512MB)
    //    info->cff = stbtt__new_buf(data + cff, 512 * 1024 * 1024);
    //    b = info->cff;

    //    // read the header
    //    stbtt__buf_skip(&b, 2);
    //    stbtt__buf_seek(&b, stbtt__buf_get8(&b)); // hdrsize

    //    // @TODO the name INDEX could list multiple fonts,
    //    // but we just use the first one.
    //    stbtt__cff_get_index(&b);  // name INDEX
    //    topdictidx = stbtt__cff_get_index(&b);
    //    topdict = stbtt__cff_index_get(topdictidx, 0);
    //    stbtt__cff_get_index(&b);  // string INDEX
    //    info->gsubrs = stbtt__cff_get_index(&b);

    //    stbtt__dict_get_ints(&topdict, 17, 1, &charstrings);
    //    stbtt__dict_get_ints(&topdict, 0x100 | 6, 1, &cstype);
    //    stbtt__dict_get_ints(&topdict, 0x100 | 36, 1, &fdarrayoff);
    //    stbtt__dict_get_ints(&topdict, 0x100 | 37, 1, &fdselectoff);
    //    info->subrs = stbtt__get_subrs(b, topdict);

    //    // we only support Type 2 charstrings
    //    if (cstype != 2) return 0;
    //    if (charstrings == 0) return 0;

    //    if (fdarrayoff) {
    //        // looks like a CID font
    //        if (!fdselectoff) return 0;
    //        stbtt__buf_seek(&b, fdarrayoff);
    //        info->fontdicts = stbtt__cff_get_index(&b);
    //        info->fdselect = stbtt__buf_range(&b, fdselectoff, b.size - fdselectoff);
    //    }

    //    stbtt__buf_seek(&b, charstrings);
    //    info->charstrings = stbtt__cff_get_index(&b);
    //}

    //t = stbtt__find_table(data, fontstart, "maxp");
    //if (t)
    //    info->numGlyphs = ttUSHORT(data + t + 4);
    //else
    //    info->numGlyphs = 0xffff;

    //info->svg = -1;

    //// find a cmap encoding table we understand *now* to avoid searching
    //// later. (todo: could make this installable)
    //// the same regardless of glyph.
    //numTables = ttUSHORT(data + cmap + 2);
    //info->index_map = 0;
    //for (i = 0; i < numTables; ++i) {
    //    stbtt_uint32 encoding_record = cmap + 4 + 8 * i;
    //    // find an encoding we understand:
    //    switch (ttUSHORT(data + encoding_record)) {
    //    case STBTT_PLATFORM_ID_MICROSOFT:
    //        switch (ttUSHORT(data + encoding_record + 2)) {
    //        case STBTT_MS_EID_UNICODE_BMP:
    //        case STBTT_MS_EID_UNICODE_FULL:
    //            // MS/Unicode
    //            info->index_map = cmap + ttULONG(data + encoding_record + 4);
    //            break;
    //        }
    //        break;
    //    case STBTT_PLATFORM_ID_UNICODE:
    //        // Mac/iOS has these
    //        // all the encodingIDs are unicode, so we don't bother to check it
    //        info->index_map = cmap + ttULONG(data + encoding_record + 4);
    //        break;
    //    }
    //}
    //if (info->index_map == 0)
    //    return 0;

    //info->indexToLocFormat = ttUSHORT(data + info->head + 50);
    //return 1;
    }
}

void Font::LoadFntFileData(const char* path)
{
}
