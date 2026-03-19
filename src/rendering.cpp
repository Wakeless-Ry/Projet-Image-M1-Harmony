// renderer.cpp
#include "rendering.hpp"
#include <stb_image.h>

GLuint Renderer::load_texture(const std::string& path, int& width, int& height)
{
    int n;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &n, 4);
    if (!data)
        return 0;

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(data);
    return tex;
}

void Renderer::draw_texture(GLuint tex, int img_w, int img_h,
                             int win_w, int win_h,
                             float x_left, float x_right)
{
    float zone_w     = (x_right - x_left) * win_w / 2.0f;
    float zone_h     = (float)win_h;
    float img_ratio  = (float)img_w / img_h;
    float zone_ratio = zone_w / zone_h;

    float draw_w, draw_h;
    if (img_ratio > zone_ratio) { draw_w = zone_w; draw_h = draw_w / img_ratio; }
    else                        { draw_h = zone_h; draw_w = draw_h * img_ratio; }

    float cx = (x_left + x_right) / 2.0f;
    float l = cx - (draw_w / win_w);
    float r = cx + (draw_w / win_w);
    float t =       draw_h / win_h;
    float b =      -draw_h / win_h;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex2f(l, b);
    glTexCoord2f(1, 1); glVertex2f(r, b);
    glTexCoord2f(1, 0); glVertex2f(r, t);
    glTexCoord2f(0, 0); glVertex2f(l, t);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

void Renderer::set_current(const std::string& path)
{
    free_current();
    current_tex = load_texture(path, tex_w, tex_h);
}

void Renderer::set_result(const std::string& path)
{
    free_result();
    result_tex = load_texture(path, res_w, res_h);
}

void Renderer::draw(int win_w, int win_h) const
{
    if (current_tex)
        draw_texture(current_tex, tex_w, tex_h, win_w, win_h, -1.0f, 0.0f);
    if (result_tex)
        draw_texture(result_tex,  res_w, res_h, win_w, win_h,  0.0f, 1.0f);
}

void Renderer::free_current() { if (current_tex) { glDeleteTextures(1, &current_tex); current_tex = 0; } }
void Renderer::free_result()  { if (result_tex)  { glDeleteTextures(1, &result_tex);  result_tex  = 0; } }
void Renderer::free_all()     { free_current(); free_result(); }