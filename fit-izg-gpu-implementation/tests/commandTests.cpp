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

using namespace tests;

SCENARIO("00"){
  std::cerr << "00 - clear commands" << std::endl;
  {
    MEMCB();

    auto pix        = glm::uvec2(10,10);
    auto clearColor = glm::vec4(1,0,0,1);
    auto clearDepth = 10e9f;
    auto initDepth  = 4.f;

    auto framebuffer = std::make_shared<Framebuffer>(100,100);

    mem.framebuffer = framebuffer->getFrame();

    writeColor(mem.framebuffer,pix,glm::uvec4(1,2,3,4));
    writeDepth(mem.framebuffer,pix,initDepth);

    pushClearCommand(cb,clearColor,clearDepth,true,false);

    gpu_execute(mem,cb);

    auto studentColor = getColor(mem.framebuffer,pix);
    auto correctColor = floatColorToBytes(clearColor);

    auto studentDepth = getDepth(mem.framebuffer,pix);
    auto correctDepth = initDepth;


    if(breakTest(true) || studentColor != correctColor || studentDepth != correctDepth){
      std::cerr << R".(
      TEST SELHAL!

      Vašim úkolem v tomto projektu je (mimo jiné) naprogramovat funkcionalitu grafické karty.
      Funkcionalita grafické karty je reprezentována funkcí:
      gpu_execute(GPUMemomry const& mem,CommnadBuffer const& cmdBuffer)
      v souboru student/gpu.cpp.
      
      Mezi funkcionality grafické karty patří kreslení nebo také čistění obrazovky.

      S realnými gpu se komunikuje skrze fronty přikazů, po kterých se posílají úlohy.
      Tyto úlohy bývají zabalené do balíčků práce (tzn. commnad buffer).
      Podobně je to například v API Vulkan:
      https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkCommandBuffer.html

      V tomto projektu je to simulováno obdobně - funkce obdrží paměť grafické karty a command buffer.
      Úkolem funkce gpu_execute je odsimulovat balíček práce nad pamětí grafické karty.

      Tento test zkouší, zda váš kód čistí framebuffer v paměti grafické karty.

      Paměť grafické karty je ve struktuře GPUMemory.

      Příkaz pro čištění framebufferu uvedený v souboru student/fwd.hpp vypadá takto:
      struct ClearCommand{
        glm::vec4   color      = glm::vec4(0);
        float       depth      = 1e10        ;
        bool        clearColor = true        ;
        bool        clearDepth = true        ;
      };

      V tomto testu vypadá command buffer takto:
      )." << std::endl;

      std::cerr << commandBufferToStr(6,cb);

      std::cerr << R".(
      Jak můžete vidět, obsahuje jen jeden příkaz - vyčištění obrazovky.

      Můžete vidět barvu, na kterou se framebuffer čistí (r,g,b,a), hloubku (depth)
      a zda je čištění framebuffer povoleno.
      Framebuffer se skládá ze dvou částí color buffer a depth buffer - paměť barvy a hloubky.
      Pokud je příznak clearColor == false, neprovede se čistění color bufferu.
      Pokud je příznak clearDepth == false, neprovede se čistění depth bufferu.

      Barvy jsou v ClearCommand uvedeny ve floatech v rozsahu: [0-1].
      Při zápisu do framebufferu je nutné je převést na uint8_t rozsahu: [0,255];

      Paměť grafické karty v tomto testu vypadá takto:
      )." << std::endl;

      std::cerr << gpuMemoryToStr(6,mem,cb);

      std::cerr << R".(

      Test selhal, protože hodnoty ve framebufferu jsou špatně.
      Vaše Barva je  : )."<<str(studentColor)<<R".(
      Vaše Hloubka je: )."<<str(studentDepth)<<R".(
      Správná barva  : )."<<str(correctColor)<<R".(
      Správná hloubka: )."<<str(correctDepth)<<R".(

      Tak správně vyčistěte framebuffer!
      Upravte funkci gpu_execute v souboru student/gpu.cpp

      Nápověda k implementaci:
      void gpu_execute(GPUMemory&mem,CommandBuffer const&cb){
        for(uint32_t i=0;i<cb.nofCommands;++i){
          if(cb.commands[i].type == CommandType::CLEAR){
            clear(mem,cb.commands[i].commandData.clearCommand);
          }
        }
      }

      ).";

      REQUIRE(false);
    }
  }

  {
    MEMCB();

    auto framebuffer = std::make_shared<Framebuffer>(100,100);
    mem.framebuffer = framebuffer->getFrame();
    auto pix        = glm::uvec2(10,10);
    auto clearColor = glm::vec4(1,1,0,1);
    auto clearDepth = 10e9f;
    auto initDepth  = 4.f;
    auto initColor  = glm::uvec4(1,2,3,4);
    writeColor(mem.framebuffer,pix,initColor);
    writeDepth(mem.framebuffer,pix,initDepth);
    pushClearCommand(cb,clearColor,clearDepth,false,true);
    gpu_execute(mem,cb);

    auto studentColor = getColor(mem.framebuffer,pix);
    auto correctColor = initColor;

    auto  studentDepth = getDepth(mem.framebuffer,pix);
    float correctDepth = clearDepth;

    if(breakTest() || studentColor != correctColor || studentDepth != correctDepth){
      std::cerr << R".(
      TEST SELHAL!

      V tomto testu se ve framebufferu čistí pouze hloubka.

      Test selhal, protože hodnoty ve framebufferu jsou špatně.
      Vaše Barva je  : )."<<str(studentColor)<<R".(
      Vaše Hloubka je: )."<<str(studentDepth)<<R".(
      Správná barva  : )."<<str(correctColor)<<R".(
      Správná hloubka: )."<<str(correctDepth)<<R".(
      ).";

      std::cerr << std::endl;
      std::cerr << commandBufferToStr(6,cb);
      std::cerr << std::endl;
      std::cerr << gpuMemoryToStr(6,mem,cb);
      
      REQUIRE(false);
    }
  }
  {
    MEMCB();

    auto framebuffer = std::make_shared<Framebuffer>(100,100);
    mem.framebuffer                = framebuffer->getFrame();
    auto pix        = glm::uvec2(10,10);
    auto clearColor = glm::vec4(1,0,1,1);
    auto clearDepth = 10e7f;
    auto initDepth  = 4.f;
    auto initColor  = glm::uvec4(1,2,3,4);
    writeColor(mem.framebuffer,pix,initColor);
    writeDepth(mem.framebuffer,pix,initDepth);
    pushClearCommand(cb,clearColor,7,true,false);
    pushClearCommand(cb,glm::vec4(4),clearDepth,false,true);
    gpu_execute(mem,cb);

    auto studentColor = getColor(mem.framebuffer,pix);
    auto correctColor = floatColorToBytes(clearColor);

    auto  studentDepth = getDepth(mem.framebuffer,pix);
    float correctDepth = clearDepth;

    if(breakTest() || studentColor != correctColor || studentDepth != correctDepth){
      std::cerr << R".(
      TEST SELHAL!

      V tomto testu obsahuje command buffer dva příkazy pro čištění framebufferu.

      Test selhal, protože hodnoty ve framebufferu jsou špatně.
      Vaše Barva je  : )."<<str(studentColor)<<R".(
      Vaše Hloubka je: )."<<str(studentDepth)<<R".(
      Správná barva  : )."<<str(correctColor)<<R".(
      Správná hloubka: )."<<str(correctDepth)<<R".(
      ).";

      std::cerr << std::endl;
      std::cerr << commandBufferToStr(6,cb);
      std::cerr << std::endl;
      std::cerr << gpuMemoryToStr(6,mem,cb);
      
      REQUIRE(false);
    }
  }

  {
    MEMCB();

    auto framebuffer = std::make_shared<Framebuffer>(100,100);
    mem.framebuffer = framebuffer->getFrame();
    auto pix        = glm::uvec2(10,10);
    auto clearColor = glm::vec4(1,0,1,1);
    auto clearDepth = 10e7f;
    auto initDepth  = 4.f;
    auto initColor  = glm::uvec4(1,2,3,4);
    writeColor(mem.framebuffer,pix,initColor);
    writeDepth(mem.framebuffer,pix,initDepth);
    pushClearCommand(cb,clearColor,clearDepth,false,false);
    pushClearCommand(cb,glm::vec4(1,1,1,1),7,true,false);
    pushClearCommand(cb,glm::vec4(0,1,1,1),2,true,true );
    pushClearCommand(cb,glm::vec4(0,0,0,1),3,true,false);
    pushClearCommand(cb,clearColor,clearDepth,true,true);
    gpu_execute(mem,cb);

    auto studentColor = getColor(mem.framebuffer,pix);
    auto correctColor = floatColorToBytes(clearColor);

    auto  studentDepth = getDepth(mem.framebuffer,pix);
    float correctDepth = clearDepth;

    if(breakTest() || studentColor != correctColor || studentDepth != correctDepth){
      std::cerr << R".(
      TEST SELHAL!

      V tomto testu obsahuje command buffer vícero čistítích příkazů.

      Test selhal, protože hodnoty ve framebufferu jsou špatně.
      Vaše Barva je  : )."<<str(studentColor)<<R".(
      Vaše Hloubka je: )."<<str(studentDepth)<<R".(
      Správná barva  : )."<<str(correctColor)<<R".(
      Správná hloubka: )."<<str(correctDepth)<<R".(
      ).";

      std::cerr << std::endl;
      std::cerr << commandBufferToStr(6,cb);
      std::cerr << std::endl;
      std::cerr << gpuMemoryToStr(6,mem,cb);
      
      REQUIRE(false);
    }
  }
}

void nofVerticesTest(uint32_t N,uint32_t prg){
  MEMCB();

  auto framebuffer = std::make_shared<Framebuffer>(100,100);

  mem.framebuffer = framebuffer->getFrame();
  mem.programs[0].vertexShader   = vertexShaderDump0  ;
  mem.programs[0].fragmentShader = fragmentShaderEmpty;
  mem.programs[1].vertexShader   = vertexShaderDump1  ;
  mem.programs[1].fragmentShader = fragmentShaderEmpty;
  mem.programs[2].vertexShader   = vertexShaderDump2  ;
  mem.programs[2].fragmentShader = fragmentShaderEmpty;
  mem.programs[3].vertexShader   = vertexShaderDump3  ;
  mem.programs[3].fragmentShader = fragmentShaderEmpty;

  pushDrawCommand(cb,N,prg);

  dumpInject.init(&mem);

  gpu_execute(mem,cb);

  auto check = checkDump(mem,cb,DumpDiff::DIFFERENT_NOF_VERTICES);

  if(check.status == DumpDiff::SAME)return;

  std::cerr << R".(
  TEST SELHAL!

  Tento test počítá, kolikrát jste pustili points shader.

  gpu_execute umožňuje také kreslit.
  V takovém případě obsahuje command buffer kreslící příkazy.

  Příkaz pro kreslení je uvedený v souboru student/fwd.hpp vypadá takto:
  struct DrawCommand{
    int32_t     programID       = -1   ;
    uint32_t    nofVertices     = 0    ;
    bool        backfaceCulling = false;
    VertexArray vao                    ;
  };

  V tomto testu je důležitá komponenta programID, což je číslo programu
  v paměti grafické karty, který by se měl volat.

  Dopadlo to takto:
  )." << std::endl;

  std::cerr << std::endl;
  std::cerr << commandBufferDumpToStr(6,check);
  std::cerr << std::endl;

  std::cerr << "      command buffer vypadá takto:" << std::endl;

  std::cerr << commandBufferToStr(6,cb);

  std::cerr<<R".(

  Zavolejte správný points shader správný počet krát!

  Nápověda:
  void gpu_execute(GPUMemory&mem,CommandBuffer const&cb){
    for(uint32_t i=0;i<cb.nofCommands;++i){
      if(cb.commands[i].type == CommandType::DRAW){
        DrawCommand cmd = cb.commands[i].commandData.drawCommand;
        VertexShader vs = mem.programs[cmd.programID].vertexShader;
        for(uint32_t n=0;n<cmd.nofVertices;++n){
          InVertex inVertex;
          OuntVertex outVertex;
          ShaderInterface si;
          vs(outVertex,inVertex,si);
        }
      }
    }
  }

  ).";
  REQUIRE(false);
}

SCENARIO("01"){
  std::cerr << "01 - draw commands" << std::endl;
  nofVerticesTest(3,2);
  nofVerticesTest(6,0);
  nofVerticesTest(9,1);
  nofVerticesTest(1800,0);
}

SCENARIO("02"){
  std::cerr << "02 - mixed clear and draw commands" << std::endl;

  MEMCB();
  auto framebuffer = std::make_shared<Framebuffer>(100,100);

  mem.framebuffer = framebuffer->getFrame();
  mem.programs[0].vertexShader   = vertexShaderDump0  ;
  mem.programs[0].fragmentShader = fragmentShaderEmpty;
  mem.programs[1].vertexShader   = vertexShaderDump1  ;
  mem.programs[1].fragmentShader = fragmentShaderEmpty;
  mem.programs[2].vertexShader   = vertexShaderDump2  ;
  mem.programs[2].fragmentShader = fragmentShaderEmpty;
  mem.programs[3].vertexShader   = vertexShaderDump3  ;
  mem.programs[3].fragmentShader = fragmentShaderEmpty;

  pushClearCommand(cb,glm::vec4(.1,.1,.1,1),3,true,true);
  pushDrawCommand(cb,3,0);
  pushClearCommand(cb,glm::vec4(.1,.1,.1,1),4,false,true);
  pushDrawCommand(cb,6,1);
  pushDrawCommand(cb,9,2);
  pushClearCommand(cb,glm::vec4(.1,.1,.1,1),5,false,true);
  pushDrawCommand(cb,12,3);

  dumpInject.init(&mem);

  gpu_execute(mem,cb);

  auto check = checkDump(mem,cb,DumpDiff::DIFFERENT_CLEAR);

  if(check.status == DumpDiff::SAME)return;

  std::cerr << R".(
  TEST SELHAL!

  Tento test zkouší command buffer s několik promíchanými příkazy.

  CommandBuffer vypadá takto:
  )." << std::endl;

  std::cerr << std::endl;
  std::cerr << commandBufferDumpToStr(2,check);
  std::cerr << std::endl;

  std::cerr<<R".(

  Správně zpracovávejte příkazy v command bufferu!
  ).";
  REQUIRE(false);
}

SCENARIO("03"){
  std::cerr << "03 - drawID" << std::endl;

  MEMCB();

  auto framebuffer = std::make_shared<Framebuffer>(100,100);

  mem.framebuffer = framebuffer->getFrame();
  mem.programs[0].vertexShader   = vertexShaderDump0  ;
  mem.programs[0].fragmentShader = fragmentShaderEmpty;
  mem.programs[1].vertexShader   = vertexShaderDump1  ;
  mem.programs[1].fragmentShader = fragmentShaderEmpty;

  pushClearCommand(cb,glm::vec4(.1,.1,.1,1),3,true,true);
  pushDrawCommand(cb,3,0);
  pushClearCommand(cb,glm::vec4(.1,.1,.1,1),3,true,true);
  pushDrawCommand(cb,3,1);
  pushDrawCommand(cb,3,0);
  pushDrawCommand(cb,3,1);
  pushDrawCommand(cb,3,0);
  pushClearCommand(cb,glm::vec4(.1,.1,.1,1),5,false,true);
  pushDrawCommand(cb,3,1);
  pushDrawCommand(cb,3,0);

  dumpInject.init(&mem);

  gpu_execute(mem,cb);

  auto check = checkDump(mem,cb,DumpDiff::DIFFERENT_DRAWID);

  if(check.status == DumpDiff::SAME)return;

  std::cerr << R".(
  TEST SELHAL!

  Tento test zkouší, zda kreslící příkazy mají správné gl_DrawID.
  Všechny kreslící příkazy v command bufferu by měly být číslovány počadovým číslem.
  Číslují se pouze kreslící příkazy.
  Pokud jsou v command bufferu například 4 kreslící příkazy proloženy
  jinými příkazy, budou mít tyto kreslící příkazy číslo 0,1,2,3.

  gl_DrawID je ve struktuře InVertex jejíž instance vstupuje do points shaderu.
  Podle gl_DrawID se obvykle vybírá nastavení pro danou část modelu (například textury, materiály).

  Takto to dopadlo:
  )." << std::endl;

  std::cerr << std::endl;
  std::cerr << commandBufferDumpToStr(2,check);
  std::cerr << std::endl;

  std::cerr<<R".(

  Správně číslujte kreslící příkazy!
  ).";
  REQUIRE(false);
}

