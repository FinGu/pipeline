#include "math.hpp"
#include "color.hpp"

struct vertex {
    vec in_point;

    vec mean_unit_vec;

    float temp_h;
};

struct edge{
     int first;
     int second;
};

struct face{
   vec n; 
   vec centroid;
   vec o; // centroide da face que mira pro centro da camera
   std::vector<edge> edges;
};

struct material{
    color ka;
    color kd;
    color ks;

    float n;
};

struct clipped_face{
    face face_subset;
    std::vector<vertex> vertexes;
};

struct cube{ 
    int id;

    face faces[6]; 

    std::vector<vertex> vertexes;

    material mat;

    void calculate_centroid(const vmatrix &v);

    vec centroid;

    void move(float x, float y, float z);

    void set_x(float x);
    void set_y(float y);
    void set_z(float z);

    void set_rot_x(float x);
    float get_rot_x();
    void set_rot_y(float y);
    float get_rot_y();
    void set_rot_z(float z);
    float get_rot_z();

    vec rotation = {0,0,0};
    vec position = {0,0,0};
    float scale;
};

struct camera{
    //(VRP, Ponto focal, ViewUp, Cu, Cv, Su, Sv, Dp, Near e Far)
    vec view_up;
    vec view_reference_point;
    vec focal_point;

    vec u, v, n;

    void set_view_up(vec vup);
    void set_vrp(vec vrp);
    void set_fp(vec fp);
    
    float cu, cv;
    float su, sv;
    void set_window(float cu, float cv, float su, float sv);
    //SetWindow(cu, cv, su, sv)

    float dp;
    void set_view_distance(float dp);
    //SetViewDistance(d)

    float near, far;
    void set_view_depth(float near, float far);
    //SetViewDepth(near, far)

    void set_viewport(float umin, float umax, float vmin, float vmax);
    float umin, umax, vmin, vmax;
    
    void set_z(float zmin, float zmax);
    float zmin, zmax;

    matrix<4, 4> make_m1();

    matrix<4, 4> make_m2();

    private:
        void calculate_uvn();
};

struct scene_light{
    vec pos;
    color col;
    float brightness; //^n
};

struct vec_with_color{
    vec v;
    color c;
    vec n;
};

using object = cube;
using fillpoly_block = std::vector<std::vector<vec_with_color>>;

struct vec_h{
    vec v;
    float h;
};

struct draw_buffer{
    std::vector<float> z_buffer;
    std::vector<color> image_buffer;
};

struct scene{
    int cur_object_id;

    camera cam;

    color ambient_light;
    scene_light slight;

    std::vector<object> objects;

    draw_buffer out_buffer;

    scene(camera cam);

    vec_h vec_from_matrix(const vmatrix &pts, const int index);

    void set_scene_light_pos(vec pos);
    void set_scene_light_col(color col);
    void set_scene_light_brightness(float n);

    void set_scene_light(scene_light s);

    void set_ambient_light(color c);

    void add_object(const object &obj);
    void remove_object(const int index);
    object &get_object(const int index);

    std::vector<vec> get_vecs_from_face(const vmatrix &pts, face &f);
    std::vector<vertex> get_updated_vertexes_from_face(const vmatrix &pts, object &obj, face &f);

    vmatrix clipped_faces_to_matrix(std::vector<clipped_face> &faces);

    void update_clipped_faces_with_matrix(std::vector<clipped_face> &faces, const vmatrix &v);

    color get_color_of_point(material mat, vec centroid, vec n);

    void fillpoly(const std::vector<vertex> &vertexes, draw_buffer &buf, face &f, material m, vec centroid);

    const std::vector<color> &render();

    void set_phong_enabled(bool yes);
    bool phong_enabled;

    private:
        vmatrix apply_transformations(object &obj, vmatrix v);

        void calculate_metadata(object &obj, const vmatrix &v);

        std::vector<clipped_face> clip(object &obj, const vmatrix &v);

        void interpolate_points(object &obj, std::vector<clipped_face> &faces);

        void process_object(object &obj); 

        void fillpoly_process_points_y(
                fillpoly_block &intersections, 
                material mat,
                int y_min, 
                vertex v1, vertex v2, 
                vec centroid,
                face &f);

        void fillpoly_process_points_x(
                fillpoly_block &intersections, 
                draw_buffer &buf,
                material mat,
                vec centroid);
};

cube make_cube(scene &scene, color col, float size);
