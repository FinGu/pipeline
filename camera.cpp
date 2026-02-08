#include "pipeline.hpp"

void camera::set_vrp(vec vrp){
    this->view_reference_point = vrp;
    calculate_uvn();
}

void camera::set_fp(vec fp){
    this->focal_point = fp;
    calculate_uvn();
}

void camera::set_view_up(vec vup){
    this->view_up = vup;
    calculate_uvn();
}
    
void camera::set_window(float cu, float cv, float su, float sv){
    this->cu = cu; 
    this->cv = cv; //centro
    this->su = su; //meio tamanho
    this->sv = sv;
}

void camera::set_view_distance(float dp){
    this->dp = dp;
}

void camera::set_view_depth(float near, float far){
    this->near = near;
    this->far = far;
}

void camera::set_viewport(float umin, float umax, float vmin, float vmax){
    if((umax - umin) > 5000 || (vmax - vmin) > 5000 || vmin > vmax || umin > umax){
        return;
    }

    this->umin = umin;
    this->umax = umax;
    this->vmin = vmin;
    this->vmax = vmax;
}

void camera::set_z(float zmin, float zmax){
    this->zmin = zmin;
    this->zmax = zmax;
}

void camera::calculate_uvn(){
    this->n = (this->focal_point - this->view_reference_point).normalize();

    float up_n = this->view_up.dprod(this->n);

    vec up_n_n = this->n.sprod(up_n);

    this->v = (view_up - up_n_n).normalize();

    this->u = this->n * this->v;
}

matrix<4, 4> camera::make_m1(){
    const matrix<4, 4> A = {{
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {
            -view_reference_point.x, 
            -view_reference_point.y, 
            -view_reference_point.z, 
            1
        }
    }};

    const matrix<4, 4> B = {{
        {u.x, v.x, n.x, 0},
        {u.y, v.y, n.y, 0},
        {u.z, v.z, n.z, 0},
        {0, 0, 0, 1}
    }};

    const matrix<4, 4> C = {{
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {(-cu/dp), (-cv/dp), 1, 0},
        {0, 0, 0, 1}
    }};

    const matrix<4, 4> D = {{
        {dp / (su * far), 0, 0, 0},
        {0, dp / (sv * far), 0, 0},
        {0, 0, 1 / far, 0},
        {0, 0, 0, 1}
    }};

    const matrix<4, 4> P = {{
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, (far / (far - near)), 1},
        {0, 0, (-near / (far - near)), 0}
    }};
    
    const matrix<4,4> mF = mmul(mmul(mmul(mmul(A, B), C), D), P);

    return mF; 
}

matrix<4, 4> camera::make_m2(){
    const matrix<4, 4> J = {{
        {1, 0, 0, 0},
        {0, -1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1}
    }};

    const matrix<4, 4> K = {{
        {0.5, 0, 0, 0},
        {0, 0.5, 0 ,0},
        {0, 0, 1, 0},
        {0.5, 0.5, 0, 1},
    }};

    const matrix<4, 4> L = {{
        {(umax - umin), 0, 0, 0},
        {0, (vmax - vmin), 0, 0},
        {0, 0, (zmax - zmin), 0},
        {umin, vmin, zmin, 1 }
    }};

    const matrix<4, 4> M = {{
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {0.5, 0.5, 0, 1},
    }};

    const matrix<4,4> mF = mmul(mmul(mmul(J, K), L), M);

    return mF;
}

