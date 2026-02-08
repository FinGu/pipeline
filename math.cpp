#include "math.hpp"
#include <cmath>
#include <iostream>

vec vec::normalize(){
    float magn = std::sqrt(std::pow(x, 2) + std::pow(y, 2) + std::pow(z, 2));

    return {x / magn, y / magn, z / magn};
}

vec vec::operator +(const vec &other) const{
    return {x + other.x, y + other.y, z + other.z};
}

void vec::operator +=(const vec &other){
    this->x += other.x;
    this->y += other.y;
    this->z += other.z;
}

vec vec::operator -(const vec &other) const{
    return {x - other.x, y - other.y, z - other.z};
}

vec vec::operator *(const vec &other) const{
    return {
        (y * other.z) - (z * other.y),
        (z * other.x) - (x * other.z),
        (x * other.y) - (y * other.x)
    };
}

vec vec::operator /(const float v) const{
    return { x / v, y / v, z / v };
}

bool vec::operator ==(const vec &other) const{
    return x == other.x && y == other.y && z == other.z;
}

float vec::dprod(const vec &b){
    return x * b.x + 
           y * b.y +
           z * b.z;
}

vec vec::sprod(float i){
    return {x * i, y * i, z * i};
}

float vec::dist(const vec &p){
    return std::sqrt( std::pow(p.x - x, 2) + std::pow(p.y - y, 2) );
}

void mprint(const vmatrix &a){
    for(size_t i = 0; i < a.size(); ++i){
        for(size_t j = 0; j < a[i].size(); ++j){
            std::cout << a[i][j] << ' ';
        }
        std::cout << '\n';
    }
    std::cout << '\n';
}

vmatrix mscale(const vmatrix &a, const vec &point){
    matrix<4, 4> o = {{
        {point.x, 0, 0, 0},
        {0, point.y, 0, 0},
        {0, 0, point.z, 0},
        {0, 0, 0, 1}
    }};

    return mmul(a, o);
}

vmatrix mtranslate(const vmatrix &a, const vec &point){
    matrix<4, 4> o = {{
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {point.x, point.y, point.z, 1}
    }};

    return mmul(a, o);
}

vmatrix mrotx(const vmatrix &a, float b){
    matrix<4, 4> o = {{
        {1, 0, 0, 0},
        {0, std::cos(b), -std::sin(b), 0},
        {0, std::sin(b), std::cos(b), 0},
        {0, 0, 0, 1}
    }};

    return mmul(a, o);
}

vmatrix mroty(const vmatrix &a, float b){
    matrix<4, 4> o = {{
        {std::cos(b), 0, std::sin(b), 0},
        {0, 1, 0, 0},
        {-std::sin(b), 0, std::cos(b), 0},
        {0, 0, 0, 1}
    }};

    return mmul(a, o);
}

vmatrix mrotz(const vmatrix &a, float b){
    matrix<4, 4> o = {{
        {std::cos(b), -std::sin(b), 0, 0},
        {std::sin(b), std::cos(b), 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1}
    }};

    return mmul(a, o);
}
