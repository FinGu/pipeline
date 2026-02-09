#include "pipeline.hpp"

#include <limits>
#include <algorithm>
#include <cmath>
#include <cassert>

int n_scanlines(const std::vector<vertex> &vertexes, int &y_min, int &y_max){
    for(size_t i = 0; i < vertexes.size(); ++i){
        auto &ver = vertexes[i].in_point;

        if(ver.y > y_max){
            y_max = ver.y;
        }

        if(ver.y < y_min){
            y_min = ver.y;
        }
    }    

    return y_max - y_min;
}

void max_or_zero(vec &v){
    v.x = std::max(v.x, (float)0);
    v.y = std::max(v.y, (float)0);
    v.z = std::max(v.z, (float)0);
}

void max_or_zero(color &c){
    c.r = std::max(c.r, (float)0);
    c.g = std::max(c.g, (float)0);
    c.b = std::max(c.b, (float)0);
    c.a = std::max(c.a, (float)0);
}

void scene::set_phong_enabled(bool yes){
    this->phong_enabled = yes;
}

color scene::get_color_of_point(material mat, vec centroid, vec n){
    auto vecL = (this->slight.pos - centroid).normalize();

    auto vecS = (this->cam.view_reference_point - centroid).normalize();

    auto la = this->ambient_light * mat.ka;
    max_or_zero(la);

    auto ld = this->slight.col * mat.kd * n.dprod(vecL);
    max_or_zero(ld);

    color ls;

    if(this->phong_enabled){
        auto vecH = (vecL + vecS).normalize();

        max_or_zero(vecH);

        ls = this->slight.col * mat.ks * std::pow(n.dprod(vecH), mat.n);
    } else{
        auto vecR = n.sprod( vecL.sprod(2).dprod(n)) - vecL;

        ls = this->slight.col * mat.ks * (vecR.dprod(vecS));
    }
    max_or_zero(ls);

    return la + ld + ls;
}

void scene::fillpoly_process_points_y(fillpoly_block &intersections, 
        material mat,
        int y_min, 
        vertex v1, vertex v2, 
        vec centroid,
        face &f) { 
    auto &p1 = v1.in_point;
    auto &p2 = v2.in_point;

    if(p1.y > p2.y){
        std::swap(v1, v2);
    }

    float yv = p2.y - p1.y;

    if(yv == 0){ //horizontal
        return;
    }

    float tx = (p2.x - p1.x) / yv; //1/m
    float tz = (p2.z - p1.z) / yv;
    vec tn = (v2.mean_unit_vec - v1.mean_unit_vec) / yv;
    
    int scanline = (int)p1.y - y_min;

    float cur_x = p1.x;
    float cur_z = p1.z;

    vec cur_n = (this->phong_enabled ? v1.mean_unit_vec : f.n);

    for(int cur_y = p1.y; cur_y < (int)p2.y; ++cur_y, ++scanline){
        vec_with_color pnt = {
            (vec){cur_x, (float)cur_y, cur_z},
            {},
            cur_n
        };

        intersections[scanline].push_back(pnt);

        cur_x += tx;
        cur_z += tz;

        if(this->phong_enabled){
            cur_n += tn;
            cur_n = cur_n.normalize();
        }
    }
}

void scene::fillpoly_process_points_x(fillpoly_block& intersections, draw_buffer &buf, material mat, vec centroid){
    int width = cam.umax - cam.umin;

    for(size_t scanline = 0; scanline < intersections.size(); ++scanline) {
        auto cur = intersections[scanline];

        for(size_t i = 0; i + 1 < cur.size(); i += 2){
            auto p1 = cur[i];
            auto p2 = cur[i+1];

            float xv = (p2.v.x - p1.v.x);

            if(xv == 0){
                continue; 
            }

            vec tn = (p2.n - p1.n) / xv;
            float tz = (p2.v.z - p1.v.z) / xv;

            float walked_inbetween_point = (ceil(p1.v.x) - p1.v.x);
            //imagine x = 8.5, ceil(x) = 9, (9 - 8.5) = 0.5, aplicamos 0.5 da interpol

            float cur_z = p1.v.z;

            vec cur_n = p1.n;

            cur_z += tz * walked_inbetween_point; 
            cur_n += tn.sprod(walked_inbetween_point);

            int cur_y = p1.v.y;

            int limit = std::min((double)ceil(p2.v.x), (double)cam.umax);

            for(int cur_x = ceil(p1.v.x); cur_x < limit; ++cur_x){
                vec normalized_n = cur_n.normalize();
                
                int cur_x_res = cur_x - cam.umin;
                int cur_y_res = cur_y - cam.vmin;

                if(cur_z < buf.z_buffer[cur_x_res + cur_y_res * width]){
                    auto col = get_color_of_point(mat, centroid, normalized_n);

                    buf.image_buffer[cur_x_res + cur_y_res * width] = col;

                    buf.z_buffer[cur_x_res + cur_y_res * width] = cur_z;
                }

                cur_z += tz;

                if(this->phong_enabled){
                    cur_n += tn;
                }
            }
        }
    }
}

void scene::fillpoly(
        const std::vector<vertex> &vertexes, 
        draw_buffer &buf,
        face &f, 
        material m, 
        vec centroid
){
    int y_max = -1, y_min = std::numeric_limits<int>::max();

    int scanlines = n_scanlines(vertexes, y_min, y_max);

    fillpoly_block intersections;
    intersections.reserve(scanlines);

    for(int i = 0; i < scanlines; ++i){
        intersections.push_back({});
    }

    for(size_t i = 0; i + 1 < vertexes.size(); ++i){
        auto p1 = vertexes[i];
        auto p2 = vertexes[i+1];

        fillpoly_process_points_y(intersections, m, y_min, p1, p2, centroid, f);
    }

    fillpoly_process_points_y(intersections, m, y_min, vertexes[vertexes.size()-1], vertexes[0], centroid, f);

    for(int i = 0; i < scanlines; ++i){
        auto &cur = intersections[i];

        std::sort(cur.begin(), cur.end(), [](auto &a, auto &b){
            return a.v.x < b.v.x;
        });
    }

    fillpoly_process_points_x(intersections, buf, m, centroid);
}

