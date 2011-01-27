
#include <wendy/Wendy.h>

#include <cstdlib>

using namespace wendy;

class Demo : public Trackable
{
public:
  ~Demo(void);
  bool init(void);
  void run(void);
private:
  bool render(void);
  ResourceIndex index;
  Ptr<render::GeometryPool> pool;
  Ref<render::Camera> camera;
  scene::Graph graph;
  scene::ModelNode* modelNode;
  scene::CameraNode* cameraNode;
  Timer timer;
  Time currentTime;
};

Demo::~Demo(void)
{
  graph.destroyRootNodes();

  pool = NULL;

  GL::Context::destroySingleton();
}

bool Demo::init(void)
{
  if (!index.addSearchPath(Path("../media")))
    return false;

  if (!GL::Context::createSingleton(index))
    return false;

  GL::Context* context = GL::Context::getSingleton();
  context->setTitle("Program");

  pool = new render::GeometryPool(*context);

  Ref<render::Model> model = render::Model::read(*context, Path("thingy.model"));
  if (!model)
  {
    logError("Failed to load model");
    return false;
  }

  modelNode = new scene::ModelNode();
  modelNode->setModel(model);
  graph.addRootNode(*modelNode);

  camera = new render::Camera();
  camera->setFOV(60.f);
  camera->setAspectRatio(0.f);

  cameraNode = new scene::CameraNode();
  cameraNode->setCamera(camera);
  cameraNode->getLocalTransform().position.z = model->getBounds().radius * 3.f;
  graph.addRootNode(*cameraNode);

  timer.start();

  return true;
}

void Demo::run(void)
{
  render::Queue queue(*pool, *camera);
  GL::Context& context = pool->getContext();

  do
  {
    currentTime = timer.getTime();

    modelNode->getLocalTransform().rotation.setAxisRotation(Vec3(0.f, 1.f, 0.f),
							    (float) currentTime);

    graph.update();

    context.clearDepthBuffer();
    context.clearColorBuffer(ColorRGBA(0.2f, 0.2f, 0.2f, 1.f));

    graph.enqueue(queue);
    queue.render();

    queue.removeOperations();
  }
  while (context.update());
}

int main()
{
  if (!wendy::initialize())
    std::exit(EXIT_FAILURE);

  Ptr<Demo> demo(new Demo());
  if (demo->init())
    demo->run();

  demo = NULL;

  wendy::shutdown();
  std::exit(EXIT_SUCCESS);
}
