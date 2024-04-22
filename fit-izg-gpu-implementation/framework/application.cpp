/*!
 * @file
 * @brief This file contains application class implementation
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 */

#include <assert.h>
#include <framework/application.hpp>



void defaultSceneParameters(
    basicCamera::OrbitCamera&orbit,
    basicCamera::PerspectiveCamera&proj,
    glm::vec3&light,
    uint32_t w,uint32_t h){
  orbit.addDistance(35.f);
  orbit.addYAngle(glm::radians(-20.f));
  proj.setNear(0.1f);
  proj.setFovy(glm::pi<float>()/4.f);
  auto const aspect = static_cast<float>(w) / static_cast<float>(h);
  proj.setAspect(aspect);
  light  = glm::vec3(100.f,100.f,100.f);
}

/**
 * @brief Constructor
 *
 * @param width width of the window
 * @param height height of the window
 */
Application::Application(int32_t width,int32_t height):Window(width,height,"izgProject"),windowSize(width,height){
  setIdleCallback([&](){idle();});
  setWindowCallback(SDL_WINDOWEVENT_RESIZED,[&](SDL_Event const&event){resize     (event);});
  setCallback      (SDL_MOUSEMOTION        ,[&](SDL_Event const&event){mouseMotion(event);});
  setCallback      (SDL_KEYDOWN            ,[&](SDL_Event const&event){keyDown    (event);});
  defaultSceneParameters(orbitCamera,perspectiveCamera,light,width,height);
  timer.reset();
}

/**
 * @brief Destructor
 */
Application::~Application(){}

    
/**
 * @brief Starts the main loop
 */
void Application::start(){
  mainLoop();
}

/**
 * @brief This function selects a rendering method
 *
 * @param m method id
 */
void Application::setMethod(uint32_t m){
  auto&mr=ProgramContext::get().methods;
  auto const nofMethods = mr.methodFactories.size();
  if(m >= nofMethods) m = static_cast<uint32_t>(nofMethods - 1);
  mr.selectedMethod = m;
}

void Application::createMethodIfItDoesNotExist(){
  auto&mr=ProgramContext::get().methods;
  if(mr.method)return;
  int w,h;
  SDL_GetWindowSize(getWindow(),&w,&h);
  framebuffer = std::make_shared<Framebuffer>(w,h);

  mr.method = mr.methodFactories[mr.selectedMethod](&*mr.methodConstructData[mr.selectedMethod]);
  SDL_SetWindowTitle(getWindow(),mr.methodName.at(mr.selectedMethod).c_str());
}

void Application::idle(){
  auto&mr=ProgramContext::get().methods;
  createMethodIfItDoesNotExist();

  mr.method->onUpdate(timer.elapsedFromLast());

  SceneParam sceneParam;
  sceneParam.proj   = perspectiveCamera.getProjection();
  sceneParam.view   = orbitCamera      .getView      ();
  sceneParam.camera = glm::vec3(glm::inverse(sceneParam.view)*glm::vec4(0.f,0.f,0.f,1.f));
  sceneParam.light  = light;

  auto frame = framebuffer->getFrame();
  mr.method->onDraw(frame,sceneParam);

  swap();
}

void Application::resize(SDL_Event const&event){
  auto&mr=ProgramContext::get().methods;
  auto const width  = event.window.data1;
  auto const height = event.window.data2;
  auto const aspect = static_cast<float>(width) / static_cast<float>(height);
  perspectiveCamera.setAspect(aspect);
  if(mr.method){
    framebuffer->resize(event.window.data1,event.window.data2);
  }
  reInitRenderer();
}

void Application::mouseMotionLMask(uint32_t mState,float xrel,float yrel){
  if((mState & SDL_BUTTON_LMASK) == 0)return;
  orbitCamera.addXAngle(yrel * sensitivity);
  orbitCamera.addYAngle(xrel * sensitivity);
}

void Application::mouseMotionRMask(uint32_t mState,float yrel){
  if ((mState & SDL_BUTTON_RMASK) == 0)return;
  orbitCamera.addDistance(yrel * orbitZoomSpeed);
}

void Application::mouseMotionMMask(uint32_t mState,float xrel,float yrel){
  if ((mState & SDL_BUTTON_MMASK) == 0) return;
  orbitCamera.addXPosition(+orbitCamera.getDistance() * xrel /
                            float(windowSize.x) * 2.f);
  orbitCamera.addYPosition(-orbitCamera.getDistance() * yrel /
                            float(windowSize.y) * 2.f);
}

void Application::mouseMotion(SDL_Event const&event){
  auto const xrel   = static_cast<float>(event.motion.xrel);
  auto const yrel   = static_cast<float>(event.motion.yrel);
  auto const mState = event.motion.state;
  mouseMotionLMask(mState,xrel,yrel);
  mouseMotionRMask(mState,yrel);
  mouseMotionMMask(mState,xrel,yrel);
}

void Application::nextMethod(uint32_t key){
  auto&mr=ProgramContext::get().methods;
  if (key != SDLK_n)return;
  auto const nofMethods = mr.methodFactories.size();
  mr.selectedMethod++;
  if(mr.selectedMethod >= nofMethods)mr.selectedMethod=0;
  mr.method = nullptr;
}

void Application::prevMethod(uint32_t key){
  auto&mr=ProgramContext::get().methods;
  if (key != SDLK_p)return;
  auto const nofMethods = mr.methodFactories.size();
  if(mr.selectedMethod > 0)mr.selectedMethod--;
  else mr.selectedMethod = nofMethods-1;
  mr.method = nullptr;
}

void Application::quit      (uint32_t key){
  if (key != SDLK_ESCAPE)return;
  running = false;
}

void Application::keyDown(SDL_Event const&event){
  auto key = event.key.keysym.sym;
  nextMethod(key);
  prevMethod(key);
  quit      (key);
  float speed = 1;
  if(event.key.keysym.mod&KMOD_LSHIFT)speed = .1f;
  if(key == SDLK_a)this->orbitCamera.addXPosition(+speed);
  if(key == SDLK_d)this->orbitCamera.addXPosition(-speed);
  if(key == SDLK_w)this->orbitCamera.addZPosition(+speed);
  if(key == SDLK_s)this->orbitCamera.addZPosition(-speed);
  if(key == SDLK_e)this->orbitCamera.addYPosition(-speed);
  if(key == SDLK_q)this->orbitCamera.addYPosition(+speed);
}

void Application::swap(){
  auto       frame = framebuffer->color.data();
  auto const w     = framebuffer->width;
  auto const h     = framebuffer->height; 

  copyToSDLSurface(surface,frame,w,h);
}

void copyToSDLSurface(SDL_Surface*surface,uint8_t const*const frame,uint32_t width,uint32_t height){
  uint32_t const bitsPerByte    = 8;
  uint32_t const swizzleTable[] = {
      surface->format->Rshift / bitsPerByte,
      surface->format->Gshift / bitsPerByte,
      surface->format->Bshift / bitsPerByte,
  };

  uint8_t* const  pixels      = (uint8_t*)surface->pixels;
  for (size_t y = 0; y < height; ++y) {
    size_t const reversedY = height - y - 1;
    for (size_t x = 0; x < width; ++x) {
      auto const color    = frame + (y*width+x)*4;
      auto const dstPixel = pixels + reversedY * surface->pitch + x * surface->format->BytesPerPixel;
      for (uint32_t c = 0; c < 3; ++c)
        dstPixel[swizzleTable[c]] = color[c];
    }
  }
}

