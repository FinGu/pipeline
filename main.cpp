#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "pipeline.hpp"

#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> 

#include <iostream>
#include <cmath>

constexpr auto vertex_size = 1;

int current_selected_cube = 0;

float rot_x_value = 0;
float rot_y_value = 0;
float rot_z_value = 0;

float scale_value = 1;

float trs_z_value = 0;

float ka[4] = {1,0,0,1};
bool ka_open = false;

float kd[4] = {0,0,0,1};
bool kd_open = false;

float ks[4] = {0,0,0,1};
bool ks_open = false;

color default_color = {1,0,0,1};

bool phong_enabled = false;

static void glfw_error_callback(int error, const char* description){
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void draw_vertexes(camera &cam, std::vector<color> &points, const ImVec2 &offset){
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    auto width = cam.umax - cam.umin;
    auto height = cam.vmax - cam.vmin;

    for(int i = 0; i < width; ++i){
        for(int j = 0; j < height; ++j){
            auto col = points[j * width + i];

            float x = offset.x + i;
            float y = offset.y + j;

            draw_list->AddRectFilled({x - vertex_size, y - vertex_size}, {x + vertex_size, y + vertex_size}, 
                IM_COL32(
                    std::floor(col.r >= 1.0 ? 255 : col.r * 256),
                    std::floor(col.g >= 1.0 ? 255 : col.g * 256),
                    std::floor(col.b >= 1.0 ? 255 : col.b * 256),
                    std::floor(col.a >= 1.0 ? 255 : col.a * 256)
                )
            );
            /*draw_list->AddCircleFilled({x, y}, vertex_size, 
                IM_COL32(
                    std::floor(col.r >= 1.0 ? 255 : col.r * 256),
                    std::floor(col.g >= 1.0 ? 255 : col.g * 256),
                    std::floor(col.b >= 1.0 ? 255 : col.b * 256),
                    std::floor(col.a >= 1.0 ? 255 : col.a * 256)
                )
            );*/
        }
    }
}

void draw(camera &cam, std::vector<color> &points, 
        const ImVec2 &canvas_p0, 
        const ImVec2 &canvas_total)
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    draw_list->AddRectFilled(canvas_p0, 
        ImVec2(canvas_p0.x + canvas_total.x, canvas_p0.y + canvas_total.y), 
        IM_COL32(0, 0, 0, 255)
    );

    draw_vertexes(cam, points, canvas_p0);
}

GLFWwindow *create_window(){
    glfwSetErrorCallback(glfw_error_callback);
    
    if (!glfwInit()){
        return nullptr;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);  
    glfwWindowHint(GLFW_VISIBLE, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(1360, 900, "glfw window", nullptr, nullptr);

    if (window == nullptr){
        return window;
    }

    return window;
}

void destroy_window(GLFWwindow *window){
    glfwDestroyWindow(window);
    glfwTerminate();
}

ImGuiIO &imgui_configure(){
    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;
    //io.ConfigDpiScaleViewports = true;

    return io;
}

void imgui_opengl_init(GLFWwindow *window){
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();
}

void imgui_opengl_uninit(){
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void render_mat_opt(object &obj){
    if(ImGui::ColorEdit4("Setar Ka", ka)){
        obj.mat.ka = ka;
    }

    if(ImGui::ColorEdit4("Setar Kd", kd)){
        obj.mat.kd = kd;
    }

    if(ImGui::ColorEdit4("Setar Ks", ks)){
        obj.mat.ks = ks;
    }
}

void render_ops(scene &scene){
    if(scene.objects.empty()){
        return;
    }

    auto &object = scene.get_object(current_selected_cube);

    ImGui::Text("Operacoes: ");

    if(ImGui::SliderFloat("Rot X", &rot_x_value, 0, 360, "%0.f")){ 
        object.set_rot_x(rot_x_value);
    }

    if(ImGui::SliderFloat("Rot Y", &rot_y_value, 0, 360, "%0.f")){
        object.set_rot_y(rot_y_value);
    }

    if(ImGui::SliderFloat("Rot Z", &rot_z_value, 0, 360, "%0.f")){
        object.set_rot_z(rot_z_value);
    }

    ImGui::NewLine();

    if(ImGui::InputFloat("Escala", &scale_value)){
        object.scale = scale_value;
    }

    ImGui::NewLine();

    ImGui::SetCursorPosX(20);

    if(ImGui::Button("^")){
        object
            .move(0, 1, 0);
    }

    if(ImGui::Button("<")){
        object
            .move(-1, 0, 0);
    }

    ImGui::SameLine();

    ImGui::SetCursorPosX(40);

    if(ImGui::Button(">")){
        object
            .move(1, 0, 0);
    }

    ImGui::SameLine();

    {
        ImGui::SetCursorPosX(80);

        if(ImGui::Button("<##")){
            object
                .move(0, 0, 1);
        }

        ImGui::SetCursorPosX(100);
        ImGui::SameLine();

        ImGui::Text("Z");

        ImGui::SetCursorPosX(120);
        ImGui::SameLine();

        if(ImGui::Button(">##")){
            object
                .move(0, 0, -1);
        }
    }

    ImGui::SetCursorPosX(20);

    if(ImGui::Button("v")){
        object
            .move(0, -1, 0);
    }

    ImGui::NewLine();

    render_mat_opt(object);
}

void render_list(scene &scene){
    ImGui::BeginChild("List");

    int items_count = scene.objects.size();
    int height_in_items = 4;

    char **items = new char*[items_count];
    for(int i = 0; i < items_count; ++i){
        items[i] = new char[50];
        sprintf(items[i], "Cubo %i", scene.get_object(i).id);
    }

    if(ImGui::ListBox("##", 
                &current_selected_cube, items, items_count, height_in_items)){
        auto &obj = scene.get_object(current_selected_cube);

        rot_x_value = obj.get_rot_x();
        rot_y_value = obj.get_rot_y();
        rot_z_value = obj.get_rot_z();

        scale_value = obj.scale;

        trs_z_value = obj.position.z;

        ka[0] = obj.mat.ka.r;
        ka[1] = obj.mat.ka.g;
        ka[2] = obj.mat.ka.b;
        ka[3] = obj.mat.ka.a;

        kd[0] = obj.mat.kd.r;
        kd[1] = obj.mat.kd.g;
        kd[2] = obj.mat.kd.b;
        kd[3] = obj.mat.kd.a;

        ks[0] = obj.mat.ks.r;
        ks[1] = obj.mat.ks.g;
        ks[2] = obj.mat.ks.b;
        ks[3] = obj.mat.ks.a;
    }
   
    for(int i = 0; i < items_count; ++i){
        delete[] items[i];
    }
    delete[] items;

    ImGui::SameLine();

    ImGui::BeginGroup();
    if(ImGui::Button("+")){
        scene.add_object(make_cube(scene, default_color, 1));
    }
    ImGui::NewLine();

    if(items_count && ImGui::Button("-")){
        scene.remove_object(current_selected_cube);

        current_selected_cube = ((items_count - 2 >= 0) ? items_count-2 : 0);
    }
    ImGui::EndGroup();

    ImGui::NewLine();

    render_ops(scene);

    ImGui::EndChild();
}

bool camera_config_enabled = false;
bool light_scene_config_enabled = false;
bool light_ambient_config_enabled = false;

void draw_scene_config_menu(scene &scene){
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Cena")) {
            if(ImGui::MenuItem("Camera")){
                camera_config_enabled = true;
            }

            if(ImGui::MenuItem("Luz da cena")){
                light_scene_config_enabled = true;
            }

            if(ImGui::MenuItem("Luz do ambiente")){
                light_ambient_config_enabled = true;
            }

            ImGui::Separator();

            if(ImGui::Checkbox("Phong simplificado", &phong_enabled)){
                scene.set_phong_enabled(phong_enabled);
            }

            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    if(light_scene_config_enabled){
        ImGui::OpenPopup("Luz cena");
    }

    if(light_ambient_config_enabled){
        ImGui::OpenPopup("Luz ambiente");
    }

    if(camera_config_enabled){
        ImGui::OpenPopup("Camera config");
    }
    
    if(ImGui::BeginPopupModal("Luz cena", &light_scene_config_enabled)){
        int pos_light_scene[3] = {
            (int)scene.slight.pos.x, 
            (int)scene.slight.pos.y, 
            (int)scene.slight.pos.z
        };
        
        float color_light_scene[4] = {
            scene.slight.col.r,
            scene.slight.col.g,
            scene.slight.col.b,
            scene.slight.col.a
        };

        float n = scene.slight.brightness;
        
        if(ImGui::InputInt3("Posicao", pos_light_scene)){
            scene.set_scene_light_pos({
                (float)pos_light_scene[0], 
                (float)pos_light_scene[1], 
                (float)pos_light_scene[2]
            });
        }

        if(ImGui::ColorEdit4("Cor", color_light_scene)){
            scene.set_scene_light_col(color_light_scene);
        }

        ImGui::NewLine();

        if(phong_enabled && ImGui::InputFloat("Brilho (n)", &n)){
            scene.set_scene_light_brightness(n);
        }
   
        ImGui::EndPopup();
    }

    if(ImGui::BeginPopupModal("Luz ambiente", &light_ambient_config_enabled)){
        float color_light_ambient[4] = {
            scene.ambient_light.r,
            scene.ambient_light.g,
            scene.ambient_light.b,
            scene.ambient_light.a,
        };
        
        if(ImGui::ColorEdit4("Cor", color_light_ambient)){
            scene.ambient_light = color_light_ambient;
        }
   
        ImGui::EndPopup();
    }

    if(ImGui::BeginPopupModal("Camera config", &camera_config_enabled)){
        auto &cam = scene.cam;

        int vrp[3] = {
            (int)cam.view_reference_point.x, 
            (int)cam.view_reference_point.y, 
            (int)cam.view_reference_point.z
        };
        
        if(ImGui::InputInt3("VRP", vrp)){
            cam.set_vrp({(float)vrp[0], (float)vrp[1], (float)vrp[2]});
        }

        int vup[3] = {
            (int)cam.view_up.x,
            (int)cam.view_up.y,
            (int)cam.view_up.z
        };

        if(ImGui::InputInt3("VUP (view up)", vup)){
            cam.set_view_up({(float)vup[0], (float)vup[1], (float)vup[2]});
        }

        int fp[3] = {
            (int)cam.focal_point.x, 
            (int)cam.focal_point.y, 
            (int)cam.focal_point.z
        };

        if(ImGui::InputInt3("Ponto focal", fp)){
            cam.set_fp({(float)fp[0], (float)fp[1], (float)fp[2]});
        }

        ImGui::NewLine();

        int dp = cam.dp; 

        if(ImGui::InputInt("Distancia de visao (dp)", &dp)){
            cam.set_view_distance(dp);
        }

        int depth[2] = {
            (int)cam.near,
            (int)cam.far
        }; 

        if(ImGui::InputInt2("Profundiade de visao (near, far)", depth)){
            cam.set_view_depth(depth[0], depth[1]);
        }

        int window[4] = {
            (int)cam.cu, 
            (int)cam.cv, 
            (int)cam.su,
            (int)cam.sv
        };

        if(ImGui::InputInt4("Janela (cu,cv,su,sv)", window)){
            cam.set_window(window[0], window[1], window[2], window[3]);
        }

        int viewport[4] = {
            (int)cam.umin,
            (int)cam.umax,
            (int)cam.vmin,
            (int)cam.vmax
        };

        if(ImGui::InputInt4("Viewport (umin,umax,vmin,vmax)", viewport)){
            if(viewport[1] - viewport[0] != 0 && viewport[3] - viewport[2] != 0){
                cam.set_viewport(viewport[0], viewport[1], viewport[2], viewport[3]);
            }
        }

        float z[2] = {
            cam.zmin,
            cam.zmax
        };

        if(ImGui::InputFloat2("Z (zmin, zmax)", z)){
            cam.set_z(z[0], z[1]);
        }

        ImGui::EndPopup();
    }
}

void render(scene &scene, bool &main_window){
    auto &cam = scene.cam;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("Pipeline", &main_window, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar);

    draw_scene_config_menu(scene);

    ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();

    float w = cam.umax - cam.umin; 
    float h = cam.vmax - cam.vmin;

    ImVec2 canvas_total = ImVec2(w, h);

    if(ImGui::InvisibleButton("my_canvas", canvas_total)){
        /*
        auto pos = ImGui::GetMousePos();

        std::cout << "apertei no canvas " << pos.x << " " << pos.y;
        fflush(nullptr);*/
    }

    auto points = scene.render();

    draw(cam, points, canvas_p0, canvas_total);

    ImGui::SameLine();

    render_list(scene);

    ImGui::End();
    ImGui::PopStyleVar(1);
}

int main(int, char**){ 
    camera cam;

    cam.set_view_up({0, 1, 0});
    cam.set_vrp({30, 40, 100});
    cam.set_fp({1, 2, 1});

    cam.set_window(0, 0, 10, 8);

    cam.set_view_distance(50);

    cam.set_view_depth(20, 120);

    cam.set_viewport(100, 1000, 300, 900);

    cam.set_z(0, 1);

    scene scn(cam);

    scn.set_scene_light({
        .pos = {30, 40, 150},
        .col = {1, 1, 1, 1},
        .brightness = 1
    });

    scn.set_ambient_light({1,1,1,1});

    scn.add_object(make_cube(scn, default_color, 1));

    scn.set_phong_enabled(false);

    auto window = create_window();

    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    auto io = imgui_configure();
    imgui_opengl_init(window);

    bool main_window = true;

    ImVec2 cur_point;

    while (!glfwWindowShouldClose(window)){
        glfwPollEvents();

        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0){
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        if(!main_window){
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        render(scn, main_window);

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    imgui_opengl_uninit();
    destroy_window(window);

    return 0;
}

