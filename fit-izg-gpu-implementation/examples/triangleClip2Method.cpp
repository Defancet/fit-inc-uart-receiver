#include <framework/programContext.hpp>
#include <framework/method.hpp>

namespace triangleClip2Method{

/**
 * @brief Triangle clipping 2 rendering method
 */
class Method: public ::Method{
  public:
    Method(MethodConstructionData const*);
    virtual ~Method(){}
    virtual void onDraw(Frame&frame,SceneParam const&sceneParam) override;
    CommandBuffer commandBuffer;
    GPUMemory     mem;
};

/**
 * @brief Vertex shader
 *
 * @param outVertex output points
 * @param inVertex input points
 * @param uniforms uniform variables
 */
void vertexShader(OutVertex&outVertex,InVertex const&inVertex,ShaderInterface const&){
  float nnear = 1.f;
  float ffar  = 2.f;
  float e = - (ffar + nnear) / (ffar - nnear);
  float f = - 2.f * ffar * nnear / (ffar - nnear);

  float z = 2.f/3.f;

  if(inVertex.gl_VertexID == 0)
    outVertex.gl_Position = glm::vec4(-z,-z,-z*e+f,+z);
  if(inVertex.gl_VertexID == 1)
    outVertex.gl_Position = glm::vec4(+z,-z,-z*e+f,+z);

  if(inVertex.gl_VertexID == 2)
    outVertex.gl_Position = glm::vec4(-2,+2,+2,+2);
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
  commandBuffer.nofCommands = 2;

  mem.programs[0].vertexShader   = vertexShader;
  mem.programs[0].fragmentShader = fragmentShader;

  pushClearCommand(commandBuffer,glm::vec4(.1,.1,.1,1));
  pushDrawCommand (commandBuffer,3);
}

void Method::onDraw(Frame&frame,SceneParam const&){
  mem.framebuffer = frame;
  gpu_execute(mem,commandBuffer);
}

EntryPoint main = [](){registerMethod<Method>("izg04 Triangle Clip2");};

}
