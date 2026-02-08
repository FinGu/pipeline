#include "pipeline.hpp"

#include <cmath>
#include <tuple>
#include <algorithm>
#include <limits>

scene::scene(camera cam){
    this->cam = cam;

    this->objects = {};

    this->cur_object_id = 0;
}

void scene::add_object(const object &obj){
    this->objects.push_back(obj);
}

void scene::remove_object(const int index){
    if(this->objects.empty()){
        return;
    }

    this->objects.erase(this->objects.begin() + index);
}

vmatrix vertexes_to_matrix(const std::vector<vertex> &vertexes){
    vmatrix out{};

    // x1 y1 z1 1
    // x2 y2 z2 1

    for(size_t i = 0; i < vertexes.size(); ++i){
        auto cur_point = vertexes[i].in_point;
        out.push_back({cur_point.x, cur_point.y, cur_point.z, 1});
    }
    
    return out;
}

vec_h scene::vec_from_matrix(const vmatrix &pts, const int index){
    return vec_h{
        pts[index][0],
        pts[index][1],
        pts[index][2],
        pts[index][3]
    };
}

vmatrix scene::clipped_faces_to_matrix(std::vector<clipped_face> &faces){
    vmatrix out;

    for(size_t i = 0; i < faces.size(); ++i){
        auto &face = faces[i];
        for(size_t j = 0; j < face.vertexes.size(); ++j){
            auto &ver = face.vertexes[j];
            out.push_back({ver.in_point.x, ver.in_point.y, ver.in_point.z, ver.temp_h});
        }
    }

    return out;
}

void scene::update_clipped_faces_with_matrix(std::vector<clipped_face> &faces, const vmatrix &v){
    int cur_ver_idx = 0;

    for(size_t i = 0; i < faces.size(); ++i){
        auto &face = faces[i];
        auto &ver = face.vertexes;

        for(size_t j = 0; j < ver.size(); ++j){
            auto mp = v[cur_ver_idx++];

            ver[j].temp_h = mp[3];

            ver[j].in_point = {
                mp[0],
                mp[1],
                mp[2]
            };
        }
    }
}

vmatrix scene::apply_transformations(object &obj, vmatrix pts){
    if(obj.scale){
        pts = mscale(pts, {obj.scale, obj.scale, obj.scale});
    }

    if(obj.rotation.x){
        pts = mrotx(pts, obj.rotation.x);
    }

    if(obj.rotation.y){
        pts = mroty(pts, obj.rotation.y);
    }

    if(obj.rotation.z){
        pts = mrotz(pts, obj.rotation.z);
    }

    if(obj.position.x || obj.position.y || obj.position.z){
        pts = mtranslate(pts, obj.position);
    }

    return pts;
}

void scene::calculate_metadata(object &obj, const vmatrix &v){
    auto &cam = this->cam;

    // {{x,y,z,1},
    //  {x,y,z,1}}
    for(int i = 0; i < 6; ++i){
        std::vector<vec> vecs = get_vecs_from_face(v, obj.faces[i]);

        auto centroid = (vecs[0] + vecs[1] + vecs[2] + vecs[3]) / 4;

        obj.faces[i].centroid = centroid;
        
        obj.faces[i].n = ((vecs[2] - vecs[1]) * (vecs[0] - vecs[1])).normalize();

        obj.faces[i].o = (cam.view_reference_point - centroid).normalize();
    }

    obj.vertexes[0].mean_unit_vec = (obj.faces[0].n + obj.faces[3].n + obj.faces[5].n)
        .normalize();

    obj.vertexes[1].mean_unit_vec = (obj.faces[0].n + obj.faces[1].n + obj.faces[5].n)
        .normalize();

    obj.vertexes[2].mean_unit_vec = (obj.faces[0].n + obj.faces[1].n + obj.faces[4].n)
        .normalize();

    obj.vertexes[3].mean_unit_vec = (obj.faces[0].n + obj.faces[3].n + obj.faces[4].n)
        .normalize();

    obj.vertexes[4].mean_unit_vec = (obj.faces[2].n + obj.faces[3].n + obj.faces[5].n)
        .normalize();

    obj.vertexes[5].mean_unit_vec = (obj.faces[1].n + obj.faces[2].n + obj.faces[5].n)
        .normalize();

    obj.vertexes[6].mean_unit_vec = (obj.faces[1].n + obj.faces[2].n + obj.faces[4].n)
        .normalize();

    obj.vertexes[7].mean_unit_vec = (obj.faces[2].n + obj.faces[3].n + obj.faces[4].n)
        .normalize();

    obj.calculate_centroid(v);
}

void scene::interpolate_points(object &obj, std::vector<clipped_face> &faces){
    for(size_t i = 0; i < faces.size(); ++i){
        auto &clip_face = faces[i];

        auto &face = clip_face.face_subset;

        auto centroid = this->phong_enabled ? obj.centroid : face.centroid;

        fillpoly(
            clip_face.vertexes, out_buffer, face, obj.mat, centroid
        );
    }
}

void scene::process_object(object &obj){
    auto &cam = this->cam;

    matrix<4, 4> first_part = cam.make_m1(); //matriz m1
    
    matrix<4, 4> second_part = cam.make_m2();

    vmatrix pts = vertexes_to_matrix(obj.vertexes);
        
    pts = apply_transformations(obj, pts);
        
    calculate_metadata(obj, pts);
                
    vmatrix first_part_out = mmul(pts, first_part);

    //recorte seria feito aqui
    std::vector<clipped_face> clipped_faces = this->clip(obj, first_part_out);

    if(clipped_faces.size() <= 0){
        return;
    }

    //div h
    for(auto &face: clipped_faces){
        for(auto &ver: face.vertexes){
            float h = ver.temp_h;
            ver.in_point.x /= h;
            ver.in_point.y /= h;
            ver.in_point.z /= h;
            ver.temp_h /= h;
        }
    }

    vmatrix clipped_m = clipped_faces_to_matrix(clipped_faces);

    vmatrix second_part_out = mmul(clipped_m, second_part);

    update_clipped_faces_with_matrix(clipped_faces, second_part_out);

    interpolate_points(obj, clipped_faces);
}

const std::vector<color> &scene::render(){
    int height = this->cam.vmax - this->cam.vmin;
    int width = this->cam.umax - this->cam.umin;

    color background_color = {0, 0, 0, 0}; 

    out_buffer.image_buffer.resize(height * width);
    std::fill(out_buffer.image_buffer.begin(), out_buffer.image_buffer.end(), background_color);

    if(!this->objects.size()){
        return out_buffer.image_buffer;
    }

    out_buffer.z_buffer.resize(height * width); 

    std::fill(out_buffer.z_buffer.begin(), out_buffer.z_buffer.end(), INFINITY);

    for(auto &obj: this->objects){
        process_object(obj);
    }

    return out_buffer.image_buffer;
}

object &scene::get_object(const int index){
    return this->objects[index]; 
}

std::vector<vec> scene::get_vecs_from_face(const vmatrix &pts, face &f){
    std::vector<vec> vecs;

    vecs.push_back(vec_from_matrix(pts, f.edges[0].first).v);

    for(size_t i = 0; i < f.edges.size(); ++i){
        vecs.push_back(vec_from_matrix(pts, f.edges[i].second).v);
    }

    return vecs;
}

std::vector<vertex> scene::get_updated_vertexes_from_face(const vmatrix &pts, object &obj, face &f){
    std::vector<vertex> out;

    auto v1_index = f.edges[0].first;
    vertex vv1 = obj.vertexes[v1_index];

    vec_h vvh1 = vec_from_matrix(pts, v1_index);
    vv1.in_point = vvh1.v;
    vv1.temp_h = vvh1.h;

    for(size_t i = 0; i < f.edges.size(); ++i){
        auto v_index = f.edges[i].second;
        vertex v = obj.vertexes[v_index];

        vec_h vh = vec_from_matrix(pts, v_index);

        v.in_point = vh.v;
        v.temp_h = vh.h;

        out.push_back(v);
    }

    return out;
}

void scene::set_scene_light(scene_light slight){
    this->slight = slight;
}

void scene::set_scene_light_pos(vec pos){
    this->slight.pos = pos;
}

void scene::set_scene_light_col(color col){
    this->slight.col = col;
}

void scene::set_scene_light_brightness(float n){
    this->slight.brightness = n;
}

void scene::set_ambient_light(color c){
    this->ambient_light = c;
}
