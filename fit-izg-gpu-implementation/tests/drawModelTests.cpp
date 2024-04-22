#include <iostream>
#include <string.h>

#include <catch2/catch_test_macros.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <tests/modelTestUtils.hpp>

using namespace tests;
using namespace tests::model;


SCENARIO("32"){
  std::cerr << "32 - prepareModel - memory - shaders" << std::endl;

  auto n1 = NodeI(0);
  auto n2 = NodeI(0,{NodeI(1),NodeI(2)});
  checkModelMemory(createModel({3},{n1}),Diff::SHADERS);
  checkModelMemory(createModel({3},{n1}),Diff::SHADERS);
}

SCENARIO("33"){
  std::cerr << "33 - prepareModel - memory - vs2fs" << std::endl;

  auto n1 = NodeI(0);
  auto n2 = NodeI(0,{NodeI(1),NodeI(2)});
  checkModelMemory(createModel({3},{n1}),Diff::VS2FS);
  checkModelMemory(createModel({3},{n1}),Diff::VS2FS);
}

SCENARIO("34"){
  std::cerr << "34 - prepareModel - memory - buffers" << std::endl;

  Model model;
  model.buffers.push_back({(void*)13,13});
  model.buffers.push_back({(void*)14,14});
  model.buffers.push_back({(void*)15,15});
  checkModelMemory(model,Diff::BUFFERS);
}

SCENARIO("35"){
  std::cerr << "35 - prepareModel - memory - textures" << std::endl;

  Model model;
  model.textures.push_back({(uint8_t*)13,13,14,3});
  model.textures.push_back({(uint8_t*)14,14,14,3});
  model.textures.push_back({(uint8_t*)15,15,15,4});
  checkModelMemory(model,Diff::TEXTURES);
}

SCENARIO("36"){
  std::cerr << "36 - prepareModel - uniforms - diffuse color" << std::endl;

  auto m1 = MeshI(3,glm::vec4(.5));
  auto m2 = MeshI(3,glm::vec4(.2,.3,.4,.1));
  auto m3 = MeshI(3,glm::vec4(.7,.2,.1,.1));
  auto n1 = NodeI(0);
  auto n2 = NodeI(1,{n1,n1});
  auto n3 = NodeI(2,{n2,n2});
  auto model = createModel({m1,m2,m3},{n3});
  checkModelMemory(model,Diff::DIFFUSE_COLOR);
}

SCENARIO("37"){
  std::cerr << "37 - prepareModel - uniforms - texture id" << std::endl;

  auto m1 = MeshI(3,glm::vec4(.5),3);
  auto m2 = MeshI(3,glm::vec4(.2,.3,.4,.1),4);
  auto m3 = MeshI(3,glm::vec4(.7,.2,.1,.1),7);
  auto n1 = NodeI(0);
  auto n2 = NodeI(1,{n1,n1});
  auto n3 = NodeI(2,{n2,n2});
  auto model = createModel({m1,m2,m3},{n3});
  checkModelMemory(model,Diff::TEXTURE_ID);
}

SCENARIO("38"){
  std::cerr << "38 - prepareModel - uniforms - model matrix" << std::endl;

  std::vector<glm::mat4>mats;
  mats.push_back(glm::translate(glm::mat4(1),     glm::vec3(1,2,3)));
  mats.push_back(glm::rotate   (glm::mat4(1),0.3f,glm::vec3(0,1,0)));
  mats.push_back(glm::scale    (glm::mat4(1),     glm::vec3(.3,-2.1,4.3)));

  auto m1 = MeshI(3,glm::vec4(.5),3);
  auto m2 = MeshI(3,glm::vec4(.2,.3,.4,.1),4);
  auto m3 = MeshI(3,glm::vec4(.7,.2,.1,.1),7);
  auto n1 = NodeI(0,{     },mats[0]);
  auto n2 = NodeI(1,{n1,n1},mats[1]);
  auto n3 = NodeI(2,{n2,n2},mats[2]);

  auto model = createModel({m1,m2,m3},{n3});

  checkModelMemory(model,Diff::MODEL_MATRIX);
}

SCENARIO("39"){
  std::cerr << "39 - prepareModel - uniforms - inverse model matrix" << std::endl;

  std::vector<glm::mat4>mats;
  mats.push_back(glm::translate(glm::mat4(1),     glm::vec3(1,2,3)));
  mats.push_back(glm::rotate   (glm::mat4(1),0.3f,glm::vec3(0,1,0)));
  mats.push_back(glm::scale    (glm::mat4(1),     glm::vec3(.3,-2.1,4.3)));

  auto m1 = MeshI(3,glm::vec4(.5),3);
  auto m2 = MeshI(3,glm::vec4(.2,.3,.4,.1),4);
  auto m3 = MeshI(3,glm::vec4(.7,.2,.1,.1),7);
  auto n1 = NodeI(0,{     },mats[0]);
  auto n2 = NodeI(1,{n1,n1},mats[1]);
  auto n3 = NodeI(2,{n2,n2},mats[2]);

  auto model = createModel({m1,m2,m3},{n3});

  checkModelMemory(model,Diff::INV_MATRIX);
}

