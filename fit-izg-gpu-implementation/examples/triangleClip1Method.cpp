/*!
 * @file
 * @brief This file contains implementation of triangle clipping rendering method
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 */

#include <framework/programContext.hpp>

namespace triangleClip1Method{

/**
 * @brief Triangle clipping method 1
 */
class Method: public ::Method{
  public:
    Method(MethodConstructionData const*);
    virtual ~Method(){}
    virtual void onDraw(Frame&frame,SceneParam const&sceneParam) override;
    GPUMemory     mem;
    CommandBuffer commandBuffer;
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
    outVertex.gl_Position = glm::vec4(-2,-2,+2,+2);
  if(inVertex.gl_VertexID == 1)
    outVertex.gl_Position = glm::vec4(+2,-2,+2,+2);
  float nnear = 1.f;
  float ffar  = 2.f;
  float e = - (ffar + nnear) / (ffar - nnear);
  float f = - 2.f * ffar * nnear / (ffar - nnear);

  float z = 2.f/3.f;

  if(inVertex.gl_VertexID == 2)
    outVertex.gl_Position = glm::vec4(-z,+z,-z*e+f,+z);
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

  mem.programs[0].vertexShader   = vertexShader;
  mem.programs[0].fragmentShader = fragmentShader;

  pushClearCommand(commandBuffer,glm::vec4(.1,.1,.1,1));
  pushDrawCommand(commandBuffer ,3);
}

void Method::onDraw(Frame&frame,SceneParam const&){
  mem.framebuffer = frame;
  gpu_execute(mem,commandBuffer);
}

EntryPoint main = [](){registerMethod<Method>("izg03 Triangle Clip1");};

}
