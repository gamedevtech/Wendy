//////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
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

#include <moira/Moira.h>

#include <wendy/Config.h>

#include <wendy/OpenGL.h>
#include <wendy/GLContext.h>
#include <wendy/GLCanvas.h>
#include <wendy/GLTexture.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLLight.h>
#include <wendy/GLShader.h>
#include <wendy/GLPass.h>
#include <wendy/GLRender.h>

#include <wendy/RenderFont.h>

#include <wendy/UIRender.h>
#include <wendy/UIWidget.h>
#include <wendy/UIEntry.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

Entry::Entry(const String& initText):
  text(initText),
  startPosition(0),
  caretPosition(0)
{
  const float em = Renderer::get()->getDefaultEM();

  setSize(Vector2(em * 10.f, em * 1.5f));

  getButtonClickedSignal().connect(*this, &Entry::onButtonClicked);
  getKeyPressedSignal().connect(*this, &Entry::onKeyPressed);
  getCharInputSignal().connect(*this, &Entry::onCharInput);
}

const String& Entry::getText(void) const
{
  return text;
}

void Entry::setText(const String& newText)
{
  text = newText;
}

unsigned int Entry::getCaretPosition(void) const
{
  return caretPosition;
}

void Entry::setCaretPosition(unsigned int newPosition)
{
  setCaretPosition(newPosition, false);
}

SignalProxy1<void, Entry&> Entry::getTextChangedSignal(void)
{
  return textChangedSignal;
}

SignalProxy1<void, Entry&> Entry::getCaretMovedSignal(void)
{
  return caretMovedSignal;
}

void Entry::draw(void) const
{
  const Rectangle& area = getGlobalArea();

  Renderer* renderer = Renderer::get();
  if (renderer->pushClipArea(area))
  {
    renderer->drawWell(area, getState());

    const float em = renderer->getDefaultEM();

    Rectangle textArea = area;
    textArea.position.x += em / 2.f;
    textArea.size.x -= em;

    renderer->drawText(textArea, text, LEFT_ALIGNED);

    if (isActive() && ((unsigned int) (Timer::getCurrentTime() * 2.f) & 1))
    {
      float position = 0.f;

      if (caretPosition > startPosition)
      {
	render::Font::LayoutList layouts;
	renderer->getDefaultFont()->getTextLayout(layouts, text.substr(startPosition, caretPosition));

	const render::Font::Layout& glyph = layouts[caretPosition - 1];
	position = glyph.penOffset.x + glyph.area.size.x;
      }

      Segment2 segment;
      segment.start.set(textArea.position.x + position,
                        textArea.position.y);
      segment.end.set(textArea.position.x + position,
                      textArea.position.y + textArea.size.y);

      GL::Renderer::get()->setColor(ColorRGBA::BLACK);
      GL::Renderer::get()->drawLine(segment);
    }

    Widget::draw();

    renderer->popClipArea();
  }
}

void Entry::onButtonClicked(Widget& widget,
			    const Vector2& point,
			    unsigned int button,
			    bool clicked)
{
  float position = transformToLocal(point).x;

  Renderer* renderer = Renderer::get();

  const float em = renderer->getDefaultEM();

  render::Font::LayoutList layouts;
  renderer->getDefaultFont()->getTextLayout(layouts, text.substr(startPosition, String::npos));

  float offset = em / 2.f;
  unsigned int index;

  // TODO: Improve this, it sucks.

  for (index = 0;  index < layouts.size();  index++)
  {
    if (offset > position)
      break;

    offset += layouts[index].area.size.x;
  }

  setCaretPosition(startPosition + index, true);
}

void Entry::onKeyPressed(Widget& widget, GL::Key key, bool pressed)
{
  if (!pressed)
    return;

  switch (key)
  {
    case GL::Key::BACKSPACE:
    {
      if (!text.empty() && caretPosition > 0)
      {
	text.erase(caretPosition - 1, 1);
	textChangedSignal.emit(*this);
	setCaretPosition(caretPosition - 1, true);
      }

      break;
    }

    case GL::Key::DELETE:
    {
      if (!text.empty() && caretPosition < text.length())
      {
	text.erase(caretPosition, 1);
	textChangedSignal.emit(*this);
      }

      break;
    }

    case GL::Key::LEFT:
    {
      if (caretPosition > 0)
	setCaretPosition(caretPosition - 1, true);
      break;
    }

    case GL::Key::RIGHT:
    {
      setCaretPosition(caretPosition + 1, true);
      break;
    }

    case GL::Key::HOME:
    {
      setCaretPosition(0, true);
      break;
    }

    case GL::Key::END:
    {
      setCaretPosition(text.length(), true);
      break;
    }
  }
}

void Entry::onCharInput(Widget& widget, wchar_t character)
{
  text.insert(caretPosition, 1, (char) character);
  textChangedSignal.emit(*this);
  setCaretPosition(caretPosition + 1, true);
}

void Entry::setCaretPosition(unsigned int newPosition, bool notify)
{
  if (newPosition > text.length())
    newPosition = text.length();

  if (newPosition == caretPosition)
    return;

  caretPosition = newPosition;

  if (notify)
    caretMovedSignal.emit(*this);
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////