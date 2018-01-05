// include glad, GLFW
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// include our own files
#include "camera.h"
#include "shader.h"
#include "stb_image.h"
#include "model.h"

// include glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// include C++ library
#include <iostream>

// function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void renderScene(Shader wallShader, Shader modelShader, Model ourModel, float clipPlane[4]);
unsigned int initializeReflectionFBO();
unsigned int initializeRefractionFBO();

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;    // time between current frame and last frame
float lastFrame = 0.0f;

// Global var for convenience
unsigned int reflectionColorBuffer;
unsigned int refractionColorBuffer;

float reflect_plane[4] = { 0, 1, 0, 0 };
float refract_plane[4] = { 0, -1, 0, 0 };
float plane[4] = { 0, -1, 0, 100000 };

unsigned int wallVAO, floorVAO;
unsigned int texture1, texture2;

unsigned int DuDvTexture;

float wave_speed = 0.03f;
float moveFactor = 0;

int main()
{
    
    // --------------- set glfw, glad ------------------
    
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Graphics Engine", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback); // set mouse callback
    glfwSetScrollCallback(window, scroll_callback); // set scroll callback
    
    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    // ------- configure global opengl state -------
    glEnable(GL_CULL_FACE);
    
    glEnable(GL_CLIP_DISTANCE0);
    
    // ------------------ shaders ------------------
    
    Shader waterShader("./water.vs", "./water.frag");
    
    Shader wallShader("./wallShader.vs", "./wallShader.frag");
    
    Shader screenShader("./screenShader.vs", "./screenShader.frag");
    
    Shader modelShader("./model_loading.vs", "./model_loading.frag");
    
    // ----------------- load models ----------------
    
    Model ourModel(string("../models/nanosuit/nanosuit.obj"));
    
    // ----------------- data processing ----------------
    
    // vertex data
    float water[] = { // all y set to 0
        /*
        -2, 5, // botton left
        2, 5, // bottom right
        2, -5, // top right
        -2, 5, // botton left
        2, -5, // top right
        -2, -5 // top left
         */
        -1, 1,
        1, 1,
        1, -1,
        -1, 1,
        1, -1,
        -1, -1
    };

    float wall[] = { 
        // position texture
        -2, 2, -5,   0, 1,
        -2, -2, -5,  0, 0,
        2, -2, -5,   1, 0,
        2, -2, -5,   1, 0,
        2, 2, -5,    1, 1,
        -2, 2, -5,   0, 1,
        
        2, 2, 5,     0, 1,
        2, -2, 5,    0, 0,
        -2, -2, 5,   1, 0,
        -2, -2, 5,   1, 0,
        -2, 2, 5,    1, 1,
        2, 2, 5,     0, 1,

        -2, 2, 5,    0, 1,
        -2, -2, 5,   0, 0,
        -2, -2, -5,  1, 0,
        -2, -2, -5,  1, 0,
        -2, 2, -5,   1, 1,
        -2, 2, 5,    0, 1,

        2, 2, -5,    0, 1,
        2, -2, -5,   0, 0,
        2, -2, 5,    1, 0,
        2, -2, 5,    1, 0,
        2, 2, 5,     1, 1,
        2, 2, -5,    0, 1
    };
    
    float floor[] = {
        -2, -2, -5,  0, 1,
        -2, -2, 5,   0, 0,
        2, -2, 5,    1, 0,
        2, -2, 5,    1, 0,
        2, -2, -5,   1, 1,
        -2, -2, -5,  0, 1,
    };
    
    float quadVertices[] = { // for rendering to screen space
        // positions   // texCoords
        /*
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        
        -1.0f,  1.0f,  0.0f, 1.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 1.0f
         */
        -1.0f,  1.0f,  0.0f, 0.0f,
        -1.0f, -1.0f,  0.0f, 1.0f,
        1.0f, -1.0f,  1.0f, 1.0f,
        
        -1.0f,  1.0f,  0.0f, 0.0f,
        1.0f, -1.0f,  1.0f, 1.0f,
        1.0f,  1.0f,  1.0f, 0.0f
          
    };
    
    unsigned int waterVBO, waterVAO, wallVBO, floorVBO; // NOTE: wallVAO is declared as global var for convenience
    glGenVertexArrays(1, &waterVAO);
    glGenBuffers(1, &waterVBO);
    glGenVertexArrays(1, &wallVAO);
    glGenBuffers(1, &wallVBO);
    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);
    
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(waterVAO);
    glBindBuffer(GL_ARRAY_BUFFER, waterVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(water), water, GL_STATIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(wallVAO);
    glBindBuffer(GL_ARRAY_BUFFER, wallVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(wall), wall, GL_STATIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(floorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floor), floor, GL_STATIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // screen quad VAO
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    
    
    // -------------- textures ---------------------
    
    // texture 1
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char *data = stbi_load("../textures/marble.bmp", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture1" << std::endl;
    }
    stbi_image_free(data);
    
    // texture 2
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);    // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    data = stbi_load("../textures/bamboo.jpg", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture2" << std::endl;
    }
    stbi_image_free(data);
    
    // load DuDv texture
    glGenTextures(1, &DuDvTexture);
    glBindTexture(GL_TEXTURE_2D, DuDvTexture);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);    // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    data = stbi_load("../textures/waterDUDV1.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load DuDv texture" << std::endl;
    }
    stbi_image_free(data);
    
    // ------------ shader configuration ---------------
    
    wallShader.use();
    wallShader.setInt("texture1", 0);
    
    screenShader.use();
    screenShader.setInt("screenTexture", 0);

    // ----------- frame buffer configuration ----------
    
    unsigned int reflectionFBO = initializeReflectionFBO();
    unsigned int refractionFBO = initializeRefractionFBO();
    
    
    // --------------- drawing mode ---------------------
    
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // uncomment to see that we only rendered a single quad in screen space using FBOs
    
    // --------------- render loop ------------------------
    while (!glfwWindowShouldClose(window))
    {
        glEnable(GL_CLIP_DISTANCE0); // enable clip distance
        
        // timing
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        // input
        processInput(window);
        
        // ------------------ 1st pass ---------------
        
        glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)
        
        // render reflection texture
        glBindFramebuffer(GL_FRAMEBUFFER, reflectionFBO);
        float distance = 2 * ( camera.Position.y - 0 );
        camera.Position.y -= distance;
        camera.invertPitch(); // invert camera pitch
        renderScene(wallShader, modelShader, ourModel, reflect_plane);
        // reset camera back to original position
        camera.Position.y += distance;
        camera.invertPitch(); // invert back camera pitch
        
        // render refraction texture
        glBindFramebuffer(GL_FRAMEBUFFER, refractionFBO);
        renderScene(wallShader, modelShader, ourModel, refract_plane);
        
        
        // render to screen
        glDisable(GL_CLIP_DISTANCE0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // now bind back to default framebuffer
        renderScene(wallShader, modelShader, ourModel, plane);
        // render water
        waterShader.use();
        waterShader.setInt("reflectionTexture", 0);
        waterShader.setInt("refractionTexture", 1);
        waterShader.setInt("dudvMap", 2);
        
        // wave
        moveFactor += wave_speed * currentFrame * 0.001; 
        if (moveFactor >= 1)
            moveFactor -= 1;
        waterShader.setFloat("moveFactor", moveFactor);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, reflectionColorBuffer);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, refractionColorBuffer);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, DuDvTexture);
        glBindVertexArray(waterVAO);
        // do transformations
        glm::mat4 projection;
        projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        waterShader.setMat4("projection", projection);
        // camera/view transformation
        glm::mat4 view = camera.GetViewMatrix();
        waterShader.setMat4("view", view);
        glm::mat4 model= glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(2.0, 1.0, 5.0));
        waterShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        // std::cout << camera.Position.y << "\n";
        
        // ------------------ 2nd pass ---------------
        
        /*
        // draw a quad plane with the attached framebuffer color texture
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // now bind back to default framebuffer
        glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
        // clear all relevant buffers
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessery actually, since we won't be able to see behind the quad anyways)
        glClear(GL_COLOR_BUFFER_BIT);
        screenShader.use();
        glBindVertexArray(quadVAO);
        glBindTexture(GL_TEXTURE_2D, reflectionColorBuffer);
        // glBindTexture(GL_TEXTURE_2D, refractionColorBuffer);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        */
        
        // check and call events and swap the buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // deallocate resources
    glDeleteVertexArrays(1, &waterVAO);
    glDeleteBuffers(1, &waterVBO);
    glDeleteFramebuffers(1, &reflectionFBO);
    glDeleteFramebuffers(1, &refractionFBO);
    // ToDo: Delete textures, rbo
    
    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
    return 0;
}

// reflectionColorBuffer set as global var for convenience
unsigned int initializeReflectionFBO()
{
    // reflection frame buffer
    unsigned int fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    // create a color attachment texture
    unsigned int depthRenderBuffer, depthBuffer;
    glGenTextures(1, &reflectionColorBuffer);
    glBindTexture(GL_TEXTURE_2D, reflectionColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, reflectionColorBuffer, 0);
    
    // create a depth render buffer attachment
    glGenRenderbuffers(1, &depthRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer); // now actually attach it
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Reflection framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return fbo;
}

unsigned int initializeRefractionFBO()
{
    // refraction frame buffer
    unsigned int fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    // create a color attachment texture
    unsigned int depthBuffer;
    glGenTextures(1, &refractionColorBuffer);
    glBindTexture(GL_TEXTURE_2D, refractionColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, refractionColorBuffer, 0);
    // create a depth attachment texture
    glGenTextures(1, &depthBuffer);
    glBindTexture(GL_TEXTURE_2D, depthBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthBuffer, 0);
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Refraction framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    return fbo;
}

// draw everything aside from water
void renderScene(Shader wallShader, Shader modelShader, Model ourModel, float clipPlane[4])
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1); // marble texture
    
    wallShader.use();
    // pass clip plane
    GLuint plane_location = glGetUniformLocation(wallShader.ID, "plane");
    glUniform4fv(plane_location, 1, clipPlane);
    // do transformations
    glm::mat4 projection;
    projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    wallShader.setMat4("projection", projection);
    // camera/view transformation
    glm::mat4 view = camera.GetViewMatrix();
    wallShader.setMat4("view", view);
    glBindVertexArray(wallVAO);
    glm::mat4 model= glm::mat4(1.0f);
    wallShader.setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 24);
    // draw floor
    glBindVertexArray(floorVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture2); // floor texture
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    // draw model
    /*
    modelShader.use();
    model = glm::mat4(1.0f); // load identity matrix
    model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));    // it's a bit too big for our scene, so scale it down
    modelShader.setMat4("model", model);
    modelShader.setMat4("projection", projection);
    modelShader.setMat4("view", view);
    // pass clip plane to model shader`
    plane_location = glGetUniformLocation(modelShader.ID, "plane");
    glUniform4fv(plane_location, 1, clipPlane);
    ourModel.Draw(modelShader);
     */
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    
    lastX = xpos;
    lastY = ypos;
    
    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

