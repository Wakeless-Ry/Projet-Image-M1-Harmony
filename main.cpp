#include "interface.hpp"
#include <cstdio>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include "interface.hpp"
#include "src/image.hpp"
#include "src/template.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>
#include <cmath>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
static bool show_imgui = true;
static bool space_pressed = false;
static int max_w = 1500;
static int max_h = 1500;
std::vector<Pixel> algo_color_harmonization(std::string& img_path, double lambda)
{
    printf("Image : %s\n", img_path.c_str());
    printf("\n --- partie 3 ---\n");
    
    Template t(std::vector<double>{}, std::vector<double>{});
    t.set_image(img_path);

    auto [format, angle] = t.bestTemplate();
    printf("Best template: %d\n", (int)format);
    printf("Best angle: %.6f\n", angle);

    printf("\n --- partie 4 --- \n");
    printf("Lambda : %.2f\n", lambda);

    Template tmpl(format);
    tmpl.set_image(img_path);
    tmpl.rotate(angle);
    std::vector<Pixel> pixels = tmpl.get_img();

    int n = pixels.size();
    std::vector<int> v;

    tmpl.compute_thetas(pixels, v);

    double e_before = tmpl.compute_energie(lambda, v);
    printf("Energie avant graph cut : %.6f\n", e_before);

    tmpl.run_graphcut(pixels, lambda, v);

    int n_plus = 0, n_minus = 0;
    for (int idx = 0; idx < n; idx++)
    {
        if (v[idx] ==  1) n_plus++;
        if (v[idx] == -1) n_minus++;
    }
    printf("Labels apres graph cut : +1=%d  -1=%d\n", n_plus, n_minus);

    double e_after = tmpl.compute_energie(lambda, v);
    printf("Energie apres  graph cut : %.6f\n", e_after);

    if (e_after <= e_before)
        printf("Energie reduite de %.6f\n", e_before - e_after);
    else
        printf("Energie n'a pas diminue (diff=%.6f)\n", e_after - e_before);

    printf("\n --- partie 4.1 --- \n");
    std::vector<Pixel> result = tmpl.projectPixels(pixels, tmpl, v);
    printf("Done\n");
    return result;
}


auto draw_texture = [](GLuint tex, int img_w, int img_h,
                       int win_w, int win_h,
                       float x_left, float x_right)
{
    float zone_w = (x_right - x_left) * win_w / 2.0f;
    float zone_h = (float)win_h;

    float img_ratio  = (float)img_w / img_h;
    float zone_ratio = zone_w / zone_h;

    float draw_w, draw_h;

    if (img_ratio > zone_ratio)
    {
        draw_w = zone_w;
        draw_h = draw_w / img_ratio;
    }
    else
    {
        draw_h = zone_h;
        draw_w = draw_h * img_ratio;
    }

    float cx = (x_left + x_right) / 2.0f;
    float cy = 0.0f;

    float l = cx - (draw_w / win_w);
    float r = cx + (draw_w / win_w);
    float t = (draw_h / win_h);
    float b = -(draw_h / win_h);

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
};

GLuint load_texture(const std::string& path, int& width, int& height)
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

int main()
{
    // glfw
    if (!glfwInit())
        return -1;
    GLFWwindow* window = glfwCreateWindow(1500, 1500, "Image Harmony", NULL, NULL);
    if (!window)
    { 
        glfwTerminate(); 
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // glew
    glewExperimental = true;
    if (glewInit() != GLEW_OK)
    { 
        printf("Failed GLEW\n"); 
        return -1;
    }

    // imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    ImGui::StyleColorsDark();

    Interface interface;
    interface.load_images("../assets/img");
    
    GLuint current_tex = 0;
    GLuint result_tex = 0; 
    int tex_w = 0, tex_h = 0;
    int res_w = 0, res_h = 0;
    std::string last_image;
    int last_algo = 0;
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !space_pressed)
        {
            show_imgui = !show_imgui;
            space_pressed = true;
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
            space_pressed = false;
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        interface.render();

        std::string img_path = interface.get_img();
        if (img_path != last_image)
        {
            if (current_tex)
                glDeleteTextures(1, &current_tex);
            current_tex = load_texture(img_path, tex_w, tex_h);
            last_image = img_path;
            float scale = std::min(
                (float)max_w / (tex_w * 2),
                (float)max_h / tex_h
            );

            int win_w = tex_w * 2 * scale;
            int win_h = tex_h * scale;

            glfwSetWindowSize(window, win_w, win_h);
        }
        int current_algo = interface.get_algo();
        if (last_algo != current_algo)
        {
            if (current_algo == 1)
            {
                std::string filename = img_path.substr(img_path.find_last_of("/\\") + 1);

                std::vector<Pixel> data = algo_color_harmonization(img_path, 1.0);
                Image img(img_path);
                std::vector<unsigned char> buffer;
                buffer.reserve(data.size() * 3);
                for (const Pixel& p : data)
                {
                    buffer.push_back(p.r);
                    buffer.push_back(p.g);
                    buffer.push_back(p.b);
                }
                Image result(buffer, img.get_width(), img.get_height());

                std::string out_path = "../assets/out/color_harmonization/" 
                       + filename.substr(0, filename.find_last_of('.')) 
                       + ".ppm";

                result.write_ppm(out_path);

                if (result_tex)
                    glDeleteTextures(1, &result_tex);
                result_tex = load_texture(out_path, res_w, res_h);
            }
            else if (current_algo == 2)
            {
                
            }
            last_algo = current_algo;
        }
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        int win_w, win_h;
        glfwGetFramebufferSize(window, &win_w, &win_h);
        glViewport(0, 0, win_w, win_h);
        if (current_tex)
            draw_texture(current_tex, tex_w, tex_h, win_w, win_h, -1.0f, 0.0f);

        if (result_tex)
            draw_texture(result_tex, res_w, res_h, win_w, win_h,  0.0f, 1.0f);

        if (show_imgui)
        {
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
        else
            ImGui::EndFrame();

        glfwSwapBuffers(window);
    }

    if (current_tex)
        glDeleteTextures(1, &current_tex);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}