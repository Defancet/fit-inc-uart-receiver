/*!
 * @file
 * @brief This file contains implementation of 2D triangle rendering method
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 */

#include <framework/programContext.hpp>

namespace triangleMethod{

/**
 * @brief 2D Triangle rendering method
 */
class Method: public ::Method{
  public:
    Method(MethodConstructionData const*);
    virtual ~Method();
    virtual void onDraw(Frame&frame,SceneParam const&sceneParam) override;
    CommandBuffer commandBuffer;
    GPUMemory     mem          ;
};

/**
 * @brief Vertex shader
 *
 * @param outVertex output points
 * @param inVertex input points
 * @param uniforms uniform variables
 */
void vertexShader(OutVertex&outVertex,InVertex const&inVertex,ShaderInterface const&){
  if(inVertex.gl_VertexID == 0)
    outVertex.gl_Position = glm::vec4(-1,-1,0,1);
  if(inVertex.gl_VertexID == 1)
    outVertex.gl_Position = glm::vec4(+1,-1,0,1);
  if(inVertex.gl_VertexID == 2)
    outVertex.gl_Position = glm::vec4(-1,+1,0,1);
}

/**
 * @brief Fragment shader
 *
 * @param outFragment output fragment
 * @param inFragment input fragment
 * @param uniforms uniform variables
 */
void fragmentShader(OutFragment&outFragment,InFragment const&,ShaderInterface const&){
  outFragment.gl_FragColor = glm::vec4(1.f);
}


Method::Method(MethodConstructionData const*){
  mem.programs[0].vertexShader   = vertexShader  ;
  mem.programs[0].fragmentShader = fragmentShader;

  pushClearCommand(commandBuffer,glm::vec4(.1,.6,.1,1));
  pushDrawCommand (commandBuffer,3);
}

Method::~Method(){
}


void Method::onDraw(Frame&frame,SceneParam const&){
  mem.framebuffer = frame;
  gpu_execute(mem,commandBuffer);
}

EntryPoint main = [](){registerMethod<Method>("izg01 triangle 2D");};

}
