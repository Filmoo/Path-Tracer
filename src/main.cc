    #include "sphere.hh"
    #include "plane.hh"
    #include "box.hh"
    #include "camera.hh"
    #include "light.hh"
    #include "scene.hh"
    #include "blob.hh"
    #include "obj_reader.hh"
    #include "triangle.hh"
    #include "imgui.h"
    #include "imgui_impl_glut.h"
    #include "imgui_impl_opengl3.h"
    #include <chrono>
    #include <iostream>
    #include <thread>
    #include <GL/freeglut.h> // For FreeGLUT

    #define WINDOW_HEIGHT 500
    #define WINDOW_WIDTH 800
    #define HEIGHT 500
    #define WIDTH 800
    #define NB_SAMPLES 1
    #define DEPTH 1
    #define NB_THREADS 0

    #define MAX(a,b) (((a) > (b)) ? (a) : (b))
    #define MIN(a,b) (((a) < (b)) ? (a) : (b))

    // Data
    static bool done = false;
    static int current_sample = 1;
    static Image current_image(WIDTH, HEIGHT);
    static std::vector<Color> accumulation_buffer(WIDTH * HEIGHT);
    static std::chrono::duration<double> elapsed(0);
    static std::chrono::duration<double> renderTime(0);
    std::mutex cameraMutex;

    //Variables
    Vector3 cameraPosition(2, 0, 2);
    Vector3 cameraDirection(1, 0, 0);
    Vector3 cameraUp(0, 0, -1);
    bool paused = false;
    bool warning_popup = false;
    bool save_image_popup = false;

    //Parameters
    float cameraSpeed = 0.1f;
    float mouseSensitivity = 0.001f;
    float yaw = -90.0f;
    float pitch = 0.0f;

    void RenderScene();
    void ComputeThread(Scene* scene);
    void KeyboardCallback(unsigned char key, int x, int y);
    void MouseMotionCallback(int x, int y);
    void SaveImage(const char* filename);

    void RenderUI();
    void UpdateCameraDirection(float xOffset, float yOffset);
    void UpdateCameraPosition(Vector3 position);
    inline float Clamp(float x, float minVal, float maxVal);
    void CloseCallback() {done = true;}
    int main(int argc, char** argv)
    {
        try {
            // Initialize GLUT
            glutInit(&argc, argv);
            glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
            glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
            glutCreateWindow("Filmo's Path Tracer");
            glutWMCloseFunc(CloseCallback);
            // Initialize OpenGL
            glEnable(GL_DEPTH_TEST);

            // Initialize ImGui
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            ImGui_ImplGLUT_Init();
            ImGui_ImplGLUT_InstallFuncs();
            ImGui_ImplOpenGL3_Init();

            // Scene setup
            Camera* camera = new Camera(&cameraPosition, &cameraDirection, &cameraUp, 20, 20, 1, 1, 2, 16.0f / 9.0f);
            Scene* scene = new Scene(camera);

            // Add objects to the scene
            Plane* ground = new Plane(new Uniform_Texture(Color(0.4, 0.4, 0.4), 0.9, 0.5, 32, 0, 0), Point3(0, 0, 0), Vector3(0, 0, 1));
            scene->addObject(ground);
            Plane* back = new Plane(new Uniform_Texture(Color(0.2, 0.2, 0.2), 0.9, 0.5, 32, 0, 0), Point3(15, 0, 0), Vector3(-1, 0, 0));
            scene->addObject(back);
            Plane* left = new Plane(new Uniform_Texture(Color(0, 0, 1), 0.9, 0.5, 32, 0, 0), Point3(0, 5, 0), Vector3(0, -1, 0));
            scene->addObject(left);
            Plane* right = new Plane(new Uniform_Texture(Color(1, 0, 0), 0.9, 0.5, 32, 0, 0), Point3(0, -5, 0), Vector3(0, 1, 0));
            scene->addObject(right);
            Plane* top = new Plane(new Uniform_Texture(Color(0.2, 0.2, 0.2), 0.9, 0.5, 32, 0, 0), Point3(5, 0, 5), Vector3(0, 0, -1));
            scene->addObject(top);
            Sphere* glassSphere = new Sphere(new Dieletric_Texture(Color(1, 1, 1), 1.7), Point3(8, 1, 1), 1);
            scene->addObject(glassSphere);
            Sphere* normalSphere1 = new Sphere(new Uniform_Texture(Color(0, 1, 0), 0.5, 0.5, 32, 0, 0), Point3(10, -2, 1), 1);
            scene->addObject(normalSphere1);
            Sphere* metalSphere1 = new Sphere(new Metal_Texture(Color(1, 1, 1), 0.5), Point3(10, 0, 2), 1);
            scene->addObject(metalSphere1);
            Sphere_Light* lightSphere = new Sphere_Light(Color(1, 1, 1), Point3(10, 0, 104.7), 100, 1);
            scene->addObject(lightSphere);

            std::thread compute_thread(ComputeThread, scene);

            // Set GLUT callbacks
            glutDisplayFunc(RenderScene);
            glutKeyboardFunc(KeyboardCallback);
            glutMainLoop();

            done = true;
            compute_thread.join();

            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGLUT_Shutdown();
            ImGui::DestroyContext();
        }
        catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
        catch (...) {
            std::cerr << "Unknown exception" << std::endl;
        }
        return 0;
    }


    void RenderScene()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGLUT_NewFrame();
        RenderUI();
        // Rendering image as a texture with OpenGL
        glDrawPixels(WIDTH, HEIGHT, GL_RGB, GL_FLOAT, current_image.dataPointer());

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glutSwapBuffers();
        glutPostRedisplay();
    }

    void ComputeThread(Scene* scene)
    {
        while (!done) {
            if (paused)
                continue;

            auto start = std::chrono::high_resolution_clock::now();
            Image output = scene->Render(HEIGHT, WIDTH, NB_SAMPLES, DEPTH, NB_THREADS, &accumulation_buffer, current_sample);
            current_sample++;
            {
                std::lock_guard<std::mutex> lock(cameraMutex);
                current_image = output;
            }
            auto end = std::chrono::high_resolution_clock::now();
            elapsed = end - start;
            renderTime += elapsed;
        }
    }

    void RenderUI()
    {
        ImGui::NewFrame();
        ImGui::Begin("Filmo's Path Tracer");
        ImGui::Text("Average render time: %dms/frame | Sample count: %d", static_cast<int>(elapsed.count() * 1000), current_sample);
        if (ImGui::Button("Save Image")) {
            if (!paused) {
                warning_popup = true;
            }
            else {
                save_image_popup = true;
            }
        }
        ImGui::Checkbox("Pause", &paused);
        ImGui::End();
    }

    void KeyboardCallback(unsigned char key, int x, int y)
    {
        switch (key) {
        case 'z': // Move forward
            UpdateCameraPosition(cameraPosition + cameraDirection * cameraSpeed);
            current_sample = 1;
            accumulation_buffer.assign(WIDTH * HEIGHT, Color(0));
            break;
        case 's': // Move backward
            UpdateCameraPosition(cameraPosition - cameraDirection * cameraSpeed);
            current_sample = 1;
            accumulation_buffer.assign(WIDTH * HEIGHT, Color(0));
            break;
        case 'q': // Move left
            UpdateCameraPosition(cameraPosition - cameraDirection.cross(cameraUp).normalize() * cameraSpeed);
            current_sample = 1;
            accumulation_buffer.assign(WIDTH * HEIGHT, Color(0));
            break;
        case 'd': // Move right
            UpdateCameraPosition(cameraPosition + cameraDirection.cross(cameraUp).normalize() * cameraSpeed);
            current_sample = 1;
            accumulation_buffer.assign(WIDTH * HEIGHT, Color(0));
            break;
        case 27: // ESC key
            done = true;
            break;
        }
    }
    /*
    void MouseMotionCallback(int x, int y)
    {
        static int lastX = x, lastY = y;
        float xOffset = x - lastX;
        float yOffset = lastY - y;
        lastX = x;
        lastY = y;

        xOffset *= mouseSensitivity;
        yOffset *= mouseSensitivity;

        UpdateCameraDirection(xOffset, yOffset);
        current_sample = 1;
        accumulation_buffer.assign(WIDTH * HEIGHT, Color(0));
    }
    float toRadians(float degrees) {
        return degrees * (M_PI / 180.0f);
    }
    void UpdateCameraDirection(float xOffset, float yOffset)
    {
        yaw += xOffset;
        pitch += yOffset;

        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        Vector3 front;
        front.x = cos(toRadians(yaw)) * cos(toRadians(pitch));
        front.y = sin(toRadians(pitch));
        front.z = sin(toRadians(yaw)) * cos(toRadians(pitch));
        cameraDirection = front.normalize();
    }
    */
    void UpdateCameraPosition(Vector3 position)
    {
        cameraPosition = position;
    }

    inline float Clamp(float x, float minVal, float maxVal)
    {
        return MAX(minVal, MIN(maxVal, x));
    }

    void SaveImage(const char* filename)
    {
        std::lock_guard<std::mutex> lock(cameraMutex);
        current_image.save(filename);
    }
