#pragma once

#include<cstddef>
#include<string>
#include<vector>
#include<memory>
#include<sstream>

#include<glm/glm.hpp>
#include<student/fwd.hpp>

namespace tests{

float const extern floatErr;

bool equalFloats(float const& a, float const& b,float err = floatErr);
bool equalVec2(glm::vec2 const&a,glm::vec2 const&b,float err = floatErr);
bool equalVec3(glm::vec3 const&a,glm::vec3 const&b,float err = floatErr);
bool equalVec4(glm::vec4 const&a,glm::vec4 const&b,float err = floatErr);

bool equalCounts(size_t a,size_t b,size_t err = 10);

bool greaterFloat(float a,float b,float err = floatErr);

bool lessFloat(float a,float b,float err = floatErr);

template<typename TYPE>
std::string typeToStr();

template<>inline std::string typeToStr<float   >(){return "float"   ;}
template<>inline std::string typeToStr<uint32_t>(){return "uint32_t";}
template<>inline std::string typeToStr<uint16_t>(){return "uint16_t";}
template<>inline std::string typeToStr<uint8_t >(){return "uint8_t" ;}

template<typename T>
std::string str(T const&a){
  std::stringstream ss;
  ss << a;
  return ss.str();
}



template<> std::string str(bool      const&a);
template<> std::string str(uint8_t  const&a);

template<> std::string str(glm::vec2 const&a);
template<> std::string str(glm::vec3 const&a);
template<> std::string str(glm::vec4 const&a);
template<> std::string str(glm::uvec2 const&a);
template<> std::string str(glm::uvec3 const&a);
template<> std::string str(glm::uvec4 const&a);
template<> std::string str(glm::ivec2 const&a);
template<> std::string str(glm::ivec3 const&a);
template<> std::string str(glm::ivec4 const&a);

template<> std::string str(glm::mat4 const&m);
template<> std::string str(IndexType const&i);
template<> std::string str(AttributeType const&a);
template<> std::string str(CommandType const&a);
std::string padding(size_t n=2);

void printModel(size_t p,Model const&m);
void printVertexAttrib(VertexAttrib const&a,uint32_t p);

uint8_t    floatColorToByte(float c);
glm::uvec4 floatColorToBytes(glm::vec4 const&col);

glm::uvec4 getColor(Frame const&frame,glm::uvec2 const&pix);
void       writeColor(Frame&frame,glm::uvec2 const&pix,glm::uvec4 const&c);
float      getDepth(Frame const&frame,glm::uvec2 const&pix);
void       writeDepth(Frame&frame,glm::uvec2 const&pix,float d);

std::vector<InVertex>computeExpectedInVertices(GPUMemory const&mem,DrawCommand const&cmd);

bool operator==(VertexAttrib const&a,VertexAttrib const&b);
bool operator==(InVertex const&a,InVertex const&b);

struct MemCb{
  GPUMemory     mem;
  CommandBuffer cb ;
};

std::shared_ptr<MemCb>createMemCb();

#define MEMCB()                       \
  auto          memCb = createMemCb();\
  GPUMemory    &mem   = memCb->mem   ;\
  CommandBuffer&cb    = memCb->cb   

template<typename T>
Buffer vectorToBuffer(std::vector<T>const&d){
  Buffer res;
  res.data = d.data();
  res.size = sizeof(T)*d.size();
  return res;
}

struct DrawDump;
struct ClearDump;

struct CommandDump{
  CommandType type;
  CommandDump(CommandType t):type(t){}
  virtual ~CommandDump(){}
  DrawDump  const*asDraw ();
  ClearDump const*asClear();
};

struct DrawDump: public CommandDump{
  DrawDump(void*v):CommandDump(CommandType::DRAW){vs = v;}
  void*vs = nullptr;
  int32_t     programID = -1;
  VertexArray vao           ;
  std::vector<InVertex>inVertices;
  std::vector<ShaderInterface>sis;
};

struct ClearDump: public CommandDump{
  ClearDump(bool cc,bool cd,glm::uvec4 const&c,float d):CommandDump(CommandType::CLEAR){
    clearColor = cc;
    clearDepth = cd;
    color = c;
    depth = d;
  }
  bool       clearColor;
  bool       clearDepth;
  glm::uvec4 color     ;
  float      depth     ;
};

inline DrawDump  const*CommandDump::asDraw (){return dynamic_cast<DrawDump *>(this);}
inline ClearDump const*CommandDump::asClear(){return dynamic_cast<ClearDump*>(this);}

struct FragmentShaderDump{
  FragmentShaderDump();
  void*fs = nullptr;
  std::vector<InFragment>inFragments;
  Uniform unif[GPUMemory::maxUniforms];
  Texture texs[GPUMemory::maxTextures];
};

using CmdsDump = std::vector<std::shared_ptr<CommandDump>>;

struct DumpInject{
  DumpInject(){clear();}
  void init(GPUMemory*m){
    mem = m;
    clear();
    writeColor(mem->framebuffer,pix,initColor);
    writeDepth(mem->framebuffer,pix,initDepth);
  }
  CmdsDump commands;
  glm::uvec2   pix      ;
  glm::uvec4   initColor;
  float        initDepth;
  GPUMemory*   mem = nullptr;
  std::vector<OutVertex>outVertices;
  std::vector<InFragment>inFragments;
  void clear(){
    commands.clear();
    outVertices.clear();
    pix       = glm::uvec2(10,10);
    initColor = glm::uvec4(1,0,3,7);
    initDepth = 1234.56f;
  }
};

extern DumpInject dumpInject;

CmdsDump computeExpectedDump(GPUMemory const&mem,CommandBuffer const&cb);

enum class DumpDiff{
  NOF_COMMANDS_DIFFER    ,
  DIFFERENT_COMMAND_TYPES,
  DIFFERENT_CLEAR,
  DIFFERENT_PROGRAM,
  DIFFERENT_NOF_VERTICES,
  DIFFERENT_DRAWID,
  DIFFERENT_VERTEXID,
  DIFFERENT_ATTRIBUTE,
  DIFFERENT_SHADERINTERFACE,
  SAME,
};

DumpDiff isCommandBufferDumpSame(
    CmdsDump const&expected                 ,
    CmdsDump const&student                  ,
    DumpDiff const&level    = DumpDiff::SAME);

std::string commandDumpToStr(size_t p,GPUMemory const&mem);
bool hasCommandDumpRightDrawID();
std::string commandDumpDrawIDToStr(size_t p);

struct DumpCheck{
  DumpDiff status;
  CmdsDump expected;
};

std::string commandBufferDumpToStr(size_t p,DumpCheck const&check);

void vertexShaderDump0(OutVertex&,InVertex const&i,ShaderInterface const&u);
void vertexShaderDump1(OutVertex&,InVertex const&i,ShaderInterface const&u);
void vertexShaderDump2(OutVertex&,InVertex const&i,ShaderInterface const&u);
void vertexShaderDump3(OutVertex&,InVertex const&i,ShaderInterface const&u);

DumpCheck checkDump(GPUMemory const&mem,CommandBuffer const&cb,DumpDiff whatToTest);

void vertexShaderInject(OutVertex&outVertex,InVertex const&inVertex,ShaderInterface const&);
void fragmentShaderDump(OutFragment&,InFragment const&inF,ShaderInterface const&);
void vertexShaderEmpty(OutVertex&,InVertex const&inF,ShaderInterface const&);
void fragmentShaderEmpty(OutFragment&,InFragment const&inF,ShaderInterface const&);
void clearFrame(Frame&frame,glm::vec3 const&c = glm::vec3(0.f),float d=1.f);
void clearFrame(Frame&frame,glm::uvec3 const&c = glm::uvec3(0),float d=1.f);
glm::uvec3 readColor(Frame const&frame,glm::uvec2 const&coord);
float      readDepth(Frame const&frame,glm::uvec2 const&coord);
glm::uvec3 alphaMix(glm::vec4 const&frameColor,glm::vec4 const&fragColor);


bool breakTest(bool first = false);

}
