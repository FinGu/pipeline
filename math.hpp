#include <iostream>
#include <array>
#include <cstddef>
#include <vector>
#include <functional>

template <std::size_t M, std::size_t N>
using matrix = std::array<std::array<float, N>, M>;
using vmatrix = std::vector<std::vector<float>>;

struct vec{
    float x; 
    float y; 
    float z;

    vec operator +(const vec &other) const;
    void operator +=(const vec &other);
    
    vec operator -(const vec &other) const;

    //cross
    vec operator *(const vec &other) const;

    vec operator /(const float v) const;

    bool operator ==(const vec &other) const;

    //dot
    float dprod(const vec &p);
    vec sprod(float i);

    vec normalize();

    float dist(const vec &p);
};

template <size_t M, size_t N>
void mprint(const matrix<M, N> &a){
    for(size_t i = 0; i < M; ++i){
        for(size_t j = 0; j < N; ++j){
            std::cout << a[i][j] << ' ';
        }
        std::cout << '\n';
    }

    std::cout << '\n';
}

void mprint(const vmatrix &a);

//m1_n1 x m2_n2, n1 = m2
template <size_t M1, size_t N1, size_t M2, size_t N2>
matrix<M1, N2> mmul(const matrix<M1, N1> &a, const matrix<M2, N2> &b) {
    matrix<M1, N2> out = {0};
                              
    for (size_t i = 0; i < M1; ++i) {
        for (size_t j = 0; j < N2; ++j) {
            for (size_t k = 0; k < N1; ++k) {
                out[i][j] += a[i][k] * b[k][j];
            }
        }
    }

    return out;
}

template <size_t M2, size_t N2>
vmatrix mmul(const vmatrix &a, const matrix<M2, N2> &b){
    size_t M1 = a.size();
    size_t N1 = a[0].size();

    vmatrix out(M1, std::vector<float>(N2, 0.0));
    
    for (size_t i = 0; i < M1; ++i) {
        for (size_t j = 0; j < N2; ++j) {
            for (size_t k = 0; k < N1; ++k) {
                out[i][j] += a[i][k] * b[k][j];
            }
        }
    }

    return out;
}

vmatrix mscale(const vmatrix &a, const vec &point);

vmatrix mtranslate(const vmatrix &a, const vec &point);

vmatrix mrotx(const vmatrix &a, float b);
vmatrix mroty(const vmatrix &a, float b);
vmatrix mrotz(const vmatrix &a, float b);

