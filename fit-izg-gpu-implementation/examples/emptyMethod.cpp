#include <framework/programContext.hpp>

namespace emptyMethod{

/**
 * @brief Empty rendering method
 */
class Method: public ::Method{
  public:
    Method(MethodConstructionData const*);
    virtual ~Method();
    virtual void onDraw(Frame&frame,SceneParam const&sceneParam) override;
    CommandBuffer commandBuffer;
    GPUMemory mem;
};


Method::Method(MethodConstructionData const*){
  pushClearCommand(commandBuffer,glm::vec4(.5,.2,0,1));
}

Method::~Method(){
}

void Method::onDraw(Frame&frame,SceneParam const&){
  mem.framebuffer = frame;
  gpu_execute(mem,commandBuffer);
}

EntryPoint main = [](){registerMethod<Method>("izg00 empty window");};

}
