#include "interface.hpp"
#include "rendering.hpp"
#include "image.hpp"
#include "src/harmonization.hpp"
#include "src/mosaique.hpp"
#include "template.hpp"
#include "harmonization.hpp"
#include "mosaique.hpp"

#include <cstdio>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

static bool show_imgui   = true;
static bool space_pressed = false;
static int  max_w = 1500;
static int  max_h = 1500;
// rendering
Interface interface;
Renderer renderer;
std::string last_image;
int last_algo = 0;
// algo Color Harmonization
Harmonization harmo;
double last_lambda = -1.0;
double last_sigma = -1.0;
double last_angle = 0.0;
Template_format last_fmt = Template_format::i;
// algo mosa
Mosaique mosa;
double last_lambda_2 = 1.0;
double last_sigma_2 = 0.5;
int last_block_size = 8;

int main()
{
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

    glewExperimental = true;
    if (glewInit() != GLEW_OK)
        {
            printf("Failed GLEW\n");
            return -1;
        }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    ImGui::StyleColorsDark();

    interface.load_images("../assets/img");

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !space_pressed)
        {
            show_imgui    = !show_imgui;
            space_pressed = true;
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
            space_pressed = false;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        interface.render();
        int current_algo = interface.get_algo();
        std::string img_path = interface.get_img();
        if (img_path != last_image)
        {
            last_image = img_path;
            renderer.free_result();
            renderer.set_current(img_path);
            interface.set_algo(0);
            current_algo = 0;
            last_algo = 0;
            last_image = img_path;
            float scale = std::min((float)max_w / (renderer.get_tex_w() * 2),
                                   (float)max_h /  renderer.get_tex_h());
            glfwSetWindowSize(window,
                (int)(renderer.get_tex_w() * 2 * scale),
                (int)(renderer.get_tex_h() * scale));
        }

        if (current_algo == 0 && last_algo != 0)
            renderer.free_result();

        if (current_algo == 1)
        {
            double current_lambda = interface.get_lambda();
            double current_sigma  = interface.get_sigma();
            double current_angle = interface.get_angle();
            Template_format current_fmt = interface.get_fmt();
            bool recompute_template = (current_angle != last_angle) || (current_fmt != last_fmt);
            bool recompute_labels = (last_algo != 1) || recompute_template || (current_lambda != last_lambda);
            bool recompute_shift = recompute_labels  || (current_sigma  != last_sigma);

            if (recompute_shift)
            {
                if (last_algo != 1)
                {
                    harmo.set_image(img_path);
                    double agl;
                    Template_format fmt;
                    harmo.compute_best_template(agl, fmt);
                    interface.set_angle(agl);
                    interface.set_fmt(fmt);
                    harmo.build_graph();
                }
                if (recompute_template)
                    harmo.new_template(current_angle, current_fmt); 

                if (recompute_labels)
                {
                    harmo.set_lambda(current_lambda);
                    harmo.solve_graph();
                }

                harmo.set_sigma(current_sigma);
                std::vector<Pixel> pixels = harmo.shift_hues();
                std::string filename = img_path.substr(img_path.find_last_of("/\\") + 1);
                std::string out_path = "../assets/out/color_harmonization/"
                    + filename.substr(0, filename.find_last_of('.')) + ".ppm";

                Image original(img_path);
                std::vector<unsigned char> buffer;
                buffer.reserve(pixels.size() * 3);
                for (const Pixel& p : pixels)
                {
                    buffer.push_back(p.r);
                    buffer.push_back(p.g);
                    buffer.push_back(p.b);
                }
                Image(buffer, original.get_width(), original.get_height()).write_ppm(out_path);
                renderer.set_result(out_path);

                last_fmt = current_fmt;
                last_angle = current_angle;
                last_lambda = current_lambda;
                last_sigma = current_sigma;
            }
        } else if (current_algo == 2)
        {
            int current_block_size = interface.get_bloc_size();
            float current_lambda_2 = interface.get_lambda_2();
            float current_sigma_2 = interface.get_sigma_2();
            if (last_algo != 2 || current_block_size != last_block_size)
            {
                mosa.set_size_bloc(current_block_size);
                mosa.set_img(img_path);
                mosa.compute_mean();
                mosa.compute_mosaique();
                std::string filename = img_path.substr(img_path.find_last_of('/') + 1);
                renderer.set_result("../assets/out/mosaique/mosaique_" + std::to_string(current_block_size) + "_" + filename);
            }
            else if (current_lambda_2 != last_lambda_2 || current_sigma_2 != last_sigma_2)
            {
                mosa.set_lambda(current_lambda_2);
                mosa.set_sigma(current_sigma_2);
                mosa.recompute_lambda_sigma();
                std::string filename = img_path.substr(img_path.find_last_of('/') + 1);
                renderer.set_result("../assets/out/mosaique/mosaique_" + std::to_string(current_block_size) + "_" + filename);
            }
            last_lambda_2 = current_lambda_2;
            last_sigma_2 = current_sigma_2;
            last_block_size = current_block_size;
        }

        last_algo = current_algo;
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        int win_w, win_h;
        glfwGetFramebufferSize(window, &win_w, &win_h);
        glViewport(0, 0, win_w, win_h);

        renderer.draw(win_w, win_h);

        if (show_imgui)
        {
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
        else
            ImGui::EndFrame();

        glfwSwapBuffers(window);
    }

    renderer.free_all();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}