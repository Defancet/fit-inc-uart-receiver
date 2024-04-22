/*!
 * @file
 * @brief This file contains implementation of model visualizer
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 */

#include <framework/model.hpp>
#include <framework/programContext.hpp>
#include <student/drawModel.hpp>
#include <examples/modelMethod.hpp>

namespace modelMethod{

/**
 * @brief Constructor
 */
Method::Method(MethodConstructionData const*){
  modelData.load(ProgramContext::get().args.modelFile);
  model = modelData.getModel();

  prepareModel(mem,commandBuffer,model);
}


/**
 * @brief This function is called every frame and should render a model
 *
 * @param proj projection matrix
 * @param view view matrix
 * @param light light position
 * @param camera camera position
 */
void Method::onDraw(Frame&frame,SceneParam const&sceneParam){
  mem.framebuffer = frame;
  mem.uniforms[0].m4 = sceneParam.proj * sceneParam.view;
  mem.uniforms[1].v3 = sceneParam.light;
  mem.uniforms[2].v3 = sceneParam.camera;
  gpu_execute(mem,commandBuffer);
}

EntryPoint main = [](){registerMethod<Method>("izg13 model loader");};

}
