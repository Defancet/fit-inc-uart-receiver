/*!
 * @file
 * @brief This file contains implementation of 3D triangle rendering method.
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 */

#include <framework/programContext.hpp>

namespace texturedQuad{

/**
 * @brief This class represents texture rendering method
 */
class Method: public ::Method{
  public:
    Method(MethodConstructionData const*);
    virtual ~Method();
    virtual void onDraw(Frame&frame,SceneParam const&sceneParam) override;
    CommandBuffer commandBuffer;
    GPUMemory     mem          ;
    TextureData tex;///< texture
};

/**
 * @brief This function represents points shader of texture rendering method.
 *
 * @param outVertex output points
 * @param inVertex input points
 * @param uniforms uniform variables
 */
void vertexShader(OutVertex&outVertex,InVertex const&inVertex,ShaderInterface const&){
  outVertex.gl_Position = glm::vec4(0.f,0.f,0.f,1.f);

  //fullscreen quad
  glm::vec2 const verts[]={
    glm::vec2(-1.f,-1.f),
    glm::vec2(+1.f,-1.f),
    glm::vec2(-1.f,+1.f),
    glm::vec2(-1.f,+1.f),
    glm::vec2(+1.f,-1.f),
    glm::vec2(+1.f,+1.f),
  };

  outVertex.gl_Position = glm::vec4(verts[inVertex.gl_VertexID],0.f,1.f);
  outVertex.attributes[0].v2 = (glm::vec2(outVertex.gl_Position)+1.f)*.5f;
}

/**
 * @brief This functionrepresents fragment shader of texture rendering method.
 *
 * @param outFragment output fragment
 * @param inFragment input fragment
 * @param uniforms uniform variables
 */
void fragmentShader(OutFragment&outFragment,InFragment const&inFragment,ShaderInterface const&si){
  auto uv = inFragment.attributes[0].v2;
  outFragment.gl_FragColor = read_texture(si.textures[0],uv);
}

/**
 * @brief Constructor
 */
Method::Method(MethodConstructionData const*){
  tex = loadTexture(ProgramContext::get().args.imageFile);

  mem.textures[0] = tex.getTexture();
  mem.programs[0].vertexShader   = vertexShader  ; 
  mem.programs[0].fragmentShader = fragmentShader;
  mem.programs[0].vs2fs[0]       = AttributeType::VEC2;//tex coords

  pushClearCommand(commandBuffer,glm::vec4(0,0,0,1));
  pushDrawCommand (commandBuffer,6);
}

/**
 * @brief This function is called every frame and should render 3D triangle
 *
 * @param proj projection matrix
 * @param view view matrix
 * @param light light position
 * @param camera camera position
 */
void Method::onDraw(Frame&frame,SceneParam const&){
  mem.framebuffer = frame;
  gpu_execute(mem,commandBuffer);
}

/**
 * @brief Descturctor
 */
Method::~Method(){}

EntryPoint main = [](){registerMethod<Method>("izg11 Neutitschein 1863");};

}
