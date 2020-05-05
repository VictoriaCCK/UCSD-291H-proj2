////////////////////////////////////////
// Window.cpp
////////////////////////////////////////

#include "Window.h"
#include "core.h"
#include <glm/gtx/string_cast.hpp>
////////////////////////////////////////////////////////////////////////////////

// Window Properties
int Window::width;
int Window::height;
const char* Window::windowTitle = "CSE 291H";
int w = 9, h = 11, l = 9;
float dis = 0.29999999;
float start_x = -5*dis, start_y = -6*dis, start_z = 5*dis;
float min_num = -2, max_num = 2;
float bottom = min_num, top = max_num, front = max_num, back = min_num, left = min_num, right = max_num;
float k_force = 300;
// Objects to render
Cube * Window::cube;
int particle_size = 1200;
Particle* particles[1200];
int hash_size = 1000;
std::vector<int> hash_table[1000];
int p1 = 73856093, p2 = 19349663, p3 = 83492791;
// Camera Properties
Camera* Cam;

// Interaction Variables
bool LeftDown, RightDown;
int MouseX, MouseY;

// The shader program id
GLuint Window::shaderProgram;


////////////////////////////////////////////////////////////////////////////////

// Constructors and desctructors 
bool Window::initializeProgram() {

	// Create a shader program with a vertex shader and a fragment shader.
	shaderProgram = LoadShaders("shaders/shader.vert", "shaders/shader.frag");

	// Check the shader program.
	if (!shaderProgram)
	{
		std::cerr << "Failed to initialize shader program" << std::endl;
		return false;
	}

	return true;
}

bool Window::initializeObjects()
{
	// Create a cube
	cube = new Cube();
	//cube = new Cube(glm::vec3(-1, 0, -2), glm::vec3(1, 1, 1));
    for (int i = 0; i<=h; i++) {
        for (int j = 0; j<=l; j++) {
            for (int k = 0; k<=w; k++) {
                int idx = i*(w+1)*(l+1) + j*(w+1) + k;
                particles[idx] = new Particle(glm::vec3(start_x + k * dis, start_y + i * dis, start_z - j * dis));
                int hash_key = particles[idx]->compute_hash(0, 0, 0);
                hash_table[hash_key].push_back(idx);
            }
        }
    }
	return true;
}

void Window::cleanUp()
{
	// Deallcoate the objects.
	delete cube;
    for (int i = 0; i<particle_size; i++) {
        delete particles[i];
    }
	// Delete the shader program.
	glDeleteProgram(shaderProgram);
}

////////////////////////////////////////////////////////////////////////////////

// for the Window
GLFWwindow* Window::createWindow(int width, int height)
{
	// Initialize GLFW.
	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return NULL;
	}

	// 4x antialiasing.
	glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__ 
	// Apple implements its own version of OpenGL and requires special treatments
	// to make it uses modern OpenGL.

	// Ensure that minimum OpenGL version is 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Enable forward compatibility and allow a modern OpenGL context
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// Create the GLFW window.
	GLFWwindow* window = glfwCreateWindow(width, height, windowTitle, NULL, NULL);

	// Check if the window could not be created.
	if (!window)
	{
		std::cerr << "Failed to open GLFW window." << std::endl;
		glfwTerminate();
		return NULL;
	}

	// Make the context of the window.
	glfwMakeContextCurrent(window);

#ifndef __APPLE__
	// On Windows and Linux, we need GLEW to provide modern OpenGL functionality.

	// Initialize GLEW.
	if (glewInit())
	{
		std::cerr << "Failed to initialize GLEW" << std::endl;
		return NULL;
	}
#endif

	// Set swap interval to 1.
	glfwSwapInterval(0);

	// set up the camera
	Cam = new Camera();
	Cam->SetAspect(float(width) / float(height));

	// initialize the interaction variables
	LeftDown = RightDown = false;
	MouseX = MouseY = 0;

	// Call the resize callback to make sure things get drawn immediately.
	Window::resizeCallback(window, width, height);

	return window;
}

void Window::resizeCallback(GLFWwindow* window, int width, int height)
{
#ifdef __APPLE__
	// In case your Mac has a retina display.
	glfwGetFramebufferSize(window, &width, &height); 
#endif
	Window::width = width;
	Window::height = height;
	// Set the viewport size.
	glViewport(0, 0, width, height);

	Cam->SetAspect(float(width) / float(height));
}

////////////////////////////////////////////////////////////////////////////////

// update and draw functions
void Window::idleCallback()
{
	// Perform any updates as necessary. 
	Cam->Update();
    for (int i = 0; i<particle_size; i++) {
        particles[i]->update();
        
        if (particles[i]->position.x <= left) {
//            std::cout<<"a "<<std::to_string((left - particles[i]->position.x) * k_force)<<std::endl;
            particles[i]->force.x += (left - particles[i]->position.x) * k_force;
        } else if (particles[i]->position.x >= right) {
//            std::cout<<"b "<<std::to_string((left - particles[i]->position.x) *  k_force)<<std::endl;
            particles[i]->force.x -= (particles[i]->position.x - right) * k_force;
        }
        
        if (particles[i]->position.y <= bottom) {
//            std::cout<<"c "<<std::to_string((bottom - particles[i]->position.y) * k_force)<<std::endl;
            particles[i]->force.y += (bottom - particles[i]->position.y) * k_force;
        } else if (particles[i]->position.y >= top) {
//            std::cout<<"d "<<std::to_string((particles[i]->position.y - top)  * k_force)<<std::endl;
            particles[i]->force.y -= (particles[i]->position.y - top) * k_force;
        }
        
        if (particles[i]->position.z <= back) {
//            std::cout<<"e "<<std::to_string((particles[i]->position.z - back) * k_force)<<std::endl;
            particles[i]->force.z += (back - particles[i]->position.z) * k_force;
        } else if (particles[i]->position.z >= front) {
//            std::cout<<"f "<<std::to_string((front - particles[i]->position.z) * k_force)<<std::endl;
            particles[i]->force.z -= (particles[i]->position.z - front) * k_force;
        }
        
    }
    for (int i = 0; i<hash_size; i++) {
        std::vector<int>().swap(hash_table[i]);
    }
    
    for (int i = 0; i<particle_size; i++) {
        particles[i]->integrate(0.006);
        int hk = particles[i]->compute_hash(0, 0, 0);
        hash_table[hk].push_back(i);
    }
    
	//cube->update();
}

void Window::displayCallback(GLFWwindow* window)
{	
	// Clear the color and depth buffers.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

	// Render the object.
	cube->draw(Cam->GetViewProjectMtx(), Window::shaderProgram);
    for (int i = 0; i<particle_size; i++) {
        particles[i]->draw(Cam->GetViewProjectMtx(), Window::shaderProgram);
    }
	// Gets events, including input such as keyboard and mouse or window resizing.
	glfwPollEvents();
	// Swap buffers.
	glfwSwapBuffers(window);
}

////////////////////////////////////////////////////////////////////////////////

// helper to reset the camera
void Window::resetCamera() 
{
	Cam->Reset();
	Cam->SetAspect(float(Window::width) / float(Window::height));
}

////////////////////////////////////////////////////////////////////////////////

// callbacks - for Interaction 
void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	/*
	 * TODO: Modify below to add your key callbacks.
	 */
	
	// Check for a key press.
	if (action == GLFW_PRESS)
	{
		switch (key) 
		{
		case GLFW_KEY_ESCAPE:
			// Close the window. This causes the program to also terminate.
			glfwSetWindowShouldClose(window, GL_TRUE);				
			break;

		case GLFW_KEY_R:
			resetCamera();
			break;

		default:
			break;
		}
	}
}

void Window::mouse_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		LeftDown = (action == GLFW_PRESS);
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		RightDown = (action == GLFW_PRESS);
	}
}

void Window::cursor_callback(GLFWwindow* window, double currX, double currY) {

	int maxDelta = 100;
	int dx = glm::clamp((int)currX - MouseX, -maxDelta, maxDelta);
	int dy = glm::clamp(-((int)currY - MouseY), -maxDelta, maxDelta);

	MouseX = (int)currX;
	MouseY = (int)currY;

	// Move camera
	// NOTE: this should really be part of Camera::Update()
	if (LeftDown) {
		const float rate = 1.0f;
		Cam->SetAzimuth(Cam->GetAzimuth() + dx * rate);
		Cam->SetIncline(glm::clamp(Cam->GetIncline() - dy * rate, -90.0f, 90.0f));
	}
	if (RightDown) {
		const float rate = 0.005f;
		float dist = glm::clamp(Cam->GetDistance() * (1.0f - dx * rate), 0.01f, 1000.0f);
		Cam->SetDistance(dist);
	}
}

////////////////////////////////////////////////////////////////////////////////
