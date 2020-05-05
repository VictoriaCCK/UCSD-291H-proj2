//
//  Particle.hpp
//  291hw2
//
//  Created by Chiu Chi Kwan on 2020/4/30.
//  Copyright © 2020 ChiuChiuKwan. All rights reserved.
//

#ifndef Particle_hpp
#define Particle_hpp

#include <stdio.h>
#include "core.h"

////////////////////////////////////////////////////////////////////////////////

class Particle
{
public:
    glm::mat4 model = glm::mat4(1);
    // Vertice Information
    glm::vec3 position = glm::vec3(0, 0, 0);
    glm::vec3 velocity = glm::vec3(0, 0, 0);
    std::vector<int> neighbors;
    float mass = 1;
    float density = 1;
    float initial_density = 1;
    float nu = 2;
    glm::vec3 viscosity = glm::vec3(0, 0, 0);
    glm::vec3 pressure = glm::vec3(0, 0, 0);
    glm::vec3 force = glm::vec3(0, 0, 0);
    float Pi = 1.0;
    float dis = 0.3;
    float h = 0.15; // radius
    GLuint VAO;
    GLuint VBO;
    glm::vec3 color = glm::vec3(0, 0, 1);
    
public:
    Particle(glm::vec3 point);
    ~Particle();
    void draw(const glm::mat4& viewProjMtx, GLuint shader);
    void search_neighbor();
    void cal_density();
    float cal_W(glm::vec3 a, glm::vec3 b);
    glm::vec3 cal_deltaW(glm::vec3 a, glm::vec3 b);
    glm::vec3 cal_deltaP();
    glm::vec3 cal_deltaVi2();
    void compute_force();
    int compute_hash(int x, int y, int z);
    void update();
    void integrate (float deltaTime);
    int XOR (int x, int y);
};

////////////////////////////////////////////////////////////////////////////////

#endif /* Particle_hpp */
