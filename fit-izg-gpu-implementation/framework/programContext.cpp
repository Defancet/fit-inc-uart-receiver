#include<framework/programContext.hpp>

ProgramContext&ProgramContext::get(){
  if(!reg)reg = new ProgramContext();
  return *reg;
}

ProgramContext::ProgramContext(){}
ProgramContext::~ProgramContext(){
  delete reg;
}

ProgramContext*ProgramContext::reg = nullptr;
