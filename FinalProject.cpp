#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//SOIL
#include <SOIL2/SOIL2.h>;

using namespace std;

int width, height;
const double PI = 3.14159;
const float toRadians = PI / 180.0f;

// Declare Input Callback Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mode);

// Declare View Matrix
glm::mat4 viewMatrix;

// Camera Field of View
GLfloat fov = 45.0f;

void initiateCamera();

// Define Camera Attributes
glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 3.0f); // Move 3 units back in z towards screen
glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f); // What the camera points to
glm::vec3 cameraDirection = glm::normalize(cameraPosition - target); // direction z
glm::vec3 worldUp = glm::vec3(0.0, 1.0f, 0.0f);
glm::vec3 cameraRight = glm::normalize(glm::cross(worldUp, cameraDirection));// right vector x
glm::vec3 cameraUp = glm::normalize(glm::cross(cameraDirection, cameraRight)); // up vector y
glm::vec3 CameraFront = glm::vec3(0.0f, 0.0f, -1.0f); // 1 unit away from lense



// Camera Transformation Prototype
void TransformCamera();

// Boolean array for keys and mouse buttons
bool keys[1024], mouseButtons[3];

// Input state booleans
bool isPanning = false, isOrbiting = false;

//Boolean to check for ortho or perspective
bool viewType = false;

// Pitch and Yaw
GLfloat radius = 3.0f, rawYaw = 0.0f, rawPitch = 0.0f, degYaw, degPitch;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
GLfloat lastX = 320, lastY = 240, xChange, yChange; // Center mouse cursor
bool firstMouseMove = true;

//Light position 1
glm::vec3 lightPosition1(-1.0f, 1.0f, 1.0f);

//Light position 2
glm::vec3 lightPosition2(1.0f, 1.0f, 1.0f);



void drawLamp()
{
	GLenum mode = GL_TRIANGLES;
	GLsizei indices = 6;
	glDrawElements(mode, indices, GL_UNSIGNED_BYTE, nullptr);
}

// Draw Primitive(s)
void drawKnife()
{
	glClear(GL_STENCIL_BUFFER_BIT);
	GLenum mode = GL_QUADS;
	GLsizei indices = 104;
	glDrawElements(mode, indices, GL_UNSIGNED_BYTE, nullptr);
}




// Create and Compile Shaders
static GLuint CompileShader(const string& source, GLuint shaderType)
{
	// Create Shader object
	GLuint shaderID = glCreateShader(shaderType);
	const char* src = source.c_str();

	// Attach source code to Shader object
	glShaderSource(shaderID, 1, &src, nullptr);

	// Compile Shader
	glCompileShader(shaderID);

	// Return ID of Compiled shader
	return shaderID;

}

// Create Program Object
static GLuint CreateShaderProgram(const string& vertexShader, const string& fragmentShader)
{
	// Compile vertex shader
	GLuint vertexShaderComp = CompileShader(vertexShader, GL_VERTEX_SHADER);

	// Compile fragment shader
	GLuint fragmentShaderComp = CompileShader(fragmentShader, GL_FRAGMENT_SHADER);

	// Create program object
	GLuint shaderProgram = glCreateProgram();

	// Attach vertex and fragment shaders to program object
	glAttachShader(shaderProgram, vertexShaderComp);
	glAttachShader(shaderProgram, fragmentShaderComp);

	// Link shaders to create executable
	glLinkProgram(shaderProgram);

	// Delete compiled vertex and fragment shaders
	glDeleteShader(vertexShaderComp);
	glDeleteShader(fragmentShaderComp);

	// Return Shader Program
	return shaderProgram;

}


int main(void)
{
	width = 640; height = 480;

	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(width, height, "Main Window", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	// Set input callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	if (glewInit() != GLEW_OK)
		cout << "Error!" << endl;

	GLfloat lampVertices[] =
	{
		-0.5, -0.5, 0.0, //index 0


		-0.5, 0.5, 0.0, //index 1


		0.5, -0.5, 0.0,  //index 2	


		0.5, 0.5, 0.0  //index 3	
	};


	GLfloat vertices[] = {
		//Handle
		0.0f, -0.2f, 0.1f,//0
		.6f, 0.8f, 0.8f,
		0.0f, -0.2f,
		0.0f, 0.0f, 1.0f,

		0.0f, 0.2f, 0.1f,//1
		.6f, 0.8f, 0.8f,
		0.0f, .2f,
		0.0f, 0.0f, 1.0f,

		1.6f, 0.2f, 0.1f,//2
		0.6f, 0.8f, 0.8f,
		1.6f, 0.2f,
		0.0f, 0.0f, 1.0f,

		1.6f, -0.2f, 0.1f,//3
		.6f, 0.8f, 0.8f,
		1.6f, -0.2f,
		0.0f, 0.0f, 1.0f,

		0.0f, -0.2f, -0.1f,//4
		.6f, 0.8f, 0.8f,
		0.0f, -0.2f,
		0.0f, 0.0f, 1.0f,

		0.0f, 0.2f, -0.1f,//5
		.6f, 0.8f, 0.8f,
		0.0f, 0.2f,
		0.0f, 0.0f, 1.0f,

		1.6f, 0.2f,-0.1f,//6
		.6f, 0.8f, 0.8f,
		1.6f, 0.2f,
		0.0f, 0.0f, 1.0f,

		1.6f, -0.2f, -0.1f,//7
		.6f, 0.8f, 0.8f,
		1.6f, -0.2f,
		0.0f, 0.0f, 1.0f,

		//Neck to blade
		2.0f, 0.2f, 0.025f,//2 to 8
		1.0f, 0.0f, 0.0f,
		2.0f, 0.2f,
		0.0f, 0.0f, 1.0f,

		2.0f, -0.15f, 0.025f,//9 to 3
		1.0f, 0.0f, 0.0f,
		2.0f, -0.15f,
		0.0f, 0.0f, 1.0f,

		2.0f, 0.2f, -0.025f,//6 to 10
		1.0f, 0.0f, 0.0f,
		2.0f, 0.2f,
		0.0f, 0.0f, 1.0f,

		2.f, -0.15f, -0.025f,// 11 to 7
		0, .15, 0.0f,
		2.f, -0.15f,
		0.0f, 0.0f, 1.0f,

		//Transition up to blade body
		2.1f, 0.2f, 0.025f,//12   8 to 12
		1.0f, 0.0f, 0.0f,
		2.1f, 0.2f,
		0.0f, 0.0f, 1.0f,

		2.1f, -0.2f, 0.025f,//13  13 to 9
		1.0f, 0.0f, 0.0f,
		2.1f, -0.2f,
		0.0f, 0.0f, 1.0f,

		2.1f, 0.2f, -0.025f,//14  6 to 10
		1.0f, 0.0f, 0.0f,
		2.1f, 0.2f,
		0.0f, 0.0f, 1.0f,

		2.1f, -0.2f, -0.025f,//15   11 to 7
		1.0f, 0.0f, 0.0f,
		2.1f, -0.2f,
		0.0f, 0.0f, 1.0f,

		//Blade body 
		3.5f, 0.2f, 0.025f,//16 
		1.0f, 1.0f, 0.0f,
		3.5f, 0.2f,
		0.0f, 0.0f, 1.0f,

		3.5f, -0.2f, 0.025f,//17 
		1.0f, 1.0f, 0.0f,
		3.5f, -0.2f,
		0.0f, 0.0f, 1.0f,

		3.5f, 0.2f, -0.025f,//18
		1.0f, 1.0f, 0.0f,
		3.5f, 0.2f,
		0.0f, 0.0f, 1.0f,

		3.5f, -0.2f, -0.025f,//19
		0.0f, 1.0f, 0.0f,
		3.5f, -0.2f,
		0.0f, 0.0f, 1.0f,

		//Blade rounded end 
		3.7f, 0.1f, 0.025f,//20 
		1.0f, 1.0f, 0.0f,
		3.7f, 0.1f,
		0.0f, 0.0f, 1.0f,

		3.7f, -0.1f, 0.025f,//21
		1.0f, 1.0f, 0.0f,
		3.7f, -0.1f,
		0.0f, 0.0f, 1.0f,

		3.7f, 0.1f, -0.025f,//22
		1.0f, 1.0f, 0.0f,
		3.7f, 0.1f,
		0.0f, 0.0f, 1.0f,

		3.7f, -0.1f, -0.025f,//23
		0.0f, 1.0f, 0.0f,
		3.7f, -0.1f,
		0.0f, 0.0f, 1.0f,

		//Second curve
		3.8f, 0.05f, 0.025f,//24
		1.0f, 1.0f, 0.0f,
		3.8f, 0.05f,
		0.0f, 0.0f, 1.0f,

		3.8f, -0.05f, 0.025f,//25
		1.0f, 1.0f, 0.0f,
		3.8f, -0.05f,
		0.0f, 0.0f, 1.0f,

		3.8f, 0.05f, -0.025f,//26
		1.0f, 1.0f, 0.0f,
		3.8f, 0.05f,
		0.0f, 0.0f, 1.0f,


		3.8f, -0.05f, -0.025f,//27
		0.0f, 1.0f, 0.0f,
		3.8f, -0.05f, 
		0.0f, 0.0f, 1.0f
	};

	// Define element indices
	GLubyte indicesKnife[] = {
		//Blade Handle
		0,1,2,3,
		4,5,6,7,
		0,3,7,4,
		1,2,6,5,

		//Neck
		2,8,9,3,
		6,10,11,7,
		2,6,10,8,
		3,7,11,9,

		//Transition up to blade body
		8,12,13,9,
		10,14,15,11,
		9,11,15,13,
		8,10,14,12,

		//Blade Body
		12,16,17,13,
		14,18,19,15,
		12,14,16,18,
		13,15,17,19,

		//First Curve
		16,20,21,17,
		18,22,23,19,
		16,18,20,22,
		17,19,21,23,

		//Second Curve
		20,24,25,21,
		22,26,27,23,
		20,22,26,24,
		21,23,27,25,
		//End Flat
		24,25,26,27,
		0,1,5,4
	};




	GLfloat verticesSquare[] = {
		// Triangle 1
		-0.5, 0.0, 0.0,//0
		1.0, 0.0, 0.0,
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,//normal postive z

		0.5, 0.0, 0.0, //1
		0.0, 1.0, 0.0,
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,//normal postive z

		0.5, 0.0, 1.0,//2
		0.0, 1.0, 0.0,
		1.0, 1.0,
		0.0f, 0.0f, 1.0f,//normal postive z

		-0.5, 0.0, 1.0,//3
		0.0, 1.0, 0.0,
		0.0, 1.0,
		0.0f, 0.0f, 1.0f//normal postive z
	};


	// Define element indices
	GLubyte indicesSquare[] = {
		0, 1, 2, 3
	};

	//Plane positions
	glm::vec3 planePositions[] =
	{
		glm::vec3(0.0f, 0.0f, 0.5f),
		glm::vec3(0.5f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, -0.5f),
		glm::vec3(-0.5f, 0.0f, 0.0f)
	};

	//Plane rotations
	glm::float32 planeRotationsY[] =
	{
		180.0f, 270.0f, 00.0f, 90.0f
	};
	glm::float32 planeRotationsX[] =
	{
		-45.0f, -45.0f, 45.0f, 45.0f
	};

	glm::vec3 planeScale[] =
	{
	glm::vec3(.5f, .5f, .5f)
	};

	// Define element indices
	GLubyte indicesBox[] =
	{
		0, 1, 2,
		1, 2, 3
	};

	// Plane Transforms
	glm::vec3 planePositionsBox[] = {
		glm::vec3(0.0f,  0.0f,  0.5f),
		glm::vec3(0.5f,  0.0f,  0.0f),
		glm::vec3(0.0f,  0.0f,  -0.5f),
		glm::vec3(-0.5f, 0.0f,  0.0f),
		glm::vec3(0.0f, 0.5f,  0.0f),
		glm::vec3(0.0f, -0.5f,  0.0f)
	};

	glm::float32 planeRotationsBox[] = {
		0.0f, 90.0f, 180.0f, -90.0f, -90.f, 90.f
	};

	glEnable(GL_DEPTH_TEST);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


	GLuint knifeVBO, knifeEBO, knifeVAO, lightVBO, lightEBO, lightVAO, light2VBO, light2EBO, light2VAO;

	glGenBuffers(1, &knifeVBO); // Create VBO
	glGenBuffers(1, &knifeEBO); // Create EBO

	glGenBuffers(1, &lightVBO); // Create VBO
	glGenBuffers(1, &lightEBO); // Create EBO

	glGenBuffers(1, &light2VBO); // Create VBO
	glGenBuffers(1, &light2EBO); // Create EBO

	glGenVertexArrays(1, &knifeVAO); // Create VOA
	glGenVertexArrays(1, &lightVAO); // Create VAO
	glGenVertexArrays(1, &light2VAO); // Create VAO

	glBindVertexArray(knifeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, knifeVBO); // Select VBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, knifeEBO); // Select EB
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // Load vertex attributes
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesKnife), indicesKnife, GL_STATIC_DRAW); // Load indices 
	 // Specify attribute location and layout to GPU
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);
	glBindVertexArray(0); // Unbind VOA or close off (Must call VOA explicitly in loop)


	//Bind and release light object
	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lightVBO); // Select VBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lightEBO); // Select EBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(lampVertices), lampVertices, GL_STATIC_DRAW); // Load vertex attributes
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesBox), indicesBox, GL_STATIC_DRAW); // Load indices 
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	//Bind and release light2 object
	glBindVertexArray(light2VAO);
	glBindBuffer(GL_ARRAY_BUFFER, light2VBO); // Select VBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, light2EBO); // Select EBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(lampVertices), lampVertices, GL_STATIC_DRAW); // Load vertex attributes
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesBox), indicesBox, GL_STATIC_DRAW); // Load indices 
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	//Load textures
	int knifeTexWidth, knifeTexHeight;
	unsigned char* knifeImage = SOIL_load_image("metalTex.png", &knifeTexWidth, &knifeTexHeight, 0, SOIL_LOAD_RGB);

	//Generate textures 
	GLuint knifeTextures;
	glGenTextures(1, &knifeTextures);
	glBindTexture(GL_TEXTURE_2D, knifeTextures);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, knifeTexWidth, knifeTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, knifeImage);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(knifeImage);
	glBindTexture(GL_TEXTURE_2D, 0);


	// Vertex shader source code
	string vertexShaderSource =
		"#version 330 core\n"
		"layout(location = 0) in vec3 vPosition;"
		"layout(location = 1) in vec3 aColor;"
		"layout(location = 2) in vec2 texCoord;"
		"layout(location = 3) in vec3 normal;"
		"out vec3 oColor;"
		"out vec2 oTexCoord;"
		"out vec3 oNormal;"
		"out vec3 FragPos;"
		"uniform mat4 model;"
		"uniform mat4 view;"
		"uniform mat4 projection;"

		"void main()\n"
		"{\n"
		"gl_Position = projection * view * model * vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);"
		"oColor = aColor;"
		"oTexCoord = vec2(1.0f - texCoord.x, 1.0f - texCoord.y);"
		"oNormal = mat3(transpose(inverse(model))) * normal;"
		"FragPos = vec3(model * vec4(vPosition, 1.0f));"
		"}\n";

	// Fragment shader source code
	string fragmentShaderSource =
		"#version 330 core\n"
		"in vec3 oColor;"
		"in vec2 oTexCoord;"
		"in vec3 oNormal;"
		"in vec3 FragPos;"

		"out vec4 fragColor;"

		"uniform sampler2D myTexture;"
		"uniform vec3 objectColor;"
		"uniform vec3 viewPos;"
		"uniform vec3 lightColor;"
		"uniform vec3 lightPos;"
		"uniform vec3 lightColor1;"
		"uniform vec3 lightPos1;"
		"uniform vec3 result;"

		"void main()\n"
		"{\n"
		"//Ambient\n"
		"float ambientStrength = 0.3f;"
		"vec3 ambient = ambientStrength * lightColor;"
		"vec3 ambient1 = ambientStrength * lightColor1;"

		"//Diffuse\n"
		"vec3 norm = normalize(oNormal);"
		"vec3 lightDir = normalize(lightPos - FragPos);"
		"vec3 lightDir1 = normalize(lightPos1 - FragPos);"
		"float diff = max(dot(norm, lightDir), 0.0);"
		"float diff1 = max(dot(norm, lightDir1), 0.0);"
		"vec3 diffuse = diff * lightColor;"
		"vec3 diffuse1 = diff1 * lightColor1;"

		"//Specularity\n"
		"float specularStrength = 10.25f;"
		"vec3 viewDir = normalize(viewPos - FragPos);"
		"vec3 reflectDir = reflect(-lightDir, norm);"
		"vec3 reflectDir1 = reflect(-lightDir1, norm);"
		"float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);"
		"float spec1 = pow(max(dot(viewDir, reflectDir1), 0.0), 128);"
		"vec3 specular = specularStrength * spec * lightColor;"
		"vec3 specular1 = specularStrength * spec1 * lightColor1;"

		"vec3 result = (ambient + diffuse + specular + ambient1 + diffuse1 + specular1) * objectColor;"
		"fragColor = texture(myTexture, oTexCoord) * vec4(result, 1.0f);"

		"}\n";


	// Lamp Vertex shader source code
	string lampVertexShaderSource =
		"#version 330 core\n"
		"layout(location = 0) in vec3 vPosition;"
		"uniform mat4 model;"
		"uniform mat4 view;"
		"uniform mat4 projection;"
		"void main()\n"
		"{\n"
		"gl_Position = projection * view * model * vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);"
		"}\n";

	// Lamp Fragment shader source code
	string lampFragmentShaderSource =
		"#version 330 core\n"
		"out vec4 fragColor;"
		"void main()\n"
		"{\n"
		"fragColor = vec4(0.1f, 0.0f, 0.0f, 0.0f);"
		"}\n";

	// Lamp Vertex shader source code
	string lamp2VertexShaderSource =
		"#version 330 core\n"
		"layout(location = 0) in vec3 vPosition;"
		"uniform mat4 model;"
		"uniform mat4 view;"
		"uniform mat4 projection;"
		"void main()\n"
		"{\n"
		"gl_Position = projection * view * model * vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);"
		"}\n";

	// Lamp Fragment shader source code
	string lamp2FragmentShaderSource =
		"#version 330 core\n"
		"out vec4 fragColor;"
		"void main()\n"
		"{\n"
		"fragColor = vec4(1.0f);"
		"}\n";


	// Creating Shader Program
	GLuint shaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);

	//Creating Lamp Shader Program
	GLuint lampShaderProgram = CreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource);
	GLuint lamp2ShaderProgram = CreateShaderProgram(lamp2VertexShaderSource, lamp2FragmentShaderSource);
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		// Set frame time
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Resize window and graphics simultaneously
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);

		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use Shader Program exe and select VAO before drawing 
		glUseProgram(shaderProgram); // Call Shader per-frame when updating attributes


		//Declare transformations (can be initialized outside loop)		
		glm::mat4 projectionMatrix;

		//Define LookAt Matrix
		viewMatrix = glm::lookAt(cameraPosition, target, worldUp);

		// Define projection matrix
		if (viewType == true) {
			//Switches to orthographic projection
			projectionMatrix = glm::ortho(-5.0f, (GLfloat)width/100, (GLfloat)height/125, -1.0f, 0.1f, 100.0f);
		}
		else
		{
			//Standard set for perspective projection
			projectionMatrix = glm::perspective(fov, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
		}

		// Get matrix's uniform location and set matrix
		GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
		GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
		GLint projLoc = glGetUniformLocation(shaderProgram, "projection");

		//Get light and object color location and lightPos
		GLint objectColorLoc = glGetUniformLocation(shaderProgram, "objectColor");
		GLint objectLightCol = glGetUniformLocation(shaderProgram, "lightColor");
		GLint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
		GLint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
		GLint objectLightCol1 = glGetUniformLocation(shaderProgram, "lightColor1");
		GLint lightPosLoc1 = glGetUniformLocation(shaderProgram, "lightPos1");


		//Assign Light and object Colors
		glUniform3f(objectColorLoc, 1.0f, 1.0f, 1.0f);
		glUniform3f(objectLightCol, 0.1f, 0.0f, 0.0f);

		//Assign second light color 
		glUniform3f(objectLightCol1, 1.0f, 1.0f, 1.0f);

		//Assign light positions
		glUniform3f(lightPosLoc, lightPosition1.x, lightPosition1.y, lightPosition1.z);
		glUniform3f(lightPosLoc1, lightPosition2.x, lightPosition2.y, lightPosition2.z);

		//Specify View position
		glUniform3f(viewPosLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

		//Bind texture
		glBindTexture(GL_TEXTURE_2D, knifeTextures);
		glBindVertexArray(knifeVAO); // User-defined VAO must be called before draw. 

		//Draw Knife
		for (GLuint i = 0; i <= 104; i++)
		{
			glm::mat4 modelMatrix;
			modelMatrix = glm::scale(modelMatrix, planeScale[0]);
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
			// Draw primitive(s)
			drawKnife();
		}

		// Unbind Shader exe and VOA after drawing per frame
		glBindVertexArray(0); //Incase different VAO wii be used after
		glUseProgram(0);


		glUseProgram(lampShaderProgram);
		GLint lampModelLoc = glGetUniformLocation(lampShaderProgram, "model");
		GLint lampViewLoc = glGetUniformLocation(lampShaderProgram, "view");
		GLint lampProjLoc = glGetUniformLocation(lampShaderProgram, "projection");
		glUniformMatrix4fv(lampViewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
		glUniformMatrix4fv(lampProjLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
		glBindVertexArray(lightVAO); // User-defined VAO must be called before draw. 
		//Loop to make first light
		for (GLuint i = 0; i < 6; i++)
		{
			glm::mat4 modelMatrix;
			modelMatrix = glm::translate(modelMatrix, planePositionsBox[i] / glm::vec3(8.0, 8.0, 8.0) + lightPosition1);
			modelMatrix = glm::rotate(modelMatrix, planeRotationsBox[i] * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			modelMatrix = glm::scale(modelMatrix, glm::vec3(.125f, .125f, .125f));
			if (i >= 4)
				modelMatrix = glm::rotate(modelMatrix, planeRotationsBox[i] * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
			glUniformMatrix4fv(lampModelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
			// Draw primitive(s)
			drawLamp();
		}
		glBindVertexArray(0); //Incase different VAO wii be used after
		glUseProgram(0);

		glUseProgram(lamp2ShaderProgram);

		//Specify View position
		GLint lamp2ModelLoc = glGetUniformLocation(lamp2ShaderProgram, "model");
		GLint lamp2ViewLoc = glGetUniformLocation(lamp2ShaderProgram, "view");
		GLint lamp2ProjLoc = glGetUniformLocation(lamp2ShaderProgram, "projection");
		glUniformMatrix4fv(lamp2ViewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
		glUniformMatrix4fv(lamp2ProjLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

		glBindVertexArray(light2VAO); // User-defined VAO must be called before draw. 

		//Loop to make second light
		for (GLuint i = 0; i < 6; i++)
		{
			glm::mat4 modelMatrix;
			modelMatrix = glm::translate(modelMatrix, planePositionsBox[i] / glm::vec3(8.0, 8.0, -8.0) + lightPosition2);
			modelMatrix = glm::rotate(modelMatrix, planeRotationsBox[i] * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			modelMatrix = glm::scale(modelMatrix, glm::vec3(0.125f, 0.125f, 0.125f));
			if (i >= 4)
				modelMatrix = glm::rotate(modelMatrix, planeRotationsBox[i] * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
			glUniformMatrix4fv(lamp2ModelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
			// Draw primitive(s)
			drawLamp();
		}
		glBindVertexArray(0);
		glUseProgram(0); // Incase different shader will be used after
		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();

		// Poll Camera Transformations
		TransformCamera();

	}

	//Clear GPU resources
	glDeleteVertexArrays(1, &knifeVAO);
	glDeleteBuffers(1, &knifeVBO);
	glDeleteBuffers(1, &knifeEBO);
	glDeleteVertexArrays(1, &lightVAO);
	glDeleteBuffers(1, &lightVBO);
	glDeleteBuffers(1, &lightEBO);
	glDeleteVertexArrays(1, &light2VAO);
	glDeleteBuffers(1, &light2VBO);
	glDeleteBuffers(1, &light2EBO);
	glfwTerminate();
	return 0;
}

// Define input functions
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	// Display ASCII Key code
	//std::cout <<"ASCII: "<< key << std::endl;	

	// Close window
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	// Assign true to Element ASCII if key pressed
	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE) // Assign false to Element ASCII if key released
		keys[key] = false;

}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{

	// Clamp FOV
	if (fov >= 1.0f && fov <= 55.0f)
		fov -= yoffset * 0.01;

	// Default FOV
	if (fov < 1.0f)
		fov = 1.0f;
	if (fov > 55.0f)
		fov = 55.0f;

}
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{

	if (firstMouseMove)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouseMove = false;
	}
	// Calculate mouse offset (Easing effect)
	xChange = xpos - lastX;
	yChange = lastY - ypos; // Inverted cam

							// Get current mouse (always starts at 0)
	lastX = xpos;
	lastY = ypos;


	if (isOrbiting)
	{
		// Update raw yaw and pitch with mouse movement
		rawYaw += xChange;
		rawPitch += yChange;

		// Conver yaw and pitch to degrees, and clamp pitch
		degYaw = glm::radians(rawYaw);
		degPitch = glm::clamp(glm::radians(rawPitch), -glm::pi<float>() / 2.f + .1f, glm::pi<float>() / 2.f - .1f);

		// Azimuth Altitude formula
		cameraPosition.x = target.x + radius * cosf(degPitch) * sinf(degYaw);
		cameraPosition.y = target.y + radius * sinf(degPitch);
		cameraPosition.z = target.z + radius * cosf(degPitch) * cosf(degYaw);
	}
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mode)
{
	// Assign boolean state to element Button code
	if (action == GLFW_PRESS)
		mouseButtons[button] = true;
	else if (action == GLFW_RELEASE)
		mouseButtons[button] = false;
}

// Define TransformCamera function
void TransformCamera()
{
	// Pan Camera
	if (keys[GLFW_KEY_LEFT_ALT] && mouseButtons[GLFW_MOUSE_BUTTON_MIDDLE]) {
		isPanning = true;
	}
	else {
		isPanning = false;
	}

	//Orbit Camera 
	if (keys[GLFW_KEY_LEFT_ALT] && mouseButtons[GLFW_MOUSE_BUTTON_LEFT]) {
		isOrbiting = true;
	}
	else {
		isOrbiting = false;
	}

	//Reset Camera 
	if (keys[GLFW_KEY_F]) {
		initiateCamera();
	}

	//Swith between Ortho
	if (keys[GLFW_KEY_HOME]) {
		viewType = true;
	}
	else
	{
		viewType = false;
	}
}

// Define 
void initiateCamera()
{	// Define Camera Attributes
	cameraPosition = glm::vec3(0.0f, 0.0f, 3.0f); // Move 3 units back in z towards screen
	target = glm::vec3(0.0f, 0.0f, 0.0f); // What the camera points to
	cameraDirection = glm::normalize(cameraPosition - cameraDirection); // direction z
	worldUp = glm::vec3(0.0, 1.0f, 0.0f);
	cameraRight = glm::normalize(glm::cross(worldUp, cameraDirection));// right vector x
	cameraUp = glm::normalize(glm::cross(cameraDirection, cameraRight)); // up vector y
	CameraFront = glm::vec3(0.0f, 0.0f, -1.0f); // 1 unit away from lense
}
