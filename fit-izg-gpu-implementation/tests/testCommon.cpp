#include <memory>
#include <framework/programContext.hpp>
#include <tests/testCommon.hpp>

#include <glm/glm.hpp>
#include <sstream>
#include <iostream>

void drawModel_vertexShader  (OutVertex  &,InVertex   const&,ShaderInterface const&);
void drawModel_fragmentShader(OutFragment&,InFragment const&,ShaderInterface const&);

namespace tests{

float const floatErr = 0.001f;

bool equalFloats(float const& a, float const& b,float err) {
  return glm::abs(a - b) <= err;
}

bool equalVec4(glm::vec4 const&a,glm::vec4 const&b,float err){
  return
    equalFloats(a.x,b.x,err) &&
    equalFloats(a.y,b.y,err) &&
    equalFloats(a.z,b.z,err) &&
    equalFloats(a.w,b.w,err) ;
}

bool equalVec3(glm::vec3 const&a,glm::vec3 const&b,float err){
  return
    equalFloats(a.x,b.x,err) &&
    equalFloats(a.y,b.y,err) &&
    equalFloats(a.z,b.z,err) ;
}

bool equalVec2(glm::vec2 const&a,glm::vec2 const&b,float err){
  return
    equalFloats(a.x,b.x,err) &&
    equalFloats(a.y,b.y,err);
}

bool equalCounts(size_t a,size_t b,size_t err){
  if(a<b)return (b-a)<err;
  return (a-b)<err;
}

bool greaterFloat(float a,float b,float err){
  return a>b-err;
}

bool lessFloat(float a,float b,float err){
  return a<b+err;
}

std::string str(bool v){
  if(v)return "true" ;
  else return "false";
}

template<>std::string str(uint8_t     const&a){
  std::stringstream ss;
  ss << (uint32_t)a;
  return ss.str();
}

template<>std::string str(glm::vec2 const&a){
  std::stringstream ss;
  ss << "(" << a.x << "," << a.y << ")";
  return ss.str();
}

template<>std::string str(glm::vec3 const&a){
  std::stringstream ss;
  ss << "(" << a.x << "," << a.y << "," << a.z << ")";
  return ss.str();
}

template<>std::string str(glm::vec4 const&a){
  std::stringstream ss;
  ss << "(" << a.x << "," << a.y << "," << a.z << "," << a.w << ")";
  return ss.str();
}

template<>std::string str(glm::uvec2 const&a){
  std::stringstream ss;
  ss << "(" << a.x << "," << a.y << ")";
  return ss.str();
}

template<>std::string str(glm::uvec3 const&a){
  std::stringstream ss;
  ss << "(" << a.x << "," << a.y << "," << a.z << ")";
  return ss.str();
}

template<>std::string str(glm::uvec4 const&a){
  std::stringstream ss;
  ss << "(" << a.x << "," << a.y << "," << a.z << "," << a.w << ")";
  return ss.str();
}

template<>std::string str(glm::ivec2 const&a){
  std::stringstream ss;
  ss << "(" << a.x << "," << a.y << ")";
  return ss.str();
}

template<>std::string str(glm::ivec3 const&a){
  std::stringstream ss;
  ss << "(" << a.x << "," << a.y << "," << a.z << ")";
  return ss.str();
}

template<>std::string str(glm::ivec4 const&a){
  std::stringstream ss;
  ss << "(" << a.x << "," << a.y << "," << a.z << "," << a.w << ")";
  return ss.str();
}

template<>std::string str(glm::mat4 const&m){
  std::stringstream ss;
  ss << "(";
  for(int i=0;i<16;++i){
    if(i>0)ss<<",";
    ss << m[i/4][i%4];
  }
  ss << ")";
  return ss.str();
}

template<>std::string str(IndexType const&i){
  if(i==IndexType::UINT8 )return "IndexType::UINT8" ;
  if(i==IndexType::UINT16)return "IndexType::UINT16";
  if(i==IndexType::UINT32)return "IndexType::UINT32";
  return "unknown";
}

template<>std::string str(AttributeType const&a){
  switch(a){
    case AttributeType::EMPTY:return "AttributeType::EMPTY";
    case AttributeType::FLOAT:return "AttributeType::FLOAT";
    case AttributeType::VEC2 :return "AttributeType::VEC2" ;
    case AttributeType::VEC3 :return "AttributeType::VEC3" ;
    case AttributeType::VEC4 :return "AttributeType::VEC4" ;
    case AttributeType::UINT :return "AttributeType::UINT" ;
    case AttributeType::UVEC2:return "AttributeType::UVEC2";
    case AttributeType::UVEC3:return "AttributeType::UVEC3";
    case AttributeType::UVEC4:return "AttributeType::UVEC4";
    default: return "unknown";
  }
}
template<> std::string str(CommandType const&a){
  switch(a){
    case CommandType::CLEAR:return "clear";
    case CommandType::DRAW :return "draw" ;
    default:return "unknown";
  }
}

template<>std::string str(bool const&a){
  if(a)return "true";
  return "false";
}

void createDrawCMD(
    CommandBuffer&cb,
    int32_t        prg,
    uint32_t       n
    ){
  auto&a=cb.commands[cb.nofCommands];
  cb.nofCommands++;
  a.type = CommandType::DRAW;
  auto&cmd = a.data.drawCommand;

  cmd.programID   = prg;
  cmd.nofVertices = n  ;
}


uint8_t floatColorToByte(float c){
  return (uint8_t)(c*255.f);
}

glm::uvec4 getColor(Frame const&frame,glm::uvec2 const&pix){
  auto const colorStart = (pix.y*frame.width+pix.x)*frame.channels;
  auto res = glm::uvec4(0);
  for(uint32_t i=0;i<frame.channels;++i)
    res[i] = frame.color[colorStart+i];
  return res;
}

void writeColor(Frame&frame,glm::uvec2 const&pix,glm::uvec4 const&c){
  auto const colorStart = (pix.y*frame.width+pix.x)*frame.channels;
  for(uint32_t i=0;i<frame.channels;++i)
    frame.color[colorStart+i] = c[i];
}

float getDepth(Frame const&frame,glm::uvec2 const&pix){
  return frame.depth[pix.y*frame.width+pix.x];
}

void  writeDepth(Frame&frame,glm::uvec2 const&pix,float d){
  frame.depth[pix.y*frame.width+pix.x] = d;
}

glm::uvec4 floatColorToBytes(glm::vec4 const&col){
  auto res = glm::uvec4(0);
  for(uint32_t i=0;i<4;++i)
    res[i] = floatColorToByte(col[i]);
  return res;
}

struct DumpInject dumpInject;

std::shared_ptr<MemCb>createMemCb(){
  return std::make_shared<MemCb>();
}

bool operator==(Attribute const&a,Attribute const&b){
  return a.v4 == b.v4;
}

bool operator==(InVertex const&a,InVertex const&b){
  for(size_t i=0;i<maxAttributes;++i)
    if(!(a.attributes[i] == b.attributes[i]))return false;
  return a.gl_VertexID == b.gl_VertexID;
}

std::vector<InVertex>computeExpectedInVertices(GPUMemory const&mem,DrawCommand const&cmd,uint32_t drawID){
  std::vector<InVertex>result;
  auto const&vao = cmd.vao;
  for(uint32_t i=0;i<cmd.nofVertices;++i){
    InVertex inV;
    OutVertex outV;
    if(vao.indexBufferID>=0){
      auto ind = (uint8_t*)mem.buffers[vao.indexBufferID].data+vao.indexOffset;
      if(vao.indexType == IndexType::UINT8 )inV.gl_VertexID = ((uint8_t *)ind)[i];
      if(vao.indexType == IndexType::UINT16)inV.gl_VertexID = ((uint16_t*)ind)[i];
      if(vao.indexType == IndexType::UINT32)inV.gl_VertexID = ((uint32_t*)ind)[i];
    }else{
      inV.gl_VertexID = i;
      inV.gl_DrawID   = drawID;
    }
    for(uint32_t a=0;a<maxAttributes;++a){
      auto const&at = cmd.vao.vertexAttrib[a];
      if(at.bufferID<0)continue;
      auto buf = mem.buffers[at.bufferID].data;
      if(at.type == AttributeType::FLOAT)inV.attributes[a].v1 = *(float    *)(((uint8_t*)buf)+at.stride*inV.gl_VertexID+at.offset);
      if(at.type == AttributeType::VEC2 )inV.attributes[a].v2 = *(glm::vec2*)(((uint8_t*)buf)+at.stride*inV.gl_VertexID+at.offset);
      if(at.type == AttributeType::VEC3 )inV.attributes[a].v3 = *(glm::vec3*)(((uint8_t*)buf)+at.stride*inV.gl_VertexID+at.offset);
      if(at.type == AttributeType::VEC4 )inV.attributes[a].v4 = *(glm::vec4*)(((uint8_t*)buf)+at.stride*inV.gl_VertexID+at.offset);
    }
    result.push_back(inV);
  }
  return result;
}

std::vector<std::shared_ptr<CommandDump>>computeExpectedDump(GPUMemory const&mem,CommandBuffer const&cb){
  std::vector<std::shared_ptr<CommandDump>>res;
  uint32_t drawID = 0;
  for(uint32_t c=0;c<cb.nofCommands;++c){
    auto const&cmd = cb.commands[c];
    if(cmd.type == CommandType::CLEAR){
      auto const&cc = cmd.data.clearCommand;
      res.push_back(std::make_shared<ClearDump>(
            cc.clearColor,
            cc.clearDepth,
            floatColorToBytes(cc.color),
            cc.depth));
    }
    if(cmd.type == CommandType::DRAW){
      auto const&dc = cmd.data.drawCommand;
      auto dump = std::make_shared<DrawDump>((void*)mem.programs[dc.programID].vertexShader);
      dump->inVertices = computeExpectedInVertices(mem,dc,drawID++);
      dump->vao       = dc.vao      ;
      dump->programID = dc.programID;
      res.push_back(dump);
    }
  }

  return res;
}

DumpDiff filterErrorLevel(DumpDiff status,DumpDiff maxLevel){
  if(status <= maxLevel)return status;
  return DumpDiff::SAME;
}

DumpDiff isCommandBufferDumpSame(
    std::vector<std::shared_ptr<CommandDump>>const&expected,
    std::vector<std::shared_ptr<CommandDump>>const&student ,
    DumpDiff                                 const&level   
    ){
  if(expected.size() != student.size())return filterErrorLevel(DumpDiff::NOF_COMMANDS_DIFFER,level);

  for(uint32_t i=0;i<expected.size();++i){
    if(expected.at(i)->type != student.at(i)->type)return filterErrorLevel(DumpDiff::DIFFERENT_COMMAND_TYPES,level);
    if(expected.at(i)->type == CommandType::CLEAR){
      auto const& ecmd = expected.at(i)->asClear();
      auto const& scmd = student .at(i)->asClear();
      if(ecmd->clearColor != scmd->clearColor)return filterErrorLevel(DumpDiff::DIFFERENT_CLEAR,level);
      if(ecmd->clearDepth != scmd->clearDepth)return filterErrorLevel(DumpDiff::DIFFERENT_CLEAR,level);
      if(ecmd->clearColor){
        if(ecmd->color != scmd->color)return filterErrorLevel(DumpDiff::DIFFERENT_CLEAR,level);
      }
      if(ecmd->clearDepth){
        if(ecmd->depth != scmd->depth)return filterErrorLevel(DumpDiff::DIFFERENT_CLEAR,level);
      }
    }
    if(expected.at(i)->type == CommandType::DRAW){
      auto const& ecmd = expected.at(i)->asDraw();
      auto const& scmd = student .at(i)->asDraw();
      if(ecmd->vs != scmd->vs)return filterErrorLevel(DumpDiff::DIFFERENT_PROGRAM,level);

      auto const& eIns = ecmd->inVertices;
      auto const& sIns = scmd->inVertices;

      if(eIns.size() != sIns.size())return filterErrorLevel(DumpDiff::DIFFERENT_NOF_VERTICES,level);

      for(size_t v=0;v<eIns.size();++v){
        auto const&eIn = eIns.at(v);
        auto const&sIn = sIns.at(v);
        if(eIn.gl_DrawID != sIn.gl_DrawID)return filterErrorLevel(DumpDiff::DIFFERENT_DRAWID,level);
        if(eIn.gl_VertexID != sIn.gl_VertexID)return filterErrorLevel(DumpDiff::DIFFERENT_VERTEXID,level);
        for(size_t a=0;a<maxAttributes;++a)
          if(eIn.attributes[a].v4 != sIn.attributes[a].v4)return filterErrorLevel(DumpDiff::DIFFERENT_ATTRIBUTE,level);
      }

      auto const& eSIs = ecmd->sis;
      auto const& sSIs = scmd->sis;

      if(eSIs.size() != sSIs.size()) filterErrorLevel(DumpDiff::DIFFERENT_SHADERINTERFACE,level);
      for(size_t j=0;j<eSIs.size();++i){
        if(eSIs.at(j).textures != sSIs.at(j).textures)return filterErrorLevel(DumpDiff::DIFFERENT_SHADERINTERFACE,level);;
        if(eSIs.at(j).uniforms != sSIs.at(j).uniforms)return filterErrorLevel(DumpDiff::DIFFERENT_SHADERINTERFACE,level);;
      }


    }
  }

  return DumpDiff::SAME;
}

std::string clearCommandToStr(size_t p,ClearDump const*cc){
  std::stringstream ss;
  ss << padding(p);
  if(cc->clearColor){
    if(cc->clearDepth){
      ss << "clearColorAndDepth(uvec4"<<str(cc->color)<<","<<cc->depth<<")";
    }else{
      ss << "clearColor(uvec4"<<str(cc->color)<<")";
    }
  }else{
    ss << "clearDepth("<<cc->depth<<")";
  }
  ss << std::endl;
  return ss.str();
}

int32_t findProgramID(GPUMemory const&mem,void*ptr){
  for(uint32_t i=0;i<mem.maxPrograms;++i)
    if((void*)mem.programs[i].vertexShader == ptr)return i;
  return -1;
}

std::string drawCommandToStr(size_t p,GPUMemory const&mem,DrawDump const*cc){
  std::stringstream ss;
  ss << padding(p) << "draw(nofVertices=";
  ss << cc->inVertices.size();
  ss << ",program=" << findProgramID(mem,cc->vs);
  ss << ")";
  ss << std::endl;
  return ss.str();
}

std::string commandDumpToStr(size_t p,GPUMemory const&mem){
  std::stringstream ss;
  ss << padding(p) << "počet příkazů: " << dumpInject.commands.size() << std::endl;
  for(auto const&c:dumpInject.commands){
    if(c->type == CommandType::CLEAR)ss << clearCommandToStr(p,c->asClear());
    if(c->type == CommandType::DRAW )ss <<  drawCommandToStr(p,mem,c->asDraw ());
  }
  return ss.str();
}

std::string drawCommandToStr(size_t p,DrawDump const*cc){
  std::stringstream ss;
  ss << padding(p) << "program    : " << cc->vs        << std::endl;
  ss << padding(p) << "programID  : " << cc->programID << std::endl;
  ss << padding(p) << "nofVertices: " << cc->inVertices.size() << std::endl;
  auto const& verts = cc->inVertices;
  for(size_t i=0;i<verts.size();++i){
    auto const& v = verts.at(i);
    ss << padding(p+2) << "points" << i << ":" << std::endl;
    ss << padding(p+4) << "gl_VertexID: " << v.gl_VertexID << std::endl;
    ss << padding(p+4) << "gl_DrawID  : " << v.gl_DrawID   << std::endl;
    for(size_t a=0;a<maxAttributes;++a){
      auto const& type = cc->vao.vertexAttrib[a].type;
      auto const& att  = v.attributes[a];
      if(type == AttributeType::EMPTY)continue;
      ss << padding(p+6) << "attribute: ";
      if(type == AttributeType::FLOAT)ss << str(att.v1) << std::endl;
      if(type == AttributeType::VEC2 )ss << str(att.v2) << std::endl;
      if(type == AttributeType::VEC3 )ss << str(att.v3) << std::endl;
      if(type == AttributeType::VEC4 )ss << str(att.v4) << std::endl;
      if(type == AttributeType::UINT )ss << str(att.u1) << std::endl;
      if(type == AttributeType::UVEC2)ss << str(att.u2) << std::endl;
      if(type == AttributeType::UVEC3)ss << str(att.u3) << std::endl;
      if(type == AttributeType::UVEC4)ss << str(att.u4) << std::endl;
    }
  }
  return ss.str();
}

std::string nofCommandDifferStr(
    size_t p,
    CmdsDump const&expected,
    CmdsDump const&student ){
  std::stringstream ss;
  ss << padding(p) << "Liší se počet zpracovaných příkazů." << std::endl;
  ss << padding(p) << "Očekávaný počet: " << expected.size() << std::endl;
  ss << padding(p) << "Váš počet      : " << student .size() << std::endl;
  return ss.str();
}

std::string listCommandTypes(
    size_t p,
    CmdsDump const&dump
    ){
  std::stringstream ss;
  for(auto const&c:dump){
    ss << padding(p);
    if(c->type == CommandType::CLEAR) ss << "clear";
    if(c->type == CommandType::DRAW ) ss << "draw" ;
    ss << std::endl;
  }
  return ss.str();
}

std::string differentCommandsStr(
    size_t p,
    CmdsDump const&expected,
    CmdsDump const&student ){
  std::stringstream ss;
  ss << padding(p) << "Liší se typ zpracovaných příkazů." << std::endl;
  ss << padding(p) << "Očekáváné příkazy:" << std::endl;
  ss << listCommandTypes(p+2,expected);
  ss << padding(p) << "Váše příkazy:" << std::endl;
  ss << listCommandTypes(p+2,student );
  return ss.str();
}


std::string listClearStr(
    size_t p,
    CmdsDump const&dump){
  std::stringstream ss;

  for(auto const&c:dump){
    if(c->type != CommandType::CLEAR)continue;
    ss << clearCommandToStr(p,c->asClear());
  }
  return ss.str();
}

std::string differentClearStr(
    size_t p,
    CmdsDump const&expected,
    CmdsDump const&student ){
  std::stringstream ss;
  ss << padding(p) << "Liší se čistící příkazy." << std::endl;
  ss << padding(p) << "Očekáváné čistící příkazy:" << std::endl;
  ss << listCommandTypes(p+2,expected);
  ss << padding(p) << "Váše příkazy:" << std::endl;
  ss << listCommandTypes(p+2,student );
  return ss.str();
}

std::string listPrograms(
    size_t p,
    CmdsDump const&dump){
  std::stringstream ss;

  for(auto const&c:dump){
    if(c->type != CommandType::DRAW)continue;
    ss << padding(p) << c->asDraw()->programID << std::endl;
  }
  return ss.str();
}

std::string differentProgramStr(
    size_t p,
    CmdsDump const&expected,
    CmdsDump const&student ){
  std::stringstream ss;
  ss << padding(p) << "Liší se program vykreslovacího příkazu." << std::endl;
  ss << padding(p) << "Očekáváné programy:" << std::endl;
  ss << listPrograms(p+2,expected);
  ss << padding(p) << "Váše programy:" << std::endl;
  ss << listPrograms(p+2,student );
  return ss.str();
}

std::string listNofVertices(
    size_t p,
    CmdsDump const&dump){
  std::stringstream ss;

  for(auto const&c:dump){
    if(c->type != CommandType::DRAW)continue;
    ss << padding(p) << c->asDraw()->inVertices.size() << std::endl;
  }
  return ss.str();
}

std::string differentNofVerticesStr(
    size_t p,
    CmdsDump const&expected,
    CmdsDump const&student ){
  std::stringstream ss;
  ss << padding(p) << "Liší se počet spuštění points shaderu." << std::endl;
  ss << padding(p) << "Očekáváný počet spuštění:" << std::endl;
  ss << listNofVertices(p+2,expected);
  ss << padding(p) << "Váš počet spuštění:" << std::endl;
  ss << listNofVertices(p+2,student );
  return ss.str();
}

std::string listDrawID(
    size_t p,
    CmdsDump const&dump){
  std::stringstream ss;

  uint32_t counter = 0;
  for(auto const&c:dump){
    if(c->type != CommandType::DRAW)continue;
    ss << padding(p) << "draw" << counter++ << " ";
    for(auto const&v:c->asDraw()->inVertices)
      ss << v.gl_DrawID << ", ";
    ss << std::endl;
  }
  return ss.str();
}

std::string differentDrawIDStr(
    size_t p,
    CmdsDump const&expected,
    CmdsDump const&student ){
  std::stringstream ss;
  ss << padding(p) << "Liší se gl_DrawID." << std::endl;
  ss << padding(p) << "Očekávané hodnoty gl_DrawID:" << std::endl;
  ss << listDrawID(p+2,expected);
  ss << padding(p) << "Vaše hodnoty gl_DrawID:" << std::endl;
  ss << listDrawID(p+2,student );
  return ss.str();
}

std::string listVertexID(
    size_t p,
    CmdsDump const&dump){
  std::stringstream ss;

  uint32_t counter = 0;
  for(auto const&c:dump){
    if(c->type != CommandType::DRAW)continue;
    ss << padding(p) << "drawCommand" << counter++ << ": ";
    for(auto const&v:c->asDraw()->inVertices)
      ss << v.gl_VertexID << ", ";
    ss << std::endl;
  }
  return ss.str();
}


std::string differentVertexIDStr(
    size_t p,
    CmdsDump const&expected,
    CmdsDump const&student ){
  std::stringstream ss;
  ss << padding(p) << "Liší se gl_VertexID." << std::endl;
  ss << padding(p) << "Očekávané hodnoty gl_VertexID:" << std::endl;
  ss << listVertexID(p+2,expected);
  ss << padding(p) << "Vaše hodnoty gl_VertexID:" << std::endl;
  ss << listVertexID(p+2,student );
  return ss.str();
}

std::string listAttributes(
    size_t p,
    CmdsDump const&dump    ,
    CmdsDump const&expected){
  std::stringstream ss;

  uint32_t counter = 0;
  for(size_t i=0;i<dump.size();++i){
    auto const&c = dump    .at(i);
    auto const&e = expected.at(i);
    if(c->type != CommandType::DRAW)continue;
    ss << padding(p) << "draw" << counter++ << std::endl;
    for(uint32_t a=0;a<maxAttributes;++a){
      auto const&type = e->asDraw()->vao.vertexAttrib[a].type;
      if(type == AttributeType::EMPTY)continue;
      ss << padding(p+2) << "attrib" << a << ": ";
      for(auto const&v:c->asDraw()->inVertices){
        auto const&att = v.attributes[a];
        if(type == AttributeType::FLOAT)ss << str(att.v1) << ", ";
        if(type == AttributeType::VEC2 )ss << str(att.v2) << ", ";
        if(type == AttributeType::VEC3 )ss << str(att.v3) << ", ";
        if(type == AttributeType::VEC4 )ss << str(att.v4) << ", ";
        if(type == AttributeType::UINT )ss << str(att.u1) << ", ";
        if(type == AttributeType::UVEC2)ss << str(att.u2) << ", ";
        if(type == AttributeType::UVEC3)ss << str(att.u3) << ", ";
        if(type == AttributeType::UVEC4)ss << str(att.u4) << ", ";
      }
      ss << std::endl;
    }
    ss << std::endl;
  }
  return ss.str();
}

std::string differentAttributeStr(
    size_t p,
    CmdsDump const&expected,
    CmdsDump const&student ){
  std::stringstream ss;
  ss << padding(p) << "Liší se attributy vrcholů." << std::endl;
  ss << padding(p) << "Očekávané attributy:" << std::endl;
  ss << listAttributes(p+2,expected,expected);
  ss << padding(p) << "Vaše hodnoty attributů:" << std::endl;
  ss << listAttributes(p+2,student ,expected);
  return ss.str();
}


std::string commandBufferDumpToStr(
    size_t p,
    DumpCheck const&check){
  auto const&expected = check.expected;
  auto const&student  = dumpInject.commands;
  switch(check.status){
    case DumpDiff::NOF_COMMANDS_DIFFER    :return nofCommandDifferStr     (p,expected,student);
    case DumpDiff::DIFFERENT_COMMAND_TYPES:return differentCommandsStr    (p,expected,student);
    case DumpDiff::DIFFERENT_CLEAR        :return differentClearStr       (p,expected,student);
    case DumpDiff::DIFFERENT_PROGRAM      :return differentProgramStr     (p,expected,student);
    case DumpDiff::DIFFERENT_NOF_VERTICES :return differentNofVerticesStr (p,expected,student);
    case DumpDiff::DIFFERENT_DRAWID       :return differentDrawIDStr      (p,expected,student);
    case DumpDiff::DIFFERENT_VERTEXID     :return differentVertexIDStr    (p,expected,student);
    case DumpDiff::DIFFERENT_ATTRIBUTE    :return differentAttributeStr   (p,expected,student);

    default:return padding(p) + "Nemáte problém.";
  }
  return "";
}


bool hasCommandDumpRightDrawID(){
  uint32_t counter=0;
  for(auto const&c:dumpInject.commands)
    if(c->type == CommandType::DRAW){
      if(c->asDraw()->inVertices.at(0).gl_DrawID != counter)return false;
      counter++;
    }
  return true;
}

std::string commandDumpDrawIDToStr(size_t p){
  std::stringstream ss;
  for(auto const&c:dumpInject.commands)
    if(c->type == CommandType::DRAW)
      ss << padding(p) << c->asDraw()->inVertices.at(0).gl_DrawID << std::endl;
  return ss.str();
}

void vertexShaderDumpGeneral(InVertex const&i,ShaderInterface const&si,void*vs){
  auto color = getColor(dumpInject.mem->framebuffer,dumpInject.pix);
  auto depth = getDepth(dumpInject.mem->framebuffer,dumpInject.pix);
  if(color != dumpInject.initColor){
    if(depth != dumpInject.initDepth){//color & depth
      dumpInject.commands.push_back(std::make_shared<ClearDump>(true,true,color,depth));
    }else{//only color changed
      dumpInject.commands.push_back(std::make_shared<ClearDump>(true,false,color,depth));
    }
  }else{
    if(depth != dumpInject.initDepth){//depth
      dumpInject.commands.push_back(std::make_shared<ClearDump>(false,true,color,depth));
    }
  }
  writeColor(dumpInject.mem->framebuffer,dumpInject.pix,dumpInject.initColor);
  writeDepth(dumpInject.mem->framebuffer,dumpInject.pix,dumpInject.initDepth);

  if(
      !dumpInject.commands.empty() && 
      dumpInject.commands.back()->type == CommandType::DRAW &&
      std::dynamic_pointer_cast<DrawDump>(dumpInject.commands.back())->vs == vs){
    auto&last = dumpInject.commands.back();
    auto draw = std::dynamic_pointer_cast<DrawDump>(last);
    draw->sis.push_back(si);
    draw->inVertices.push_back(i);
  }else{
    auto drawDump = std::make_shared<DrawDump>(vs);
    drawDump->sis.push_back(si);
    drawDump->inVertices.push_back(i);
    drawDump->programID = findProgramID(*dumpInject.mem,vs);
    dumpInject.commands.push_back(drawDump);
  }
}

void vertexShaderDump0(OutVertex&,InVertex const&i,ShaderInterface const&si){
  vertexShaderDumpGeneral(i,si,(void*)vertexShaderDump0);
}
void vertexShaderDump1(OutVertex&,InVertex const&i,ShaderInterface const&si){
  vertexShaderDumpGeneral(i,si,(void*)vertexShaderDump1);
}
void vertexShaderDump2(OutVertex&,InVertex const&i,ShaderInterface const&si){
  vertexShaderDumpGeneral(i,si,(void*)vertexShaderDump2);
}
void vertexShaderDump3(OutVertex&,InVertex const&i,ShaderInterface const&si){
  vertexShaderDumpGeneral(i,si,(void*)vertexShaderDump3);
}

DumpCheck checkDump(GPUMemory const&mem,CommandBuffer const&cb,DumpDiff whatToTest){
  DumpCheck res;
  res.expected = computeExpectedDump(mem,cb);
  res.status = isCommandBufferDumpSame(res.expected,dumpInject.commands,whatToTest);
  if(breakTest())res.status = whatToTest;
  return res;
}

void vertexShaderInject(OutVertex&outVertex,InVertex const&inVertex,ShaderInterface const&){
  if(inVertex.gl_VertexID<dumpInject.outVertices.size())
    outVertex = dumpInject.outVertices.at(inVertex.gl_VertexID);
}

Uniform unif[GPUMemory::maxUniforms];
Texture texs[GPUMemory::maxTextures];

void fragmentShaderDump(OutFragment&,InFragment const&inF,ShaderInterface const&){
  dumpInject.inFragments.push_back(inF);
}

void vertexShaderEmpty(OutVertex&,InVertex const&,ShaderInterface const&){
}
void fragmentShaderEmpty(OutFragment&,InFragment const&,ShaderInterface const&){
}


void clearFrame(Frame&frame,glm::uvec3 const&color,float d){
  for(uint32_t y=0;y<frame.height;++y)
    for(uint32_t x=0;x<frame.width;++x){
      auto pix = y*frame.width+x;
      for(uint32_t c=0;c<3;++c)
        frame.color[pix*4+c] = color[c];
      frame.color[pix*4+3] = 0;
      frame.depth[pix] = d;
    }
}

void clearFrame(Frame&frame,glm:: vec3 const&c,float d){
  clearFrame(frame,glm::uvec3(glm::clamp(c*255.f,glm::vec3(0.f),glm::vec3(255.))),d);
}

glm::uvec3 readColor(Frame const&frame,glm::uvec2 const&coord){
  auto pix = coord.y*frame.width + coord.x;
  return glm::uvec3(frame.color[pix*4+0],frame.color[pix*4+1],frame.color[pix*4+2]);
}

float      readDepth(Frame const&frame,glm::uvec2 const&coord){
  auto pix = coord.y*frame.width + coord.x;
  return frame.depth[pix];
}

glm::uvec3  alphaMix(glm::vec4 const&frameColor,glm::vec4 const&fragColor){
  auto frame = floatColorToBytes(frameColor);
  return glm::clamp(glm::mix(glm::vec3(frame)/255.f,glm::vec3(fragColor),fragColor[3]),0.f,1.f)*255.f;
}


std::string padding(size_t n){
  return std::string(n,' ');
}

void printChild(Node const&node,size_t child,size_t p);

void printNodeContent(Node const&node,size_t p){
  std::cerr << padding(p) << "  mesh = " << node.mesh << std::endl;
  //if(node.modelMatrix != glm::mat4(1))
  std::cerr << padding(p) << "  modelMatrix = " << str(node.modelMatrix) << std::endl;
  std::cerr << padding(p) << "  children["<<node.children.size()<<"] = {";
  if(!node.children.empty()){
    std::cerr << std::endl;
    for(size_t i=0;i<node.children.size();++i)
      printChild(node.children.at(i),i,p+2);
    std::cerr << padding(p) << "  ";
  }
  std::cerr << "}" << std::endl;
}

void printChild(Node const&node,size_t child,size_t p){
  std::cerr << padding(p+2) << "child" << child << "{" << std::endl;
  printNodeContent(node,p+2);
  std::cerr << padding(p+2) << "}" << std::endl;
}

void printVertexAttrib(std::string const&name,VertexAttrib const&a,uint32_t p){
  if(a.bufferID <0)return;
  std::cerr << padding(p) << name<<"{" << std::endl;
  std::cerr << padding(p) << "  bufferID   = " << a.bufferID  << std::endl;
  std::cerr << padding(p) << "  type       = " << str(a.type) << std::endl;
  std::cerr << padding(p) << "  stride     = " << a.stride    << std::endl;
  std::cerr << padding(p) << "  offset     = " << a.offset    << std::endl;
  std::cerr << padding(p) << "}" << std::endl;
}

void printModel(size_t p,Model const&model){
  std::cerr << padding(p)<<"Model vypadá takto:" << std::endl;
  std::cerr << padding(p)<<"model{" << std::endl;
  std::cerr << padding(p)<<"  meshes  [" << model.meshes  .size() << "]"<< std::endl;
  std::cerr << padding(p)<<"  roots   [" << model.roots   .size() << "]"<< std::endl;
  std::cerr << padding(p)<<"  textures[" << model.textures.size() << "]"<< std::endl;
  std::cerr << padding(p)<<"}" << std::endl;
  for(size_t i=0;i<model.meshes.size();++i){
    auto const&mesh = model.meshes.at(i);
    std::cerr << padding(p) << "mesh" << i << "{" << std::endl;
    std::cerr << padding(p) << "  nofIndices     = " << mesh.nofIndices        << std::endl;
    std::cerr << padding(p) << "  diffuseColor   = " << str(mesh.diffuseColor) << std::endl;
    std::cerr << padding(p) << "  doubleSided    = " << str(mesh.doubleSided)  << std::endl;
    std::cerr << padding(p) << "  diffuseTexture = " << mesh.diffuseTexture    << std::endl;
    std::cerr << padding(p) << "  indexBufferID  = " << mesh.indexBufferID     << std::endl;
    std::cerr << padding(p) << "  indexOffset    = " << mesh.indexOffset       << std::endl;
    std::cerr << padding(p) << "  indexType      = " << str(mesh.indexType)    << std::endl;
    printVertexAttrib("position",mesh.position,8);
    printVertexAttrib("normal"  ,mesh.normal  ,8);
    printVertexAttrib("texCoord",mesh.texCoord,8);
    std::cerr << padding(p) << "  }" << std::endl;
    std::cerr << padding(p) << "}" << std::endl;
  }
  for(size_t i=0;i<model.textures.size();++i){
    auto const&texture = model.textures.at(i);
    std::cerr << padding(p) << "texture" << i << "{" << std::endl;
    std::cerr << padding(p) << "  data     = " << (void*)texture.data << std::endl;
    std::cerr << padding(p) << "  channels = " << texture.channels    << std::endl;
    std::cerr << padding(p) << "  width    = " << texture.width       << std::endl;
    std::cerr << padding(p) << "  height   = " << texture.height      << std::endl;
    std::cerr << padding(p) << "}" << std::endl;
  }
  for(size_t i=0;i<model.roots.size();++i){
    auto const&node = model.roots.at(i);
    std::cerr << padding(p) << "root" << i << "{" << std::endl;
    printNodeContent(node,p);
    std::cerr << padding(p) << "}" << std::endl;
  }
}


bool operator==(VertexAttrib const&a,VertexAttrib const&b){
  if(a.bufferID != b.bufferID)return false;
  if(a.offset != b.offset)return false;
  if(a.stride != b.stride)return false;
  if(a.type != b.type)return false;
  return true;
}



bool breakTest(bool first){
  static int32_t counter = 0;
  if(first)counter = 0;
  bool res = false;

  res = counter == ProgramContext::get().args.testToBreak;

  counter++;

  return res;
}

}
