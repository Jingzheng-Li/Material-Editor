
#pragma once

#include <core/GL_utils.h>

//���p1��p2�ֱ���ʲô �����
struct MassSpring {
	int p1, p2;
	float rest_length;
	float Ks, Kd;
	int type;
};

class CLOTH_SIMULATION
{

public:
	void IntegrateVerlet(float deltaTime);
	glm::vec3 GetVerletVelocity(glm::vec3 xi_new, glm::vec3 xi_last, float dt);
	void ComputeForces(float dt);
	void ApplyProvotDynamicInverse();
	void EllipsoidCollision();
	void StepPhysics(float dt);
	void OnIdle();


private:




};