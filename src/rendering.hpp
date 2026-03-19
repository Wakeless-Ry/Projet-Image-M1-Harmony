// renderer.hpp
#ifndef RENDERING_HPP
#define RENDERING_HPP

#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Renderer
{
private:
    GLuint current_tex = 0;
    GLuint result_tex  = 0;
    int tex_w = 0, tex_h = 0;
    int res_w = 0, res_h = 0;

    static void draw_texture(GLuint tex, int img_w, int img_h,
                             int win_w, int win_h,
                             float x_left, float x_right);

public:
    GLuint load_texture(const std::string& path, int& width, int& height);

    void set_current(const std::string& path);
    void set_result(const std::string& path);

    void draw(int win_w, int win_h) const;

    void free_current();
    void free_result();
    void free_all();

    int get_tex_w() const { return tex_w; }
    int get_tex_h() const { return tex_h; }
};

#endif