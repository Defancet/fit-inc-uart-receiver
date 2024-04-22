#pragma once

#include<framework/method.hpp>
#include<functional>
#include<memory>
#include<framework/arguments.hpp>


template<typename CLASS>
void registerMethod(std::string const&name,std::shared_ptr<MethodConstructionData>const&mcd = nullptr);

class MethodDatabase{
  private:
    using MethodFactory = std::function<std::shared_ptr<Method>(MethodConstructionData const*)>;
    std::vector<MethodFactory>     methodFactories                              ;
    std::vector<std::string>       methodName                                   ;
    std::vector<std::shared_ptr<MethodConstructionData>> methodConstructData    ;
    size_t                         selectedMethod    = 0                        ;
    std::shared_ptr<Method>        method                                       ;

    template<typename CLASS>
    friend
    void registerMethod(std::string const&name,std::shared_ptr<MethodConstructionData>const&mcd);
    friend class Application;
};

class ProgramContext{
  public:
    static ProgramContext&get();
    ProgramContext();
    ~ProgramContext();

    Arguments args;
    MethodDatabase methods;
  protected:
    ProgramContext(ProgramContext &other) = delete;
    void operator=(const ProgramContext &) = delete;
    static ProgramContext*reg;
};

template<typename CLASS>
void registerMethod(std::string const&name,std::shared_ptr<MethodConstructionData>const&mcd){
  
  auto factory = [&](MethodConstructionData const*m){
    return std::make_shared<CLASS>(m);
  };
  
  auto&mr = ProgramContext::get().methods;
  mr.methodFactories.push_back(factory);
  mr.methodName.push_back(name);
  mr.methodConstructData.push_back(mcd);
}




struct EntryPoint{
  template<typename T>
  EntryPoint(T t){
    t();
  }
};
