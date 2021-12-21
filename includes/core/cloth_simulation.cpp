

//想着这样应该是不靠谱的 因为点数太多了 根本不能实时模拟 上千个点

//所以必然不能所有的点都做一遍弹簧质点模拟 只能选取骨骼动画的关键点 然后通过关键点进行模拟 弹簧质点系统应该是必要的
//

#include <core/cloth_simulation.h>

//现在先定义一块布 然后把布料的点放进去
//首先没说的 最好肯定是放在GPU中 用GPU去计算
const unsigned int numX = 20, numY = 20;
const unsigned int total_points = (numX + 1) * (numY + 1);

float fullsize = 4.0f;
float halfsize = fullsize / 2.0f;


vector<GLuint> indices;
vector<MassSpring> springs;
vector<glm::vec3> X_new;//mesh point position
vector<glm::vec3> X_last;//pre_mesh point position
vector<glm::vec3> F;//force of every point


int oldX = 0, oldY = 0;
float rX = 15, rY = 0;
int state = 1;
float dist = -23;
const int GRID_SIZE = 10;


const int STRUCTURAL_SPRING = 0;
const int SHEAR_SPRING = 1;
const int BEND_SPRING = 2;
int spring_count = 0;

char info[MAX_PATH] = { 0 };

//dampling part
const float DEFAULT_DAMPING = -0.0125f;
//ks弹簧弹性系数 kd弹簧阻尼系数
float	KsStruct = 50.75f, KdStruct = -0.25f;
float	KsShear = 50.75f, KdShear = -0.25f;
float	KsBend = 50.95f, KdBend = -0.25f;
glm::vec3 gravity = glm::vec3(0.0f, -0.00981f, 0.0f);
float mass = 1.0f;

float timeStep = 1 / 60.0f;
float currentTime = 0;
double accumulator = timeStep;


void CLOTH_SIMULATION::IntegrateVerlet(float deltaTime)
{
	float deltaTime2Mass = (deltaTime * deltaTime) / mass;
	size_t i = 0;

	//update point position information
	for (i = 0; i < total_points; i++)
	{
		glm::vec3 buffer = X_new[i];
		X_new[i] = X_new[i] + (X_new[i] - X_last[i]) + deltaTime2Mass * F[i];
		X_last[i] = buffer;
		if (X_new[i].y < 0) {
			X_new[i].y = 0;
		}
	}
}

glm::vec3 CLOTH_SIMULATION::GetVerletVelocity(glm::vec3 xi_new, glm::vec3 xi_last, float dt)
{
	return (xi_new - xi_last) / dt;
}

void CLOTH_SIMULATION::ComputeForces(float dt)
{
	for (unsigned int i = 0; i < total_points; i++)
	{
		F[i] = glm::vec3(0);
		glm::vec3 V = GetVerletVelocity(X_new[i], X_last[i], dt);//产生t时刻的速度
		//add gravity force
		if (i != 0 && i != (numX)) F[i] += gravity * mass;//为非边界点添加力
		F[i] += DEFAULT_DAMPING * V;
	}

	for (unsigned int i = 0; i < springs.size(); ++i)
	{
		glm::vec3 p1 = X_new[springs[i].p1];
		glm::vec3 p1Last = X_last[springs[i].p1];
		glm::vec3 p2 = X_new[springs[i].p2];
		glm::vec3 p2Last = X_last[springs[i].p2];

		glm::vec3 v1 = GetVerletVelocity(p1, p1Last, dt);
		glm::vec3 v2 = GetVerletVelocity(p2, p2Last, dt);

		glm::vec3 deltaP = p1 - p2;
		glm::vec3 deltaV = v1 - v2;
		float dist = glm::length(deltaP);

		float leftTerm = -springs[i].Ks * (dist - springs[i].rest_length);
		float rightTerm = springs[i].Kd * (glm::dot(deltaV, deltaP) / dist);
		glm::vec3 springForce = (leftTerm + rightTerm) * glm::normalize(deltaP);

		if (springs[i].p1 != 0 && springs[i].p1 != numX) F[springs[i].p1] += springForce;
		if (springs[i].p2 != 0 && springs[i].p2 != numX) F[springs[i].p2] -= springForce;

	}
}

void CLOTH_SIMULATION::ApplyProvotDynamicInverse()
{
	for (unsigned int i = 0; i < springs.size(); ++i)
	{
		glm::vec3 p1 = X_new[springs[i].p1];
		glm::vec3 p2 = X_new[springs[i].p2];
		glm::vec3 deltaP = p1 - p2;
		float dist = glm::length(deltaP);
		if (dist > springs[i].rest_length)
		{
			dist -= (springs[i].rest_length);
			dist *= 0.5f;
			deltaP = glm::normalize(deltaP);
			deltaP *= dist;
			if (springs[i].p1 == 0 || springs[i].p1 == numX) {
				X_new[springs[i].p2] += deltaP;
			}
			else if (springs[i].p2 == 0 || springs[i].p2 == numX) {
				X_new[springs[i].p1] -= deltaP;
			}
			else {
				X_new[springs[i].p1] -= deltaP;
				X_new[springs[i].p2] += deltaP;
			}
		}
	}
}



