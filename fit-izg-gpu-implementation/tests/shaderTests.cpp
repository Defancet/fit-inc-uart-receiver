#include<iostream>

#include<catch2/catch_test_macros.hpp>
#include<glm/gtc/matrix_transform.hpp>

#include<framework/textureData.hpp>

#include<student/drawModel.hpp>
#include<student/fwd.hpp>
#include<student/gpu.hpp>

#include<tests/testCommon.hpp>
#include<tests/modelTestUtils.hpp>

using namespace tests;
using namespace tests::model;

glm::vec4 computeExpectedColor(
    glm::vec3           const&pos        ,
    glm::vec3           const&nor2       ,
    glm::vec2           const&coord      ,
    int32_t                   textureID  ,
    glm::vec3           const&light      ,
    glm::vec4           const&diffC      ,
    float               const&aF         ,
    std::vector<Texture>const&texs       ,
    float                     doubleSided,
    glm::vec3           const&cam        ){

  auto nor = nor2;
  if(doubleSided > 0.f)
    nor *= glm::sign(glm::dot(cam-pos,nor));

  glm::vec4 dC;
  if(textureID<0)
    dC = diffC;
  else
    dC = read_texture(texs[textureID],coord);
  
  auto L = glm::normalize(light-pos);
  float dF = glm::max(glm::dot(L,glm::normalize(nor)),0.f);

  glm::vec3 dL = glm::vec3(dC)*dF;
  glm::vec3 aL = glm::vec3(dC)*aF;

  return glm::vec4(aL+dL,dC.a);
}

glm::vec4 computeColor(
    glm::vec3           const&pos        ,
    glm::vec3           const&nor        ,
    glm::vec2           const&coord      ,
    uint32_t            const&drawID     ,
    int32_t             const&textureID  ,
    glm::vec3           const&light      ,
    glm::vec4           const&diffC      ,
    std::vector<Texture>const&texs       ,
    float                     doubleSided,
    glm::vec3 const&cam        ){

  InFragment inF;
  OutFragment outF;

  Uniform uniforms[GPUMemory::maxUniforms];
  Texture textures[GPUMemory::maxTextures];

  ShaderInterface si;
  si.textures = textures;
  si.uniforms = uniforms;

  inF.attributes[0].v3 = pos;
  inF.attributes[1].v3 = nor;
  inF.attributes[2].v2 = coord;
  inF.attributes[3].u1 = drawID;
  
  uniforms[1].v3 = light;
  uniforms[2].v3 = cam  ;
  uniforms[drawCallUniforms(drawID)+DIFF_COLOR  ].v4 = diffC      ;
  uniforms[drawCallUniforms(drawID)+TEXTURE_ID  ].i1 = textureID  ;
  uniforms[drawCallUniforms(drawID)+DOUBLE_SIDED].v1 = doubleSided;

  for(size_t i=0;i<texs.size();++i)
    textures[i] = texs[i];

  drawModel_fragmentShader(outF,inF,si);

  return outF.gl_FragColor;
}

void testLambert(
    glm::vec3           const&pos        ,
    glm::vec3           const&nor        ,
    glm::vec2           const&coord      ,
    uint32_t            const&drawID     ,
    int32_t             const&textureID  ,
    glm::vec3           const&light      ,
    glm::vec4           const&diffC      ,
    float               const&aF         ,
    std::vector<Texture>const&texs       ,
    float                     doubleSided,
    glm::vec3           const&cam        ){
  auto expected = computeExpectedColor(pos,nor,coord,       textureID,light,diffC,aF,texs,doubleSided,cam);
  auto color    = computeColor        (pos,nor,coord,drawID,textureID,light,diffC,texs,doubleSided,cam);

  auto printInfo = [&](){
    std::cerr<<R".(

    Tento test zkouší, zda je funkce drawModel_fragmentShader správně naimplementována.
    Tato funkce reprezentuje fragment shader.
    Fragment shader by měl spočítat barvu pomocí lambertova osvětlovacího modelu.
    https://en.wikipedia.org/wiki/Lambertian_reflectance
    Vstupem FS je fragment a výstupem je fragment s vypočtenou barvou.

    Vstupní fragment je tvořen čtyřmi atributy: pozice, normála, texrovací koordináty a číslo gl_DrawID.

    inFragment.attributes[0].v3 - pozice 
    inFragment.attributes[1].v3 - normala
    inFragment.attributes[2].v2 - texturovaci souřadnice
    inFragment.attributes[3].u1 - gl_DrawID

    Shader ma přístup k uniformním proměnným a texturám:
    si.uniforms[1].v3 - pozice světla
    si.uniforms[2].v3 - pozice kamery
    si.uniforms[)."<<drawCallUniformOffset<<R".(+gl_DrawID*)."<<drawCallNofUniforms<<R".(+)."<<DrawCallUniform::DIFF_COLOR  <<R".(].v4 - difuzní barva materialu, - pokud není textura
    si.uniforms[)."<<drawCallUniformOffset<<R".(+gl_DrawID*)."<<drawCallNofUniforms<<R".(+)."<<DrawCallUniform::TEXTURE_ID  <<R".(].i1 - číslo textury nebo -1 pokud textura není.
    si.uniforms[)."<<drawCallUniformOffset<<R".(+gl_DrawID*)."<<drawCallNofUniforms<<R".(+)."<<DrawCallUniform::DOUBLE_SIDED<<R".(].v1 - 0.f znamená, že je to jednostraný povrch, 1.f znamená, že je to doubleSided

    výstupní barva by měla být zapsána do proměnné outFragment.gl_FragColor

    Lambertův osvětlovcí model počítá barvu ze dvou složek: ambientní a diffusní.

    ambietní složka
    aL = dC*aF;
    difuzní složka
    dL = dC*dF;

    výsledná barva je:
    vec4(aL+dL,dC.a);

    dC je difuzní barva matriálu, je to buď hodnota z textury nebo z uniformní proměnné, když není textura k dispozici.
    Pro čtení z textur použijte funkci read_texture(Texture const&tex,glm::vec2 const&coord);

    aF je ambietní faktor == )."<<aF<<R".( v našem případě.
    dF je difuzní faktor
    difuzní faktor je spočítán jako 
    skalární součin vektoru z fragmentu do světla a normály fragmentu ořezaný do nezáporných hodnot.
    dF = glm::clamp(glm::dot(L,N),0.f,1.f)

    Pozor oba vektory musí být normalizované!
    pro normalizaci můžete využít funkci: glm::normalize()

    Pokud je zapnutý příznak doubleSided, je potřeba otočit normálu, pokud směruje od kamery.
    Tzn. vektor od kamery k povrchu (atribut 0) má stejný směr jako normála.

    dC.a je alpha - neprůhlednost

    Něco se pokazilo...)."<<std::endl;
  };

  if(!equalVec4(expected,color)){
    printInfo();
    std::cerr << R".(
    Barva měla být: )."<<str(expected)<<R".(
    Ale byla: )."<<str(color)<<R".(

    Pozice                 fragmentu byla: )."<<str(pos      )<<R".(
    Normála                fragmentu byla: )."<<str(nor      )<<R".(
    Texturovací souřadnice fragmentu byly: )."<<str(coord    )<<R".(
    Pozice světla byla                   : )."<<str(light    )<<R".(
    Pozice kamery byla                   : )."<<str(cam      )<<R".(
    Diffuzní barva byla                  : )."<<str(diffC    )<<R".(
    textureID byl                        : )."<<str(textureID)<<R".(
    Barva v textuře byla                 : )."<<(textureID<0?std::string("nebyla textura"):str(read_texture(texs[textureID],coord)))<<R".(
    Přiznak doubleSided byl              : )."<<str(doubleSided)<<R".(

    ).";
    REQUIRE(false);
  }
}

SCENARIO("40"){
  std::cerr << "40 - drawModel_VertexShader" << std::endl;


  InVertex inV;
  OutVertex outV;

  auto proj  = glm::perspective(glm::radians(60.f),1.f,1.f,100.f);
  auto view  = glm::lookAt(glm::vec3(100.f,120.f,-30.f),glm::vec3(0.f,0.f,10.f),glm::vec3(0.f,1.f,0.f));
  auto model = glm::translate(glm::rotate(glm::scale(glm::mat4(1.f),glm::vec3(3.f,5.f,-3.f)),glm::radians(5.f),glm::vec3(1.f,2.f,1.f)),glm::vec3(10.f,30.f,-4.f));
  auto itm   = glm::transpose(glm::inverse(model));
  auto pos   = glm::vec3(1.f,2.f,-3.f);
  auto nor   = glm::vec3(glm::cos(1)*glm::cos(2),glm::sin(1),glm::sin(1)*glm::cos(2));
  auto coord = glm::vec2(0.3,0.8);

  inV.attributes[0].v3 = pos;
  inV.attributes[1].v3 = nor;
  inV.attributes[2].v2 = coord;
  inV.gl_DrawID        = 13;


  Uniform uniforms[GPUMemory::maxUniforms];

  ShaderInterface si;
  si.uniforms = uniforms;

  uniforms[0].m4 = proj*view;
  uniforms[drawCallUniforms(inV.gl_DrawID)+DrawCallUniform::MODEL    ].m4 = model;
  uniforms[drawCallUniforms(inV.gl_DrawID)+DrawCallUniform::INV_MODEL].m4 = itm  ;


  drawModel_vertexShader(outV,inV,si);

  glm::vec4 egl_Position = proj*view*model*glm::vec4(pos,1.f);
  glm::vec3 epos         = glm::vec3(model*glm::vec4(pos,1.f));
  glm::vec3 enor         = glm::vec3(itm*glm::vec4(nor,0.f));
  glm::vec2 ecoord       = coord;

  auto printInfo = [&](){
    std::cerr<<R".(

    Tento test zkouší, zda je funkce drawModel_vertexShader správně naimplementována.
    Tato funkce reprezentuje points shader.
    Funkce points shaderu je ztransformovat vstupní vrchol pomocí matic na výstupní vrchol.
    Vstupní vrchol je tvořen třemi atributy: pozice, normála, texturovací koordináty.
    Vstupní vrchol má také informace o čísle vrcholu: gl_VertexID a čísle vykreslovacího příkazu: gl_DrawID.

    inVertex.attributes[0].v3 // pozice
    inVertex.attributes[1].v3 // normala
    inVertex.attributes[2].v2 // tex. coordinaty

    Shader ma přístup k třem maticím: viewProjectionMatrix, modelMatrix, invModelMatrix
    viewProjectionMatrix je výsledkem násobení projection*view.
    invModelMatrix je inverzní transponovaná modelová matice.
    viewProjectionMatrix je jako jediná stejná pro celý model.
    modelMatrix, invModelMatrix se liší pro každý vykreslovací příkaz.

    si.uniforms[0].m4 // viewProjectionMatrix
    si.uniforms[)."<<drawCallUniformOffset<<R".(+gl_Draw*)."<<drawCallNofUniforms<<R".(+)."<<DrawCallUniform::MODEL<<R".(].m4 // modelMatrix
    si.uniforms[)."<<drawCallUniformOffset<<R".(+gl_Draw*)."<<drawCallNofUniforms<<R".(+)."<<DrawCallUniform::INV_MODEL<<R".(].m4 // invModelMatrix

    Výstup shaderu je pozice výstupního vrcholu ve clip space - gl_Position a 
    čtyři atributy: pozice ve world-space, normála ve world space, texturovací souřadnice a gl_DrawID.
    outVertex.gl_Position      = ...; // pozice v clip-space
    outVertex.attributes[0].v3 = ...; // pozice ve world-space
    outVertex.attributes[1].v3 = ...; // normala ve world-space
    outVertex.attributes[2].v2 = ...; // texturovací souřadnice
    outVertex.attributes[3].u1 = ...; // gl_DrawID

    gl_DrawID by se neměl interpolovat pomocí barycentrických souřadnic, protože je to integer.
    Tato hodnota se použije pro výběr textury a dalších materiálovách vlastností.

    Pamatujte, pro transformaci pozice do clip-space je potřeba pronásobit pozici projekční, view i modelovou maticí.

    Normály by se měly násobit inverzní transponovanou maticí.
    Normály by měly mít homogenní složku = 0 protože to jsou vektory.

    Něco se pokazilo...)."<<std::endl;
  };

  if(!equalVec4(egl_Position,outV.gl_Position)){
    printInfo();
    std::cerr << R".(
    Pozice gl_Position je špatně.
    Měla být: )."<<str(egl_Position)<<R".(
    Ale  je : )."<<str(outV.gl_Position)<<R".(
    ).";
    REQUIRE(false);
  }

  if(!equalVec3(epos,outV.attributes[0].v3)){
    printInfo();
    std::cerr << R".(
    Pozice ve world space je špatně.
    Měla být: )."<<str(epos)<<R".(
    Ale  je : )."<<str(outV.attributes[0].v3)<<R".(
    ).";
    REQUIRE(false);
  }

  if(!equalVec3(enor,outV.attributes[1].v3)){
    printInfo();
    std::cerr << R".(
    Normála ve world space je špatně.
    Měla být: )."<<str(enor)<<R".(
    Ale  je : )."<<str(outV.attributes[1].v3)<<R".(
    ).";
    REQUIRE(false);
  }

  if(!equalVec2(ecoord,outV.attributes[2].v2)){
    printInfo();
    std::cerr << R".(
    Texturovací koordináty jsou špatně.
    Měly být: )."<<str(ecoord)<<R".(
    Ale  je : )."<<str(outV.attributes[2].v2)<<R".(
    ).";
    REQUIRE(false);
  }

  if(inV.gl_DrawID != outV.attributes[3].u1){
    printInfo();
    std::cerr << R".(
    gl_DrawID je špatně.
    Měl být: )." << str(inV.gl_DrawID        )<<R".(
    Ale je : )." << str(outV.attributes[3].u1)<<R".(
    ).";
    REQUIRE(false);
  }


}


SCENARIO("41"){
  std::cerr << "41 - drawModel_fragmentShader" << std::endl;

  auto tex0 = TextureData(100,100,3);
  auto tex1 = TextureData(100,100,3);

  auto genTex0 = [&](TextureData&d){
    for(uint32_t y=0;y<d.height;++y)
      for(uint32_t x=0;x<d.width;++x)
        for(uint32_t c=0;c<d.channels;++c){
          auto uv = glm::vec2(x,y)/glm::vec2(d.width,d.height);
          d.data[(y*d.width+x)*d.channels+c]=(uint8_t)(glm::clamp(glm::sin(uv.x+uv.y*c),0.f,1.f)*255.f);
        }
  };

  genTex0(tex1);

  std::vector<Texture>texs;
  texs.push_back(tex0.getTexture());
  texs.push_back(tex1.getTexture());

  std::vector<uint32_t>drawIds = {0,1,10,23,24};

  float aF = 0.2f;
  for(uint32_t f=0;f<2;++f){//flip normal
    float flip = f==0?1.f:-1.f;
    for(uint32_t d=0;d<2;++d){
      float doubleSided = (float)d;
      for(int32_t textureID=-1;textureID<2;++textureID){//isTexture
        for(auto drawID:drawIds){
          testLambert(glm::vec3(0.f)     ,flip*glm::vec3(0,1,0)   ,glm::vec2(0.3,0.6)  ,drawID,textureID,glm::vec3(0,1,0)   ,glm::vec4(.5),aF,texs,doubleSided,glm::vec3(0.f,1.f,0.f));
          testLambert(glm::vec3(0.f)     ,flip*glm::vec3(0,1000,0),glm::vec2(0.2,0.8)  ,drawID,textureID,glm::vec3(0,1,0)   ,glm::vec4(.5),aF,texs,doubleSided,glm::vec3(0.f,1.f,0.f));
          testLambert(glm::vec3(0.f)     ,flip*glm::vec3(0,1,0)   ,glm::vec2(0.5,0.5)  ,drawID,textureID,glm::vec3(0,1000,0),glm::vec4(.5),aF,texs,doubleSided,glm::vec3(0.f,1.f,0.f));
          testLambert(glm::vec3(0.f)     ,flip*glm::vec3(0,1000,0),glm::vec2(0.2,0.6)  ,drawID,textureID,glm::vec3(0,1000,0),glm::vec4(.5),aF,texs,doubleSided,glm::vec3(0.f,1.f,0.f));
          testLambert(glm::vec3(0,-1.f,0),flip*glm::vec3(0,1,0)   ,glm::vec2(0.9,0.1)  ,drawID,textureID,glm::vec3(0,0,0)   ,glm::vec4(.5),aF,texs,doubleSided,glm::vec3(0.f,1.f,0.f));
          testLambert(glm::vec3(0,0.f,0) ,flip*glm::vec3(1,0,0)   ,glm::vec2(0.55,0.45),drawID,textureID,glm::vec3(0,1,0)   ,glm::vec4(.5),aF,texs,doubleSided,glm::vec3(1.f,0.f,0.f));
          testLambert(glm::vec3(0,0.f,0) ,flip*glm::vec3(0,-1,0)  ,glm::vec2(0.45,0.55),drawID,textureID,glm::vec3(0,1,0)   ,glm::vec4(.5),aF,texs,doubleSided,glm::vec3(0.f,-1.f,0.f));
        }
      }
    }
  }

}
