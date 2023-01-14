//
// Created by pbialas on 25.09.2020.
//

#include "app.h"

#include <iostream>
#include <vector>
#include <valarray>

#include "Application/utils.h"
#include "glm/glm.hpp"
#include "glm/ext/scalar_constants.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "XeEngine/ColorMaterial.h"
#include "stb/stb_image.h"

void SimpleShapeApplication::init() {
    // A utility function that reads the shader sources, compiles them and creates the program object
    // As everything in OpenGL we reference program by an integer "handle".
    float strength = 1;
    float color[3] = {0.8, 0.8, 0.8};
    float theta = 1.0 * glm::pi<float>() / 6.0f;
    auto cs = std::cos(theta);
    auto ss = std::sin(theta);
    glm::mat2 rot{cs, ss, -ss, cs};
    glm::vec2 trans{0.0, -0.25};
    glm::vec2 scale{0.5, 0.5};

    glm::mat4 projection = glm::perspective(45.0f, (float) 650 / 400, 1.0f, 100.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(-1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                 glm::vec3(0.0f, 1.0f, 1.0f));

    glm::mat4 model(1.0f);

    int width, height;
    std::tie(width, height) = frame_buffer_size();

    set_camera(new Camera());
    set_controler(new CameraControler(camera()));
    camera()->perspective(glm::pi<float>() / 3.5, (float) width / height, 0.1f, 100.0f);
    camera()->look_at(glm::vec3(-1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                      glm::vec3(0.0f, 1.0f, 1.0f));


    auto program = xe::utils::create_program(
            {{GL_VERTEX_SHADER,   std::string(PROJECT_DIR) + "/shaders/base_vs.glsl"},
             {GL_FRAGMENT_SHADER, std::string(PROJECT_DIR) + "/shaders/base_fs.glsl"}});

    if (!program) {
        std::cerr << "Invalid program" << std::endl;
        exit(-1);
    }

    // A vector containing the x,y,z vertex coordinates for the triangle.
    std::vector<GLfloat> vertices = {
            -0.5f, 0.0f, -0.5f, 0.5000f, 0.1910f,
            0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
            0.5f, 0.f, -0.5f, 0.8090f, 0.5000f,

            0.5f, 0.0f, -0.5f, 0.8090f, 0.5000f,
            0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
            0.5f, 0.0f, 0.5f, 0.5000f, 0.8090f,

            -0.5f, 0.0f, 0.5f, 0.1910f, 0.5000f,
            0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            0.5f, 0.0f, 0.5f, 0.5000f, 0.8090f,

            -0.5f, 0.0f, 0.5f, 0.1910f, 0.5000f,
            0.f, 1.0f, 0.0f, 0.0f, 0.0f,
            -0.5f, 0.0f, -0.5f, 0.5000f, 0.1910f,

            -0.5f, 0.0f, -0.5f, 0.5000f, 0.1910f,
            -0.5f, 0.0f, 0.5f, 0.1910f, 0.5000f,
            0.5f, 0.0f, -0.5f, 0.8090f, 0.5000f,
            0.5f, 0.0f, 0.5f, 0.5000f, 0.8090f,
    };

    std::vector<GLushort> indexVector = {0, 1, 2, 3, 4, 5, 7, 6, 8, 9, 10, 11, 12, 14, 13, 13, 14, 15};

    // Generating the buffer and loading the vertex data into it.
    GLuint v_buffer_handle;
    glGenBuffers(1, &v_buffer_handle);
    OGL_CALL(glBindBuffer(GL_ARRAY_BUFFER, v_buffer_handle));
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint v_buffer_indices;
    glGenBuffers(1, &v_buffer_indices);
    OGL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, v_buffer_indices));
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexVector.size() * sizeof(GLfloat), indexVector.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    GLuint v_buffer_uniforms_color;
    glGenBuffers(1, &v_buffer_uniforms_color);
    OGL_CALL(glBindBuffer(GL_UNIFORM_BUFFER, v_buffer_uniforms_color));
    glBufferData(GL_UNIFORM_BUFFER, 8 * sizeof(GLfloat), nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, v_buffer_uniforms_color);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float), &strength);
    glBufferSubData(GL_UNIFORM_BUFFER, 4 * sizeof(float), 3 * sizeof(float), &color);

    glGenBuffers(1, &u_pvm_buffer_);
    OGL_CALL(glBindBuffer(GL_UNIFORM_BUFFER, u_pvm_buffer_));
    glBufferData(GL_UNIFORM_BUFFER, 16 * sizeof(GLfloat), nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, u_pvm_buffer_);


    xe::ColorMaterial colorMaterial({1.0f, 1.0f, 1.0f, 1.0f});
    colorMaterial.set_texture(xe::create_texture(std::string(ROOT_DIR) + "/Models/multicolor.png"));
    xe::ColorMaterial::init();
    colorMaterial.bind();

    // This setups a Vertex Array Object (VAO) that  encapsulates
    // the state of all vertex buffers needed for rendering
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, v_buffer_handle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, v_buffer_indices);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // This indicates that the data for attribute 0 should be read from a vertex buffer.
    glEnableVertexAttribArray(0);

    // and this specifies how the data is layout in the buffer.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), reinterpret_cast<GLvoid *>(0));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
                          reinterpret_cast<GLvoid *>(3 * sizeof(GLfloat)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    //end of vao "recording"

    // Setting the background color of the rendering window,
    // I suggest not to use white or black for better debuging.
    glClearColor(0.81f, 0.81f, 0.8f, 1.0f);

    // This setups an OpenGL vieport of the size of the whole rendering window.
    auto [w, h] = frame_buffer_size();
    glViewport(0, 0, w, h);

    glUseProgram(program);
}

//This functions is called every frame and does the actual rendering.
void SimpleShapeApplication::frame() {
    // Binding the VAO will setup all the required vertex buffers.
    glBindVertexArray(vao_);
    glEnable(GL_DEPTH_TEST);

    glm::mat4 PVM = camera()->projection() * camera()->view();
    glBindBuffer(GL_UNIFORM_BUFFER, u_pvm_buffer_);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, 16 * sizeof(float), &PVM);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_SHORT, nullptr);
    glBindVertexArray(0);
}

void SimpleShapeApplication::framebuffer_resize_callback(int w, int h) {
    Application::framebuffer_resize_callback(w, h);
    glViewport(0, 0, w, h);
}


