///////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
// Copyright (c) 2005 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_SPRITE_HPP
#define WENDY_SPRITE_HPP
///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

enum SpriteType3
{
  STATIC_SPRITE,
  CYLINDRIC_SPRITE,
  SPHERICAL_SPRITE
};

///////////////////////////////////////////////////////////////////////

class Sprite3 : public Renderable
{
public:
  Sprite3();
  void enqueue(RenderQueue& queue,
               const Camera& camera,
               const Transform3& transform) const override;
  Sphere bounds() const override;
  vec2 size;
  float angle;
  SpriteType3 type;
  Ref<Material> material;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_SPRITE_HPP*/
///////////////////////////////////////////////////////////////////////
