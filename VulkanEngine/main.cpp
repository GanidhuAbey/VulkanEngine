//IF THE PROGRAM DOESNT WORK FOR SOME REASON CHECK IF THE VECTOR INSIDE ENGINE IS CONSISTENT WITH THE INDIVIDUAL OBJECTS

#include "engine.hpp"

#include <chrono>
#include <iostream>
#include <algorithm>

//probably not the best idea to rely on this library in the main file, change later
#include <glm/glm.hpp>

const int WIDTH = 800;
const int HEIGHT = 600;

const float CAMERA_SPEED = 0.05f;
const float MOUSE_SENSITIVITY = 1.0f;

int main() {
    //initialize engine
    create::Engine engine(WIDTH, HEIGHT, "Test123456");

    //create a camera for the scene
    glm::vec3 camera_pos = glm::vec3(0.0, 0.0, 3.0); //where camera is located
    //where the camera is looking
    glm::vec3 camera_front = glm::vec3(0.0, 0.0, -1.0);
    glm::vec3 up = glm::vec3(0.0, -1.0, 0.0); //the orientation of the camera

    float camera_speed = CAMERA_SPEED;
    float sensitivity = MOUSE_SENSITIVITY;
    
    create::UserCamera* camera = engine.createCamera(camera_pos, camera_front, up, glm::radians(45.0f));

    //create an object
    create::UserObject* someObject = engine.createObject();
    //add some mesh data to this object
    auto t1 = std::chrono::high_resolution_clock::now();
    someObject->addMesh("objects/test_object/car.obj", create::Color(0.1, 0.2, 0.6));
    
    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms_double = t2 - t1;
    printf("object load time: %f \n", ms_double.count());
    //someObject->scale(0.01, 0.01, 0.01);

    //create light source
    create::LightSource* light = engine.createLight(glm::vec3(1, 3.0, 3.0), create::Color(1.0, 1.0, 1.0));
    
    printf("the colour of this light is : <%f  %f  %f> \n", light->light.color.x, light->light.color.y, light->light.color.z);

    int frameCount = 0;
    //in radians
    float angle = 0.0f;

    double yaw = 180.0;
    double pitch = 0.0;

    //capture the cursor
    engine.captureCursor();

    //setup callback function
    double lastX = WIDTH/2, lastY = HEIGHT/2;
    double xPos = WIDTH/2, yPos = HEIGHT/2;

    bool close_now = false;
    while (!engine.checkCloseRequest() && !close_now) {

        auto t1 = std::chrono::high_resolution_clock::now();

        //test.addTransform(0, 0, 0, angle);

        double offsetX = (xPos - lastX) * sensitivity;
        double offsetY = (yPos - lastY) * sensitivity;

        yaw += offsetX;
        pitch = std::clamp(pitch - offsetY, -89.0, 89.0);
        
        
        lastX = xPos;
        lastY = yPos;

        //get cursor position
        engine.getCursorPosition(&xPos, &yPos);

        camera_front.x = (float) (glm::sin(glm::radians(yaw)) * glm::cos(glm::radians(pitch)));
        camera_front.y = (float) glm::sin(glm::radians(pitch));
        camera_front.z = (float) (glm::cos(glm::radians(yaw)) * glm::cos(glm::radians(pitch)));

        camera_front = glm::normalize(camera_front);

        //printf("yaw: %f | pitch: %f \n", yaw, pitch);

        //check key inputs
        
        if (engine.isKeyPressed(WindowInput::W)) {
            camera_pos += camera_speed * camera_front;
        }
        else if (engine.isKeyPressed(WindowInput::A)) {
            camera_pos -= glm::normalize(glm::cross(camera_front, up)) * camera_speed;
        }
        else if (engine.isKeyPressed(WindowInput::S)) {
            camera_pos -= camera_speed * camera_front;
        }
        else if (engine.isKeyPressed(WindowInput::D)) {
            camera_pos += glm::normalize(glm::cross(camera_front, up)) * camera_speed;
        }
        
        frameCount++;
        angle += 0.01f;

        //this is probably not the optimal way to do this
        camera->update(camera_pos, camera_front);
        engine.render();

        auto t2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> ms_double = t2 - t1;
        camera_speed = CAMERA_SPEED * (float)(ms_double.count() / 10);
        //TODO: figure out cause of the inconsistent frames.
        sensitivity = MOUSE_SENSITIVITY * 0.1; //(float)(ms_double.count() / 10);

        if (engine.isKeyPressed(WindowInput::X)) {
            printf("frametime: %f \n", ms_double.count());
            //close_now = true;
        }
    }
    
    return 1;
}


