#include "pipeline.hpp"

#include <cmath>

cube make_cube(scene &scene, color col, float size){
    cube cb;
    auto &vertexes = cb.vertexes;

    vertexes.insert(vertexes.end(), {
        {-size, -size, size}, //0, canto inferior esquerdo frontal
        {size, -size, size}, //1, canto inferior direito frontal 
        {size, size, size}, //2
        {-size, size, size},

        {-size, -size, -size},
        {size, -size, -size},
        {size, size, -size},
        {-size, size, -size}
    });

    //1 face
    cb.faces[0].edges.push_back({0, 1});

    cb.faces[0].edges.push_back({1, 2});

    cb.faces[0].edges.push_back({2, 3});
    
    cb.faces[0].edges.push_back({3, 0});

    //2
    cb.faces[1].edges.push_back({1,5});

    cb.faces[1].edges.push_back({5,6});

    cb.faces[1].edges.push_back({6,2});

    cb.faces[1].edges.push_back({2,1});

    //3
    cb.faces[2].edges.push_back({5,4});

    cb.faces[2].edges.push_back({4,7});

    cb.faces[2].edges.push_back({7,6});

    cb.faces[2].edges.push_back({6,5});

    //4
    cb.faces[3].edges.push_back({4, 0});

    cb.faces[3].edges.push_back({0, 3});

    cb.faces[3].edges.push_back({3, 7});

    cb.faces[3].edges.push_back({7, 4});

    //5
    cb.faces[4].edges.push_back({3, 2});

    cb.faces[4].edges.push_back({2, 6});

    cb.faces[4].edges.push_back({6, 7});

    cb.faces[4].edges.push_back({7, 3});

    //6
    cb.faces[5].edges.push_back({4, 5});

    cb.faces[5].edges.push_back({5, 1});

    cb.faces[5].edges.push_back({1, 0});

    cb.faces[5].edges.push_back({0, 4});

    cb.id = scene.cur_object_id;

    scene.cur_object_id++;

    cb.scale = cb.mat.ks.a = cb.mat.kd.a = 1;

    cb.mat.ka = col;

    cb.mat.n = 50.f;

    return cb;
}

void cube::calculate_centroid(const vmatrix &v){
    float x_sum, y_sum, z_sum;
    int count = 0;

    x_sum = y_sum = z_sum = 0;

    for(auto &r: v){
        ++count;
        x_sum += r[0];
        y_sum += r[1];
        z_sum += r[2];
    }

    if(!count){
        return;
    }

    this->centroid = {x_sum / count, y_sum / count, z_sum / count};
}

void cube::move(float x, float y, float z){
    this->position.x += x;      
    this->position.y += y;     
    this->position.z += z;
}

void cube::set_x(float x){
    this->position.x = x;
}

void cube::set_y(float y){
    this->position.y = y;
}

void cube::set_z(float z){
    this->position.z = z;
}

void cube::set_rot_x(float x){
    this->rotation.x = x * M_PI / 180;
}

void cube::set_rot_y(float y){
    this->rotation.y = y * M_PI / 180;
}

void cube::set_rot_z(float z){
    this->rotation.z = z * M_PI / 180;
}

float cube::get_rot_x(){
    return this->rotation.x * 180 / M_PI;
}

float cube::get_rot_y(){
    return this->rotation.y * 180 / M_PI;
}

float cube::get_rot_z(){
    return this->rotation.z * 180 / M_PI;
}
