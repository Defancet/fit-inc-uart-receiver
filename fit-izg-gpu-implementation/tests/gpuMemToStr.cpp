#include<sstream>
#include<map>
#include<tests/commandBufferToStr.hpp>
#include<tests/testCommon.hpp>

namespace tests{

enum class BufferType{
  FLOAT,
  U8   ,
  U16  ,
  U32  ,
  MIXED,
};

std::string getBufferName(BufferType t,uint32_t i){
  std::stringstream ss;
  switch(t){
    case BufferType::FLOAT:ss << "vertexBuffer" ;break;
    case BufferType::MIXED:ss << "generalBuffer";break;
    default               :ss << "indexBuffer"  ;break;
  }
  ss << i;
  return ss.str();
}

template<typename ELEMENT_TYPE>
std::string declareBufferImpl(size_t p,Buffer const&b,std::string const&name){
  std::stringstream ss;
  size_t n = b.size/sizeof(ELEMENT_TYPE);
  ss << padding(p) << typeToStr<ELEMENT_TYPE>() << " "<<name<< "["<<n<<"] = {" << std::endl;
  for(size_t i=0;i<n;++i)
    ss << padding(p+2) << str<ELEMENT_TYPE>(((ELEMENT_TYPE*)b.data)[i]) << "," << std::endl;
  ss << padding(p) << "};" << std::endl;
  return ss.str();
}

std::string declareGeneraBufferImpl(size_t p,Buffer const&b,std::string const&name){
  std::stringstream ss;
  ss << padding(p) << typeToStr<uint8_t>() << " " << name << "["<<b.size<<"] = {...};" << std::endl;
  return ss.str();
}

std::string declareBuffer(size_t p,uint32_t i,Buffer const&b,BufferType type){
  auto const name = getBufferName(type,i);
  if(type == BufferType::FLOAT)return declareBufferImpl<float   >(p,b,name);
  if(type == BufferType::U8   )return declareBufferImpl<uint8_t >(p,b,name);
  if(type == BufferType::U16  )return declareBufferImpl<uint16_t>(p,b,name);
  if(type == BufferType::U32  )return declareBufferImpl<uint32_t>(p,b,name);
  if(type == BufferType::MIXED)return declareGeneraBufferImpl    (p,b,name);
  return "";
}

void insertTypeToBufferTypes(std::map<uint32_t,BufferType>&bufferTypes,uint32_t b,BufferType t){
  if(bufferTypes.count(b)){
    if(bufferTypes[b] != t)
      bufferTypes[b] = BufferType::MIXED;
  }else
    bufferTypes[b] = t;
}

void insertIndexBufferToBufferTypes(std::map<uint32_t,BufferType>&bufferTypes,VertexArray const&vao){
  if(vao.indexBufferID < 0)return;
  if(vao.indexType == IndexType::UINT32)insertTypeToBufferTypes(bufferTypes,vao.indexBufferID,BufferType::U32);
  if(vao.indexType == IndexType::UINT16)insertTypeToBufferTypes(bufferTypes,vao.indexBufferID,BufferType::U16);
  if(vao.indexType == IndexType::UINT8 )insertTypeToBufferTypes(bufferTypes,vao.indexBufferID,BufferType::U8 );
}

bool offsetIsNotAlignedWithType(VertexAttrib const&attrib){
  return attrib.offset % (uint32_t)attrib.type != 0;
}

void insertAttribBufferToBufferTypes(std::map<uint32_t,BufferType>&bufferTypes,VertexAttrib const&attrib){
  if(attrib.bufferID < 0)return;
  auto type = BufferType::FLOAT;
  if(offsetIsNotAlignedWithType(attrib))type = BufferType::MIXED;
  insertTypeToBufferTypes(bufferTypes,attrib.bufferID,type);
}

void insertAttribBuffersToBufferTypes(std::map<uint32_t,BufferType>&bufferTypes,VertexArray const&vao){
  for(uint32_t i=0;i<maxAttributes;++i)
    insertAttribBufferToBufferTypes(bufferTypes,vao.vertexAttrib[i]);
}

void insertBuffersMentionedInCommandBufferToBufferTypes(std::map<uint32_t,BufferType>&bufferTypes,CommandBuffer const&cb){
  for(uint32_t i=0;i<cb.nofCommands;++i){
    if(cb.commands[i].type != CommandType::DRAW)continue;
    auto const&vao = cb.commands[i].data.drawCommand.vao;
    insertIndexBufferToBufferTypes(bufferTypes,vao);
    insertAttribBuffersToBufferTypes(bufferTypes,vao);
  }
}

void appendBuffersNotMentionedInCommandBufferToBufferTypes(std::map<uint32_t,BufferType>&bufferTypes,GPUMemory const&mem){
  for(uint32_t i=0;i<mem.maxBuffers;++i){
    if(!mem.buffers[i].data)continue;
    if(bufferTypes.count(i))continue;
    insertTypeToBufferTypes(bufferTypes,i,BufferType::MIXED);
  }
}

std::map<uint32_t,BufferType>createBufferTypes(GPUMemory const&mem,CommandBuffer const&cb){
  std::map<uint32_t,BufferType>bufferTypes;

  insertBuffersMentionedInCommandBufferToBufferTypes   (bufferTypes,cb);
  appendBuffersNotMentionedInCommandBufferToBufferTypes(bufferTypes,mem);

  return bufferTypes;
}

std::string declareBuffers(size_t p,GPUMemory const&mem,CommandBuffer const&cb){
  std::stringstream ss;

  auto bufferTypes = createBufferTypes(mem,cb);

  for(auto const&bt:bufferTypes)
    ss << declareBuffer(p,bt.first,mem.buffers[bt.first],bt.second);
  return ss.str();
}

std::string gpuMemBuffersToString(size_t p,GPUMemory const&mem,CommandBuffer const&cb){
  std::stringstream ss;

  auto bufferTypes = createBufferTypes(mem,cb);
  for(auto const&b:bufferTypes){
    ss << padding(p) << "mem.buffers["<<b.first<<"].data = " << getBufferName(b.second,b.first) << ";" << std::endl;
    ss << padding(p) << "mem.buffers["<<b.first<<"].size = " << mem.buffers[b.first].size << ";" << std::endl;
  }

  return ss.str();
}

std::string gpuMemFramebufferToString(size_t p,GPUMemory const&mem){
  std::stringstream ss;
  ss << padding(p) << "mem.framebuffer.width    = " <<        mem.framebuffer.width    << ";" << std::endl;
  ss << padding(p) << "mem.framebuffer.height   = " <<        mem.framebuffer.height   << ";" << std::endl;
  ss << padding(p) << "mem.framebuffer.channels = " <<        mem.framebuffer.channels << ";" << std::endl;
  ss << padding(p) << "mem.framebuffer.color    = memory;" << std::endl;
  ss << padding(p) << "mem.framebuffer.depth    = memory;" << std::endl;
  return ss.str();
}

std::string gpuMemProgramsToString(size_t p,GPUMemory const&mem){
  std::stringstream ss;
  for(uint32_t i=0;i<mem.maxPrograms;++i){
    if(mem.programs[i].vertexShader){
      ss << padding(p) << "mem.programs["<<i<<"].vertexShader   = function;"<< std::endl;
      ss << padding(p) << "mem.programs["<<i<<"].fragmentShader = function;"<< std::endl;
      for(uint32_t j=0;j<maxAttributes;++j){
        if(mem.programs[i].vs2fs[j] == AttributeType::EMPTY)continue;
        ss << padding(p) << "mem.programs["<<i<<"].vs2fs["<<j<<"] = "<<str(mem.programs[i].vs2fs[j])<<";" << std::endl;
      }
    }
  }
  return ss.str();
}

std::string gpuMemStructToString(size_t p,GPUMemory const&mem,CommandBuffer const&cb){
  std::stringstream ss;
  ss << gpuMemFramebufferToString (p,mem);
  ss << gpuMemProgramsToString(p,mem);
  ss << gpuMemBuffersToString     (p,mem,cb);
  //ss << gpuMemTexturesToString(p,mem);
  //ss << gpuMemUniformsToString(p,mem);
  return ss.str();
}

std::string gpuMemoryToStr(size_t p,GPUMemory const&mem,CommandBuffer const&cb){
  std::stringstream ss;
  ss << declareBuffers(p,mem,cb);
  ss << padding(p) << "GPUMemory mem;" << std::endl;
  ss << gpuMemStructToString(p,mem,cb);
  //for(size_t i=0;i<mem.maxBuffers;++i)
  //  ss << str(p,i,mem.buffers[i]);

  return ss.str();
}

}
