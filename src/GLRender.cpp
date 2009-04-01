///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2004 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <wendy/GLContext.h>
#include <wendy/GLTexture.h>
#include <wendy/GLCanvas.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLShader.h>
#include <wendy/GLRender.h>

#define GLEW_STATIC
#include <GL/glew.h>

#include <algorithm>
#include <cstdlib>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

namespace
{
  
GLenum convertPrimitiveType(PrimitiveType type)
{
  switch (type)
  {
    case POINT_LIST:
      return GL_POINTS;
    case LINE_LIST:
      return GL_LINES;
    case LINE_STRIP:
      return GL_LINE_STRIP;
    case TRIANGLE_LIST:
      return GL_TRIANGLES;
    case TRIANGLE_STRIP:
      return GL_TRIANGLE_STRIP;
    case TRIANGLE_FAN:
      return GL_TRIANGLE_FAN;
    default:
      throw Exception("Invalid primitive type");
  }
}

}

///////////////////////////////////////////////////////////////////////

void Renderer::begin2D(const Vector2& resolution)
{
  Canvas* canvas = Canvas::getCurrent();
  if (!canvas)
  {
    Log::writeError("Cannot begin without a current canvas");
    return;
  }

  if (!matrixStack.isEmpty())
    throw Exception("Renderer matrix stack not empty at begin");

  Matrix4 projection;
  projection.x.x = 2.f / resolution.x;
  projection.y.y = 2.f / resolution.y;
  projection.z.z = -1.f;
  projection.w.x = -1.f;
  projection.w.y = -1.f;
  projection.w.w = 1.f;

  matrixStack.push(projection);
}

void Renderer::begin3D(float FOV, float aspect, float nearZ, float farZ)
{
  Canvas* canvas = Canvas::getCurrent();
  if (!canvas)
  {
    Log::writeError("Cannot begin without a current canvas");
    return;
  }

  if (!matrixStack.isEmpty())
    throw Exception("Renderer matrix stack not empty at begin");

  if (aspect == 0.f)
    aspect = (float) canvas->getPhysicalWidth() / (float) canvas->getPhysicalHeight();

  const float f = 1.f / tanf(FOV / 2.f);

  Matrix4 projection;
  projection.x.x = f / aspect;
  projection.y.y = f;
  projection.z.z = (farZ + nearZ) / (nearZ - farZ);
  projection.z.w = -1.f;
  projection.w.z = (2.f * farZ * nearZ) / (nearZ - farZ);

  matrixStack.push(projection);
}

void Renderer::begin3D(const Matrix4& projection)
{
  Canvas* canvas = Canvas::getCurrent();
  if (!canvas)
  {
    Log::writeError("Cannot begin without a current canvas");
    return;
  }

  if (!matrixStack.isEmpty())
    throw Exception("Renderer matrix stack not empty at begin");

  matrixStack.push(projection);
}
  
void Renderer::end(void)
{
  matrixStack.pop();

  if (!matrixStack.isEmpty())
    throw Exception("Renderer matrix stack not empty after end");
}

void Renderer::pushTransform(const Matrix4& transform)
{
  matrixStack.push(transform);
}

void Renderer::popTransform(void)
{
  matrixStack.pop();
}

bool Renderer::allocateIndices(IndexRange& range,
		               unsigned int count,
                               IndexBuffer::Type type)
{
  if (!count)
  {
    range = IndexRange();
    return true;
  }

  IndexBufferSlot* slot = NULL;

  for (IndexBufferList::iterator i = indexBufferPool.begin();  i != indexBufferPool.end();  i++)
  {
    if ((*i).indexBuffer->getType() == type && (*i).available >= count)
    {
      slot = &(*i);
      break;
    }
  }

  if (!slot)
  {
    indexBufferPool.push_back(IndexBufferSlot());
    slot = &(indexBufferPool.back());

    // Granularity of 1K
    // TODO: Make configurable or autoconfigured
    const unsigned int actualCount = 1024 * ((count + 1023) / 1024);

    slot->indexBuffer = IndexBuffer::createInstance(actualCount,
                                                    type,
						    IndexBuffer::DYNAMIC);
    if (!slot->indexBuffer)
    {
      indexBufferPool.pop_back();
      return false;
    }

    slot->available = slot->indexBuffer->getCount();
  }

  range = IndexRange(*(slot->indexBuffer), 
		     slot->indexBuffer->getCount() - slot->available,
                     count);

  slot->available -= count;
  return true;
}

bool Renderer::allocateVertices(VertexRange& range,
				unsigned int count,
				const VertexFormat& format)
{
  if (!count)
  {
    range = VertexRange();
    return true;
  }

  VertexBufferSlot* slot = NULL;

  for (VertexBufferList::iterator i = vertexBufferPool.begin();  i != vertexBufferPool.end();  i++)
  {
    if ((*i).vertexBuffer->getFormat() == format && (*i).available >= count)
    {
      slot = &(*i);
      break;
    }
  }

  if (!slot)
  {
    vertexBufferPool.push_back(VertexBufferSlot());
    slot = &(vertexBufferPool.back());

    // Granularity of 1K
    // TODO: Make configurable or autoconfigured
    const unsigned int actualCount = 1024 * ((count + 1023) / 1024);
    
    slot->vertexBuffer = VertexBuffer::createInstance(actualCount,
                                                      format,
						      VertexBuffer::DYNAMIC);
    if (!slot->vertexBuffer)
    {
      vertexBufferPool.pop_back();
      return false;
    }

    slot->available = slot->vertexBuffer->getCount();
  }

  range = VertexRange(*(slot->vertexBuffer), 
		      slot->vertexBuffer->getCount() - slot->available,
                      count);

  slot->available -= count;
  return true;
}

bool Renderer::isReservedUniform(const String& name) const
{
  return name == "MVP";
}

Context& Renderer::getContext(void) const
{
  return context;
}

Canvas* Renderer::getCurrentCanvas(void) const
{
  return currentCanvas;
}

Texture& Renderer::getDefaultTexture(void) const
{
  return *defaultTexture;
}

Program& Renderer::getDefaultProgram(void) const
{
  return *defaultProgram;
}

void Renderer::setCurrentCanvas(Canvas* newCanvas)
{
  currentCanvas = newCanvas;
}

void Renderer::setCurrentProgram(Program* newProgram)
{
  currentProgram = newProgram;
}

void Renderer::setCurrentPrimitiveRange(const PrimitiveRange& newRange)
{
  currentRange = newRange;
}

bool Renderer::create(Context& context)
{
  Ptr<Renderer> renderer = new Renderer(context);
  if (!renderer->init())
    return false;

  set(renderer.detachObject());
  return true;
}

Renderer::Renderer(Context& initContext):
  context(initContext),
  currentCanvas(NULL),
  currentVertexBuffer(NULL),
  currentIndexBuffer(NULL)
{
}

bool Renderer::init(void)
{
  if (!Context::get())
  {
    Log::writeError("Cannot create renderer without OpenGL context");
    return false;
  }

  CheckerImageGenerator generator;
  generator.setDefaultColor(ColorRGBA(1.f, 0.f, 1.f, 1.f));
  generator.setCheckerColor(ColorRGBA(0.f, 1.f, 0.f, 1.f));
  generator.setCheckerSize(1);

  Ptr<Image> image = generator.generate(ImageFormat::RGBX8888, 2, 2);
  if (!image)
  {
    Log::writeError("Failed to create image data for default texture");
    return false;
  }

  defaultTexture = Texture::createInstance(*image, Texture::DEFAULT, "default");
  if (!defaultTexture)
  {
    Log::writeError("Failed to create default texture");
    return false;
  }

  Context::get()->getFinishSignal().connect(*this, &Renderer::onContextFinish);
  return true;
}

void Renderer::onContextFinish(void)
{
  for (IndexBufferList::iterator i = indexBufferPool.begin();  i != indexBufferPool.end();  i++)
    (*i).available = (*i).indexBuffer->getCount();

  for (VertexBufferList::iterator i = vertexBufferPool.begin();  i != vertexBufferPool.end();  i++)
    (*i).available = (*i).vertexBuffer->getCount();
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
