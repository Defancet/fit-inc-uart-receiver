#pragma once

#include<string>
#include<student/fwd.hpp>

namespace tests{
std::string gpuMemoryToStr(size_t p,GPUMemory const&mem,CommandBuffer const&cb);
}
