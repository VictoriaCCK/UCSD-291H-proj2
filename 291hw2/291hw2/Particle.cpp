//
//  Particle.cpp
//  291hw2
//
//  Created by Chiu Chi Kwan on 2020/4/30.
//  Copyright © 2020 ChiuChiuKwan. All rights reserved.
//

#include "Particle.hpp"
#include <iostream>
#include "Window.h"
#include <set>
#define PI acos(-1)

Particle::Particle(glm::vec3 point)
{
    position = point;
    mass = pow(h, 3) * initial_density;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 1, std::vector<glm::vec3>{position}.data(), GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 1* sizeof(glm::vec3), (void*)0);
    // vertex colors
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 1* sizeof(glm::vec3), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
}

Particle::~Particle()
{
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}

int Particle::compute_hash(int x, int y, int z)
{
    long int index = XOR( XOR( ( floor(position.x/dis) + x ) * p1, ( floor(position.y/dis) + y ) * p2 ) ,  ( floor(position.z/dis) + z ) * p3 ) ;
    if (index<0) index *= (-1);
    int c =  index % hash_size;
    return c;
}

int Particle::XOR(int x, int y)
{
    return (x | y) & (~x | ~y);
}

void Particle::update()
{
    search_neighbor();
    cal_density();
    compute_force();
}

void Particle::integrate (float deltaTime)
{
    velocity += deltaTime * force / mass;
    position += deltaTime * velocity;
    
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferSubData(GL_ARRAY_BUFFER,0,1 * sizeof(glm::vec3),&position);
    glBindBuffer(GL_ARRAY_BUFFER,0);
}

void Particle::compute_force()
{
    pressure = - (mass / density) * cal_deltaP();
    viscosity = mass * nu * cal_deltaVi2();
    force = glm::vec3(0, -9.8 * mass, 0) + pressure + viscosity ; 
}

glm::vec3 Particle::cal_deltaVi2()
{
    glm::vec3 answer = glm::vec3(0, 0, 0);
    
    for (int j = 0; j<neighbors.size(); j++) {
        glm::vec3 temp = velocity -  particles[neighbors[j]]->velocity;
        temp *= particles[neighbors[j]]->mass / particles[neighbors[j]]->density;
        glm::vec3 dis = position - particles[neighbors[j]]->position;
        temp *= dot(dis, cal_deltaW(position, particles[neighbors[j]]->position))/ (dot(dis, dis) + 0.01 * pow(h, 2));
        answer += temp;
    }
    
    return answer * 2.0f; 
}

glm::vec3 Particle::cal_deltaP()
{
    glm::vec3 deltaP = glm::vec3(0, 0, 0);
    
    for (int j = 0; j<neighbors.size(); j++) {
        float temp = particles[neighbors[j]]->mass * (particles[neighbors[j]]->Pi /  (pow(particles[neighbors[j]]->density, 2)) + Pi / (pow(density, 2)));
        glm::vec3 W = cal_deltaW(position, particles[neighbors[j]]->position);
        deltaP += temp * W;
    }
    
    return deltaP * density;
}

glm::vec3 Particle::cal_deltaW(glm::vec3 a, glm::vec3 b)
{
   float q = glm::length(a - b);
   if(q == 0) return glm::vec3(0, 0, 0);
   float ini = (3.f / (2.f * PI)) * pow(h, -4);
   glm::vec3 answer = glm::vec3(ini, ini, ini);
   if (q < 1) {
       answer *= - 2.f * q + pow(q, 2) * 1.5;
   } else if(q <= 2) {
       answer *= pow((2-q), 2) * (-0.5);
   } else {
       return glm::vec3(0, 0, 0);
   }
   answer.x *= ((a.x - b.x) * (1/q));
   answer.y *= ((a.y - b.y) * (1/q));
   answer.z *= ((a.z - b.z) * (1/q));
   return answer;
}

void Particle::cal_density()
{
    density = 0;
    for (int i = 0; i < neighbors.size(); i++) {
        density += particles[neighbors[i]]->mass * cal_W(position, particles[neighbors[i]]->position);
    }
    float k = 3.f;
    Pi = k * (pow(density / initial_density, 7) - 1);
}

float Particle::cal_W(glm::vec3 a, glm::vec3 b)
{
    float q = glm::length(a - b);
    float answer = (3.0f * pow(h, -3)) / (2.0f * PI);
    if (q < 1) {
        answer *= (2.f/3.f) - pow(q, 2) + pow(q, 3) * 0.5;
    } else if(q <= 2) {
        answer *= pow((2-q), 3) * (1.f/6.f);
    } else {
        return 0;
    }
    return answer;
}

void Particle::search_neighbor()
{
    std::vector<int>().swap(neighbors);
//    for (int i = 0; i<particle_size; i++) {
//        if (glm::length(position - particles[i]->position) < dis) {
//            neighbors.push_back(i);
//        }
//    }

    std::set<int> keys;
    for (int x = -1; x<=1; x++) {
        for (int y = -1; y<=1; y++) {
            for (int z = -1; z<=1; z++) {
                int hash_key = compute_hash(x, y, z);
                keys.insert(hash_key);
            }
        }
    }
    std::set<int>::iterator it;
    for (it = keys.begin(); it != keys.end();it++) {
        for (int k = 0; k < hash_table[*it].size(); k++) {
            if(glm::length(position - particles[hash_table[*it][k]]->position) < dis){
                neighbors.push_back(hash_table[*it][k]);
            }
        }
    }
    
}

void Particle::draw(const glm::mat4& viewProjMtx, GLuint shader)
{
    // actiavte the shader program
    glUseProgram(shader);

    // get the locations and send the uniforms to the shader
    glUniformMatrix4fv(glGetUniformLocation(shader, "viewProj"), 1, false, (float*)&viewProjMtx);
    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, (float*)&model);
    glUniform3fv(glGetUniformLocation(shader, "DiffuseColor"), 1, &color[0]); 

    // Bind the VAO
    glBindVertexArray(VAO);

    // draw the points
    glPointSize(5);
    glDrawArrays(GL_POINTS, 0, 1);
    
    // Unbind the VAO and shader program
    glBindVertexArray(0);
    glUseProgram(0);
}
