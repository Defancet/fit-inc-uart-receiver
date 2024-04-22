#include <ios>
#include <memory>
#include <sstream>
#include <functional>

#include <catch2/catch_test_macros.hpp>

#include<tests/modelTestUtils.hpp>
#include<tests/testCommon.hpp>
#include<student/drawModel.hpp>


#include<iostream>

namespace tests::model{

uint32_t drawCallUniforms(uint32_t drawId){
  return drawCallUniformOffset + drawId*drawCallNofUniforms;
}

uint32_t uniformIdToDrawCallUniform(uint32_t uId){
  return (uId - drawCallUniformOffset) % drawCallNofUniforms;
}

void nodes(
    GPUMemory    &mem,
    CommandBuffer&commandBuffer,
    Node      const&node,
    Model     const&model,
    glm::mat4 const&modelMatrix,
    uint32_t &cmdID            ){

  if(node.mesh>=0){
    auto const&mesh = model.meshes.at(node.mesh);

    VertexArray vao;
    vao.indexBufferID = mesh.indexBufferID;
    vao.indexOffset   = mesh.indexOffset  ;
    vao.indexType     = mesh.indexType    ;
    vao.vertexAttrib[0] = mesh.position;
    vao.vertexAttrib[1] = mesh.normal  ;
    vao.vertexAttrib[2] = mesh.texCoord;
    bool doubleSided    = !mesh.doubleSided;

    pushDrawCommand(commandBuffer,mesh.nofIndices,0,vao,doubleSided);

    auto&modelMatrixUniform        = mem.uniforms[drawCallUniforms(cmdID)+MODEL       ].m4;
    auto&inverseModelMatrixUniform = mem.uniforms[drawCallUniforms(cmdID)+INV_MODEL   ].m4;
    auto&diffuseColorUniform       = mem.uniforms[drawCallUniforms(cmdID)+DIFF_COLOR  ].v4;
    auto&diffuseTextureIDUniform   = mem.uniforms[drawCallUniforms(cmdID)+TEXTURE_ID  ].u1;
    auto&doubleSidedUniform        = mem.uniforms[drawCallUniforms(cmdID)+DOUBLE_SIDED].v1;

    diffuseColorUniform       = mesh.diffuseColor;
    doubleSidedUniform        = (float)mesh.doubleSided;
    diffuseTextureIDUniform   = mesh.diffuseTexture;
    //isTextureUniform          = mesh.diffuseTexture<0?0.f:1.f;
    modelMatrixUniform        = modelMatrix*node.modelMatrix;
    inverseModelMatrixUniform = glm::transpose(glm::inverse(modelMatrixUniform));

    cmdID++;
  }
  for(auto const&n:node.children)
    nodes(mem,commandBuffer,n,model,modelMatrix*node.modelMatrix,cmdID);
}


TranslatedModel getExpectedTranslatedModel(Model const&model){
  MEMCB();

  for(size_t i=0;i<model.buffers.size();++i)
    mem.buffers[i] = model.buffers[i];

  for(size_t i=0;i<model.textures.size();++i)
    mem.textures[i] = model.textures[i];


  mem.programs[0].vertexShader   = drawModel_vertexShader;
  mem.programs[0].fragmentShader = drawModel_fragmentShader;
  mem.programs[0].vs2fs[0] = AttributeType::VEC3;
  mem.programs[0].vs2fs[1] = AttributeType::VEC3;
  mem.programs[0].vs2fs[2] = AttributeType::VEC2;
  mem.programs[0].vs2fs[3] = AttributeType::UINT;
  pushClearCommand(cb,glm::vec4(0.1,0.15,0.1,1.),10e10f);

  uint32_t cmdID = 0;
  for(auto const&root:model.roots)
    nodes(mem,cb,root,model,glm::mat4(1.f),cmdID);

  return memCb;
}

Diff filterErrorLevel(Diff status,Diff maxLevel){
  if(status <= maxLevel)return status;
  return Diff::SAME;
}

bool isGeneralUniformSame(Uniform const&a,Uniform const&b){
  auto pa = (uint32_t*)&a;
  auto pb = (uint32_t*)&b;
  for(int i=0;i<16;++i)
    if(pa[i] != pb[i])return false;
  return true;
}


Diff areTranslatedModelsSame(
    TranslatedModel const&expected,
    TranslatedModel const&student ,
    Diff            const&level   ){

  auto const&ecb  = expected->cb;
  auto const&emem = expected->mem;
  auto const&scb  = student->cb;
  auto const&smem = student->mem;

  if(ecb.nofCommands != scb.nofCommands)return filterErrorLevel(Diff::NOF_COMMANDS,level);
  for(size_t i=0;i<ecb.nofCommands;++i){
    auto const&ec = ecb.commands[i];
    auto const&sc = scb.commands[i];
    if(ec.type != sc.type)return filterErrorLevel(Diff::COMMAND_TYPE,level);
    if(ec.type == CommandType::CLEAR){
      auto const&ecc = ec.data.clearCommand;
      auto const&scc = sc.data.clearCommand;
      if(ecc.clearColor != scc.clearColor)return filterErrorLevel(Diff::CLEAR,level);
      if(ecc.clearDepth != scc.clearDepth)return filterErrorLevel(Diff::CLEAR,level);
      if(ecc.color      != scc.color     )return filterErrorLevel(Diff::CLEAR,level);
      if(ecc.depth      != scc.depth     )return filterErrorLevel(Diff::CLEAR,level);
    }
    if(ec.type == CommandType::DRAW){
      auto const&ecc = ec.data.drawCommand;
      auto const&scc = sc.data.drawCommand;
      if(ecc.backfaceCulling   != scc.backfaceCulling  )return filterErrorLevel(Diff::BACKFACE    ,level);
      if(ecc.programID         != scc.programID        )return filterErrorLevel(Diff::PROGRAMID   ,level);
      if(ecc.nofVertices       != scc.nofVertices      )return filterErrorLevel(Diff::NOF_VERTICES,level);
      if(ecc.vao.indexBufferID != scc.vao.indexBufferID)return filterErrorLevel(Diff::INDEXING    ,level);
      if(ecc.vao.indexType     != scc.vao.indexType    )return filterErrorLevel(Diff::INDEXING    ,level);
      if(ecc.vao.indexOffset   != scc.vao.indexOffset  )return filterErrorLevel(Diff::INDEXING    ,level);
      for(uint32_t a=0;a<maxAttributes;++a)
        if(!(ecc.vao.vertexAttrib[a] == scc.vao.vertexAttrib[a]))
          return filterErrorLevel(Diff::ATTRIB,level);

    }
  }

  for(uint32_t i=0;i<GPUMemory::maxPrograms;++i){
    auto const&ep = emem.programs[i];
    auto const&sp = smem.programs[i];
    if(ep.vertexShader   != sp.vertexShader  )return filterErrorLevel(Diff::SHADERS,level);
    if(ep.fragmentShader != sp.fragmentShader)return filterErrorLevel(Diff::SHADERS,level);
    for(uint32_t a=0;a<maxAttributes;++a)
      if(ep.vs2fs[a] != sp.vs2fs[a])return filterErrorLevel(Diff::VS2FS,level);
  }

  for(uint32_t i=0;i<GPUMemory::maxBuffers;++i){
    auto const&eb = emem.buffers[i];
    auto const&sb = smem.buffers[i];
    if(eb.data != sb.data)return filterErrorLevel(Diff::BUFFERS,level);
    if(eb.size != sb.size)return filterErrorLevel(Diff::BUFFERS,level);
  }

  for(uint32_t i=0;i<GPUMemory::maxTextures;++i){
    auto const&et = emem.textures[i];
    auto const&st = smem.textures[i];
    if(et.data     != st.data    )return filterErrorLevel(Diff::TEXTURES,level);
    if(et.channels != st.channels)return filterErrorLevel(Diff::TEXTURES,level);
    if(et.height   != st.height  )return filterErrorLevel(Diff::TEXTURES,level);
    if(et.width    != st.width   )return filterErrorLevel(Diff::TEXTURES,level);
  }

  for(uint32_t i=0;i<GPUMemory::maxUniforms;++i){
    auto const&eu = emem.uniforms[i];
    auto const&su = smem.uniforms[i];
    if(i<drawCallUniformOffset){
      if(!isGeneralUniformSame(eu,su))return filterErrorLevel(Diff::INV_MATRIX,level);
    }else{
      auto di = uniformIdToDrawCallUniform(i);
      if(di == DrawCallUniform::DIFF_COLOR){
        if(eu.v4 != su.v4)return filterErrorLevel(Diff::DIFFUSE_COLOR,level);
      }
      if(di == DrawCallUniform::DOUBLE_SIDED){
        if(eu.v1 != su.v1)return filterErrorLevel(Diff::DOUBLE_SIDED,level);
      }
      if(di == DrawCallUniform::TEXTURE_ID){
        if(eu.i1 != su.i1)return filterErrorLevel(Diff::TEXTURE_ID,level);
      }
      if(di == DrawCallUniform::MODEL){
        if(eu.m4 != su.m4)return filterErrorLevel(Diff::MODEL_MATRIX,level);
      }
      if(di == DrawCallUniform::INV_MODEL){
        if(eu.m4 != su.m4)return filterErrorLevel(Diff::INV_MATRIX,level);
      }
    }
  }

  return Diff::SAME;
}

std::string nofCommands(size_t p,Check const&check){
  std::stringstream ss;
  ss << padding(p) << "Počet příkazů v command bufferu je špatný." << std::endl;
  ss << padding(p) << "Měl by se vytvořit alespoň čistící příkaz." << std::endl;
  ss << padding(p) << "Takto vypadaly vaše příkazy v command bufferu:" << std::endl;
  ss << padding(p) << "příkazů je   : " << check.student ->cb.nofCommands << std::endl;
  ss << padding(p) << "správný počet: " << check.expected->cb.nofCommands << std::endl;
  return ss.str();
}

std::string listCommands(size_t p,CommandBuffer const&cb){
  std::stringstream ss;
  for(uint32_t i=0;i<cb.nofCommands;++i)
    ss << padding(p) << str(cb.commands[i].type) << std::endl;
  return ss.str();
}

std::string commandType(size_t p,Check const&check){
  std::stringstream ss;
  ss << padding(p) << "Příkazy v command bufferu jsou špatné." << std::endl;
  ss << padding(p) << "Měl by se vytvořit alespoň čistící příkaz." << std::endl;
  ss << padding(p) << "Takto vypadaly vaše příkazy v command bufferu:" << std::endl;
  ss << listCommands(p+2,check.student->cb);
  ss << padding(p) << "Takto jsou správně:" << std::endl;
  ss << listCommands(p+2,check.expected->cb);
  return ss.str();
}

std::string listClearProps(size_t p,uint32_t i,ClearCommand const&cc,Diff const&prop){
  std::stringstream ss;
  if(prop == Diff::CLEAR){
    ss << padding(p) << "cb.commands["<<i<<"].data.clearCommand.clearColor = " << str(cc.clearColor) << std::endl;
    ss << padding(p) << "cb.commands["<<i<<"].data.clearCommand.clearDepth = " << str(cc.clearDepth) << std::endl;
    ss << padding(p) << "cb.commands["<<i<<"].data.clearCommand.color      = " << str(cc.color     ) << std::endl;
    ss << padding(p) << "cb.commands["<<i<<"].data.clearCommand.depth      = " << str(cc.depth     ) << std::endl;
  }
  return ss.str();
}

std::string listDrawProps(size_t p,uint32_t i,DrawCommand const&cc,Diff const&prop){
  std::stringstream ss;
  if(prop == Diff::BACKFACE)
    ss << padding(p) << "cb.commands["<<i<<"].data.drawCommand.backFaceCulling = " << str(cc.backfaceCulling) << std::endl;
  if(prop == Diff::PROGRAMID)
    ss << padding(p) << "cb.commands["<<i<<"].data.drawCommand.programID = " << str(cc.programID) << std::endl;
  if(prop == Diff::NOF_VERTICES)
    ss << padding(p) << "cb.commands["<<i<<"].data.drawCommand.nofVertices = " << str(cc.nofVertices) << std::endl;
  if(prop == Diff::INDEXING){
    auto const&vao = cc.vao;
    ss << padding(p) << "cb.commands["<<i<<"].data.drawCommand.vao.indexType     = " << str(vao.indexType    ) << std::endl;
    ss << padding(p) << "cb.commands["<<i<<"].data.drawCommand.vao.indexOffset   = " << str(vao.indexOffset  ) << std::endl;
    ss << padding(p) << "cb.commands["<<i<<"].data.drawCommand.vao.indexBufferID = " << str(vao.indexBufferID) << std::endl;
  }
  if(prop == Diff::ATTRIB){
    auto const&vao = cc.vao;
    for(uint32_t a=0;a<maxAttributes;++a){
      auto const& aa = vao.vertexAttrib[a];
      if(aa.bufferID<0)continue;
      ss << padding(p) << "cb.commands["<<i<<"].data.drawCommand.vao.vertexAttrib["<<a<<"].bufferID = " << str(aa.bufferID) << std::endl;
      ss << padding(p) << "cb.commands["<<i<<"].data.drawCommand.vao.vertexAttrib["<<a<<"].offset   = " << str(aa.offset  ) << std::endl;
      ss << padding(p) << "cb.commands["<<i<<"].data.drawCommand.vao.vertexAttrib["<<a<<"].stride   = " << str(aa.stride  ) << std::endl;
      ss << padding(p) << "cb.commands["<<i<<"].data.drawCommand.vao.vertexAttrib["<<a<<"].type     = " << str(aa.type    ) << std::endl;
    }
  }
  return ss.str();
}

std::string listCommandBufferProp(size_t p,CommandBuffer const&cb,Diff const&prop){
  std::stringstream ss;
  for(uint32_t i=0;i<cb.nofCommands;++i){
    if(cb.commands[i].type == CommandType::CLEAR)ss << listClearProps(p,i,cb.commands[i].data.clearCommand,prop);
    if(cb.commands[i].type == CommandType::DRAW )ss << listDrawProps (p,i,cb.commands[i].data. drawCommand,prop);
  }
  return ss.str();
}

std::string listShaders(size_t p,GPUMemory const&mem){
  std::stringstream ss;
  for(uint32_t i=0;i<GPUMemory::maxPrograms;++i){
    auto const&prg = mem.programs[i];
    auto const&vs  = prg.vertexShader  ;
    auto const&fs  = prg.fragmentShader;
    if(vs || fs ){
      if(vs)ss << padding(p) << "mem.programs["<<i<<"].vertexShader   = " << vs << ";" << std::endl;
      if(fs)ss << padding(p) << "mem.programs["<<i<<"].fragmentShader = " << fs << ";" << std::endl;
    }
  }
  return ss.str();
}

std::string listVS2FS(size_t p,GPUMemory const&mem){
  std::stringstream ss;
  for(uint32_t i=0;i<GPUMemory::maxPrograms;++i){
    auto const&v = mem.programs[i].vs2fs;
    for(uint32_t a=0;a<maxAttributes;++a){
      if(v[a] == AttributeType::EMPTY)continue;
      ss << padding(p) << "mem.programs["<<i<<"].vs2fs["<<a<<"] = " << str(v[a]) << std::endl;
    }
  }
  return ss.str();
}

std::string listBuffers(size_t p,GPUMemory const&mem){
  std::stringstream ss;
  for(uint32_t i=0;i<GPUMemory::maxBuffers;++i){
    auto const&b = mem.buffers[i];
    if(!b.data)continue;
    ss << padding(p) << "mem.buffers["<<i<<"].data = " << b.data << ";" << std::endl;
    ss << padding(p) << "mem.buffers["<<i<<"].size = " << b.size << ";" << std::endl;
  }
  return ss.str();
}

std::string listTextures(size_t p,GPUMemory const&mem){
  std::stringstream ss;
  for(uint32_t i=0;i<GPUMemory::maxTextures;++i){
    auto const&t = mem.textures[i];
    if(!t.data)continue;
    ss << padding(p) << "mem.textures["<<i<<"].data     = " << t.data     << ";" << std::endl;
    ss << padding(p) << "mem.textures["<<i<<"].channels = " << t.channels << ";" << std::endl;
    ss << padding(p) << "mem.textures["<<i<<"].width    = " << t.width    << ";" << std::endl;
    ss << padding(p) << "mem.textures["<<i<<"].height   = " << t.height   << ";" << std::endl;
  }
  return ss.str();
}

enum class UniformType{
  V1,V2,V3,V4,
  U1,U2,U3,U4,
  I1,I2,I3,I4,
  M4
};

std::string uniformToStr(Uniform const&u,UniformType const&t){
  switch(t){
    case UniformType::V1:return "v1 = "+str(u.v1);
    case UniformType::V2:return "v2 = "+str(u.v2);
    case UniformType::V3:return "v3 = "+str(u.v3);
    case UniformType::V4:return "v4 = "+str(u.v4);
    case UniformType::U1:return "u1 = "+str(u.u1);
    case UniformType::U2:return "u2 = "+str(u.u2);
    case UniformType::U3:return "u3 = "+str(u.u3);
    case UniformType::U4:return "u4 = "+str(u.u4);
    case UniformType::I1:return "i1 = "+str(u.i1);
    case UniformType::I2:return "i2 = "+str(u.i2);
    case UniformType::I3:return "i3 = "+str(u.i3);
    case UniformType::I4:return "i4 = "+str(u.i4);
    case UniformType::M4:return "m4 = "+str(u.m4);
    default             :return "?? = unknown";
  }
}

UniformType determineUniformType(Uniform const&u){
  glm::mat4 m = u.m4;
  m[0] = glm::vec4(1,0,0,0);
  if(m != glm::mat4(1))return UniformType::M4;

  if(u.i1>-2&&u.i1<1000)return UniformType::I1;
  return UniformType::V4;
}

std::string listUniforms(size_t p,GPUMemory const&mem,DrawCallUniform uType){
  std::stringstream ss;
  for(uint32_t i=0;i<GPUMemory::maxUniforms;++i){
    auto const&u=mem.uniforms[i];
    if(u.m4 == glm::mat4(1.f))continue;
    UniformType type;
    std::string uniformName;
    if(i<drawCallUniformOffset){
      type = determineUniformType(u);
    }else{
      auto mtype = uniformIdToDrawCallUniform(i);
      if(mtype != uType)continue;
      if(mtype == DrawCallUniform::MODEL       ){type = UniformType::M4;uniformName = "modelMatrix"       ;}
      if(mtype == DrawCallUniform::INV_MODEL   ){type = UniformType::M4;uniformName = "inverseModelMatrix";}
      if(mtype == DrawCallUniform::DIFF_COLOR  ){type = UniformType::V4;uniformName = "diffuseColor"      ;}
      if(mtype == DrawCallUniform::TEXTURE_ID  ){type = UniformType::I1;uniformName = "textureID"         ;}
      if(mtype == DrawCallUniform::DOUBLE_SIDED){type = UniformType::V1;uniformName = "doubleSided"       ;}
    }
    ss << padding(p) << "mem.uniforms["<<i<<"]."<<uniformToStr(u,type)<<"; // "<<uniformName<<std::endl;
  }
  return ss.str();
}

std::string listMemoryProp(size_t p,GPUMemory const&mem,Diff const&prop){
  std::stringstream ss;
  if(prop == Diff::SHADERS      )return listShaders    (p,mem);
  if(prop == Diff::VS2FS        )return listVS2FS      (p,mem);
  if(prop == Diff::BUFFERS      )return listBuffers    (p,mem);
  if(prop == Diff::TEXTURES     )return listTextures   (p,mem);
  if(prop == Diff::DOUBLE_SIDED )return listUniforms(p,mem,DrawCallUniform::DOUBLE_SIDED);
  if(prop == Diff::DIFFUSE_COLOR)return listUniforms(p,mem,DrawCallUniform::DIFF_COLOR  );
  if(prop == Diff::TEXTURE_ID   )return listUniforms(p,mem,DrawCallUniform::TEXTURE_ID  );
  if(prop == Diff::MODEL_MATRIX )return listUniforms(p,mem,DrawCallUniform::MODEL       );
  if(prop == Diff::INV_MATRIX   )return listUniforms(p,mem,DrawCallUniform::INV_MODEL   );
    
  return ss.str();
}

std::string getCommandBufferDiference(size_t p,Check const&check){
  std::stringstream ss;
  ss << padding(p) << "Takto vypadaly vaše příkazy v command bufferu:" << std::endl;
  ss << listCommandBufferProp(p,check.student ->cb,check.status);
  ss << padding(p) << "Takto jsou správně:" << std::endl;
  ss << listCommandBufferProp(p,check.expected->cb,check.status);
  return ss.str();
}

std::string getMemoryDiference(size_t p,Check const&check){
  std::stringstream ss;
  ss << padding(p) << "Takto vypadala vaše paměť na GPU:" << std::endl;
  ss << listMemoryProp(p,check.student ->mem,check.status);
  ss << padding(p) << "Takto měla vypadat:" << std::endl;
  ss << listMemoryProp(p,check.expected->mem,check.status);
  return ss.str();
}

std::string getDiference(size_t p,Check const&check){
  if(check.status<Diff::SHADERS)return getCommandBufferDiference(p,check);
  else                          return getMemoryDiference       (p,check);
}

std::string clearError(size_t p,Check const&check){
  std::stringstream ss;
  ss << "Čistící příkaz je špatný." << std::endl;
  ss << getDiference(p,check);
  return ss.str();
}

std::string backfaceCulling(size_t p,Check const&check){
  std::stringstream ss;
  ss << padding(p) << "Kreslící příkaz je špatný." << std::endl;
  ss << padding(p) << "Nemáte nastavený backfaceCulling podle meshe." << std::endl;
  ss << padding(p) << "Pokud je mesh doubleSided, měl by být backfaceCulling vypnutý." << std::endl;
  ss << padding(p) << "Jinak by měl být zapnutý." << std::endl;
  ss << getDiference(p,check);
  return ss.str();
}

std::string programID(size_t p,Check const&check){
  std::stringstream ss;
  ss << padding(p) << "Kreslící příkaz je špatný." << std::endl;
  ss << padding(p) << "Nemáte správně nastavené číslo programu." << std::endl;
  ss << padding(p) << "Pro vykreslení modelu (jako čehokoliv jiného) je potřeba program." << std::endl;
  ss << padding(p) << "Měli byste nastavit programID na 0, protože je jen jeden program." << std::endl;
  ss << getDiference(p,check);
  return ss.str();
}

std::string nofVertices(size_t p,Check const&check){
  std::stringstream ss;
  ss << padding(p) << "Kreslící příkaz je špatný." << std::endl;
  ss << padding(p) << "Nemáte správně nastavený počet vrcholů pro vykreslení." << std::endl;
  ss << getDiference(p,check);
  return ss.str();
}


std::string indexing(size_t p,Check const&check){
  std::stringstream ss;
  ss << padding(p) << "Kreslící příkaz je špatný." << std::endl;
  ss << padding(p) << "Nemáte správně nastavený počet vrcholů pro vykreslení." << std::endl;
  ss << getDiference(p,check);
  return ss.str();
}

std::string attribs(size_t p,Check const&check){
  std::stringstream ss;
  ss << padding(p) << "Kreslící příkaz je špatný." << std::endl;
  ss << padding(p) << "Nemáte správně nastavené points attributy." << std::endl;
  ss << getDiference(p,check);
  return ss.str();
}

std::string shaders(size_t p,Check const&check){
  std::stringstream ss;
  ss << padding(p) << "Paměť grafické karty není správně nastavena." << std::endl;
  ss << padding(p) << "Špatně jste jste zapsali points nebo fragment shader do paměti." << std::endl;
  ss << getDiference(p,check);
  return ss.str();
}

std::string vs2fs(size_t p,Check const&check){
  std::stringstream ss;
  ss << padding(p) << "Paměť grafické karty není správně nastavena." << std::endl;
  ss << padding(p) << "Špatně jste jste nastavili pole vs2fs pro interpolací atributů." << std::endl;
  ss << padding(p) << "Měli byste mít 4, pozice, normála, tex. coord a číslo drawID." << std::endl;
  ss << padding(p) << "Jejich typ je vec3, vec3, vec2, uint." << std::endl;
  ss << getDiference(p,check);
  return ss.str();
}

std::string buffers(size_t p,Check const&check){
  std::stringstream ss;
  ss << padding(p) << "Paměť grafické karty není správně nastavena." << std::endl;
  ss << padding(p) << "Špatně jste jste nastavili buffery s daty." << std::endl;
  ss << getDiference(p,check);
  return ss.str();
}

std::string textures(size_t p,Check const&check){
  std::stringstream ss;
  ss << padding(p) << "Paměť grafické karty není správně nastavena." << std::endl;
  ss << padding(p) << "Špatně jste jste nastavili textury." << std::endl;
  ss << getDiference(p,check);
  return ss.str();
}

std::string uniforms(size_t p,Check const&check){
  std::stringstream ss;
  ss << padding(p) << "Paměť grafické karty není správně nastavena." << std::endl;
  ss << padding(p) << "Špatně jste nastavili uniformní proměnné." << std::endl;
  ss << getDiference(p,check);
  ss << std::endl;
  ss << padding(p) << "Uniformních proměnných je několik." << std::endl;
  ss << padding(p) << "Uniformní proměnné pro každý vykreslovací příkaz:" << std::endl;
  ss << padding(p) << "  modelMatrix, invModelMatrix, diffuseColor, textureID, doubleSided." << std::endl;
  ss << padding(p) << "  mem.uniforms["<<drawCallUniformOffset<<"+drawID*"<<drawCallNofUniforms<<"+"<<MODEL       <<"].m4 = modelMatrix       ;" << std::endl;
  ss << padding(p) << "  mem.uniforms["<<drawCallUniformOffset<<"+drawID*"<<drawCallNofUniforms<<"+"<<INV_MODEL   <<"].m4 = inverseModelMatrix;" << std::endl;
  ss << padding(p) << "  mem.uniforms["<<drawCallUniformOffset<<"+drawID*"<<drawCallNofUniforms<<"+"<<DIFF_COLOR  <<"].v4 = diffuseColor      ;" << std::endl;
  ss << padding(p) << "  mem.uniforms["<<drawCallUniformOffset<<"+drawID*"<<drawCallNofUniforms<<"+"<<TEXTURE_ID  <<"].i1 = textureID         ;" << std::endl;
  ss << padding(p) << "  mem.uniforms["<<drawCallUniformOffset<<"+drawID*"<<drawCallNofUniforms<<"+"<<DOUBLE_SIDED<<"].v1 = doubleSided       ;" << std::endl;
  ss << std::endl;
  ss << padding(p) << "Tyto uniformní proměnné musíte správně nastavit a v případě matic i správně vypočítat!" << std::endl;
   
  ss << std::endl;
  ss << padding(p) << "A Uniformní proměnné pro scénu:" << std::endl;
  ss << padding(p) << "  viewProjectionMatrix, lightPosition, cameraPosition." << std::endl;
  ss << padding(p) << "  mem.uniforms[0].m4 = viewProjectionMatrix;" << std::endl;
  ss << padding(p) << "  mem.uniforms[1].v3 = lightPosition       ;" << std::endl;
  ss << padding(p) << "  mem.uniforms[2].v3 = cameraPosition      ;" << std::endl;
  ss << std::endl;
  ss << padding(p) << "Proměnné pro scénu nenastavujte! Ty se mění při kreslení a stará se o ně kreslící metoda!" << std::endl;
  return ss.str();
}


std::string translatedModelErrors(
    size_t       p    ,
    Check  const&check){
  switch(check.status){
    case Diff::NOF_COMMANDS :return nofCommands     (p,check);
    case Diff::COMMAND_TYPE :return commandType     (p,check);
    case Diff::CLEAR        :return clearError      (p,check);
    case Diff::BACKFACE     :return backfaceCulling (p,check);
    case Diff::PROGRAMID    :return programID       (p,check);
    case Diff::NOF_VERTICES :return nofVertices     (p,check);
    case Diff::INDEXING     :return indexing        (p,check);
    case Diff::ATTRIB       :return attribs         (p,check);
    case Diff::SHADERS      :return shaders         (p,check);
    case Diff::VS2FS        :return vs2fs           (p,check);
    case Diff::BUFFERS      :return buffers         (p,check);
    case Diff::TEXTURES     :return textures        (p,check);
    case Diff::TEXTURE_ID   :
    case Diff::DOUBLE_SIDED :
    case Diff::DIFFUSE_COLOR:
    case Diff::MODEL_MATRIX :
    case Diff::INV_MATRIX   :return uniforms(p,check);
                             
                            

    default:return padding(p) + "Nemáte problém.";
  }
  return "";
}

Check checkTranslation(Model const&model,Diff const&whatToTest){
  Check res;
  res.expected = getExpectedTranslatedModel(model);
  res.student  = std::make_shared<MemCb>();
  prepareModel(res.student->mem,res.student->cb,model);
  res.status = areTranslatedModelsSame(res.expected,res.student,whatToTest);

  if(breakTest())res.status = whatToTest;
  return res;
}


//NodeI::NodeI(
//    int32_t                 m ,  
//    std::vector<NodeI>const&c ,   
//    glm::mat4         const&mm){
//  mesh = m;
//  model = mm;
//  for(auto const&n:c){
//    children.emplace_back(n.mesh,n.children,n.model);
//  }
//}

NodeI::NodeI(
    int32_t                 m ,  
    std::vector<NodeI>const&c ,   
    glm::mat4         const&mm):mesh(m),children(c),model(mm){}

Model createModel(std::vector<MeshI>const&meshes,std::vector<NodeI>const&roots){
  Model model;
  for(auto const&m:meshes){
    Mesh mesh;
    mesh.nofIndices     = m.nofIndices    ;
    mesh.diffuseColor   = m.diffuseColor  ;
    mesh.diffuseTexture = m.diffuseTexture;
    mesh.doubleSided    = m.doubleSided   ;

    model.meshes.push_back(mesh);
  }

  std::function<void(Node&,NodeI const&)>rec;

  auto insert = [&](std::vector<Node>&o,std::vector<NodeI> const&i){
    for(auto const&n:i){
      o.push_back({});
      rec(o.back(),n);
    }
  };

  rec = [&](Node&o,NodeI const&i){
    o.mesh        = i.mesh;
    o.modelMatrix = i.model;
    insert(o.children,i.children);
  };
  insert(model.roots,roots);
  return model;
}

void checkModelTraverse(Model const&model){
  auto check = checkTranslation(model,Diff::ATTRIB);
  if(check.status == Diff::SAME)return;

  std::cerr<<R".(
  Tento test ověřuje funkcionalitu funkce prepareModel v souboru student/drawModel.cpp
  Je potřeba projít kořenové uzly (roots).
  Pokud nemá uzel mesh (mesh<0), nevytváří se pro něj command buffer, protože není co vykreslit.
  Je však nuté prozkoumat jeho potomky.
  Strom modelu by se měly projít pre order přístupem:
  https://en.wikipedia.org/wiki/Tree_traversal

  Nejsnazší zpracování je rekurzí, pseudokód:

  void prepareNode(Node const&node){
    if(node.mesh>=0){
      //mame mesh vkladame do command bufferu
    }
    for(size_t i=0;i<node.children.size();++i)
      prepareNode(node.children[i]);
  }

  void prepare(Model const&model){
    for(size_t i=0;i<model.roots.size();++i)
      prepareNode(model.roots[i]);
  }

  Test zkouší zpracovat model, který vypadá takto:
  )." << std::endl;
  printModel(2,model);
  std::cerr<<R".(
  A zkoumá, jaký jste vytvořili command bufferu.
  Ale něco se pokazilo...)."<<std::endl;

  std::cerr << translatedModelErrors(2,check) << std::endl;

  std::cerr << R".(
  Vytvořte správný command buffer!)."<<std::endl;
  REQUIRE(false);
}

void checkTraverse(std::vector<MeshI>const&meshes,std::vector<NodeI>const&roots){
  auto model = createModel(meshes,roots);
  checkModelTraverse(model);
}

void checkModelMemory(Model const&model,Diff const&level){
  auto check = checkTranslation(model,level);
  if(check.status == Diff::SAME)return;

  std::cerr<<R".(
  Tento test ověřuje funkcionalitu funkce prepareModel v souboru student/drawModel.cpp
  Zkouší, zda jste správně nastavili paměť grafické karty.

  void prepareModel(GPUMemory&mem,CommandBuffer&cb,Model const&model){
    ...
    mem.programs ...
    mem.buffers  ...
    mem.textures ...
    mem.uniforms ...
    ...
  }

  Test zkouší zpracovat model, který vypadá takto:
  )." << std::endl;
  printModel(2,model);
  std::cerr<<R".(
  A zkoumá, jak jste naplinili paměť.
  Ale něco se pokazilo...
  )."<<std::endl;

  std::cerr << translatedModelErrors(2,check) << std::endl;

  std::cerr << R".(
  Vytvořte správný command buffer!)."<<std::endl;
  REQUIRE(false);
}

}
