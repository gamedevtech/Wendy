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

#pragma once

namespace wendy
{

/*! @ingroup ui
 */
class Slider : public Widget
{
public:
  Slider(Layer& layer, Widget* parent, Orientation orientation);
  Orientation orientation() const { return m_orientation; }
  float minValue() const { return m_minValue; }
  float maxValue() const { return m_maxValue; }
  void setValueRange(float newMinValue, float newMaxValue);
  float stepSize() const { return m_stepSize; }
  void setStepSize(float newSize);
  float value() const { return m_value; }
  void setValue(float newValue);
  SignalProxy<void,Slider&> valueChanged() { return m_valueChanged; }
protected:
  void draw() const;
private:
  void onMouseButton(vec2 point,
                     MouseButton button,
                     Action action,
                     uint mods) override;
  void onKey(Key key, Action action, uint mods) override;
  void onScroll(vec2 offset) override;
  void onDragBegun(vec2 point, MouseButton button) override;
  void onDragMoved(vec2 point, MouseButton button) override;
  void setValue(const vec2& position);
  void setValue(float newValue, bool notify);
  Signal<void,Slider&> m_valueChanged;
  float m_minValue;
  float m_maxValue;
  float m_stepSize;
  float m_value;
  Orientation m_orientation;
};

} /*namespace wendy*/

