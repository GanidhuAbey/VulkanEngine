#include "engine.hpp"
#include "game_objects.hpp"
#include "camera.hpp"

#include <chrono>
#include <iostream>

//probably not the best idea to rely on this library in the main file, change later
#include <glm/glm.hpp>

const int WIDTH = 800;
const int HEIGHT = 600;

const double CAMERA_SPEED = 0.05f;
const double MOUSE_SENSITIVITY = 1.0f;

int main() {
    //initialize editor

    //initialize engine
    create::Engine engine(WIDTH, HEIGHT, "Test");

    //initialize camera

    //create a camera for the scene
    glm::vec3 camera_pos = glm::vec3(0.0, 0.0, 3.0); //where camera is located
    //where the camera is looking
    glm::vec3 camera_front = glm::vec3(0.0, 0.0, -1.0);
    glm::vec3 up = glm::vec3(0.0, 1.0, 0.0); //the orientation of the camera

    float camera_speed = 0.05f;
    float sensitivity = 0.05f;

    camera::Camera camera(camera::ProjectionType::PERSPECTIVE_PROJECTION, camera_pos, camera_front, up, &engine);

    //create a empty object and plug in the mesh data
    gameObject::EmptyObject test(&engine);
    //auto t1 = std::chrono::high_resolution_clock::now();
    test.addMeshData("objects/test_object/test.obj");
    auto t2 = std::chrono::high_resolution_clock::now();
    //std::chrono::duration<double, std::milli> time_took = t2 - t1;
    //std::cout << "the time the function took: " << time_took.count() << std::endl;

    double xTranslate = 0.0;
    int frameCount = 0;
    //in radians
    float angle = 0.0f;

    double yaw = 180.0;
    double pitch = 0.0;

    //capture the cursor
    engine.userWindow.captureCursor();

    //setup callback function
    double lastX = WIDTH/2, lastY = HEIGHT/2;
    double xPos = WIDTH/2, yPos = HEIGHT/2;
    while (!engine.userWindow.closeRequest()) {

        auto t1 = std::chrono::high_resolution_clock::now();

        test.addTransform(0, 0, 0, angle);

        double offsetX = (xPos - lastX) * sensitivity;
        double offsetY = (yPos - lastY) * sensitivity;

        yaw -= offsetX;
        pitch += offsetY;

        lastX = xPos;
        lastY = yPos;

        //get cursor position
        engine.userWindow.getCursorPosition(&xPos, &yPos);

        camera_front.x = (float) (glm::sin(glm::radians(yaw)) * glm::cos(glm::radians(pitch)));
        camera_front.y = (float) glm::sin(glm::radians(pitch));
        camera_front.z = (float) (glm::cos(glm::radians(yaw)) * glm::cos(glm::radians(pitch)));

        
        //check key inputs
        
        if (engine.userWindow.isKeyPressed(WindowInput::W)) {
            camera_pos += camera_speed * camera_front;
        }
        else if (engine.userWindow.isKeyPressed(WindowInput::A)) {
            camera_pos -= glm::normalize(glm::cross(camera_front, up)) * camera_speed;
        }
        else if (engine.userWindow.isKeyPressed(WindowInput::S)) {
            camera_pos -= camera_speed * camera_front;
        }
        else if (engine.userWindow.isKeyPressed(WindowInput::D)) {
            camera_pos += glm::normalize(glm::cross(camera_front, up)) * camera_speed;
        }
        
        frameCount++;
        angle += 0.01f;

        camera.update(camera_pos, camera_front);
        engine.draw();

        auto t2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> ms_double = t2 - t1;
        camera_speed = CAMERA_SPEED * (float)(ms_double.count() / 10);
        sensitivity = MOUSE_SENSITIVITY * (float)(ms_double.count() / 10);

        if (engine.userWindow.isKeyPressed(WindowInput::X)) {

            printf("frametime: %f \n", ms_double.count());
        }
    }

    return 1;
}


