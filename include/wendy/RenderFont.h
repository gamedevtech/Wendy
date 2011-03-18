///////////////////////////////////////////////////////////////////////
// Wendy default renderer
// Copyright (c) 2006 Camilla Berglund <elmindreda@elmindreda.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any
// damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any
// purpose, including commercial applications, and to alter it and
// redistribute it freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you
//     must not claim that you wrote the original software. If you use
//     this software in a product, an acknowledgment in the product
//     documentation would be appreciated but is not required.
//
//  2. Altered source versions must be plainly marked as such, and
//     must not be misrepresented as being the original software.
//
//  3. This notice may not be removed or altered from any source
//     distribution.
//
///////////////////////////////////////////////////////////////////////
#ifndef WENDY_RENDERFONT_H
#define WENDY_RENDERFONT_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {

///////////////////////////////////////////////////////////////////////

class FontGlyphData
{
public:
  vec2 bearing;
  float advance;
  Ref<Image> image;
};

///////////////////////////////////////////////////////////////////////

class FontData
{
public:
  typedef std::vector<FontGlyphData> GlyphList;
  FontData(void);
  FontData(const FontData& source);
  FontData& operator = (const FontData& source);
  GlyphList glyphs;
  int characters[256];
};

///////////////////////////////////////////////////////////////////////

/*! @brief %Font layout and rendering object.
 *
 *  This class provides layout and rendering of a single font, using data
 *  from a wendy::Font object.
 */
class Font : public Resource
{
public:
  class Layout;
  typedef std::vector<Layout> LayoutList;
  /*! Renders the specified text at the current pen position.
   *  @param text The text to render.
   */
  void drawText(const vec2& penPosition, const vec4& color, const String& text) const;
  /*! Renders the specified text at the current pen position.
   *  @param format The format string for the text to render.
   */
  void drawText(const vec2& penPosition, const vec4& color, const char* format, ...) const;
  /*! @return The width, in pixels, of the character cell for this font.
   */
  float getWidth(void) const;
  /*! @return The height, in pixels, of the character cell for this font.
   */
  float getHeight(void) const;
  /*! @return The ascender for this font.
   */
  float getAscender(void) const;
  /*! @return The descender for this font.
   */
  float getDescender(void) const;
  /*! @param text The text to measure.
   *  @return The bounding rectangle, in pixels, of the specified text as
   *  rendered by this font.
   */
  Rect getTextMetrics(const String& text) const;
  /*! @param format The format string for the text to measure.
   *  @return The bounding rectangle, in pixels, of the specified text as
   *  rendered by this font.
   */
  Rect getTextMetrics(const char* format, ...) const;
  /*! Calculates the layout of glyphs for the specified text.
   */
  void getTextLayout(LayoutList& result, const String& text) const;
  /*! Calculates the layout of glyphs for the specified text.
   */
  void getTextLayout(LayoutList& result, const char* format, ...) const;
  static Ref<Font> create(const ResourceInfo& info,
                          GeometryPool& pool,
                          const FontData& data);
  static Ref<Font> read(GeometryPool& pool, const Path& path);
private:
  class Glyph;
  Font(const ResourceInfo& info, GeometryPool& pool);
  Font(const Font& source);
  Font& operator = (const Font& source);
  bool init(const FontData& font);
  const Glyph* findGlyph(uint8_t character) const;
  bool getGlyphLayout(Layout& layout, uint8_t character) const;
  void getGlyphLayout(Layout& layout, const Glyph& glyph, uint8_t character) const;
  void realizeVertices(const Rect& pixelArea,
                       const Rect& texelArea,
                       Vertex2ft2fv* vertices) const;
  typedef std::vector<Glyph> GlyphList;
  GeometryPool& pool;
  GlyphList glyphs;
  Glyph* characters[256];
  vec2 size;
  float ascender;
  float descender;
  mutable GL::RenderState pass;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Glyph layout descriptor.
 */
class Font::Layout
{
public:
  Rect area;
  vec2 advance;
  char character;
};

///////////////////////////////////////////////////////////////////////

/*! @internal
 */
class Font::Glyph
{
public:
  Rect area;
  vec2 bearing;
  vec2 size;
  float advance;
};

///////////////////////////////////////////////////////////////////////

class FontReader : public ResourceReader, public XML::Reader
{
public:
  FontReader(GeometryPool& pool);
  Ref<Font> read(const Path& path);
private:
  bool extractGlyphs(FontData& data,
                     const Image& image,
                     const String& characters,
                     bool fixedWidth);
  bool onBeginElement(const String& name);
  bool onEndElement(const String& name);
  GeometryPool& pool;
  Ref<Font> font;
  ResourceInfo info;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_RENDERFONT_H*/
///////////////////////////////////////////////////////////////////////
