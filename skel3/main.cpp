#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <windows.h>



using namespace std;
using namespace glm;
// Структура для збереження вершин моделі
struct Vertex {
	vec3 position;
};
// Структура для збереження сполучень моделі
struct Joint {
	vec3 position;
	int parentIndex;
};
// Структура для збереження моделі
struct Model {
	vector<Vertex> vertices;
	vector<Joint> joints;
};


double prevMouseX = 0.0, prevMouseY = 0.0;

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		glfwGetCursorPos(window, &prevMouseX, &prevMouseY);
	}
}

void mouse_move_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		double deltaX = xpos - prevMouseX;
		double deltaY = ypos - prevMouseY;
		glRotatef(deltaX, 0.0, 1.0, 0.0);
		glRotatef(deltaY, 1.0, 0.0, 0.0);
	}
	prevMouseX = xpos;
	prevMouseY = ypos;
}
bool isFirstModel = false;

void keycallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		switch (key)
		{
		case GLFW_KEY_UP:
			isFirstModel = true;
			break;
		}
	}
}


void DrawSphere(const glm::vec3& position) {
	GLUquadric* quad = gluNewQuadric();
	glPushMatrix();
	glTranslatef(position.x, position.y, position.z);
	glColor3f(0.0, 1.0, 0.0);
	gluQuadricDrawStyle(quad, GLU_LINE);
	gluSphere(quad, 0.03f, 32, 32);
	glPopMatrix();
	gluDeleteQuadric(quad);
}


// Функція для завантаження моделі з файлу .obj
class ModelLoader {
public:
	ModelLoader() {}
	Model loadModel(const char* objFilename, const char* skelFilename) {
		Model model;

		loadSkel(skelFilename, model.joints);
		return model;
	}
private:
	void loadSkel(const char* filename, std::vector<Joint>& joints) {
		std::ifstream file(filename);
		std::string line;
		while (std::getline(file, line)) {
			std::istringstream iss(line);
			Joint joint;
			iss >> joint.position.x >> joint.position.y >> joint.position.z >> joint.parentIndex;
			joints.push_back(joint);
		}
	}
};


class ModelRenderer {
public:
	ModelRenderer() {}
	void drawModel(Model& model) {

		drawJoints(model.joints);
	}
private:

	void drawJoints(std::vector<Joint>& joints) {
		glBegin(GL_QUADS);
		for (Joint& joint : joints) {
			if (joint.parentIndex != -1) {
				Joint& parent = joints[joint.parentIndex];
				glVertex3f(joint.position.x-0.01, joint.position.y + 0.01, joint.position.z + 0.01);
				glVertex3f(joint.position.x - 0.01, joint.position.y - 0.01, joint.position.z - 0.01);
				glVertex3f(parent.position.x - 0.01, parent.position.y - 0.01, parent.position.z - 0.01);
				glVertex3f(parent.position.x-0.01, parent.position.y + 0.01, parent.position.z + 0.01);

				glVertex3f(joint.position.x - 0.01, joint.position.y + 0.01, joint.position.z + 0.01);
				glVertex3f(joint.position.x + 0.01, joint.position.y + 0.01, joint.position.z - 0.01);
				glVertex3f(parent.position.x + 0.01, parent.position.y + 0.01, parent.position.z - 0.01);
				glVertex3f(parent.position.x - 0.01, parent.position.y + 0.01, parent.position.z + 0.01);

				glVertex3f(joint.position.x + 0.01, joint.position.y - 0.01, joint.position.z + 0.01);
				glVertex3f(joint.position.x + 0.01, joint.position.y + 0.01, joint.position.z - 0.01);
				glVertex3f(parent.position.x + 0.01, parent.position.y + 0.01, parent.position.z - 0.01);
				glVertex3f(parent.position.x + 0.01, parent.position.y - 0.01, parent.position.z + 0.01);

				glVertex3f(joint.position.x - 0.01, joint.position.y - 0.01, joint.position.z + 0.01) ;
				glVertex3f(joint.position.x + 0.01, joint.position.y - 0.01, joint.position.z - 0.01);
				glVertex3f(parent.position.x + 0.01, parent.position.y - 0.01, parent.position.z - 0.01);
				glVertex3f(parent.position.x - 0.01, parent.position.y - 0.01, parent.position.z + 0.01);
			
			}
		}
		glEnd();
	}
};

void animateHand(std::vector<Joint>& joints, int handIndex, float time) {
	// Пример анимации руки - изменение координаты Y в соответствии с временем
	float yOffset = sin(time / 10000) * 0.2f; // Изменение высоты руки
	Joint& handJoint = joints[handIndex];
	// Определяем максимальное значение роста
	float maxHeight = 0.1f;
	// Определяем период анимации (время, за которое рука поднимается и опускается)
	float animationPeriod = 2.0f;
	// Вычисляем текущую высоту руки
	float currentHeight = sin(time * (2 * 3.14) / animationPeriod) * maxHeight;
	// Устанавливаем положение руки в зависимости от текущей высоты
	handJoint.position.z = currentHeight;
}

void animateSkeleton(std::vector<Joint>& joints, float time) {
	// Пример анимации шагания скилета
	float stepDistance = 0.2f; // Дистанция шага
	float yOffset = sin(time * 2) * 0.2f; // Изменение высоты шага
	// Левая нога
	Joint& leftLegJoint1 = joints[9];
	Joint& leftLegJoint2 = joints[11];
	leftLegJoint1.position.z = -stepDistance / 2 + yOffset;
	leftLegJoint2.position.z = stepDistance / 2 + yOffset;
	// Правая нога
	Joint& rightLegJoint1 = joints[6];
	Joint& rightLegJoint2 = joints[7];
	rightLegJoint1.position.z = -stepDistance / 2 - yOffset;
	rightLegJoint2.position.z = stepDistance / 2 - yOffset;
	Joint& rightHandJoint1 = joints[3];
	Joint& rightHandJoint2 = joints[4];
	rightHandJoint1.position.z = -stepDistance / 2 - yOffset;
	rightHandJoint2.position.z = stepDistance / 2 - yOffset;
}
 
int Wi = 800, Hi = 600;

float point[3][2] =
{
 {-0.5,0.0},
 {0.0, 0.0},
 {0.0, 0.5},
};

int main(void)
{
	//кирилиця
	SetConsoleOutputCP(1251);
	SetConsoleCP(1251);

	GLFWwindow* window;
	if (!glfwInit())
		return -1;
	window = glfwCreateWindow(Wi, Hi, "GL", nullptr, nullptr);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cout << "glew не ініціалізовано" << std::endl;
		glfwTerminate();
		system("pause");
		return -1;
	}
	glfwGetFramebufferSize(window, &Wi, &Hi);
	glViewport(0, 0, Wi, Hi);
	glMatrixMode(GL_PROJECTION);
	vec3 cameraPosition = glm::vec3(0.0f, 2.0f, 3.0f);
	vec3 cameraTarget = glm::vec3(0.5f, 0.5f, 0.5f);
	vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	mat4 viewMatrix = glm::lookAt(cameraPosition, cameraTarget, cameraUp);
	mat4 projection = glm::perspective(glm::radians(45.0f), (float)Wi / (float)Hi, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	ModelRenderer modRender;
	ModelLoader mod;
	Model model = mod.loadModel("Model1.obj", "Model1.skel");

	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, mouse_move_callback);
	glfwSetKeyCallback(window, keycallback);
	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.0,1.0,1.0,1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glColor3f(1.0f, 0.5f, 0.3f);
		glPushMatrix();
		float time = glfwGetTime(); //визначення часу


		animateSkeleton(model.joints, time);
		modRender.drawModel(model);
		for (Joint& joint : model.joints) {
			DrawSphere(joint.position);
		}
		glPopMatrix();
		glPushMatrix();
		glDisableClientState(GL_VERTEX_ARRAY);
		glPopMatrix();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
	return 0;
}



