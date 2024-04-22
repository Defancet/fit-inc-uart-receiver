#pragma once

#include <framework/method.hpp>
#include <framework/model.hpp>

namespace modelMethod{

/**
 * @brief This class represents model visualizer
 */
class Method: public ::Method{
  public:
    Method(MethodConstructionData const*cd = nullptr);
    /**
     * @brief Descturctor
     */
    virtual ~Method(){};
    virtual void onDraw(Frame&frame,SceneParam const&sceneParam) override;
    ModelData     modelData;
    Model         model;
    CommandBuffer commandBuffer;
    GPUMemory     mem;
};

}
