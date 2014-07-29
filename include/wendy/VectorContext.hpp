///////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
// Copyright (c) 2014 Camilla Berglund <elmindreda@elmindreda.org>
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

#pragma once

#include <nanovg.h>

namespace wendy
{

class RenderContext;

class VectorContext
{
public:
  ~VectorContext();
  void beginFrame(uint width, uint height, float aspectRatio);
  void endFrame();
  void save();
  void restore();
  void reset();
  void strokeColor(vec4 color);
  // strokePaint
  void fillColor(vec4 color);
  // fillPaint
  // miterLimit
  void strokeWidth(float size);
  void lineCap(NVGlineCap cap);
  void lineJoin(NVGlineCap join);
  void globalAlpha(float alpha);
  void resetTransform();
  // transform
  void translate(vec2 offset);
  void rotate(float angle);
  void skewX(float angle);
  void skewY(float angle);
  void scale(vec2 scale);
  // createImage
  // updateImage
  // deleteImage
  // linearGradient
  // boxGradient
  // radialGradient
  // imagePattern
  void scissor(Rect rect);
  void intersectScissor(Rect rect);
  void resetScissor();
  void beginPath();
  void moveTo(vec2 point);
  void lineTo(vec2 point);
  // bezierTo
  // quadTo
  // arcTo
  void closePath();
  // pathWinding
  // arc
  void rect(Rect rect);
  void roundedRect(Rect rect, float radius);
  void ellipse(vec2 center, vec2 radius);
  void circle(vec2 center, float radius);
  void fill();
  void stroke();
  // createFont
  RenderContext& context() const { return m_context; }
  static VectorContext* create(RenderContext& rc);
private:
  VectorContext(RenderContext& rc);
  bool init();
  RenderContext& m_context;
  NVGcontext* m_handle;
};

} /*namespace wendy*/

