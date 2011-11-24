///////////////////////////////////////////////////////////////////////
// Wendy core library
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
#ifndef WENDY_SEGMENT_H
#define WENDY_SEGMENT_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

template <typename T>
class Segment
{
public:
  Segment();
  Segment(const T& start, const T& end);
  float length() const;
  void set(const T& newStart, const T& newEnd);
  T start;
  T end;
};

///////////////////////////////////////////////////////////////////////

template <typename T>
inline Segment<T>::Segment()
{
}

template <typename T>
inline Segment<T>::Segment(const T& initStart, const T& initEnd):
  start(initStart),
  end(initEnd)
{
}

template <typename T>
inline float Segment<T>::length() const
{
  return distance(start, end);
}

template <typename T>
inline void Segment<T>::set(const T& newStart, const T& newEnd)
{
  start = newStart;
  end = newEnd;
}

///////////////////////////////////////////////////////////////////////

typedef Segment<vec2> Segment2;
typedef Segment<vec3> Segment3;

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_SEGMENT_H*/
///////////////////////////////////////////////////////////////////////
