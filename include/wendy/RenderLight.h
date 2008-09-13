///////////////////////////////////////////////////////////////////////
// Wendy default renderer
// Copyright (c) 2008 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_RENDERLIGHT_H
#define WENDY_RENDERLIGHT_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

class Light : public RefObject<Light>
{
public:
  Light(void);
  const ColorRGB& getIntensity(void) const;
  void setIntensity(const ColorRGB& newColor);
  const Vector3 getPosition(void) const;
  void setPosition(const Vector3& newPosition);
  const Sphere& getBounds(void) const;
  void setBounds(const Sphere& newBounds);
private:
  ColorRGB intensity;
  Vector3 position;
  Sphere bounds;
};

///////////////////////////////////////////////////////////////////////

typedef Ref<Light> LightRef;

///////////////////////////////////////////////////////////////////////

class LightState
{
public:
  LightState(void);
  void apply(void) const;
  void attachLight(Light& light);
  void detachLight(Light& light);
  void detachLights(void);
  unsigned int getLightCount(void) const;
  Light& getLight(unsigned int index) const;
  static const LightState& getCurrent(void);
private:
  static void onContextDestroy(void);
  typedef std::vector<LightRef> List;
  List lights;
  static LightState current;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_RENDERLIGHT_H*/
///////////////////////////////////////////////////////////////////////
