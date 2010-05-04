//////////////////////////////////////////////////////////////////////
// Wendy user interface library
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

#include <wendy/Config.h>

#include <wendy/UIRender.h>
#include <wendy/UIDesktop.h>
#include <wendy/UIWidget.h>
#include <wendy/UILabel.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

Label::Label(Desktop& desktop, const String& initText):
  Widget(desktop),
  text(initText)
{
  Renderer& renderer = desktop.getRenderer();

  const float em = renderer.getDefaultEM();

  float textWidth;

  if (text.empty())
    textWidth = em * 3.f;
  else
    textWidth = renderer.getDefaultFont().getTextMetrics(text).size.x;

  setSize(Vec2(em * 2.f + textWidth, em * 2.f));
}

const String& Label::getText(void) const
{
  return text;
}

void Label::setText(const String& newText)
{
  text = newText;
}

void Label::setText(const char* format, ...)
{
  va_list vl;
  char* newText;

  va_start(vl, format);
  vasprintf(&newText, format, vl);
  va_end(vl);

  text = newText;
  free(newText);
}

const Alignment& Label::getTextAlignment(void) const
{
  return textAlignment;
}

void Label::setTextAlignment(const Alignment& newAlignment)
{
  textAlignment = newAlignment;
}

void Label::draw(void) const
{
  const Rect& area = getGlobalArea();

  Renderer& renderer = getDesktop().getRenderer();
  if (renderer.pushClipArea(area))
  {
    renderer.drawText(area, text, textAlignment);

    Widget::draw();

    renderer.popClipArea();
  }
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
