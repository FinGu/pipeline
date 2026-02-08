#include "pipeline.hpp"

/*enum limits{
    X_MIN = -1,
    X_MAX = 1,
    Y_MIN = -1,
    Y_MAX = 1,

    Z_MIN = 0,
    Z_MAX = 1,
};*/

enum type{
    LEFT,
    RIGHT,
    BOTTOM,
    TOP,

    NEAR,
    FAR,
};

bool clip_inside_box(vertex &v, enum type _t){
    auto &p = v.in_point;
    auto h = v.temp_h;

    switch (_t){
        case LEFT:        
            return p.x >= -h;
        case RIGHT:
            return p.x <= h; 
        case BOTTOM:
            return p.y <= h;
        case TOP:
            return p.y >= -h;

        case NEAR:
            return p.z >= 0;
        case FAR:
            return p.z <= h;
    }
    
    return false;
}

float interpol_get_u(vertex &v1, vertex &v2, enum type _t){
    if(_t == LEFT){
        return -(v1.in_point.x + v1.temp_h) / ((v2.in_point.x + v2.temp_h) - (v1.in_point.x + v1.temp_h));
    }

    if(_t == RIGHT){
        return -(v1.in_point.x - v1.temp_h) / ((v2.in_point.x - v2.temp_h) - (v1.in_point.x - v1.temp_h));
    }

    if(_t == NEAR){
        return -(v1.in_point.z) / (v2.in_point.z - v1.in_point.z); 
    }

    if(_t == FAR){
        return -(v1.in_point.z - v1.temp_h) / ((v2.in_point.z - v2.temp_h) - (v1.in_point.z - v1.temp_h));
    }

    if(_t == TOP){
        return -(v1.in_point.y + v1.temp_h) / ((v2.in_point.y + v2.temp_h) - (v1.in_point.y + v1.temp_h));
    }

    if(_t == BOTTOM){
        return -(v1.in_point.y - v1.temp_h) / ((v2.in_point.y - v2.temp_h) - (v1.in_point.y - v1.temp_h));
    }

    return 0;
}

void clip_process_pair(std::vector<vertex> &out, vertex &v1, vertex &v2, enum type _t){
    auto &p1 = v1.in_point;
    auto &p2 = v2.in_point;

    auto p1_in_box = clip_inside_box(v1, _t);
    auto p2_in_box = clip_inside_box(v2, _t);

    if(!p1_in_box && p2_in_box){
        float u = interpol_get_u(v1, v2, _t);

        out.push_back({
            .in_point = {
                p1.x + u * (p2.x - p1.x),
                p1.y + u * (p2.y - p1.y),
                p1.z + u * (p2.z - p1.z)
            },
            .mean_unit_vec = {
                v1.mean_unit_vec.x + u * (v2.mean_unit_vec.x - v1.mean_unit_vec.x),
                v1.mean_unit_vec.y + u * (v2.mean_unit_vec.y - v1.mean_unit_vec.y),
                v1.mean_unit_vec.z + u * (v2.mean_unit_vec.z - v1.mean_unit_vec.z)
            },
            .temp_h = v1.temp_h + u * (v2.temp_h - v1.temp_h)
        });

        out.push_back(v2);
    } else if(p1_in_box && p2_in_box){
        out.push_back(v2);
    } else if(p1_in_box && !p2_in_box){
        float u = interpol_get_u(v1, v2, _t);

        out.push_back({
            .in_point = {
                p1.x + u * (p2.x - p1.x),
                p1.y + u * (p2.y - p1.y),
                p1.z + u * (p2.z - p1.z)
            },
            .mean_unit_vec = {
                v1.mean_unit_vec.x + u * (v2.mean_unit_vec.x - v1.mean_unit_vec.x),
                v1.mean_unit_vec.y + u * (v2.mean_unit_vec.y - v1.mean_unit_vec.y),
                v1.mean_unit_vec.z + u * (v2.mean_unit_vec.z - v1.mean_unit_vec.z)
            },
            .temp_h = v1.temp_h + u * (v2.temp_h - v1.temp_h)
        });
    } 
}

std::vector<vertex> clip_against(std::vector<vertex> &vertexes, enum type _t){
    std::vector<vertex> out;

    if(vertexes.empty()){
        return {};
    }

    for(size_t i = 0; i + 1 < vertexes.size(); ++i){
        auto v1 = vertexes[i];
        auto v2 = vertexes[i+1];

        clip_process_pair(out, v1, v2, _t);
    }

    clip_process_pair(out, vertexes[vertexes.size() - 1], vertexes[0], _t);

    return out;
}

std::vector<clipped_face> scene::clip(object &obj, const vmatrix &v){
    std::vector<clipped_face> faces;

    for(int i = 0; i < 6; ++i){
        auto &face = obj.faces[i];

        if(face.o.dprod(face.n) < 0){
            continue;
        }

        std::vector<vertex> out;

        auto fvecs = get_updated_vertexes_from_face(v, obj, face);

        out = clip_against(fvecs, LEFT);
        out = clip_against(out, RIGHT);
        out = clip_against(out, BOTTOM);
        out = clip_against(out, TOP);
        out = clip_against(out, FAR);
        out = clip_against(out, NEAR);
 
        if(out.empty()){
            continue;
        }

        faces.push_back({
            .face_subset = {
                face.n,
                face.centroid
            },
            .vertexes = out,
        });
    }

    return faces;
}


