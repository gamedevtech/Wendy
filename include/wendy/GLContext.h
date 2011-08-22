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
#ifndef WENDY_GLCONTEXT_H
#define WENDY_GLCONTEXT_H
///////////////////////////////////////////////////////////////////////

#include <wendy/Core.h>
#include <wendy/Plane.h>
#include <wendy/Rectangle.h>
#include <wendy/AABB.h>
#include <wendy/Pixel.h>
#include <wendy/Signal.h>
#include <wendy/Timer.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {

///////////////////////////////////////////////////////////////////////

class VertexBuffer;
class IndexBuffer;
class Context;
class PrimitiveRange;

///////////////////////////////////////////////////////////////////////

/*! The invalid value for shared program state member IDs.
 */
const int INVALID_SHARED_STATE_ID = -1;

///////////////////////////////////////////////////////////////////////

/*! @brief Context window mode.
 *  @ingroup opengl
 */
enum WindowMode
{
  WINDOWED,
  FULLSCREEN,
};

///////////////////////////////////////////////////////////////////////

/*! @brief Screen mode.
 *  @ingroup opengl
 */
class ScreenMode
{
public:
  /*! Default constructor.
   */
  ScreenMode();
  /*! Constructor.
   *  @param[in] width The desired width.
   *  @param[in] height The desired height.
   *  @param[in] colorBits The desired number of color bits.
   */
  ScreenMode(unsigned int width, unsigned int height, unsigned int colorBits);
  /*! Resets all value to their defaults.
   */
  void setDefaults();
  /*! Sets the specified values.
   *  @param[in] newWidth The desired width.
   *  @param[in] newHeight The desired height.
   *  @param[in] newColorBits The desired number of color bits.
   */
  void set(unsigned int newWidth, unsigned int newHeight, unsigned int newColorBits);
  /*! The desired width of the context.
   */
  unsigned int width;
  /*! The desired height of the context.
   */
  unsigned int height;
  /*! The desired color buffer bit depth.
   */
  unsigned int colorBits;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup opengl
 */
typedef std::vector<ScreenMode> ScreenModeList;

///////////////////////////////////////////////////////////////////////

/*! @brief %Context settings.
 *  @ingroup opengl
 *
 *  This class provides the settings parameters available for OpenGL
 *  context creation, as provided through Context::create.
 */
class ContextMode : public ScreenMode
{
public:
  /*! Default constructor.
   */
  ContextMode();
  /*! Constructor.
   */
  ContextMode(unsigned int width,
	      unsigned int height,
	      unsigned int colorBits,
	      unsigned int depthBits = 0,
	      unsigned int stencilBits = 0,
	      unsigned int samples = 0,
	      WindowMode mode = WINDOWED);
  /*! Resets all value to their defaults.
   */
  void setDefaults();
  /*! Sets the specified value.
   */
  void set(unsigned int newWidth,
	   unsigned int newHeight,
	   unsigned int newColorBits,
	   unsigned int newDepthBits = 0,
	   unsigned int newStencilBits = 0,
	   unsigned int newSamples = 0,
	   WindowMode newFlags = WINDOWED);
  /*! The desired depth buffer bit depth.
   */
  unsigned int depthBits;
  /*! The desired stencil buffer bit depth.
   */
  unsigned int stencilBits;
  /*! The desired number of FSAA samples.
   */
  unsigned int samples;
  /*! The desired window mode.
   */
  WindowMode mode;
};

///////////////////////////////////////////////////////////////////////

/*! OpenGL limits data.
 *  @ingroup opengl
 */
class Limits
{
public:
  /*! Constructor.
   */
  Limits(Context& context);
  /*! @return The maximum number of color buffers that can be attached to to an
   *  image framebuffer (FBO).
   */
  unsigned int getMaxColorAttachments() const;
  /*! @return The maximum number of simultaneously active color buffers.
   */
  unsigned int getMaxDrawBuffers() const;
  /*! @return The number of available vertex shader texture image units.
   */
  unsigned int getMaxVertexTextureImageUnits() const;
  /*! @return The number of available fragment shader texture image units.
   */
  unsigned int getMaxFragmentTextureImageUnits() const;
  /*! @return The number of available vertex and fragment shader texture image units.
   */
  unsigned int getMaxCombinedTextureImageUnits() const;
  /*! @return The maximum size, in pixels, of 2D POT textures.
   */
  unsigned int getMaxTextureSize() const;
  /*! @return The maximum size, in pixels, of 3D POT textures.
   */
  unsigned int getMaxTexture3DSize() const;
  /*! @return The maximum size, in pixels, of cube map texture faces.
   */
  unsigned int getMaxTextureCubeSize() const;
  /*! @return The maximum size, in pixels, of non-POT 2D textures.
   */
  unsigned int getMaxTextureRectangleSize() const;
  /*! @return The number of available texture coordinates.
   */
  unsigned int getMaxTextureCoords() const;
  /*! @return The number of available vertex attributes.
   */
  unsigned int getMaxVertexAttributes() const;
private:
  Context& context;
  unsigned int maxColorAttachments;
  unsigned int maxDrawBuffers;
  unsigned int maxVertexTextureImageUnits;
  unsigned int maxFragmentTextureImageUnits;
  unsigned int maxCombinedTextureImageUnits;
  unsigned int maxTextureSize;
  unsigned int maxTexture3DSize;
  unsigned int maxTextureCubeSize;
  unsigned int maxTextureRectangleSize;
  unsigned int maxVertexAttributes;
  unsigned int maxTextureCoords;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Framebuffer.
 *  @ingroup opengl
 *
 *  This class represents a render target, i.e. a framebuffer.
 */
class Framebuffer : public RefObject
{
  friend class Context;
  friend class ImageFramebuffer;
public:
  /*! Destructor.
   */
  virtual ~Framebuffer();
  /*! @return The width, in pixels, of this framebuffer.
   */
  virtual unsigned int getWidth() const = 0;
  /*! @return The height, in pixels, of this framebuffer.
   */
  virtual unsigned int getHeight() const = 0;
  /*! @return The aspect ratio of the dimensions, in pixels, of this framebuffer.
   */
  float getAspectRatio() const;
  /*! @return The context within which this framebuffer was created.
   */
  Context& getContext() const;
protected:
  /*! Constructor.
   */
  Framebuffer(Context& context);
  /*! Called when this framebuffer is to be made current.
   */
  virtual void apply() const = 0;
private:
  Framebuffer(const Framebuffer& source);
  Framebuffer& operator = (const Framebuffer& source);
  Context& context;
};

///////////////////////////////////////////////////////////////////////

/*! @brief %Framebuffer for rendering to the screen.
 *  @ingroup opengl
 */
class DefaultFramebuffer : public Framebuffer
{
  friend class Context;
public:
  /*! @return The default framebuffer color depth, in bits.
   */
  unsigned int getColorBits() const;
  /*! @return The default framebuffer depth-buffer depth, in bits.
   */
  unsigned int getDepthBits() const;
  /*! @return The default framebuffer stencil buffer depth, in bits.
   */
  unsigned int getStencilBits() const;
  unsigned int getWidth() const;
  unsigned int getHeight() const;
private:
  DefaultFramebuffer(Context& context);
  void apply() const;
  ContextMode mode;
};

///////////////////////////////////////////////////////////////////////

/*! @brief %Framebuffer for rendering to images.
 *  @ingroup opengl
 */
class ImageFramebuffer : public Framebuffer
{
public:
  /*! Framebuffer image attachment point enumeration.
   */
  enum Attachment
  {
    /*! The first (default) color buffer, referenced by @c gl_FragColor or @c
     *  gl_FragData[0].
     */
    COLOR_BUFFER0,
    /*! The second color buffer, referenced in GLSL by @c gl_FragData[1].
     */
    COLOR_BUFFER1,
    /*! The third color buffer, referenced in GLSL by @c gl_FragData[2].
     */
    COLOR_BUFFER2,
    /*! The fourth color buffer, referenced in GLSL by @c gl_FragData[3].
     */
    COLOR_BUFFER3,
    /*! The depth buffer, referenced in GLSL by @c gl_FragDepth.
     */
    DEPTH_BUFFER,
  };
  /*! Destructor.
   */
  ~ImageFramebuffer();
  /*! @copydoc Framebuffer::getWidth
   */
  unsigned int getWidth() const;
  /*! @copydoc Framebuffer::getHeight
   */
  unsigned int getHeight() const;
  /*! @return The image attached to the ImageFramebuffer::COLOR_BUFFER0
   *  attachment point, or @c NULL if no image is attached to it.
   */
  Image* getColorBuffer() const;
  /*! @return The image attached to the ImageFramebuffer::DEPTH_BUFFER
   *  attachment point, or @c NULL if no image is attached to it.
   */
  Image* getDepthBuffer() const;
  /*! @return The image attached to the specified attachment point, or @c NULL
   *  if no image is attached to it.
   */
  Image* getBuffer(Attachment attachment) const;
  /*! Sets the image to use as the default color buffer for this framebuffer.
   *  @param[in] newImage The desired image, or @c NULL to detach the currently
   *  set image.
   *  @return @c true if this framebuffer is complete, or @c false otherwise.
   */
  bool setColorBuffer(Image* newImage);
  /*! sets the image to use as the depth buffer for this framebuffer.
   *  @param[in] newImage The desired image, or @c NULL to detach the currently
   *  set image.
   *  @return @c true if this framebuffer is complete, or @c false otherwise.
   */
  bool setDepthBuffer(Image* newImage);
  /*! sets the image to use for the specified attachment point of this
   *  framebuffer.
   *  @param[in] newImage The desired image, or @c NULL to detach the currently
   *  set image.
   *  @param[in] z The desired Z slice of the specified image to use.  This
   *  only applies to images of 3D textures.
   *  @return @c true if this framebuffer is complete, or @c false otherwise.
   */
  bool setBuffer(Attachment attachment, Image* newImage, unsigned int z = 0);
  /*! Creates an image framebuffer within the specified context.
   */
  static ImageFramebuffer* create(Context& context);
private:
  ImageFramebuffer(Context& context);
  bool init();
  void apply() const;
  unsigned int bufferID;
  ImageRef images[5];
};

///////////////////////////////////////////////////////////////////////

/*! @brief %Render statistics.
 *  @ingroup opengl
 */
class Stats
{
public:
  class Frame
  {
  public:
    Frame();
    unsigned int passCount;
    unsigned int vertexCount;
    unsigned int pointCount;
    unsigned int lineCount;
    unsigned int triangleCount;
    Time duration;
  };
  typedef std::deque<Frame> FrameQueue;
  Stats();
  void addFrame();
  void addPasses(unsigned int count);
  void addPrimitives(PrimitiveType type, unsigned int count);
  float getFrameRate() const;
  unsigned int getFrameCount() const;
  const Frame& getFrame() const;
private:
  unsigned int frameCount;
  float frameRate;
  FrameQueue frames;
  Timer timer;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Interface for global GPU program state requests.
 *  @ingroup opengl
 */
class SharedProgramState : public RefObject
{
  friend class ProgramState;
protected:
  virtual void updateTo(Uniform& uniform) = 0;
  virtual void updateTo(Sampler& uniform) = 0;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup opengl
 */
class SharedSampler
{
public:
  SharedSampler(const char* name, Sampler::Type type, int ID);
  String name;
  Sampler::Type type;
  int ID;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup opengl
 */
class SharedUniform
{
public:
  SharedUniform(const char* name, Uniform::Type type, int ID);
  String name;
  Uniform::Type type;
  int ID;
};

///////////////////////////////////////////////////////////////////////

/*! @brief OpenGL context singleton.
 *  @ingroup opengl
 *
 *  This class encapsulates the OpenGL context and its associtated window.
 *
 *  @remarks Yes, it's big.
 */
class Context : public Singleton<Context>
{
public:
  /*! Refresh mode enumeration.
   */
  enum RefreshMode
  {
    /*! The Context::update method does not block.
     */
    AUTOMATIC_REFRESH,
    /*! The Context::update method blocks until a refresh is requested.
     */
    MANUAL_REFRESH,
  };
  typedef std::vector<Plane> PlaneList;
  /*! Destructor.
   */
  ~Context();
  /*! Clears the current color buffer with the specified color.
   *  @param[in] color The color value to clear the color buffer with.
   */
  void clearColorBuffer(const vec4& color = vec4(0.f));
  /*! Clears the current depth buffer with the specified depth value.
   *  @param[in] depth The depth value to clear the depth buffer with.
   */
  void clearDepthBuffer(float depth = 1.f);
  /*! Clears the current stencil buffer with the specified stencil value.
   *  @param[in] value The stencil value to clear the stencil buffer with.
   */
  void clearStencilBuffer(unsigned int value = 0);
  /*! Renders the specified primitive range to the current framebuffer, using
   *  the current GPU program.
   *  @pre A GPU program must be set before calling this method.
   */
  void render(const PrimitiveRange& range);
  /*! Renders the specified primitive range to the current framebuffer, using
   *  the current GPU program.
   *  @pre A GPU program must be set before calling this method.
   */
  void render(PrimitiveType type, unsigned int start, unsigned int count);
  /*! Makes Context::update to return when in manual refresh mode, forcing
   *  a new iteration of the render loop.
   */
  void refresh();
  /*! Swaps the buffer chain, processes any queued events and, in manual
   *  refresh mode, blocks until either the window is closed or a call to
   *  Context::refresh is made.
   */
  bool update();
  /*! Emulates a user close request, causing a close request signal to be
   *  emitted.
   */
  void requestClose();
  /*! Reserves the specified sampler uniform signature as shared.
   */
  void createSharedSampler(const char* name, Sampler::Type type, int ID);
  /*! Reserves the specified non-sampler uniform signature as shared.
   */
  void createSharedUniform(const char* name, Uniform::Type type, int ID);
  /*! @return The shared ID of the specified sampler uniform signature.
   */
  int getSharedSamplerID(const char* name, Sampler::Type type) const;
  /*! @return The shared ID of the specified non-sampler uniform signature.
   */
  int getSharedUniformID(const char* name, Uniform::Type type) const;
  /*! @return The current shared program state, or @c NULL if no shared program
   *  state is currently set.
   */
  SharedProgramState* getCurrentSharedProgramState() const;
  /*! Sets the current shared program state.
   *  @param[in] newState The new state object.
   */
  void setCurrentSharedProgramState(SharedProgramState* newState);
  /*! @return GPU declarations of all shared samplers and uniforms.
   */
  const char* getSharedProgramStateDeclaration() const;
  /*! @return The current refresh mode.
   */
  RefreshMode getRefreshMode() const;
  /*! Sets the refresh mode.
   *  @param[in] newMode The desired new refresh mode.
   */
  void setRefreshMode(RefreshMode newMode);
  /*! @return The current scissor rectangle.
   */
  const Recti& getScissorArea() const;
  /*! Sets the scissor area of this context.
   *
   *  @remarks Scissor testing is enabled if the area doesn't include the
   *  entire current framebuffer.
   */
  void setScissorArea(const Recti& newArea);
  /*! @return The current viewport rectangle.
   */
  const Recti& getViewportArea() const;
  /*! Sets the current viewport rectangle.
   *  @param[in] newArea The desired viewport rectangle.
   */
  void setViewportArea(const Recti& newArea);
  /*! @return The current framebuffer.
   */
  Framebuffer& getCurrentFramebuffer() const;
  /*! @return The screen framebuffer.
   */
  DefaultFramebuffer& getDefaultFramebuffer() const;
  /*! Makes the default framebuffer current.
   */
  void setDefaultFramebufferCurrent();
  /*! Makes the specified framebuffer current.
   *  @param[in] newFramebuffer The desired framebuffer.
   *  @return @c true if successful, or @c false otherwise.
   */
  bool setCurrentFramebuffer(Framebuffer& newFramebuffer);
  /*! @return The currently set GPU program, or @c NULL if no program is set.
   */
  Program* getCurrentProgram() const;
  /*! Sets the current GPU program for use when rendering.
   *  @param[in] newProgram The desired GPU program, or @c NULL to unbind
   *  the current program.
   */
  void setCurrentProgram(Program* newProgram);
  /*! @return The currently set vertex buffer.
   */
  VertexBuffer* getCurrentVertexBuffer() const;
  /*! Sets the current vertex buffer.
   */
  void setCurrentVertexBuffer(VertexBuffer* newVertexBuffer);
  /*! @return The currently set index buffer.
   */
  IndexBuffer* getCurrentIndexBuffer() const;
  /*! Sets the current index buffer.
   */
  void setCurrentIndexBuffer(IndexBuffer* newIndexBuffer);
  /*! @note Unless you are Wendy, you probably don't need to call this.
   */
  Texture* getCurrentTexture() const;
  /*! @note Unless you are Wendy, you probably don't need to call this.
   */
  void setCurrentTexture(Texture* newTexture);
  /*! @note Unless you are Wendy, you probably don't need to call this.
   */
  unsigned int getActiveTextureUnit() const;
  /*! @note Unless you are Wendy, you probably don't need to call this.
   */
  void setActiveTextureUnit(unsigned int unit);
  Stats* getStats() const;
  void setStats(Stats* newStats);
  /*! @return The title of the context window.
   */
  const String& getTitle() const;
  /*! Sets the title of the context window.
   *  @param[in] newTitle The desired title.
   */
  void setTitle(const char* newTitle);
  /*! @return The limits of this context.
   */
  const Limits& getLimits() const;
  /*! @return The resource index used by this context.
   */
  ResourceIndex& getIndex() const;
  /*! @return The signal for per-frame post-render clean-up.
   */
  SignalProxy0<void> getFinishSignal();
  /*! @return The signal for user-initiated close requests.
   */
  SignalProxy0<bool> getCloseRequestSignal();
  /*! @return The signal for context resizing.
   */
  SignalProxy2<void, unsigned int, unsigned int> getResizedSignal();
  /*! Creates the context singleton object, using the specified settings.
   *  @param[in] index The resource index to use.
   *  @param[in] mode The requested context settings.
   *  @return @c true if successful, or @c false otherwise.
   */
  static bool createSingleton(ResourceIndex& index, const ContextMode& mode = ContextMode());
  /*! Retrieves the supported screen modes.
   *  @param[out] result The supported modes.
   */
  static void getScreenModes(ScreenModeList& result);
private:
  Context(ResourceIndex& index);
  Context(const Context& source);
  Context& operator = (const Context& source);
  bool init(const ContextMode& mode);
  static void sizeCallback(int width, int height);
  static int closeCallback();
  static void refreshCallback();
  typedef std::vector<SharedSampler> SamplerList;
  typedef std::vector<SharedUniform> UniformList;
  ResourceIndex& index;
  Signal0<void> finishSignal;
  Signal0<bool> closeRequestSignal;
  Signal2<void, unsigned int, unsigned int> resizedSignal;
  String title;
  Ptr<Limits> limits;
  RefreshMode refreshMode;
  bool needsRefresh;
  bool needsClosing;
  Recti scissorArea;
  Recti viewportArea;
  bool dirtyBinding;
  SamplerList samplers;
  UniformList uniforms;
  String declaration;
  TextureList textureUnits;
  unsigned int activeTextureUnit;
  Ref<Program> currentProgram;
  Ref<SharedProgramState> currentState;
  Ref<VertexBuffer> currentVertexBuffer;
  Ref<IndexBuffer> currentIndexBuffer;
  Ref<Framebuffer> currentFramebuffer;
  Ref<DefaultFramebuffer> defaultFramebuffer;
  PlaneList planes;
  Stats* stats;
  static Context* instance;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLCONTEXT_H*/
///////////////////////////////////////////////////////////////////////
