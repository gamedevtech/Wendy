///////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
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

#include <wendy/Config.hpp>

#include <wendy/Drawer.hpp>
#include <wendy/Layer.hpp>
#include <wendy/Widget.hpp>
#include <wendy/Layout.hpp>

namespace wendy
{

Layout::Layout(Layer& layer, Widget* parent, Orientation orientation, LayoutMode mode):
  Widget(layer, parent),
  m_borderSize(0.f),
  m_orientation(orientation),
  m_mode(mode)
{
  if (m_mode == COVER_PARENT)
  {
    if (parent)
    {
      parent->areaChanged().connect(*this, &Layout::onAreaChanged);
      onAreaChanged(*parent);
    }
    else
    {
      layer.sizeChanged().connect(*this, &Layout::onSizeChanged);
      onSizeChanged(layer);
    }
  }
}

void Layout::setBorderSize(float newSize)
{
  m_borderSize = newSize;
  update();
}

void Layout::onChildAdded(Widget& child)
{
  update();
  Widget::onChildAdded(child);
}

void Layout::onChildDesiredSizeChanged(Widget& child)
{
  update();
  Widget::onChildDesiredSizeChanged(child);
}

void Layout::onChildRemoved(Widget& child)
{
  update();
  Widget::onChildRemoved(child);
}

void Layout::onAreaChanged()
{
  update();
  Widget::onAreaChanged();
}

void Layout::onAreaChanged(Widget& parent)
{
  setArea(Rect(vec2(0.f), parent.size()));
}

void Layout::onSizeChanged(Layer& layer)
{
  Window& window = layer.window();
  setArea(Rect(vec2(0.f), vec2(float(window.width()), float(window.height()))));
}

void Layout::update()
{
  uint flexibleCount = 0;
  float stackSize = m_borderSize;
  vec2 desiredArea;

  for (Widget* c : children())
  {
    desiredArea = max(desiredArea, c->desiredSize());

    float desiredSize;

    if (m_orientation == VERTICAL)
      desiredSize = c->desiredSize().y;
    else
      desiredSize = c->desiredSize().x;

    if (desiredSize == 0.f)
      flexibleCount++;

    stackSize += desiredSize + m_borderSize;
  }

  if (m_mode == WRAP_CHILDREN)
    setDesiredSize(desiredArea);

  if (m_orientation == VERTICAL)
  {
    const float childWidth = width() - m_borderSize * 2.f;
    float flexibleHeight = 0.f, positionY = height();

    if (flexibleCount)
      flexibleHeight = (height() - stackSize) / flexibleCount;

    for (Widget* c : children())
    {
      float childHeight = c->desiredSize().y;
      if (childHeight == 0.f)
        childHeight = flexibleHeight;

      positionY -= childHeight + m_borderSize;
      c->setArea(Rect(m_borderSize, positionY, childWidth, childHeight));
    }
  }
  else
  {
    const float childHeight = height() - m_borderSize * 2.f;
    float flexibleWidth = 0.f, positionX = m_borderSize;

    if (flexibleCount)
      flexibleWidth = (width() - stackSize) / flexibleCount;

    for (Widget* c : children())
    {
      float childWidth = c->desiredSize().x;
      if (childWidth == 0.f)
        childWidth = flexibleWidth;

      c->setArea(Rect(positionX, m_borderSize, childWidth, childHeight));
      positionX += childWidth + m_borderSize;
    }
  }
}

} /*namespace wendy*/

