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

#include <wendy/Config.hpp>

#include <wendy/Texture.hpp>
#include <wendy/RenderBuffer.hpp>
#include <wendy/Program.hpp>
#include <wendy/RenderContext.hpp>

#include <wendy/VectorContext.hpp>

#include <GREG/greg.h>

#define NANOVG_GL3_IMPLEMENTATION
#include <nanovg_gl.h>

namespace wendy
{

VectorContext::~VectorContext()
{
  if (m_handle)
    nvgDeleteGL3(m_handle);
}

void VectorContext::beginFrame(uint width, uint height, float aspectRatio)
{
  nvgBeginFrame(m_handle, width, height, aspectRatio);
}

void VectorContext::endFrame()
{
  nvgEndFrame(m_handle);
}

void VectorContext::save()
{
  nvgSave(m_handle);
}

void VectorContext::restore()
{
  nvgRestore(m_handle);
}

void VectorContext::reset()
{
  nvgReset(m_handle);
}

void VectorContext::strokeColor(vec4 color)
{
  nvgStrokeColor(m_handle, *(NVGcolor*)&color);
}

void VectorContext::fillColor(vec4 color)
{
  nvgFillColor(m_handle, *(NVGcolor*)&color);
}

void VectorContext::strokeWidth(float size)
{
  nvgStrokeWidth(m_handle, size);
}

void VectorContext::lineCap(NVGlineCap cap)
{
  nvgLineCap(m_handle, cap);
}

void VectorContext::lineJoin(NVGlineCap join)
{
  nvgLineJoin(m_handle, join);
}

void VectorContext::globalAlpha(float alpha)
{
  nvgGlobalAlpha(m_handle, alpha);
}

void VectorContext::resetTransform()
{
  nvgResetTransform(m_handle);
}

void VectorContext::translate(vec2 offset)
{
  nvgTranslate(m_handle, offset.x, offset.y);
}

void VectorContext::rotate(float angle)
{
  nvgRotate(m_handle, angle);
}

void VectorContext::skewX(float angle)
{
  nvgSkewX(m_handle, angle);
}

void VectorContext::skewY(float angle)
{
  nvgSkewY(m_handle, angle);
}

void VectorContext::scale(vec2 scale)
{
  nvgScale(m_handle, scale.x, scale.y);
}

void VectorContext::scissor(Rect area)
{
  nvgScissor(m_handle, area.position.x, area.position.y, area.size.x, area.size.y);
}

void VectorContext::intersectScissor(Rect area)
{
  nvgIntersectScissor(m_handle, area.position.x, area.position.y, area.size.x, area.size.y);
}

void VectorContext::resetScissor()
{
  nvgResetScissor(m_handle);
}

void VectorContext::beginPath()
{
  nvgBeginPath(m_handle);
}

void VectorContext::moveTo(vec2 point)
{
  nvgMoveTo(m_handle, point.x, point.y);
}

void VectorContext::lineTo(vec2 point)
{
  nvgLineTo(m_handle, point.x, point.y);
}

void VectorContext::closePath()
{
  nvgClosePath(m_handle);
}

void VectorContext::rect(Rect area)
{
  nvgRect(m_handle, area.position.x, area.position.y, area.size.x, area.size.y);
}

void VectorContext::roundedRect(Rect rect, float radius)
{
  nvgRoundedRect(m_handle, rect.position.x, rect.position.y, rect.size.x, rect.size.y, radius);
}

void VectorContext::ellipse(vec2 center, vec2 radius)
{
  nvgEllipse(m_handle, center.x, center.y, radius.x, radius.y);
}

void VectorContext::circle(vec2 center, float radius)
{
  nvgCircle(m_handle, center.x, center.y, radius);
}

void VectorContext::fill()
{
  nvgFill(m_handle);
}

void VectorContext::stroke()
{
  nvgStroke(m_handle);
}

VectorContext* VectorContext::create(RenderContext& rc)
{
  std::unique_ptr<VectorContext> vc(new VectorContext(rc));
  if (!vc->init())
    return nullptr;

  return vc.release();
}

VectorContext::VectorContext(RenderContext& rc):
  m_context(rc)
{
}

bool VectorContext::init()
{
  m_handle = nvgCreateGL3(0);
  if (!m_handle)
  {
    logError("Failed to create NanoVG context");
    return false;
  }

  return true;
}

} /*namespace wendy*/

