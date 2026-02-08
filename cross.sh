export WINSDK_PATH="$HOME/winsdk"
GLFW="$HOME/Downloads/glfw-3.4.bin.WIN64"

mkdir -p build

clang-cl --target=x86_64-pc-windows-msvc -fuse-ld=lld \
/winsdkdir "$WINSDK_PATH/sdk" /vctoolsdir "$WINSDK_PATH/crt" \
/MD /O2 /std:c++14 -D_USE_MATH_DEFINES -DUNICODE \
-I imgui -I "$GLFW/include" \
main.cpp fillpoly.cpp camera.cpp scene.cpp object.cpp math.cpp clip.cpp \
imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_widgets.cpp \
imgui/imgui_tables.cpp imgui/imgui_impl_glfw.cpp imgui/imgui_impl_opengl3.cpp \
/Fe:build/pipeline.exe \
/link /LIBPATH:"$GLFW/lib-vc2022" \
glfw3.lib opengl32.lib gdi32.lib shell32.lib user32.lib kernel32.lib imm32.lib
