#include <catch2/catch_test_macros.hpp>
#include <tests/testCommon.hpp>
#include <tests/commandBufferToStr.hpp>
#include <tests/gpuMemToStr.hpp>

#include <iostream>
#include <string.h>
#include <sstream>
#include<functional>
#include<map>

#include <algorithm>
#include <numeric>

#include <student/gpu.hpp>
#include <framework/framebuffer.hpp>

#include <glm/gtc/matrix_transform.hpp>

using namespace tests;

SCENARIO("04"){
  std::cerr << "04 - points shader, gl_VertexID (no indexing)" << std::endl;

  MEMCB();

  auto framebuffer = std::make_shared<Framebuffer>(100,100);

  mem.framebuffer = framebuffer->getFrame();
  mem.programs[0].vertexShader   = vertexShaderDump0;
  mem.programs[0].fragmentShader = fragmentShaderEmpty;

  pushDrawCommand(cb,9);

  dumpInject.init(&mem);

  gpu_execute(mem,cb);

  framebuffer = nullptr;

  auto check = checkDump(mem,cb,DumpDiff::DIFFERENT_VERTEXID);

  if(check.status != DumpDiff::SAME){
    std::cerr << R".(
    Vrcholy chodí do points shaderu číslované.
    Číslo vrcholu je uloženo v položce gl_VertexID ve struktuře InVertex.
    Když není zapnuté indexování (v tomto testu není), tak je číslo vrcholu rovno číslu zavolání points shaderu.

    Proměnná gl_VertexID odpovída proměnné gl_VertexId z GLSL.)."<<std::endl;
    std::cerr << std::endl;
    std::cerr << commandBufferDumpToStr(4,check);
    std::cerr << std::endl;

    std::cerr << R".(
    Nápověda k implementaci:

    void draw(DrawCommand const&cmd){
      for(... cmd.nofVertices ... ){//smyčka přes vrcholy
        InVertex  inVertex; // vrchol, co vstupuje do points shader
        OutVertex outVertex;// vrchol, co leze ven z points shaderu
        inVertex.gl_VertexID = ??;
        ShaderInterface si;
        ctx.prg.vertexShader(outVertex,inVertex,si);
      }
    }

    ).";

    REQUIRE(false);
  }
}

template<typename T = uint32_t>
void indexTest(){
  auto bits = sizeof(T)*8;
  std::cerr << "  ("<<bits<<"bit indexing)" << std::endl;

  std::vector<T> indices = {0,1,2,2,1,3};

  MEMCB();

  auto framebuffer = std::make_shared<Framebuffer>(100,100);
  mem.framebuffer = framebuffer->getFrame();

  dumpInject.init(&mem);

  mem.programs[0].vertexShader   = vertexShaderDump0;
  mem.programs[0].fragmentShader = fragmentShaderEmpty;
  mem.buffers[0] = vectorToBuffer(indices);

  VertexArray vao;
  vao.indexBufferID  = 0;
  vao.indexOffset    = 0;
  vao.indexType      = (IndexType)sizeof(T);

  pushDrawCommand(cb,(uint32_t)indices.size(),0,vao);
  gpu_execute(mem,cb);

  framebuffer = nullptr;

  auto check = checkDump(mem,cb,DumpDiff::DIFFERENT_VERTEXID);

  if(check.status == DumpDiff::SAME)return;

  std::cerr << R".(
  TEST SELHAL!
  ).";

  std::cerr << std::endl;
  std::cerr << commandBufferDumpToStr(2,check);
  std::cerr << std::endl;

  std::cerr << "  Tahle vypadala paměť grafické karty:" << std::endl;
  std::cerr << gpuMemoryToStr(2,mem,cb) << std::endl;
  std::cerr << "  Takhle vypadal command buffer:" << std::endl;
  std::cerr << commandBufferToStr(2,cb) << std::endl;

  std::cerr << R".(

  Test kreslí )."<<indices.size()<<R".( vrcholů.
  Tyto vrcholy by měly mít správné pořadové číslo (gl_VertexID), které je rovno hodnotě z indexačního bufferu.

  Vrcholy chodí do points shaderu číslované.
  Číslo vrcholu je uloženo v položce gl_VertexID ve struktuře InVertex.
  Když je zapnuté indexování (v tomto testu je), tak je číslo vrcholu rovno hodnotě v indexačním bufferu.
  V tomto testu se předpokládá )."<<bits<<R".( bitový index.
  Číslo bufferu s indexy je v proměnné vao.indexBufferID.
  Typ indexu (8b,16b,32b) je v proměnné vao.indexType.
  Offset indexů je v proměnné vao.indexOffset.

  Nezapomeňte správně adresovat indexační buffer.
  Jedná se o tzn. "pointer arithmetic".

  Musíte si uvědomit, že vao.indexBuffer ukazuje do tabulky všech bufferů v GPUMemory.
  Indexační buffer obsahuje indexy, které mají všechny stejnou velikost, která může být 8b, 16b nebo 32 bitů.

  Položka z indexačního bufferu je vybrána číslem invokace points shaderu.

  Na buffer je možné nahlížet jako na pole:
  uint)."<<bits<<R".(_t*pole;

  číslo vrcholu gl_VertexID je potom rovno pole[číslo invokace points shaderu].

  Indexing je vypnutý, pokud je vao.indexBufferID < 0.

  Nápověda k implementaci:

  uint32_t computeVertexID(GPUMemory&mem,VertexArray const&vao,uint32_t shaderInvocation){
    if(vao.indexBufferID<0)return shaderInvocation;

    void*indexBuffer = mem.buffers[vao.indexBufferID];

    if(vao.indexType == IndexType::UIN)."<<bits<<R".(){
      uint)."<<bits<<R".(_t*ind = (uint)."<<bits<<R".(_t*)(vao.indexBuffer+vao.indexOffset);
      return ind[shaderInvocation];
    }
    ...
  }

  void draw(GPUMemory&mem,DrawCommand const&cmd){

    for(...){//smyčka přes vrcholy
      ...
      InVertex  inVertex; // vrchol, co vstupuje do points shader
      inVertex.gl_VertexID = computeVertexID(mem,cmd.vao,...);
      ...
    }

  }


  ).";

  REQUIRE(false);
}

SCENARIO("05"){
  std::cerr << "05 - points shader, gl_VertexID (indexing)" << std::endl;
  indexTest<uint32_t>();
  indexTest<uint16_t>();
  indexTest<uint8_t>();
}




SCENARIO("06"){
  std::cerr << "06 - points shader, shader interface" << std::endl;

  MEMCB();


  auto framebuffer = std::make_shared<Framebuffer>(100,100);
  mem.framebuffer = framebuffer->getFrame();
  mem.programs[0].vertexShader   = vertexShaderDump0  ;
  mem.programs[0].fragmentShader = fragmentShaderEmpty;

  pushDrawCommand(cb,3);
  dumpInject.init(&mem);


  gpu_execute(mem,cb);

  auto check = checkDump(mem,cb,DumpDiff::DIFFERENT_SHADERINTERFACE);

  if(check.status == DumpDiff::SAME)return;

  std::cerr << R".(
  TEST SELHAL

  Vertex Shader by měl obdržet správný ShaderInterface.
  Shader interface obsahuje uniformní proměnné a textury.
  Obě jsou uloženy v paměti grafické karty GPUMemory.
  ).";
  REQUIRE(false);
}

void attributeTest(uint32_t offset = 0,uint32_t stride = sizeof(float)){
  std::cerr << "  (offset="<<offset<<",stride="<<stride<<")"<<std::endl;

  MEMCB();

  auto framebuffer = std::make_shared<Framebuffer>(100,100);
  std::vector<float> vert = {0.f,1.f,2.f,3.f,4.f,5.f,6.f,7.f,8.f,9.f,10.f,11.f,12.f,13.f};

  mem.framebuffer                    = framebuffer->getFrame();
  mem.buffers [0] =  vectorToBuffer(vert);
  mem.programs[0].vertexShader   = vertexShaderDump0 ;
  mem.programs[0].fragmentShader = fragmentShaderEmpty;

  VertexArray vao;
  vao.vertexAttrib[0].bufferID   = 0;
  vao.vertexAttrib[0].type       = AttributeType::FLOAT;
  vao.vertexAttrib[0].stride     = stride;
  vao.vertexAttrib[0].offset     = offset;

  auto elementSize = sizeof(decltype(vert)::value_type);
  auto offsetInElements = offset/elementSize;
  auto strideInElements = stride/elementSize;

  auto nofVerts = (vert.size()-offsetInElements) / strideInElements;
  nofVerts = (nofVerts/3)*3;

  pushDrawCommand(cb,(uint32_t)nofVerts,0,vao);

  dumpInject.init(&mem);

  gpu_execute(mem,cb);

  auto check = checkDump(mem,cb,DumpDiff::DIFFERENT_ATTRIBUTE);

  if(check.status == DumpDiff::SAME)return;

  std::cerr << R".(

  Tento test kontroluje, jestli se do points shaderu posílají správná data z points bufferu.

  Uživatel si vytvořil buffer floatů a nastavil jeden points attribut.
  Poté pustil vykreslování a očekává, že se mu ve vstupním vrcholu (InVertex) do points shaderu objeví správná data.)." << std::endl;

  std::cerr << std::endl;
  std::cerr << commandBufferDumpToStr(2,check);
  std::cerr << std::endl;

  std::cerr << gpuMemoryToStr(2,mem,cb);
  std::cerr << commandBufferToStr(2,cb);


  REQUIRE(false);
}

SCENARIO("07"){
  std::cerr << "07 - points shader, attributes" << std::endl;
  attributeTest(0*sizeof(float),1*sizeof(float));
  attributeTest(3*sizeof(float),1*sizeof(float));
  attributeTest(0*sizeof(float),4*sizeof(float));
  attributeTest(2*sizeof(float),4*sizeof(float));
}


SCENARIO("08"){
  std::cerr << "08 - points shader, attributes, offset, stride, multiple buffers" << std::endl;

  auto framebuffer = std::make_shared<Framebuffer>(100,100);

  uint8_t b0[400];
  uint8_t b1[400];

  memset(b0,0,sizeof(b0));
  memset(b1,0,sizeof(b1));

  ((float*)(b0+2))[0] = 100.f;
  ((float*)(b0+2))[1] = 101.f;
  ((float*)(b0+2))[2] = 102.f;
  ((float*)(b0+2))[3] = 103.f;
  ((float*)(b0+2))[4] = 104.f;
  ((float*)(b0+2))[5] = 105.f;
  ((float*)(b0+2))[6] = 106.f;

  ((float*)(b1+7))[ 0] = 10.f;
  ((float*)(b1+7))[ 1] = 11.f;
  ((float*)(b1+7))[ 2] = 12.f;
  ((float*)(b1+7))[ 3] = 13.f;
  ((float*)(b1+7))[ 4] =  0.f;
  ((float*)(b1+7))[ 5] = 14.f;
  ((float*)(b1+7))[ 6] = 15.f;
  ((float*)(b1+7))[ 7] = 16.f;
  ((float*)(b1+7))[ 8] = 17.f;
  ((float*)(b1+7))[ 9] =  0.f;
  ((float*)(b1+7))[10] = 18.f;
  ((float*)(b1+7))[11] = 19.f;
  ((float*)(b1+7))[12] = 20.f;
  ((float*)(b1+7))[13] = 21.f;

  MEMCB();

  mem.framebuffer = framebuffer->getFrame();
  mem.programs[0].vertexShader   = vertexShaderDump0  ;
  mem.programs[0].fragmentShader = fragmentShaderEmpty;
  mem.buffers[0].data = b0;
  mem.buffers[0].size = 400;
  mem.buffers[1].data = b1;
  mem.buffers[1].size = 400;

  VertexArray vao;
  vao.vertexAttrib[1].bufferID   = 0;
  vao.vertexAttrib[1].type       = AttributeType::VEC3;
  vao.vertexAttrib[1].stride     = sizeof(float)*2;
  vao.vertexAttrib[1].offset     = 2;

  vao.vertexAttrib[3].bufferID   = 1;
  vao.vertexAttrib[3].type       = AttributeType::VEC4;
  vao.vertexAttrib[3].stride     = sizeof(float)*5;
  vao.vertexAttrib[3].offset     = 7;

  pushDrawCommand(cb,3,0,vao);

  dumpInject.init(&mem);


  gpu_execute(mem,cb);

  auto check = checkDump(mem,cb,DumpDiff::DIFFERENT_ATTRIBUTE);

  if(check.status == DumpDiff::SAME)return;

  std::cerr << R".(

  Tento test kontroluje, jestli se do points shaderu posílají správná data z points bufferu když se použije offset a krok a vícero bufferů.

  Nastala situace, kdy má uživatel dva buffery (b0, b1), ve kterých jsou uloženy points attributy.
  V buferu b1 jsou uloženy vec3 s offsetem 2 bajty a krokem pouze 8 (takže se komponenty překrývají).
  V buffer b0 jsou uloženy vec4 s offsetem 7 bajtů a krokem 20 (takže je mezi jednotlivými vec4 mezera).
  Uživatel využil 1. a 3. attribut.)." << std::endl;

  std::cerr << std::endl;
  std::cerr << commandBufferDumpToStr(2,check);
  std::cerr << std::endl;

  std::cerr << gpuMemoryToStr(2,mem,cb);
  std::cerr << commandBufferToStr(2,cb);

  REQUIRE(false);
}


SCENARIO("09"){
  std::cerr << "09 - points shader, attributes, offset, stride, indexing" << std::endl;

  MEMCB();

  auto framebuffer = std::make_shared<Framebuffer>(100,100);

  std::vector<glm::vec2> vert = {glm::vec2(-1.f,-1.f),glm::vec2(1.f,-1.f),glm::vec2(-1.f,1.f),glm::vec2(1.f,1.f)};

  std::vector<uint16_t> indices = {3,3,3,0,1,2,2,1,3};

  mem.framebuffer = framebuffer->getFrame();
  mem.buffers[0] = vectorToBuffer(vert);
  mem.buffers[1] = vectorToBuffer(indices);
  mem.programs[0].vertexShader   = vertexShaderDump0  ;
  mem.programs[0].fragmentShader = fragmentShaderEmpty;

  VertexArray vao;
  vao.vertexAttrib[0].bufferID   = 0;
  vao.vertexAttrib[0].type       = AttributeType::VEC2;
  vao.vertexAttrib[0].stride     = sizeof(glm::vec2);
  vao.vertexAttrib[0].offset     = 0;

  vao.indexBufferID = 1;
  vao.indexType     = IndexType::UINT16;
  vao.indexOffset   = sizeof(uint16_t)*3;

  pushDrawCommand(cb,6,0,vao);

  dumpInject.init(&mem);

  gpu_execute(mem,cb);

  auto check = checkDump(mem,cb,DumpDiff::DIFFERENT_ATTRIBUTE);

  if(check.status == DumpDiff::SAME)return;

  std::cerr << R".(
  Tento test kontroluje, jestli se do points shaderu posílají správná data z points bufferu když se použije indexing.
  Uživatel si vytvořil points buffer a index buffer a chtěl vykreslit čtverec pomocí 2 trojúhelníků.)." << std::endl;

  std::cerr << std::endl;
  std::cerr << commandBufferDumpToStr(2,check);
  std::cerr << std::endl;

  std::cerr << gpuMemoryToStr(2,mem,cb);
  std::cerr << commandBufferToStr(2,cb);

  REQUIRE(false);
}

