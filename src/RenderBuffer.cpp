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

#include <wendy/Config.hpp>

#include <wendy/Texture.hpp>
#include <wendy/RenderBuffer.hpp>
#include <wendy/Program.hpp>
#include <wendy/RenderContext.hpp>

#include <GREG/greg.h>

#include <internal/OpenGL.hpp>

namespace wendy
{

namespace
{

GLenum convertToGL(BufferUsage usage)
{
  switch (usage)
  {
    case USAGE_STATIC:
      return GL_STATIC_DRAW;
    case USAGE_STREAM:
      return GL_STREAM_DRAW;
    case USAGE_DYNAMIC:
      return GL_DYNAMIC_DRAW;
  }

  panic("Invalid buffer usage %u", usage);
}

GLenum convertToGL(TextureFramebuffer::Attachment attachment)
{
  switch (attachment)
  {
    case TextureFramebuffer::COLOR_BUFFER0:
      return GL_COLOR_ATTACHMENT0;
    case TextureFramebuffer::COLOR_BUFFER1:
      return GL_COLOR_ATTACHMENT1;
    case TextureFramebuffer::COLOR_BUFFER2:
      return GL_COLOR_ATTACHMENT2;
    case TextureFramebuffer::COLOR_BUFFER3:
      return GL_COLOR_ATTACHMENT3;
    case TextureFramebuffer::DEPTH_BUFFER:
      return GL_DEPTH_ATTACHMENT;
  }

  panic("Invalid framebuffer attachment %u", attachment);
}

const char* asString(TextureFramebuffer::Attachment attachment)
{
  switch (attachment)
  {
    case TextureFramebuffer::COLOR_BUFFER0:
      return "color buffer 0";
    case TextureFramebuffer::COLOR_BUFFER1:
      return "color buffer 1";
    case TextureFramebuffer::COLOR_BUFFER2:
      return "color buffer 2";
    case TextureFramebuffer::COLOR_BUFFER3:
      return "color buffer 3";
    case TextureFramebuffer::DEPTH_BUFFER:
      return "depth buffer";
  }

  panic("Invalid framebuffer attachment %u", attachment);
}

bool isColorAttachment(TextureFramebuffer::Attachment attachment)
{
  switch (attachment)
  {
    case TextureFramebuffer::COLOR_BUFFER0:
    case TextureFramebuffer::COLOR_BUFFER1:
    case TextureFramebuffer::COLOR_BUFFER2:
    case TextureFramebuffer::COLOR_BUFFER3:
      return true;
    default:
      return false;
  }
}

} /*namespace*/

Buffer::~Buffer()
{
  if (m_bufferID)
    glDeleteBuffers(1, &m_bufferID);

  if (RenderStats* stats = m_context.stats())
    stats->removeBuffer(m_size);
}

void Buffer::discard()
{
  m_context.setCurrentVertexArray(0);

  glBindBuffer(m_target, m_bufferID);
  glBufferData(m_target, m_size, nullptr, convertToGL(m_usage));

#if WENDY_DEBUG
  checkGL("Error during buffer discard");
#endif
}

void Buffer::copyFrom(const void* source, size_t size, size_t offset)
{
  if (offset + size > m_size)
  {
    logError("Too much data submitted to buffer");
    return;
  }

  m_context.setCurrentVertexArray(0);

  glBindBuffer(m_target, m_bufferID);
  glBufferSubData(m_target, offset, size, source);

#if WENDY_DEBUG
  checkGL("Error during copy to buffer");
#endif
}

void Buffer::copyTo(void* target, size_t size, size_t offset)
{
  if (offset + size > m_size)
  {
    logError("Too much data requested from buffer");
    return;
  }

  m_context.setCurrentVertexArray(0);

  glBindBuffer(m_target, m_bufferID);
  glGetBufferSubData(m_target, offset, size, target);

#if WENDY_DEBUG
  checkGL("Error during copy from buffer");
#endif
}

Buffer::Buffer(RenderContext& context, uint target):
  m_context(context),
  m_target(target),
  m_bufferID(0),
  m_count(0),
  m_usage(USAGE_STATIC)
{
}

bool Buffer::init(size_t size, BufferUsage usage)
{
  m_usage = usage;
  m_size = size;

  m_context.setCurrentVertexArray(0);

  glGenBuffers(1, &m_bufferID);
  glBindBuffer(m_target, m_bufferID);
  glBufferData(m_target, m_size, nullptr, convertToGL(m_usage));

  if (!checkGL("Error during creation of buffer")
    return false;

  if (RenderStats* stats = m_context.stats())
    stats->addBuffer(size());

  return true;
}

VertexBuffer* VertexBuffer::create(RenderContext& context,
                                   size_t size,
                                   BufferUsage usage)
{
  std::unique_ptr<VertexBuffer> buffer(new VertexBuffer(context));
  if (!buffer->init(format, count, usage))
    return nullptr;

  return buffer.release();
}

VertexBuffer::VertexBuffer(RenderContext& context):
  Buffer(context, GL_ARRAY_BUFFER)
{
}

size_t IndexBuffer::typeSize(IndexType type)
{
  switch (type)
  {
    case INDEX_UINT8:
      return 1;
    case INDEX_UINT16:
      return 2;
    case INDEX_UINT32:
      return 4;
    default:
      return 0;
  }
}

IndexBuffer* IndexBuffer::create(RenderContext& context,
                                 size_t size,
                                 BufferUsage usage)
{
  std::unique_ptr<IndexBuffer> buffer(new IndexBuffer(context));
  if (!buffer->init(format, count, usage))
    return nullptr;

  return buffer.release();
}

IndexBuffer::IndexBuffer(RenderContext& context):
  Buffer(context, GL_ELEMENT_ARRAY_BUFFER)
{
}

BufferRange::BufferRange():
  buffer(nullptr),
  size(0)
  offset(0),
{
}

BufferRange::BufferRange(Buffer& buffer):
  buffer(&buffer),
  size(0)
  offset(0),
{
  size = buffer->size();
}

BufferRange::BufferRange(Buffer& buffer,
                         size_t size,
                         size_t offset):
  buffer(&buffer),
  size(size)
  offset(offset),
{
  assert(buffer->size() >= offset + size);
}

void BufferRange::copyFrom(const void* source)
{
  assert(buffer);
  buffer->copyFrom(source, size, offset);
}

void BufferRange::copyTo(void* target)
{
  assert(buffer);
  buffer->copyTo(target, size, offset);
}

PrimitiveRange::PrimitiveRange():
  mode(TRIANGLE_LIST),
  type(NO_INDICES),
  start(0),
  count(0),
  base(0)
{
}

PrimitiveRange::PrimitiveRange(PrimitiveMode mode,
                               IndexType type,
                               size_t start,
                               size_t count,
                               size_t base):
  mode(mode),
  type(type),
  start(start),
  count(count),
  base(base)
{
}

Framebuffer::~Framebuffer()
{
}

Ref<Image> Framebuffer::data() const
{
  Ref<Image> image = Image::create(m_context.cache(),
                                   PixelFormat::RGB8,
                                   width(), height());
  if (!image)
    return nullptr;

  Framebuffer& previous = m_context.currentFramebuffer();
  apply();

  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadPixels(0, 0, image->width(), image->height(),
               GL_RGB, GL_UNSIGNED_BYTE,
               image->pixels());

  previous.apply();

  if (!checkGL("Error when reading framebuffer data"))
    return nullptr;

  return image;
}

void Framebuffer::setSRGB(bool enabled)
{
  if (m_sRGB == enabled)
    return;

  Framebuffer& previous = m_context.currentFramebuffer();
  apply();

  if (enabled)
  {
    glEnable(GL_FRAMEBUFFER_SRGB);
    checkGL("Failed to enable framebuffer sRGB encoding");
  }
  else
  {
    glDisable(GL_FRAMEBUFFER_SRGB);
    checkGL("Failed to disable framebuffer sRGB encoding");
  }

  m_sRGB = enabled;

  previous.apply();
}

Framebuffer::Framebuffer(RenderContext& context):
  m_context(context),
  m_sRGB(false)
{
}

DefaultFramebuffer::DefaultFramebuffer(RenderContext& context):
  Framebuffer(context)
{
}

void DefaultFramebuffer::apply() const
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

#if WENDY_DEBUG
  checkGL("Error when applying default framebuffer");
#endif
}

uint DefaultFramebuffer::width() const
{
  return context().window().width();
}

uint DefaultFramebuffer::height() const
{
  return context().window().height();
}

TextureFramebuffer::~TextureFramebuffer()
{
  if (m_bufferID)
    glDeleteFramebuffers(1, &m_bufferID);
}

uint TextureFramebuffer::width() const
{
  uint width = 0;

  for (size_t i = 0;  i < 5;  i++)
  {
    if (m_textures[i])
    {
      if (width && width != m_textures[i]->width())
        return 0;

      width = m_textures[i]->width();
    }
  }

  return width;
}

uint TextureFramebuffer::height() const
{
  uint height = 0;

  for (size_t i = 0;  i < 5;  i++)
  {
    if (m_textures[i])
    {
      if (height && height != m_textures[i]->height())
        return 0;

      height = m_textures[i]->height();
    }
  }

  return height;
}

Texture* TextureFramebuffer::buffer(Attachment attachment) const
{
  return m_textures[attachment];
}

bool TextureFramebuffer::setDepthBuffer(Texture* newTexture, const TextureImage& image, uint z)
{
  return setBuffer(DEPTH_BUFFER, newTexture, image, z);
}

bool TextureFramebuffer::setColorBuffer(Texture* newTexture, const TextureImage& image, uint z)
{
  return setBuffer(COLOR_BUFFER0, newTexture, image, z);
}

bool TextureFramebuffer::setBuffer(Attachment attachment, Texture* newTexture,
                                   const TextureImage& image, uint z)
{
  if (isColorAttachment(attachment))
  {
    const RenderLimits& limits = m_context.limits();
    const uint index = attachment - COLOR_BUFFER0;

    if (index >= limits.maxColorAttachments)
    {
      logError("OpenGL context supports at most %u FBO color attachments",
               limits.maxColorAttachments);
      return false;
    }

    if (index >= limits.maxDrawBuffers)
    {
      logError("OpenGL context supports at most %u draw buffers",
               limits.maxDrawBuffers);
      return false;
    }
  }

  Framebuffer& previous = m_context.currentFramebuffer();
  apply();

  if (m_textures[attachment])
    m_textures[attachment]->detach(convertToGL(attachment));

  m_textures[attachment] = newTexture;

  if (m_textures[attachment])
    m_textures[attachment]->attach(convertToGL(attachment), image, z);

  previous.apply();
  return true;
}

Ref<TextureFramebuffer> TextureFramebuffer::create(RenderContext& context)
{
  Ref<TextureFramebuffer> framebuffer(new TextureFramebuffer(context));
  if (!framebuffer->init())
    return nullptr;

  return framebuffer;
}

TextureFramebuffer::TextureFramebuffer(RenderContext& context):
  Framebuffer(context),
  m_bufferID(0)
{
}

bool TextureFramebuffer::init()
{
  glGenFramebuffers(1, &m_bufferID);

#if WENDY_DEBUG
  if (!checkGL("Error during image framebuffer creation"))
    return false;
#endif

  return true;
}

void TextureFramebuffer::apply() const
{
  glBindFramebuffer(GL_FRAMEBUFFER, m_bufferID);

  GLenum enables[5];
  GLsizei count = 0;

  for (size_t i = 0;  i < sizeof(enables) / sizeof(enables[0]);  i++)
  {
    Attachment attachment = (Attachment) i;

    if (m_textures[i] && isColorAttachment(attachment))
      enables[count++] = convertToGL(attachment);
  }

  if (count)
    glDrawBuffers(count, enables);
  else
    glDrawBuffer(GL_NONE);

#if WENDY_DEBUG
  checkGL("Error when applying image framebuffer");
#endif
}

} /*namespace wendy*/

