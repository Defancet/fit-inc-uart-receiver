#include<sstream>
#include<tests/commandBufferToStr.hpp>
#include<tests/testCommon.hpp>

namespace tests{

std::string commandTypeToStr(CommandType const&type){
  switch(type){
    case CommandType::CLEAR:return "CLEAR";
    case CommandType::DRAW :return "DRAW" ;
    case CommandType::EMPTY:return "EMPTY";
  }
  return "";
}

std::string clearCommandToStr(size_t p,uint32_t i,ClearCommand const&cmd){
  std::stringstream ss;
  ss << padding(p)<<"cb.commands["<<i<<"].data.clearCommand.color      = glm::vec4"<<str(cmd.color) <<";"<<std::endl;
  ss << padding(p)<<"cb.commands["<<i<<"].data.clearCommand.depth      = "<<cmd.depth               <<";"<<std::endl;
  ss << padding(p)<<"cb.commands["<<i<<"].data.clearCommand.clearColor = "<<str(cmd.clearColor)     <<";"<<std::endl;
  ss << padding(p)<<"cb.commands["<<i<<"].data.clearCommand.clearDepth = "<<str(cmd.clearDepth)     <<";"<<std::endl;
  return ss.str();
}

std::string drawCommandToStr(size_t p,uint32_t i,DrawCommand const&cmd){
  std::stringstream ss;
  ss << padding(p) << "cb.commands["<<i<<"].data.drawCommand.backfaceCulling = "<<str(cmd.backfaceCulling) <<";" << std::endl;
  ss << padding(p) << "cb.commands["<<i<<"].data.drawCommand.programID       = "<<cmd.programID            <<";" << std::endl;
  ss << padding(p) << "cb.commands["<<i<<"].data.drawCommand.nofVertices     = "<<cmd.nofVertices          <<";" << std::endl;
  if(cmd.vao.indexBufferID>=0){
    ss << padding(p) << "cb.commands["<<i<<"].data.drawCommand.vao.indexBufferID = " << cmd.vao.indexBufferID  << ";" << std::endl;
    ss << padding(p) << "cb.commands["<<i<<"].data.drawCommand.vao.indexOffset   = " << cmd.vao.indexOffset    << ";" << std::endl;
    ss << padding(p) << "cb.commands["<<i<<"].data.drawCommand.vao.indexType     = " << str(cmd.vao.indexType) << ";" << std::endl;
  }
  for(uint32_t j=0;j<maxAttributes;++j){
    if(cmd.vao.vertexAttrib[j].type == AttributeType::EMPTY)continue;
    ss << padding(p) << "cb.commands["<<i<<"].data.drawCommand.vao.vertexAttrib["<<j<<"].bufferID = " << cmd.vao.vertexAttrib[j].bufferID  << ";" << std::endl;
    ss << padding(p) << "cb.commands["<<i<<"].data.drawCommand.vao.vertexAttrib["<<j<<"].offset   = " << cmd.vao.vertexAttrib[j].offset    << ";" << std::endl;
    ss << padding(p) << "cb.commands["<<i<<"].data.drawCommand.vao.vertexAttrib["<<j<<"].stride   = " << cmd.vao.vertexAttrib[j].stride    << ";" << std::endl;
    ss << padding(p) << "cb.commands["<<i<<"].data.drawCommand.vao.vertexAttrib["<<j<<"].type     = " << str(cmd.vao.vertexAttrib[j].type) << ";" << std::endl;
  }
  return ss.str();
}

std::string commandToStr(size_t p,uint32_t i,Command const&cmd){
  std::stringstream ss;
  ss << padding(p) << "cb.commands["<<i<<"].type = CommandType::" << commandTypeToStr(cmd.type) << ";" << std::endl;
  switch(cmd.type){
    case CommandType::CLEAR:
      ss << clearCommandToStr(p,i,cmd.data.clearCommand);
      break;
    case CommandType::DRAW:
      ss << drawCommandToStr(p,i,cmd.data.drawCommand);
      break;
    case CommandType::EMPTY:
      break;
  }
  return ss.str();
}

std::string commandBufferToStr(size_t p,CommandBuffer const&cb){
  std::stringstream ss;
  ss << padding(p) << "CommandBuffer cb;" << std::endl;
  ss << padding(p) << "cb.nofCommands = " << cb.nofCommands << ";" << std::endl;
  for(uint32_t i=0;i<cb.nofCommands;++i)
    ss<<commandToStr(p,i,cb.commands[i]);
  return ss.str();
}

}
