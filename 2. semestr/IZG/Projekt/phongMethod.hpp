/*!
 * @file
 * @brief This file contains phong rendering method
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 * @author Ondřej Ondryáš, xondry02@stud.fit.vutbr.cz
 * @date 2020-05-11
 */

#pragma once

#include <student/method.hpp>

/**
 * @brief This class holds all variables of phong method.
 */
class PhongMethod : public Method {
public:
    PhongMethod();

    virtual ~PhongMethod();

    virtual void
    onDraw(glm::mat4 const &proj, glm::mat4 const &view, glm::vec3 const &light, glm::vec3 const &camera) override;

private:
    VertexPullerID vao;
    ProgramID prog;
    BufferID vbo;
    BufferID indBo;
};