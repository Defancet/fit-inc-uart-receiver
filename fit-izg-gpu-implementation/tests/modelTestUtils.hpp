#pragma once

#include<cstdint>
#include<student/fwd.hpp>
#include<student/drawModel.hpp>
#include<tests/testCommon.hpp>
#include<string>
#include<memory>

namespace tests::model{

enum DrawCallUniform{
  MODEL         = 0,
  INV_MODEL        ,
  DIFF_COLOR       ,
  TEXTURE_ID       ,
  DOUBLE_SIDED     ,
};

uint32_t const drawCallUniformOffset = 10;
uint32_t const drawCallNofUniforms   = (uint32_t)DOUBLE_SIDED+1;

uint32_t drawCallUniforms(uint32_t drawId);
uint32_t uniformIdToDrawCallUniform(uint32_t uId);

enum class Diff{
  NOF_COMMANDS,
  COMMAND_TYPE,
  CLEAR,
  BACKFACE,
  PROGRAMID,
  NOF_VERTICES,
  INDEXING,
  ATTRIB,

  SHADERS,
  VS2FS,
  BUFFERS,
  TEXTURES,

  TEXTURE_ID,
  DOUBLE_SIDED,
  DIFFUSE_COLOR,
  MODEL_MATRIX,
  INV_MATRIX,

  SAME,
};

struct MeshI{
  MeshI(
      uint32_t        n = 0             ,
      glm::vec4 const&c = glm::vec4(1.f),
      int             t = -1            ,
      bool            d = false         ):
    nofIndices(n),diffuseColor(c),diffuseTexture(t),doubleSided(d){}
  uint32_t  nofIndices     = 0;
  glm::vec4 diffuseColor   = glm::vec4(1.f);
  int       diffuseTexture = -1;
  bool      doubleSided    = false;
};

struct NodeI{
  NodeI(
      int32_t                 m     = -1,
      std::vector<NodeI>const&c     = {},
      glm::mat4         const&model = glm::mat4(1.f));
  int32_t           mesh     = -1            ;
  std::vector<NodeI>children                 ;
  glm::mat4         model    = glm::mat4(1.f);
};

Model createModel(std::vector<MeshI>const&meshes,std::vector<NodeI>const&roots);

using TranslatedModel = std::shared_ptr<MemCb>;

struct Check{
  TranslatedModel expected;
  TranslatedModel student ;
  Diff status;
};

Check checkTranslation(Model const&model,Diff const&whatToTest);

std::string translatedModelErrors(
    size_t      p,
    Check const&check);

void checkTraverse(std::vector<MeshI>const&meshes,std::vector<NodeI>const&roots);

void checkModelMemory(Model const&model,Diff const&level = Diff::INV_MATRIX);

}
