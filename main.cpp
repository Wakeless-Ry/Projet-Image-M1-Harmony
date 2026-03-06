#include "color_harmo.hpp"
#include "interface.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <iostream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"


void test()
{
    Color_harmo ch("../assets/img/baboon.ppm");

    Template t(Template_format::I);

    std::cout << "F(I-template): " << ch.F(t) << "\n";

    double best_angle = ch.bestOrientation(1000, Template_format::I);
    std::cout << "Best orientation (rad): " << best_angle << "\n";
    std::cout << "Best orientation (deg): " << best_angle * 180.0 / M_PI << "\n";

    Template t_rot = t;
    t_rot.rotate(best_angle);
    std::cout << "F(I-template tourné): " << ch.F(t_rot) << "\n";

    auto [best_format, best_angle_template] = ch.bestTemplate(1000);
    std::cout << "Best template: " << best_format << "\n";
    std::cout << "Best angle (rad): " << best_angle_template << "\n";
    std::cout << "Best angle (deg): " << best_angle_template * 180.0 / M_PI << "\n";

    double angle = 2.64522;
    std::cout << "Degrees: " << angle * 180.0 / M_PI << "\n";

    Pixel color = Pixel::toRGB(angle, 1.0, 1.0);
    std::cout << "Color: r=" << (int)color.r << " g=" << (int)color.g
              << " b=" << (int)color.b << "\n";

    Image img = Image("../assets/img/baboon.ppm");
    std::vector<int> v(img.get_width() * img.get_height(), 0);
    double E = ch.compute_energie(1.0, v);
    std::cout << "Compute Energie (lambda=1, v=0): " << E << "\n";
}

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
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Image Harmony", NULL, NULL);
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
        std::cerr << "Failed GLEW\n"; 
        return -1;
    }

    // imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    ImGui::StyleColorsDark();

    Interface iface;
    iface.load_images("../assets/img");

    GLuint current_tex = 0;
    int tex_w = 0, tex_h = 0;
    std::string last_image;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        iface.render();

        std::string img_path = iface.get_img();
        if (img_path != last_image)
        {
            if (current_tex)
                glDeleteTextures(1, &current_tex);
            current_tex = load_texture(img_path, tex_w, tex_h);
            last_image = img_path;
        }

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (current_tex)
        {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, current_tex);

            glBegin(GL_QUADS);
            glTexCoord2f(0, 1); glVertex2f(-1, -1);
            glTexCoord2f(1, 1); glVertex2f(1, -1);
            glTexCoord2f(1, 0); glVertex2f(1, 1);
            glTexCoord2f(0, 0); glVertex2f(-1, 1);
            glEnd();

            glBindTexture(GL_TEXTURE_2D, 0);
            glDisable(GL_TEXTURE_2D);
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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