/*!
 * @file
 * @brief This file contains implementation of gpu
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 */

#include <student/gpu.hpp>

void clear(GPUMemory &mem, ClearCommand cmd) {
    if (cmd.clearColor) {
        for (uint32_t i = 0; i < mem.framebuffer.width * mem.framebuffer.height; i++) {
            mem.framebuffer.color[i * mem.framebuffer.channels + 0] = (uint8_t) (cmd.color.r * 255.0f);
            mem.framebuffer.color[i * mem.framebuffer.channels + 1] = (uint8_t) (cmd.color.g * 255.0f);
            mem.framebuffer.color[i * mem.framebuffer.channels + 2] = (uint8_t) (cmd.color.b * 255.0f);
            mem.framebuffer.color[i * mem.framebuffer.channels + 3] = (uint8_t) (cmd.color.a * 255.0f);
        }
    }

    if (cmd.clearDepth) {
        for (uint32_t i = 0; i < mem.framebuffer.width * mem.framebuffer.height; i++) {
            mem.framebuffer.depth[i] = cmd.depth;
        }
    }
}

uint32_t computeVertexID(GPUMemory &mem, VertexArray const &vao, uint32_t shaderInvocation) {
    if (vao.indexBufferID < 0) {
        return shaderInvocation;
    }

    if (vao.indexType == IndexType::UINT8) {
        return ((uint8_t *) ((uint8_t *) mem.buffers[vao.indexBufferID].data + vao.indexOffset))[shaderInvocation];
    }
    if (vao.indexType == IndexType::UINT16) {
        return ((uint16_t *) ((uint8_t *) mem.buffers[vao.indexBufferID].data + vao.indexOffset))[shaderInvocation];
    }
    if (vao.indexType == IndexType::UINT32) {
        return ((uint32_t *) ((uint8_t *) mem.buffers[vao.indexBufferID].data + vao.indexOffset))[shaderInvocation];
    }
    return shaderInvocation;
}

void readAttributes(GPUMemory &mem, DrawCommand &cmd, InVertex &inVertex) {
    for (uint32_t i = 0; i < maxAttributes; i++) {
        if (cmd.vao.vertexAttrib[i].type == AttributeType::EMPTY) {
            continue;
        }

        if (cmd.vao.vertexAttrib[i].type == AttributeType::FLOAT) {
            inVertex.attributes[i].v1 = *(float *) (((uint8_t *) mem.buffers[cmd.vao.vertexAttrib[i].bufferID].data)
            +cmd.vao.vertexAttrib[i].offset + cmd.vao.vertexAttrib[i].stride * inVertex.gl_VertexID);
        }
        if (cmd.vao.vertexAttrib[i].type == AttributeType::VEC2) {
            inVertex.attributes[i].v2 = *(glm::vec2 *) (((uint8_t *) mem.buffers[cmd.vao.vertexAttrib[i].bufferID].data)
            +cmd.vao.vertexAttrib[i].offset + cmd.vao.vertexAttrib[i].stride * inVertex.gl_VertexID);
        }
        if (cmd.vao.vertexAttrib[i].type == AttributeType::VEC3) {
            inVertex.attributes[i].v3 = *(glm::vec3 *) (((uint8_t *) mem.buffers[cmd.vao.vertexAttrib[i].bufferID].data)
            +cmd.vao.vertexAttrib[i].offset + cmd.vao.vertexAttrib[i].stride * inVertex.gl_VertexID);
        }
        if (cmd.vao.vertexAttrib[i].type == AttributeType::VEC4) {
            inVertex.attributes[i].v4 = *(glm::vec4 *) (((uint8_t *) mem.buffers[cmd.vao.vertexAttrib[i].bufferID].data)
            +cmd.vao.vertexAttrib[i].offset + cmd.vao.vertexAttrib[i].stride * inVertex.gl_VertexID);
        }
    }
}

void
runVertexAssembly(GPUMemory &mem, DrawCommand cmd, Program const &prg, InVertex &inVertex, OutVertex &outVertex,
                  ShaderInterface const &si,
                  VertexArray const &vao, uint32_t shaderInvocation, uint32_t drawID) {
    for (uint32_t i = 0; i < 3; i++) {
        inVertex.gl_VertexID = computeVertexID(mem, vao, shaderInvocation + i);
        inVertex.gl_DrawID = drawID;
        readAttributes(mem, cmd, inVertex);
        prg.vertexShader(outVertex, inVertex, si);
    }
}

void draw(GPUMemory &mem, DrawCommand cmd, uint32_t drawID) {
    Program const &prg = mem.programs[cmd.programID];

    InVertex inVertex;
    OutVertex outVertex;

    for (uint32_t i = 0; i < cmd.nofVertices; i += 3) {
        ShaderInterface si;
        si.uniforms = mem.uniforms;
        si.textures = mem.textures;
        runVertexAssembly(mem, cmd, prg, inVertex, outVertex, si, cmd.vao, i, drawID);
    }
}

//! [gpu_execute]
void gpu_execute(GPUMemory&mem,CommandBuffer &cb){
    (void)mem;
    (void)cb;
    uint32_t drawID = 0;

    for (uint32_t i = 0; i < cb.nofCommands; i++) {
        CommandType type = cb.commands[i].type;
        CommandData data = cb.commands[i].data;

        if (type == CommandType::CLEAR) {
            clear(mem, data.clearCommand);
        }
        if (type == CommandType::DRAW) {
            draw(mem, data.drawCommand, drawID++);
        }
    }
    /// \todo Tato funkce reprezentuje funkcionalitu grafické karty.<br>
    /// Měla by umět zpracovat command buffer, čistit framebuffer a kresli.<br>
    /// mem obsahuje paměť grafické karty.
    /// cb obsahuje command buffer pro zpracování.
    /// Bližší informace jsou uvedeny na hlavní stránce dokumentace.
}
//! [gpu_execute]

/**
 * @brief This function reads color from texture.
 *
 * @param texture texture
 * @param uv uv coordinates
 *
 * @return color 4 floats
 */
glm::vec4 read_texture(Texture const&texture,glm::vec2 uv){
  if(!texture.data)return glm::vec4(0.f);
  auto uv1 = glm::fract(uv);
  auto uv2 = uv1*glm::vec2(texture.width-1,texture.height-1)+0.5f;
  auto pix = glm::uvec2(uv2);
  //auto t   = glm::fract(uv2);
  glm::vec4 color = glm::vec4(0.f,0.f,0.f,1.f);
  for(uint32_t c=0;c<texture.channels;++c)
    color[c] = texture.data[(pix.y*texture.width+pix.x)*texture.channels+c]/255.f;
  return color;
}

