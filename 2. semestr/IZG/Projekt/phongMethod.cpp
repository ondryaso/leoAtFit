/*!
 * @file
 * @brief This file contains implementation of phong rendering method
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 * @author Ondřej Ondryáš, xondry02@stud.fit.vutbr.cz
 * @date 2020-05-11
 */

#include <student/phongMethod.hpp>
#include <student/bunny.hpp>
#include <glm/gtc/epsilon.hpp>

using namespace glm;

/**
 * @brief This function represents vertex shader of phong method.
 *
 * @param outVertex output vertex
 * @param inVertex input vertex
 * @param uniforms uniform variables
 */
void phong_VS(OutVertex &outVertex, InVertex const &inVertex, Uniforms const &uniforms) {
    auto &pos = inVertex.attributes[0].v3;
    auto &norm = inVertex.attributes[1].v3;
    auto m = uniforms.uniform[1].m4 * uniforms.uniform[0].m4;
    outVertex.gl_Position = m * glm::vec4(pos, 1.0f);
    outVertex.attributes[0].v3 = pos;
    outVertex.attributes[1].v3 = norm;
}

/**
 * @brief This function represents fragment shader of phong method.
 *
 * @param outFragment output fragment
 * @param inFragment input fragment
 * @param uniforms uniform variables
 */
void phong_FS(OutFragment &outFragment, InFragment const &inFragment, Uniforms const &uniforms) {
    const float shininess = 40.f;

    const vec3 &pos = inFragment.attributes[0].v3;
    const vec3 &norm = normalize(inFragment.attributes[1].v3);

    // This makes stripes
    float a = floor(fract((pos.x + sin(pos.y * 10) / 10.f) * 5) + 0.5f);
    vec3 diffuseColour = mix(vec3(0, 0.5f, 0), vec3(1, 1, 0), a);

    const vec3 light = normalize(uniforms.uniform[2].v3 - pos);
    const vec3 camera = normalize(uniforms.uniform[3].v3 - pos);
    const vec3 reflectLight = (-reflect(light, norm));

    const float eps = 0.001f;

    vec3 specularColour = vec3(1.f, 1.f, 1.f);
    if (epsilonEqual(norm.y, 1.f, eps)) {
        // Normal facing up
        diffuseColour = vec3(1.f, 1.f, 1.f);
    } else if (!(norm.y < 0 || epsilonEqual(norm.y, 0.f, eps))) {
        // Normal not facing down, perform the mixing
        const float t = abs(norm.y * norm.y);
        diffuseColour = mix(diffuseColour, vec3(1.f, 1.f, 1.f), t);
    }

    // This would definitely make Phong proud
    vec3 diffuse = diffuseColour * max(dot(light, norm), 0.f);
    vec3 specular = specularColour * pow(max(dot(camera, reflectLight), 0.f), shininess);

    vec3 res = clamp(diffuse + specular, 0.f, 1.f);
    outFragment.gl_FragColor = vec4(res, 1.f);
}

/**
 * @brief Constructor of phong method
 */
PhongMethod::PhongMethod() {
    vao = gpu.createVertexPuller();
    prog = gpu.createProgram();
    vbo = gpu.createBuffer(sizeof(bunnyVertices));
    indBo = gpu.createBuffer(sizeof(bunnyIndices));

    gpu.setBufferData(vbo, 0, sizeof(bunnyVertices), bunnyVertices);
    gpu.setBufferData(indBo, 0, sizeof(bunnyIndices), bunnyIndices);
    gpu.setVertexPullerIndexing(vao, IndexType::UINT32, indBo);

    gpu.setVertexPullerHead(vao, 0, AttributeType::VEC3, sizeof(float) * 6, 0, vbo);
    gpu.setVertexPullerHead(vao, 1, AttributeType::VEC3, sizeof(float) * 6, sizeof(float) * 3, vbo);

    gpu.enableVertexPullerHead(vao, 0);
    gpu.enableVertexPullerHead(vao, 1);

    gpu.attachShaders(prog, phong_VS, phong_FS);
    gpu.setVS2FSType(prog, 0, AttributeType::VEC3);
    gpu.setVS2FSType(prog, 1, AttributeType::VEC3);
}

/**
 * @brief This function draws phong method.
 *
 * @param proj projection matrix
 * @param view view matrix
 * @param light light position
 * @param camera camera position
 */
void
PhongMethod::onDraw(glm::mat4 const &proj, glm::mat4 const &view, glm::vec3 const &light, glm::vec3 const &camera) {
    gpu.clear(.5f, .5f, .5f, 1.f);
    gpu.bindVertexPuller(vao);
    gpu.useProgram(prog);

    gpu.programUniformMatrix4f(prog, 0, view);
    gpu.programUniformMatrix4f(prog, 1, proj);
    gpu.programUniform3f(prog, 2, light);
    gpu.programUniform3f(prog, 3, camera);

    // Number of vertices, this should hopefully be optimised out
    const int vertices = sizeof(bunnyIndices) / sizeof(VertexIndex);

    gpu.drawTriangles(vertices);
    gpu.unbindVertexPuller();
}

/**
 * @brief Destructor of phong method.
 */
PhongMethod::~PhongMethod() {
    gpu.deleteProgram(prog);
    gpu.deleteVertexPuller(vao);
    gpu.deleteBuffer(vbo);
    gpu.deleteBuffer(indBo);
}