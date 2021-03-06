///////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
// Copyright (c) 2011 Camilla Berglund <elmindreda@elmindreda.org>
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

/*! @brief Audio sample data format enumeration.
 */
enum SampleFormat
{
  SAMPLE_MONO8,
  SAMPLE_MONO16,
  SAMPLE_STEREO8,
  SAMPLE_STEREO16,
};

/*! @brief Audio sample.
 */
class Sample : public Resource, public RefObject
{
public:
  Sample(const ResourceInfo& info,
         const char* data,
         size_t size,
         SampleFormat format,
         uint frequency);
  static Ref<Sample> read(ResourceCache& cache, const std::string& name);
  std::vector<char> data;
  SampleFormat format;
  uint frequency;
};

} /*namespace wendy*/

