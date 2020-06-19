/*!
 * @file
 * @brief This file contains class that represents graphic card.
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 * @author Ondřej Ondryáš, xondry02@stud.fit.vutbr.cz
 * @date 2020-05-11
 */
#pragma once

#include <student/fwd.hpp>
#include <vector>

struct VertexPullerHead {
    BufferID bufferId;
    uint64_t offset;
    uint64_t stride;
    AttributeType type;
    bool enabled;
};

struct VertexPullerSettings {
    BufferID indexingBufferId;
    IndexType indexType;
    VertexPullerHead heads[maxAttributes];
};

struct ProgramSettings {
    VertexShader vs;
    FragmentShader fs;
    Uniforms uniforms;
    AttributeType vs2fsTypes[maxAttributes];
};

struct Triangle {
    OutVertex vertices[3];
};

/**
 * @brief This class represent software GPU
 */
class GPU{
  public:
    GPU();
    virtual ~GPU();

    //buffer object commands
    BufferID  createBuffer           (uint64_t size);
    void      deleteBuffer           (BufferID buffer);
    void      setBufferData          (BufferID buffer,uint64_t offset,uint64_t size,void const* data);
    void      getBufferData          (BufferID buffer,uint64_t offset,uint64_t size,void      * data);
    bool      isBuffer               (BufferID buffer);

    //vertex array object commands (vertex puller)
    ObjectID  createVertexPuller     ();
    void      deleteVertexPuller     (VertexPullerID vao);
    void      setVertexPullerHead    (VertexPullerID vao,uint32_t head,AttributeType type,uint64_t stride,uint64_t offset,BufferID buffer);
    void      setVertexPullerIndexing(VertexPullerID vao,IndexType type,BufferID buffer);
    void      enableVertexPullerHead (VertexPullerID vao,uint32_t head);
    void      disableVertexPullerHead(VertexPullerID vao,uint32_t head);
    void      bindVertexPuller       (VertexPullerID vao);
    void      unbindVertexPuller     ();
    bool      isVertexPuller         (VertexPullerID vao);

    //program object commands
    ProgramID createProgram          ();
    void      deleteProgram          (ProgramID prg);
    void      attachShaders          (ProgramID prg,VertexShader vs,FragmentShader fs);
    void      setVS2FSType           (ProgramID prg,uint32_t attrib,AttributeType type);
    void      useProgram             (ProgramID prg);
    bool      isProgram              (ProgramID prg);
    void      programUniform1f       (ProgramID prg,uint32_t uniformId,float     const&d);
    void      programUniform2f       (ProgramID prg,uint32_t uniformId,glm::vec2 const&d);
    void      programUniform3f       (ProgramID prg,uint32_t uniformId,glm::vec3 const&d);
    void      programUniform4f       (ProgramID prg,uint32_t uniformId,glm::vec4 const&d);
    void      programUniformMatrix4f (ProgramID prg,uint32_t uniformId,glm::mat4 const&d);

    //framebuffer functions
    void      createFramebuffer      (uint32_t width,uint32_t height);
    void      deleteFramebuffer      ();
    void      resizeFramebuffer      (uint32_t width,uint32_t height);
    uint8_t*  getFramebufferColor    ();
    float*    getFramebufferDepth    ();
    uint32_t  getFramebufferWidth    ();
    uint32_t  getFramebufferHeight   ();

    //execution commands
    void      clear                  (float r,float g,float b,float a);
    void      drawTriangles          (uint32_t  nofVertices);
private:
    uint8_t *colorBuf;
    float *depthBuf;
    uint32_t fbWidth;
    uint32_t fbHeight;

    std::vector<void *> bufferStore;
    std::vector<void *> vaoStore;
    std::vector<void *> programStore;

    BufferID nextBufferId;
    ObjectID nextVaoId;
    ProgramID nextProgId;

    ObjectID boundVao;
    ProgramID boundProgram;

    ObjectID allocateNext(std::vector<void *> &store, ObjectID &nextId, uint64_t size, bool clear);
    void deleteObj(std::vector<void *> &store, ObjectID toDelete);

    InVertex assembleInVertex(uint32_t vertex, const VertexPullerSettings *vps, const ProgramSettings *prog, const void *indexBuffer);
    void clipTriangle(std::vector<Triangle> &arr, const Triangle &t, const VertexPullerSettings *vps);
    void transformToScreenSpace(Triangle &triangle);
    void rasterize(const std::vector<Triangle> &arr, const ProgramSettings *prog);
};