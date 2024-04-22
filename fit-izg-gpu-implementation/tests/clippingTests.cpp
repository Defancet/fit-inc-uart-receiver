#include <catch2/catch_test_macros.hpp>

#include <iostream>
#include <string.h>

#include <algorithm>
#include <numeric>

#include <student/gpu.hpp>
#include <framework/method.hpp>
#include <framework/framebuffer.hpp>

#include <tests/testCommon.hpp>

using namespace tests;

#include <glm/gtc/matrix_transform.hpp>

SCENARIO("21"){
  std::cerr << "21 - clipping - CW triangle behind near plane" << std::endl;

  auto&inFragments = dumpInject.inFragments;
  auto&outVertices = dumpInject.outVertices;

  OutVertex a;
  outVertices.clear();
  inFragments.clear();
  //near 1, far 2
  outVertices.push_back({{},glm::vec4(-1.f,-1.f,-7.f,-1.f)});
  outVertices.push_back({{},glm::vec4(+1.f,-1.f,-7.f,-1.f)});
  outVertices.push_back({{},glm::vec4(-1.f,+1.f,-7.f,-1.f)});

  uint32_t w = 100;
  uint32_t h = 100;

  auto framebuffer = std::make_shared<Framebuffer>(w,h);

  MEMCB();

  mem.framebuffer = framebuffer->getFrame();
  mem.programs[0].vertexShader = vertexShaderInject;
  mem.programs[0].fragmentShader = fragmentShaderDump;

  pushDrawCommand(cb,3);

  gpu_execute(mem,cb);

  REQUIRE(inFragments.size() == 0);
}


SCENARIO("22"){
  std::cerr << "22 - clipping - CCW triangle behind near plane" << std::endl;

  auto&inFragments = dumpInject.inFragments;
  auto&outVertices = dumpInject.outVertices;

  OutVertex a;
  outVertices.clear();
  inFragments.clear();
  //near 1, far 2
  outVertices.push_back({{},glm::vec4(-1.f,-1.f,-7.f,-1.f)});
  outVertices.push_back({{},glm::vec4(-1.f,+1.f,-7.f,-1.f)});
  outVertices.push_back({{},glm::vec4(+1.f,-1.f,-7.f,-1.f)});

  uint32_t w = 100;
  uint32_t h = 100;
  auto framebuffer = std::make_shared<Framebuffer>(w,h);

  MEMCB();

  mem.framebuffer = framebuffer->getFrame();
  mem.programs[0].vertexShader   = vertexShaderInject;
  mem.programs[0].fragmentShader = fragmentShaderDump;

  pushDrawCommand(cb,3);

  gpu_execute(mem,cb);

  REQUIRE(inFragments.size() == 0);
}


SCENARIO("23"){
  std::cerr << "23 - clipping - 1 points behind near plane" << std::endl;

  auto&inFragments = dumpInject.inFragments;
  auto&outVertices = dumpInject.outVertices;

  OutVertex a;
  outVertices.clear();
  inFragments.clear();
  
  float nnear = 1.f;
  float ffar  = 2.f;
  float e = - (ffar + nnear) / (ffar - nnear);
  float f = - 2.f * ffar * nnear / (ffar - nnear);
  float z = 2.f/3.f;

  outVertices.push_back({{},glm::vec4(-2.f,-2.f,+2.f,+2.f)});
  outVertices.push_back({{},glm::vec4(+2.f,-2.f,+2.f,+2.f)});
  outVertices.push_back({{},glm::vec4(-z  ,+z  ,-z*e+f,+z)});

  uint32_t w = 100;
  uint32_t h = 100;
  auto framebuffer = std::make_shared<Framebuffer>(w,h);

  MEMCB();

  mem.framebuffer = framebuffer->getFrame();
  mem.programs[0].vertexShader = vertexShaderInject;
  mem.programs[0].fragmentShader = fragmentShaderDump;


  pushDrawCommand(cb,3);

  gpu_execute(mem,cb);

  uint32_t const expectedCount = 100*100/8*3;
  uint32_t const err           = 2*100;

  if(expectedCount < inFragments.size())
    REQUIRE(inFragments.size() <= expectedCount + err);
  else
    REQUIRE(inFragments.size() >= expectedCount - err);
}

SCENARIO("24"){
  std::cerr << "24 - clipping - 2 vertices behind near plane" << std::endl;

  auto&inFragments = dumpInject.inFragments;
  auto&outVertices = dumpInject.outVertices;

  OutVertex a;
  outVertices.clear();
  inFragments.clear();
  
  float nnear = 1.f;
  float ffar  = 2.f;
  float e = - (ffar + nnear) / (ffar - nnear);
  float f = - 2.f * ffar * nnear / (ffar - nnear);
  float z = 2.f/3.f;

  outVertices.push_back({{},glm::vec4(-z,-z,-z*e+f,+z)});
  outVertices.push_back({{},glm::vec4(+z,-z,-z*e+f,+z)});
  outVertices.push_back({{},glm::vec4(-2,+2,+2,+2)});

  uint32_t w = 100;
  uint32_t h = 100;
  auto framebuffer = std::make_shared<Framebuffer>(w,h);

  MEMCB();

  mem.framebuffer = framebuffer->getFrame();
  mem.programs[0].vertexShader = vertexShaderInject;
  mem.programs[0].fragmentShader = fragmentShaderDump;


  pushDrawCommand(cb,3);

  gpu_execute(mem,cb);

  uint32_t const expectedCount = 100*100/8*1;
  uint32_t const err           = 2*100;

  if(expectedCount < inFragments.size())
    REQUIRE(inFragments.size() <= expectedCount + err);
  else
    REQUIRE(inFragments.size() >= expectedCount - err);
}

