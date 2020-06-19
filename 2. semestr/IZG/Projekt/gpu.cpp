/*!
 * @file
 * @brief This file contains implementation of gpu
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 * @author Ondřej Ondryáš, xondry02@stud.fit.vutbr.cz
 * @date 2020-05-11
 */

#include <student/gpu.hpp>
#include <cstring>
#include <iostream>
#include <glm/gtx/extended_min_max.hpp>

/**
 * @brief Constructor of GPU
 */
GPU::GPU() {
    bufferStore = std::vector<void *>();
    vaoStore = std::vector<void *>();
    programStore = std::vector<void *>();

    nextBufferId = 0;
    nextVaoId = 0;
    nextProgId = 0;

    boundProgram = emptyID;
    boundVao = emptyID;

    colorBuf = nullptr;
    depthBuf = nullptr;
    fbHeight = 0;
    fbWidth = 0;
}

/**
 * @brief Destructor of GPU
 */
GPU::~GPU() {
    for (const auto &p : bufferStore) {
        if (p != nullptr) {
            free(p);
        }
    }

    for (const auto &p : vaoStore) {
        if (p != nullptr) {
            free(p);
        }
    }

    for (const auto &p : programStore) {
        if (p != nullptr) {
            free(p);
        }
    }

    if (colorBuf != nullptr) {
        free(colorBuf);
    }

    if (depthBuf != nullptr) {
        free(depthBuf);
    }
}

ObjectID GPU::allocateNext(std::vector<void *> &store, ObjectID &nextId, uint64_t size, bool clear) {
    // TODO: come up with a better, faster solution...
    // I could set the nextId in the corresponding delete method to fill it better, maybe... w/e
    void *mem = malloc(size);
    if (mem == nullptr) {
        throw std::bad_alloc();
    }

    if (clear) {
        memset(mem, 0, size);
    }

    if (nextId == store.size()) {
        store.push_back(mem);
        nextId++;

        if (nextId == emptyID) { // :doubt:
            store.push_back(nullptr);
            nextId++;
        }

        if (nextId == -1) {
            nextId = 0;
            return nextId - 2;
        }

        return nextId - 1;
    } else {
        // Find a spot in the buffer store that is empty
        while (!((nextId != emptyID && store[nextId] == nullptr)
                 || nextId == -1)) {
            nextId++;
        }

        if (nextId == -1) {
            // We've run out of IDs. This is the absolute edge case.
            throw std::bad_alloc();
        }

        store[nextId] = mem;
        return nextId;
    }
}

void GPU::deleteObj(std::vector<void *> &store, ObjectID toDelete) {
    if (toDelete >= store.size()) {
        return;
    }

    void *mem = store[toDelete];
    if (mem == nullptr) {
        return;
    }

    free(mem);
    store[toDelete] = nullptr;
}


/**
 * @brief This function allocates buffer on GPU.
 *
 * @param size size in bytes of new buffer on GPU.
 *
 * @return unique identificator of the buffer
 */
BufferID GPU::createBuffer(uint64_t size) {
    return allocateNext(bufferStore, nextBufferId, size, false);
}

/**
 * @brief This function frees allocated buffer on GPU.
 *
 * @param buffer buffer identificator
 */
void GPU::deleteBuffer(BufferID buffer) {
    deleteObj(bufferStore, buffer);
}

/**
 * @brief This function uploads data to selected buffer on the GPU
 *
 * @param buffer buffer identificator
 * @param offset specifies the offset into the buffer's data
 * @param size specifies the size of buffer that will be uploaded
 * @param data specifies a pointer to new data
 */
void GPU::setBufferData(BufferID buffer, uint64_t offset, uint64_t size, void const *data) {
    if (buffer >= bufferStore.size()) {
        return;
    }

    void *mem = bufferStore[buffer];
    if (mem == nullptr) {
        return;
    }

    memcpy((char *) mem + offset, data, size);
}

/**
 * @brief This function downloads data from GPU.
 *
 * @param buffer specfies buffer
 * @param offset specifies the offset into the buffer from which data will be returned, measured in bytes. 
 * @param size specifies data size that will be copied
 * @param data specifies a pointer to the location where buffer data is returned. 
 */
void GPU::getBufferData(BufferID buffer,
                        uint64_t offset,
                        uint64_t size,
                        void *data) {

    if (buffer >= bufferStore.size()) {
        return;
    }

    void *mem = bufferStore[buffer];
    if (mem == nullptr) {
        return;
    }

    memcpy(data, (char *) mem + offset, size);
}

/**
 * @brief This function tests if buffer exists
 *
 * @param buffer selected buffer id
 *
 * @return true if buffer points to existing buffer on the GPU.
 */
bool GPU::isBuffer(BufferID buffer) {
    return buffer != emptyID && buffer < bufferStore.size() && bufferStore[buffer] != nullptr;
}

/**
 * @brief This function creates new vertex puller settings on the GPU,
 *
 * @return unique vertex puller identificator
 */
ObjectID GPU::createVertexPuller() {
    size_t vpsTableSize = sizeof(VertexPullerSettings);
    ObjectID o = allocateNext(vaoStore, nextVaoId, vpsTableSize, true);

    reinterpret_cast<VertexPullerSettings *>(vaoStore[o])->indexingBufferId = emptyID;

    return o;
}

/**
 * @brief This function deletes vertex puller settings
 *
 * @param vao vertex puller identificator
 */
void GPU::deleteVertexPuller(VertexPullerID vao) {
    deleteObj(vaoStore, vao);
}

/**
 * @brief This function sets one vertex puller reading head.
 *
 * @param vao identificator of vertex puller
 * @param head id of vertex puller head
 * @param type type of attribute
 * @param stride stride in bytes
 * @param offset offset in bytes
 * @param buffer id of buffer
 */
void GPU::setVertexPullerHead(VertexPullerID vao, uint32_t head, AttributeType type, uint64_t stride, uint64_t offset,
                              BufferID buffer) {
    if (vao >= vaoStore.size()) {
        return;
    }

    void *mem = vaoStore[vao];
    if (mem == nullptr) {
        return;
    }

    auto *set = reinterpret_cast<VertexPullerSettings *>(mem);
    auto h = &set->heads[head];
    h->bufferId = buffer;
    h->type = type;
    h->stride = stride;
    h->offset = offset;
}

/**
 * @brief This function sets vertex puller indexing.
 *
 * @param vao vertex puller id
 * @param type type of index
 * @param buffer buffer with indices
 */
void GPU::setVertexPullerIndexing(VertexPullerID vao, IndexType type, BufferID buffer) {
    if (vao >= vaoStore.size()) {
        return;
    }

    void *mem = vaoStore[vao];
    if (mem == nullptr) {
        return;
    }

    auto *set = reinterpret_cast<VertexPullerSettings *>(mem);
    set->indexType = type;
    set->indexingBufferId = buffer;
}

/**
 * @brief This function enables vertex puller's head.
 *
 * @param vao vertex puller 
 * @param head head id
 */
void GPU::enableVertexPullerHead(VertexPullerID vao, uint32_t head) {
    if (vao >= vaoStore.size()) {
        return;
    }

    void *mem = vaoStore[vao];
    if (mem == nullptr) {
        return;
    }

    auto *set = reinterpret_cast<VertexPullerSettings *>(mem);
    set->heads[head].enabled = true;
}

/**
 * @brief This function disables vertex puller's head
 *
 * @param vao vertex puller id
 * @param head head id
 */
void GPU::disableVertexPullerHead(VertexPullerID vao, uint32_t head) {
    if (vao >= vaoStore.size()) {
        return;
    }

    void *mem = vaoStore[vao];
    if (mem == nullptr) {
        return;
    }

    auto *set = reinterpret_cast<VertexPullerSettings *>(mem);
    set->heads[head].enabled = false;
}

/**
 * @brief This function selects active vertex puller.
 *
 * @param vao id of vertex puller
 */
void GPU::bindVertexPuller(VertexPullerID vao) {
    boundVao = vao;
}

/**
 * @brief This function deactivates vertex puller.
 */
void GPU::unbindVertexPuller() {
    boundVao = emptyID;
}

/**
 * @brief This function tests if vertex puller exists.
 *
 * @param vao vertex puller
 *
 * @return true, if vertex puller "vao" exists
 */
bool GPU::isVertexPuller(VertexPullerID vao) {
    return vao != emptyID && vao < vaoStore.size() && vaoStore[vao] != nullptr;

}

/**
 * @brief This function creates new shader program.
 *
 * @return shader program id
 */
ProgramID GPU::createProgram() {
    return allocateNext(programStore, nextProgId, sizeof(ProgramSettings), true);
}

/**
 * @brief This function deletes shader program
 *
 * @param prg shader program id
 */
void GPU::deleteProgram(ProgramID prg) {
    deleteObj(programStore, prg);
}

/**
 * @brief This function attaches vertex and frament shader to shader program.
 *
 * @param prg shader program
 * @param vs vertex shader 
 * @param fs fragment shader
 */
void GPU::attachShaders(ProgramID prg, VertexShader vs, FragmentShader fs) {
    if (prg >= programStore.size()) {
        return;
    }

    void *mem = programStore[prg];
    if (mem == nullptr) {
        return;
    }

    auto prog = reinterpret_cast<ProgramSettings *>(mem);
    prog->vs = vs;
    prog->fs = fs;
}

/**
 * @brief This function selects which vertex attributes should be interpolated during rasterization into fragment attributes.
 *
 * @param prg shader program
 * @param attrib id of attribute
 * @param type type of attribute
 */
void GPU::setVS2FSType(ProgramID prg, uint32_t attrib, AttributeType type) {
    if (prg >= programStore.size()) {
        return;
    }

    void *mem = programStore[prg];
    if (mem == nullptr) {
        return;
    }

    auto prog = reinterpret_cast<ProgramSettings *>(mem);
    prog->vs2fsTypes[attrib] = type;
}

/**
 * @brief This function actives selected shader program
 *
 * @param prg shader program id
 */
void GPU::useProgram(ProgramID prg) {
    boundProgram = prg;
}

/**
 * @brief This function tests if selected shader program exists.
 *
 * @param prg shader program
 *
 * @return true, if shader program "prg" exists.
 */
bool GPU::isProgram(ProgramID prg) {
    return prg != emptyID && prg < programStore.size() && programStore[prg] != nullptr;
}

/**
 * @brief This function sets uniform value (1 float).
 *
 * @param prg shader program
 * @param uniformId id of uniform value (number of uniform values is stored in maxUniforms variable)
 * @param d value of uniform variable
 */
void GPU::programUniform1f(ProgramID prg, uint32_t uniformId, float const &d) {
    if (prg >= programStore.size()) {
        return;
    }

    void *mem = programStore[prg];
    if (mem == nullptr) {
        return;
    }

    auto prog = reinterpret_cast<ProgramSettings *>(mem);
    prog->uniforms.uniform[uniformId].v1 = d;
}

/**
 * @brief This function sets uniform value (2 float).
 *
 * @param prg shader program
 * @param uniformId id of uniform value (number of uniform values is stored in maxUniforms variable)
 * @param d value of uniform variable
 */
void GPU::programUniform2f(ProgramID prg, uint32_t uniformId, glm::vec2 const &d) {
    if (prg >= programStore.size()) {
        return;
    }

    void *mem = programStore[prg];
    if (mem == nullptr) {
        return;
    }

    auto prog = reinterpret_cast<ProgramSettings *>(mem);
    prog->uniforms.uniform[uniformId].v2 = d;
}

/**
 * @brief This function sets uniform value (3 float).
 *
 * @param prg shader program
 * @param uniformId id of uniform value (number of uniform values is stored in maxUniforms variable)
 * @param d value of uniform variable
 */
void GPU::programUniform3f(ProgramID prg, uint32_t uniformId, glm::vec3 const &d) {
    if (prg >= programStore.size()) {
        return;
    }

    void *mem = programStore[prg];
    if (mem == nullptr) {
        return;
    }

    auto prog = reinterpret_cast<ProgramSettings *>(mem);
    prog->uniforms.uniform[uniformId].v3 = d;
}

/**
 * @brief This function sets uniform value (4 float).
 *
 * @param prg shader program
 * @param uniformId id of uniform value (number of uniform values is stored in maxUniforms variable)
 * @param d value of uniform variable
 */
void GPU::programUniform4f(ProgramID prg, uint32_t uniformId, glm::vec4 const &d) {
    if (prg >= programStore.size()) {
        return;
    }

    void *mem = programStore[prg];
    if (mem == nullptr) {
        return;
    }

    auto prog = reinterpret_cast<ProgramSettings *>(mem);
    prog->uniforms.uniform[uniformId].v4 = d;
}

/**
 * @brief This function sets uniform value (4 float).
 *
 * @param prg shader program
 * @param uniformId id of uniform value (number of uniform values is stored in maxUniforms variable)
 * @param d value of uniform variable
 */
void GPU::programUniformMatrix4f(ProgramID prg, uint32_t uniformId, glm::mat4 const &d) {
    if (prg >= programStore.size()) {
        return;
    }

    void *mem = programStore[prg];
    if (mem == nullptr) {
        return;
    }

    auto prog = reinterpret_cast<ProgramSettings *>(mem);
    prog->uniforms.uniform[uniformId].m4 = d;
}

/**
 * @brief This function creates framebuffer on GPU.
 *
 * @param width width of framebuffer
 * @param height height of framebuffer
 */
void GPU::createFramebuffer(uint32_t width, uint32_t height) {
    if (colorBuf != nullptr) {
        free(colorBuf);
        free(depthBuf);

        colorBuf = nullptr;
        depthBuf = nullptr;
    }

    fbWidth = width;
    fbHeight = height;
    colorBuf = reinterpret_cast<uint8_t *>(calloc(4 * width * height, sizeof(uint8_t)));
    depthBuf = reinterpret_cast<float *>(calloc(width * height, sizeof(float)));

    if (colorBuf == nullptr) {
        if (depthBuf != nullptr) {
            free(depthBuf);
            depthBuf = nullptr;
        }

        throw std::bad_alloc();
    }

    if (depthBuf == nullptr) {
        if (colorBuf != nullptr) {
            free(colorBuf);
            colorBuf = nullptr;
        }

        throw std::bad_alloc();
    }
}

/**
 * @brief This function deletes framebuffer.
 */
void GPU::deleteFramebuffer() {
    if (colorBuf != nullptr) {
        free(colorBuf);
        free(depthBuf);
        colorBuf = nullptr;
        depthBuf = nullptr;
        fbHeight = 0;
        fbWidth = 0;
    }
}

/**
 * @brief This function resizes framebuffer.
 *
 * @param width new width of framebuffer
 * @param height new heght of framebuffer
 */
void GPU::resizeFramebuffer(uint32_t width, uint32_t height) {
    createFramebuffer(width, height);
}

/**
 * @brief This function returns pointer to color buffer.
 *
 * @return pointer to color buffer
 */
uint8_t *GPU::getFramebufferColor() {
    return colorBuf;
}

/**
 * @brief This function returns pointer to depth buffer.
 *
 * @return pointer to dept buffer.
 */
float *GPU::getFramebufferDepth() {
    return depthBuf;
}

/**
 * @brief This function returns width of framebuffer
 *
 * @return width of framebuffer
 */
uint32_t GPU::getFramebufferWidth() {
    return fbWidth;
}

/**
 * @brief This function returns height of framebuffer.
 *
 * @return height of framebuffer
 */
uint32_t GPU::getFramebufferHeight() {
    return fbHeight;
}

/**
 * @brief This function clears framebuffer.
 *
 * @param r red channel
 * @param g green channel
 * @param b blue channel
 * @param a alpha channel
 */
void GPU::clear(float r, float g, float b, float a) {
    uint8_t ri = r * 255;
    uint8_t gi = g * 255;
    uint8_t bi = b * 255;
    uint8_t ai = a * 255;

    uint32_t c = (ai << 24u) | (bi << 16u) | (gi << 8u) | ri;
    uint32_t tot = fbWidth * fbHeight;
    for (uint32_t i = 0; i < tot; i++) {
        *(((uint32_t *) colorBuf) + i) = c;
        depthBuf[i] = 1.1f;
    }
}

InVertex
GPU::assembleInVertex(const uint32_t vertex, const VertexPullerSettings *vps, const ProgramSettings *prog,
                      const void *indexBuffer) {
    InVertex assembledInVertex = InVertex();

    if (indexBuffer == nullptr) {
        assembledInVertex.gl_VertexID = vertex;
    } else {
        switch (vps->indexType) {
            case IndexType::UINT8:
                assembledInVertex.gl_VertexID =
                        ((const uint8_t *) indexBuffer)[vertex];
                break;
            case IndexType::UINT16:
                assembledInVertex.gl_VertexID =
                        ((const uint16_t *) indexBuffer)[vertex];
                break;
            case IndexType::UINT32:
                assembledInVertex.gl_VertexID =
                        ((const uint32_t *) indexBuffer)[vertex];
                break;
        }
    }

    for (uint32_t head = 0; head < maxAttributes; head++) {
        auto &h = vps->heads[head];
        if (!h.enabled) continue;

        char *buf =
                reinterpret_cast<char *>(bufferStore[h.bufferId]) + h.offset + h.stride * assembledInVertex.gl_VertexID;

        switch (h.type) { // this is probably faster than using memset for each attribute
            case AttributeType::FLOAT:
                assembledInVertex.attributes[head].v1 = *((float *) buf);
                break;
            case AttributeType::VEC2:
                assembledInVertex.attributes[head].v2 = *((glm::vec2 *) buf);
                break;
            case AttributeType::VEC3:
                assembledInVertex.attributes[head].v3 = *((glm::vec3 *) buf);
                break;
            case AttributeType::VEC4:
                assembledInVertex.attributes[head].v4 = *((glm::vec4 *) buf);
                break;
        }
    }

    return assembledInVertex;
}

inline void mixAttr(const Attribute &first, const Attribute &second, Attribute &out, float t) {
    out.v4 = first.v4 + t * (second.v4 - first.v4);
}

inline void
mixAttrBarycentric(const Attribute &a, const Attribute &b, const Attribute &c, Attribute &out,
                   const glm::vec3 &mixFact) {
    out.v4 = ((a.v4 * mixFact.x) + (b.v4 * mixFact.y) + (c.v4 * mixFact.z)) /
             (mixFact.x + mixFact.y + mixFact.z);
}

inline float interp(const glm::vec4 &a, const glm::vec4 &b) {
    return -(a.w + a.z) / (b.w - a.w + b.z - a.z);
}

// extra fuj
void GPU::clipTriangle(std::vector<Triangle> &out, const Triangle &t, const VertexPullerSettings *vps) {
    auto &v0P = t.vertices[0].gl_Position;
    auto &v1P = t.vertices[1].gl_Position;
    auto &v2P = t.vertices[2].gl_Position;

    if (v0P == v1P || v1P == v2P || v2P == v0P) {
        return;
    }

    bool in[3] = {-v0P.w <= v0P.z, -v1P.w <= v1P.z, -v2P.w <= v2P.z};
    int inCount = (in[0] ? 1 : 0) + (in[1] ? 1 : 0) + (in[2] ? 1 : 0);

    if (inCount == 3) {
        out.emplace_back(t);
        return;
    } else if (inCount == 0) {
        // The whole triangle is behind the clipping plane.
        return;
    } else if (inCount == 2) {
        int inAI = in[0] ? 0 : (in[1] ? 1 : 2);
        int inBI = in[(inAI + 1) % 3] ? ((inAI + 1) % 3) : ((inAI + 2) % 3);

        auto &inA = t.vertices[inAI];
        auto &inB = t.vertices[inBI];
        auto &outC = t.vertices[3 - inAI - inBI];

        float at = interp(inB.gl_Position, outC.gl_Position);
        float bt = interp(inA.gl_Position, outC.gl_Position);

        auto iAP = glm::mix(inB.gl_Position, outC.gl_Position, at);
        auto iBP = glm::mix(inA.gl_Position, outC.gl_Position, bt);

        Triangle out1 = Triangle();
        out1.vertices[0].gl_Position = glm::vec4(iAP);
        out1.vertices[1].gl_Position = glm::vec4(iBP);
        out1.vertices[2].gl_Position = glm::vec4(inB.gl_Position);

        Triangle out2 = Triangle();
        out2.vertices[0].gl_Position = glm::vec4(iBP);
        out2.vertices[1].gl_Position = glm::vec4(inA.gl_Position);
        out2.vertices[2].gl_Position = glm::vec4(inB.gl_Position);

        for (uint32_t i = 0; i < maxAttributes; i++) {
            //if (!vps->heads[i].enabled) continue;

            mixAttr(inB.attributes[i], outC.attributes[i], out1.vertices[0].attributes[i], at);
            mixAttr(inA.attributes[i], outC.attributes[i], out1.vertices[1].attributes[i], bt);
            out1.vertices[2].attributes[i] = inB.attributes[i];

            out2.vertices[0].attributes[i] = out1.vertices[1].attributes[i];
            out2.vertices[1].attributes[i] = inA.attributes[i];
            out2.vertices[2].attributes[i] = inB.attributes[i];
        }

        out.emplace_back(out1);
        out.emplace_back(out2);
    } else if (inCount == 1) {
        int inCI = in[0] ? 0 : (in[1] ? 1 : 2);

        auto &inC = t.vertices[inCI];
        auto &outA = t.vertices[(inCI + 1) % 3];
        auto &outB = t.vertices[(inCI + 2) % 3];

        float at = interp(outB.gl_Position, inC.gl_Position);
        float bt = interp(outA.gl_Position, inC.gl_Position);

        auto iAP = glm::mix(outB.gl_Position, inC.gl_Position, at);
        auto iBP = glm::mix(outA.gl_Position, inC.gl_Position, bt);

        Triangle outT = Triangle();
        outT.vertices[0].gl_Position = glm::vec4(iBP);
        outT.vertices[1].gl_Position = glm::vec4(iAP);
        outT.vertices[2].gl_Position = glm::vec4(inC.gl_Position);

        for (uint32_t i = 0; i < maxAttributes; i++) {
            //if (!vps->heads[i].enabled) continue;

            mixAttr(outB.attributes[i], inC.attributes[i], outT.vertices[1].attributes[i], at);
            mixAttr(outA.attributes[i], inC.attributes[i], outT.vertices[0].attributes[i], bt);
            outT.vertices[2].attributes[i].v4 = inC.attributes[i].v4;
        }

        out.emplace_back(outT);
    } else {
        // something's wrong
        throw std::exception();
    }
}

void GPU::transformToScreenSpace(Triangle &triangle) {
    float ssw = fbWidth / 2.0f;
    float ssh = fbHeight / 2.0f;

    for (int v = 0; v < 3; v++) {
        auto &pos = triangle.vertices[v].gl_Position;
        pos.x = ((pos.x / pos.w) + 1) * ssw;
        pos.y = ((pos.y / pos.w) + 1) * ssh;
        //pos.z = ((pos.z / pos.w) + 1) / 2; // map to <0; 1>
        pos.z /= pos.w;
    }
}

// a: first vertex of the edge
// b: second vertex of the edge
// c: tested point
inline float edgeFunction(const glm::vec2 &a, const glm::vec2 &b, const glm::vec2 &c) {
    return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
}

void GPU::rasterize(const std::vector<Triangle> &arr, const ProgramSettings *prog) {
    for (auto const &t : arr) {
        float minXs = glm::min(t.vertices[0].gl_Position.x, t.vertices[1].gl_Position.x,
                               t.vertices[2].gl_Position.x);

        float minYs = glm::min(t.vertices[0].gl_Position.y, t.vertices[1].gl_Position.y,
                               t.vertices[2].gl_Position.y);

        float maxXs = glm::max(t.vertices[0].gl_Position.x, t.vertices[1].gl_Position.x,
                               t.vertices[2].gl_Position.x);

        float maxYs = glm::max(t.vertices[0].gl_Position.y, t.vertices[1].gl_Position.y,
                               t.vertices[2].gl_Position.y);

        minXs = glm::max(0.f, minXs);
        minYs = glm::max(0.f, minYs);
        maxXs = glm::clamp(maxXs, 0.f, (float) fbWidth - 1);
        maxYs = glm::clamp(maxYs, 0.f, (float) fbHeight - 1);

        auto minX = static_cast<uint32_t>(minXs);
        auto minY = static_cast<uint32_t>(minYs);
        auto maxX = static_cast<uint32_t>(maxXs);
        auto maxY = static_cast<uint32_t>(maxYs);

        auto p = glm::vec2(minX + 0.5f, minY + 0.5f);

        float a[3] = {t.vertices[0].gl_Position.y - t.vertices[1].gl_Position.y,
                      t.vertices[1].gl_Position.y - t.vertices[2].gl_Position.y,
                      t.vertices[2].gl_Position.y - t.vertices[0].gl_Position.y};

        float b[3] = {t.vertices[0].gl_Position.x - t.vertices[1].gl_Position.x,
                      t.vertices[1].gl_Position.x - t.vertices[2].gl_Position.x,
                      t.vertices[2].gl_Position.x - t.vertices[0].gl_Position.x};

        float wr[3] = {
                edgeFunction(t.vertices[1].gl_Position, t.vertices[2].gl_Position, p),
                edgeFunction(t.vertices[2].gl_Position, t.vertices[0].gl_Position, p),
                edgeFunction(t.vertices[0].gl_Position, t.vertices[1].gl_Position, p)
        };

        // the actual area is tArea/2, but the number would get multiplied by 2 in calculating the coords anyway
        float tArea = glm::abs(edgeFunction(t.vertices[0].gl_Position, t.vertices[1].gl_Position,
                                            t.vertices[2].gl_Position));

        glm::vec3 hom = glm::vec3(t.vertices[0].gl_Position.w, t.vertices[1].gl_Position.w,
                                  t.vertices[2].gl_Position.w);

        for (uint32_t y = minY; y <= maxY; y++) {
            float w[3] = {wr[0], wr[1], wr[2]};

            for (uint32_t x = minX; x <= maxX; x++) {
                if ((w[0] >= 0 && w[1] >= 0 && w[2] >= 0)) { // culling?
                    // This pixel is inside, calculate barycentric coordinates
                    // lambda_A for pt = S_lambda_A / S = (edgeFunc(B, C, pt)/2) / S
                    // => lambda_0 = edgeFunc(vert[1].pos, vert[2].pos, (x,y)) / tArea
                    glm::vec3 barycentric = glm::vec3(w[0], w[1], w[2]) / tArea;
                    glm::vec3 barPersp = glm::vec3(barycentric.x / hom.x, barycentric.y / hom.y, barycentric.z / hom.z);

                    float z = (t.vertices[0].gl_Position.z * barPersp.x + t.vertices[1].gl_Position.z * barPersp.y
                               + t.vertices[2].gl_Position.z * barPersp.z) / (barPersp.x + barPersp.y + barPersp.z);

                    InFragment f;
                    f.gl_FragCoord = glm::vec4(x + 0.5f, y + 0.5f, z, 0);

                    for (uint32_t attr = 0; attr < maxAttributes; attr++) {
                        if (prog->vs2fsTypes[attr] == AttributeType::EMPTY) continue;

                        mixAttrBarycentric(t.vertices[0].attributes[attr], t.vertices[1].attributes[attr],
                                           t.vertices[2].attributes[attr], f.attributes[attr], barPersp);
                    }

                    OutFragment outFrag = OutFragment();
                    prog->fs(outFrag, f, prog->uniforms);

                    uint32_t pIndex = (y * fbWidth) + x;

                    if (depthBuf[pIndex] > f.gl_FragCoord.z) {
                        depthBuf[pIndex] = f.gl_FragCoord.z;

                        uint8_t ri = glm::clamp(outFrag.gl_FragColor.r, 0.f, 1.f) * 255;
                        uint8_t gi = glm::clamp(outFrag.gl_FragColor.g, 0.f, 1.f) * 255;
                        uint8_t bi = glm::clamp(outFrag.gl_FragColor.b, 0.f, 1.f) * 255;
                        uint8_t ai = glm::clamp(outFrag.gl_FragColor.a, 0.f, 1.f) * 255;

                        uint32_t c = (ai << 24u) | (bi << 16u) | (gi << 8u) | ri;
                        *(((uint32_t *) colorBuf) + pIndex) = c;
                    }
                }

                w[0] -= a[1];
                w[1] -= a[2];
                w[2] -= a[0];
            }

            wr[0] += b[1];
            wr[1] += b[2];
            wr[2] += b[0];
        }
    }
}

void GPU::drawTriangles(uint32_t nofVertices) {
    if (boundVao == emptyID || boundProgram == emptyID) {
        return;
    }

    auto *vps = reinterpret_cast<VertexPullerSettings *>(vaoStore[boundVao]);
    auto *prog = reinterpret_cast<ProgramSettings *>(programStore[boundProgram]);

    if (vps == nullptr || prog == nullptr) {
        return;
    }

    void *indexBuffer = nullptr;
    if (vps->indexingBufferId != emptyID) {
        indexBuffer = bufferStore[vps->indexingBufferId]; // TODO: check?
    }

    std::vector<Triangle> trianglesIn;
    std::vector<Triangle> trianglesOut;

    Triangle nextTriangle;

    for (uint32_t vertex = 0; vertex < nofVertices; vertex++) {
        // 1) Vertex Puller
        InVertex assembledInVertex = assembleInVertex(vertex, vps, prog, indexBuffer);
        // 2) Vertex Processor
        OutVertex outVertex = OutVertex();
        prog->vs(outVertex, assembledInVertex, prog->uniforms);
        // 3) Primitives Assembly
        nextTriangle.vertices[2 - (vertex % 3)] = outVertex;
        if (vertex % 3 == 2) {
            trianglesIn.push_back(nextTriangle);
        }
    }

    for (auto &triangle : trianglesIn) {
        // 4) Clipping
        clipTriangle(trianglesOut, triangle, vps);
    }

    for (auto &triangle : trianglesOut) {
        // 5) Perspective Division and Viewport Transformation
        transformToScreenSpace(triangle);
    }

    rasterize(trianglesOut, prog);
}