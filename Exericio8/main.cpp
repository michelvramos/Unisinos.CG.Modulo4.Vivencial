/*
* 
MODULO 04 VIVENCIAL 02
1) Implementar um fator de atenuação na parcela de reflexão difusa
2) Utilizar 3 fontes de luz pontuais posicionadas a partir de um objeto principal da cena, com intensidades
calibradas de acordo com sua função (principal, preenchimento e de fundo). Como são luzes pontuais,
elas não possuem rotação. Apesar de existirem algumas diretrizes de ângulos, posicionamentos e
intensidades para essas 3 luzes, não existe uma regra única. O importante é que elas cumpram as suas
funções como descrito acima.
3) Permitir, através de teclas, desabilitar e habilitar cada uma das 3 fontes de luz.

***************  INSTRUÇÕES AO PROFESSOR  ***************

	Utilize as teclas 8,9 e 0 para habilitar/desabilitar a luz 1, 2 e 3 respectivamente.

*/
#include <iostream>
#include <string>
#include <assert.h>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "SimpleOBJLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Mesh.h"
#include "MyMaterial.h"


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
int setupShader(GLint& success);
void setupCube(Mesh& mesh);
void Draw(GLint location, glm::mat4 model, Mesh& mesh);
GLuint loadTexture(const char* filePath);
std::string LoadShaderSource(const std::string& filePath);
void EnableLight(GLuint shaderId, const char* name, glm::vec3 color);

const GLuint WIDTH = (GLuint)(720 * 1.7), HEIGHT = 720;

bool rotateX = false, rotateY = false, rotateZ = false,
moveLeft = false, moveRight = false,
moveUp = false, moveDown = false,
moveFront = false, moveBack = false,
scaleUp = false, scaleDown = false,
enableLight1 = true, enableLight2 = true, enableLight3 = true;


int main()
{
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, u8"Computação Gráfica - Módulo 04 - Vivencial 2", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);

	// GLAD: carrega todos os ponteiros de funções da OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
	}

	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	GLint setupShaderSuccess;
	GLuint texturedAOShaderID = setupShader(setupShaderSuccess);

	if (!setupShaderSuccess)
	{
		return 0;
	}

	Mesh cubeMesh;
	Mesh sphereMesh;
	MyMaterial cubeMaterial, sphereMaterial;

	SimpleOBJLoader::Load("Assets/cube.obj", cubeMesh);
	SimpleOBJLoader::Load("Assets/sphere.obj", sphereMesh);
	SimpleOBJLoader::LoadMTL("Assets/cube.mtl", cubeMaterial);
	SimpleOBJLoader::LoadMTL("Assets/sphere.mtl", sphereMaterial);

	glUseProgram(texturedAOShaderID);

	GLuint colorTextureID = loadTexture("Assets/Bricks059_1K-JPG_Color.jpg");
	GLuint ambientOclusionTextureID = loadTexture("Assets/Bricks059_1K-JPG_AmbientOcclusion.jpg");

	GLint textureUniLocation = glGetUniformLocation(texturedAOShaderID, "colorTexture");
	GLint aoTextureUniLocation = glGetUniformLocation(texturedAOShaderID, "aoMap");
	GLint modelUniLocation = glGetUniformLocation(texturedAOShaderID, "model");

	glm::mat4 cubeModel1Transform = glm::mat4(1.0f);
	glm::mat4 cubeModel2Transform = glm::mat4(1.0f);
	glm::mat4 sphereModel1Transform = glm::mat4(1.0f);

	const glm::vec3 cube1StartPosition = glm::vec3(-1.0f, 0.0f, 0.0f);
	const glm::vec3 cube2StartPosition = glm::vec3(1.0f, 0.0f, 0.0f);
	const glm::vec3 sphere1StartPosition = glm::vec3(0.0f, -0.0f, 0.0f);

	cubeModel1Transform = glm::translate(cubeModel1Transform, cube1StartPosition);
	cubeModel2Transform = glm::translate(cubeModel2Transform, cube2StartPosition);
	sphereModel1Transform = glm::translate(sphereModel1Transform, sphere1StartPosition);

	// Matriz de projeção (perspectiva)
	glm::mat4 projectionMatrix = glm::perspective(glm::radians(10.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
	GLint projectionUniLocation = glGetUniformLocation(texturedAOShaderID, "projectionMatrix");
	glUniformMatrix4fv(projectionUniLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

	// Matriz de visão (posição da câmera)
	glm::mat4 viewMatrix = glm::lookAt(
		glm::vec3(0.0f, 0.0f, 15.0f), // Posição da câmera
		glm::vec3(0.0f, 0.0f, 0.0f),  // Para onde a câmera olha
		glm::vec3(0.0f, 1.0f, 0.0f)   // Direção "para cima"
	);

	GLint viewUniLocation = glGetUniformLocation(texturedAOShaderID, "viewMatrix");
	glUniformMatrix4fv(viewUniLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	float ka = 0.5f, kd = 0.8f, ks = 0.05, shininess = 8.0f;

	glm::vec3 lightTransform = glm::vec3(0.6f, 1.2f, 2.5f);
	glm::vec3 lightTransformFill = glm::vec3(-1.0f, 0.8f, 2.0f); // Luz secundária (fill light)
	glm::vec3 lightTransformBack = glm::vec3(0.0f, 1.5f, -2.5f); // Luz de recorte (back light)

	glm::vec3 lightColor = glm::vec3(1.0f, 0.957f, 0.898f); // main light color (warm)
	glm::vec3 lightColorFill = glm::vec3(0.7f, 0.7f, 0.8f); // fill light (cooler, bluish)
	glm::vec3 lightColorBack = glm::vec3(1.0f, 1.0f, 1.0f); // back light (neutral white)
	glm::vec3 blackColor = glm::vec3(0.0f, 0.0f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); // Descarta apenas faces traseiras
	glFrontFace(GL_CCW); // Define sentido anti-horário como frente	

	float angleIncrement = 180.0f;
	float speedFactor = 2.5f;
	float scaleFactor = 1.0f;
	float currentFrameTime = 0.0f;
	float lastFrameTime = (float)glfwGetTime();
	float deltaTime = 0.0f;

	glm::vec3 translationOffset = glm::vec3(0.0f);
	glm::vec3 scaleFactorAccumulated = glm::vec3(1.0f);
	glm::vec3 rotationAngle = glm::vec3(0.0f);

	while (!glfwWindowShouldClose(window))
	{
		currentFrameTime = (float)glfwGetTime();
		deltaTime = currentFrameTime - lastFrameTime;
		lastFrameTime = currentFrameTime;

		glfwPollEvents();
		glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorTextureID);
		glUniform1i(textureUniLocation, 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, ambientOclusionTextureID);
		glUniform1i(aoTextureUniLocation, 1);

		glUniform3f(glGetUniformLocation(texturedAOShaderID, "ka"), cubeMaterial.ka.x, cubeMaterial.ka.y, cubeMaterial.ka.z);
		glUniform3f(glGetUniformLocation(texturedAOShaderID, "kd"), cubeMaterial.kd.x, cubeMaterial.kd.y, cubeMaterial.kd.z);

		glUniform3f(glGetUniformLocation(texturedAOShaderID, "mainLight"), lightTransform.x, lightTransform.y, lightTransform.z);
		glUniform3f(glGetUniformLocation(texturedAOShaderID, "fillLight"), lightTransformFill.x, lightTransformFill.y, lightTransformFill.z);
		glUniform3f(glGetUniformLocation(texturedAOShaderID, "backLight"), lightTransformBack.x, lightTransformBack.y, lightTransformBack.z);

		EnableLight(texturedAOShaderID, "lightColor", enableLight1 ? lightColor : blackColor);
		EnableLight(texturedAOShaderID, "lightColorFill", enableLight2 ? lightColorFill : blackColor);
		EnableLight(texturedAOShaderID, "lightColorBack", enableLight3 ? lightColorBack : blackColor);

		glUniform1f(glGetUniformLocation(texturedAOShaderID, "ks"), ks);
		glUniform1f(glGetUniformLocation(texturedAOShaderID, "shininess"), shininess);

		glm::vec3 modelTranslation = glm::vec3(0.0f);
		glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scaleFactorAccumulated);
		glm::mat4 rotationMatrix = glm::mat4(1.0f);

		// Translação
		if (moveLeft)
		{
			modelTranslation.x -= speedFactor * deltaTime;
		}
		if (moveRight)
		{
			modelTranslation.x += speedFactor * deltaTime;
		}
		if (moveUp)
		{
			modelTranslation.y += speedFactor * deltaTime;
		}
		if (moveDown)
		{
			modelTranslation.y -= speedFactor * deltaTime;
		}
		if (moveFront)
		{
			modelTranslation.z -= speedFactor * deltaTime;
		}
		if (moveBack)
		{
			modelTranslation.z += speedFactor * deltaTime;
		}

		translationOffset += modelTranslation;

		// Escala
		if (scaleDown)
		{
			scaleFactorAccumulated *= (1.0f - deltaTime * scaleFactor);
		}
		else if (scaleUp)
		{
			scaleFactorAccumulated *= (1.0f + deltaTime * scaleFactor);
		}

		// Rotação
		if (rotateX)
		{
			rotationAngle.x += angleIncrement * deltaTime;
		}
		if (rotateY)
		{
			rotationAngle.y += angleIncrement * deltaTime;
		}
		if (rotateZ)
		{
			rotationAngle.z += angleIncrement * deltaTime;
		}

		// rotação acumulada
		rotationMatrix = glm::rotate(rotationMatrix, glm::radians(rotationAngle.x), glm::vec3(1, 0, 0));
		rotationMatrix = glm::rotate(rotationMatrix, glm::radians(rotationAngle.y), glm::vec3(0, 1, 0));
		rotationMatrix = glm::rotate(rotationMatrix, glm::radians(rotationAngle.z), glm::vec3(0, 0, 1));

		// translação acumulada
		glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), translationOffset);

		// transformações locais de rotação e escala
		glm::mat4 localTransform = rotationMatrix * scaleMatrix;

		// Aplicar transformações
		cubeModel1Transform = glm::translate(glm::mat4(1.0f), translationOffset + cube1StartPosition) * localTransform;
		cubeModel2Transform = glm::translate(glm::mat4(1.0f), translationOffset + cube2StartPosition) * localTransform;
		sphereModel1Transform = glm::translate(glm::mat4(1.0f), translationOffset + sphere1StartPosition) * localTransform;

		// desenhar na tela
		Draw(modelUniLocation, cubeModel1Transform, cubeMesh);
		Draw(modelUniLocation, cubeModel2Transform, cubeMesh);
		Draw(modelUniLocation, sphereModel1Transform, sphereMesh);

		glfwSwapBuffers(window);
	}

	// Pede pra OpenGL desalocar os buffers
	glDeleteVertexArrays(1, &cubeMesh.VAO);
	glDeleteTextures(1, &colorTextureID);
	glDeleteTextures(1, &ambientOclusionTextureID);
	glDeleteProgram(texturedAOShaderID);

	// Finaliza a execução da GLFW, limpando os recursos alocados por ela
	glfwTerminate();
	return 0;
}

/// <summary>
/// Desenha um mesh 3D aplicando a transformação fornecida.
/// </summary>
/// <param name="uniLocationID">Localização do uniform 'model' no shader program.</param>
/// <param name="objectTransform">matriz de transformação do objeto no mundo (posição, rotação e escala).</param>
/// <param name="mesh">Objeto contendo VAO e vertex count.</param>
void Draw(GLint uniLocationID, glm::mat4 objectTransform, Mesh& mesh)
{
	glUniformMatrix4fv(uniLocationID, 1, FALSE, glm::value_ptr(objectTransform));

	glBindVertexArray(mesh.VAO);
	glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount);

	glBindVertexArray(0);
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	rotateX = (key == GLFW_KEY_X) ? (action != GLFW_RELEASE) : rotateX;
	rotateY = (key == GLFW_KEY_Y) ? (action != GLFW_RELEASE) : rotateY;
	rotateZ = (key == GLFW_KEY_Z) ? (action != GLFW_RELEASE) : rotateZ;

	moveLeft = (key == GLFW_KEY_A) ? (action != GLFW_RELEASE) : moveLeft;
	moveRight = (key == GLFW_KEY_D) ? (action != GLFW_RELEASE) : moveRight;
	moveUp = (key == GLFW_KEY_I) ? (action != GLFW_RELEASE) : moveUp;
	moveDown = (key == GLFW_KEY_J) ? (action != GLFW_RELEASE) : moveDown;
	moveFront = (key == GLFW_KEY_W) ? (action != GLFW_RELEASE) : moveFront;
	moveBack = (key == GLFW_KEY_S) ? (action != GLFW_RELEASE) : moveBack;

	scaleDown = (key == GLFW_KEY_1) ? (action != GLFW_RELEASE) : scaleDown;
	scaleUp = (key == GLFW_KEY_2) ? (action != GLFW_RELEASE) : scaleUp;

	enableLight1 = (key == GLFW_KEY_8) && (action != GLFW_RELEASE) ? !enableLight1 : enableLight1;
	enableLight2 = (key == GLFW_KEY_9) && (action != GLFW_RELEASE) ? !enableLight2 : enableLight2;
	enableLight3 = (key == GLFW_KEY_0) && (action != GLFW_RELEASE) ? !enableLight3 : enableLight3;

}

//Esta função está basntante hardcoded - objetivo é compilar e "buildar" um programa de
// shader simples e único neste exemplo de código
// O código fonte do vertex e fragment shader está nos arrays vertexShaderSource e
// fragmentShader source no iniçio deste arquivo
// A função retorna o identificador do programa de shader
int setupShader(GLint& success)
{
	std::string vertexCode = LoadShaderSource("Shaders/cube.vert");
	std::string fragmentCode = LoadShaderSource("Shaders/cube.frag");

	const char* vertexShaderSource = vertexCode.c_str();
	const char* fragmentShaderSource = fragmentCode.c_str();

	// Vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	// Checando erros de compilação (exibição via log no terminal)
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		return 0;
	}

	// Fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	// Checando erros de compilação (exibição via log no terminal)
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		return 0;
	}

	// Linkando os shaders e criando o identificador do programa de shader
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	// Checando por erros de linkagem
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		return 0;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

GLuint loadTexture(const char* filePath)
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Parâmetros da textura
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Carregar imagem
	int width, height, nrChannels;
	unsigned char* data = stbi_load(filePath, &width, &height, &nrChannels, 0);

	if (data)
	{
		GLenum format;
		switch (nrChannels)
		{
		case 1:
			format = GL_RED;
			break;
		case 3:
			format = GL_RGB;
			break;
		case 4:
			format = GL_RGBA;
			break;
		default:
			format = GL_RGB; // fallback seguro
			break;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cerr << "Erro ao carregar a textura: " << filePath << std::endl;
	}

	stbi_image_free(data);
	return textureID;
}

std::string LoadShaderSource(const std::string& filePath)
{
	std::ifstream file(filePath);
	if (!file.is_open())
	{
		std::cerr << "Erro ao abrir o arquivo de shader: " << filePath << std::endl;
		return "";
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

void EnableLight(GLuint shaderId, const char* name, glm::vec3 color)
{
	glUniform3f(glGetUniformLocation(shaderId, name), color.x, color.y, color.z);
}
