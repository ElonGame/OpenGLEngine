#include "SelectionManager.h"

#include "PathPointMesh.h"
#include "PathPointShapes.h"

using namespace std;
using namespace glm;

SelectionManager* SelectionManager::instance;

void SelectionManager::Start()
{
	instance = this;
}

void SelectionManager::Update(GLfloat deltaTime)
{
	if (InputManager::GetMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT))
	{
		RaycastHit hit;
		vec3 rayPos = Camera::main->transform->position;
		vec3 rayDir = Physics::GetRayFromMouse();
		if (selectedEnt && Physics::RaycastScene(rayPos, rayDir, hit, 1))
		{
			selectedHandle = hit.entity;
			HandleSelection(hit);
		}
		else if (Physics::RaycastScene(rayPos, rayDir, hit))
		{
			selectedEnt = hit.entity;
			selectedHandle = 0;
			HandleSelection(hit);
		}
		else
		{
			selectedEnt = 0;
			selectedHandle = 0;
		}
	}

	if (selectedEnt) 
	{
		if (InputManager::GetMouseButton(GLFW_MOUSE_BUTTON_LEFT))
		{
			if (InputManager::GetKey(GLFW_KEY_LEFT_SHIFT))
				RotateSelection();
			else
				MoveSelection();
		}
		else if (InputManager::GetKeyDown(GLFW_KEY_SPACE))
			CreatePathPoint();
	}
}

void SelectionManager::HandleSelection(RaycastHit& hit)
{
	planePos = hit.point;
	useHeightPlane = InputManager::GetKey(GLFW_KEY_LEFT_CONTROL);
	selectionOffset = hit.entity->transform->position - planePos;
	lastMove = vec3(0.0f);
}

void SelectionManager::RotateSelection()
{
	// Deform path if possible
	PathPoint* point = selectedEnt->GetComponent<PathPoint>();
	if (point)
	{
		float rotateAmount = 0.01f * InputManager::mouseDelta.x;
		point->angle = fmodf((point->angle + rotateAmount), two_pi<float>()); // Keep angle within 360 degrees
		//selectedEnt->transform->rotation *= angleAxis(rotateAmount, vec3(0.0f, 0.0f, 1.0f));

		DeformPoint(point);

		// Draw the new up vector
		quat rot = angleAxis(point->angle, point->GetDirection());
		vec3 up = QuaternionUtil::GetUp(rot);
		LineUtil::DrawRay(selectedEnt->transform->position, up * 4.0f);
		//LineUtil::DrawRay(selectedEnt->transform->position, selectedEnt->transform->GetUp() * 4.0f);
	}
}

void SelectionManager::MoveSelection()
{
	vec3 rayPos = Camera::main->transform->position;
	vec3 rayDir = Physics::GetRayFromMouse();
	vec3 planeNorm = (useHeightPlane) ? Camera::main->GetForward() : vec3(0.0f, 1.0f, 0.0f);
	RaycastHit hit;
	if (Physics::RaycastPlane(rayPos, rayDir, planePos, planeNorm, hit))
	{
		if (useHeightPlane)
		{
			hit.point.x = planePos.x;
			hit.point.z = planePos.z;
		}

		// Move a handle if it is selected, otherwise move the selected entity
		Transform* moveObj = (selectedHandle) ? selectedHandle->transform : selectedEnt->transform;
		moveObj->position = hit.point + selectionOffset;

		// Calculate move delta
		vec3 move = hit.point - planePos;
		vec3 moveDelta = move - lastMove;
		lastMove = move;
		//cout << moveDelta.x << ", " << moveDelta.y << ", " << moveDelta.z << endl;

		// Update paths if possible
		PathPoint* point = selectedEnt->GetComponent<PathPoint>();
		if (point)
		{
			if (selectedHandle)
			{
				PathPointHandle* handle = selectedHandle->GetComponent<PathPointHandle>();
				if (handle)
				{
					handle->UpdatePosition(moveDelta);

					vec3 dir = -point->GetDirection();
					if (useHeightPlane)
					{
						vec3 up = normalize(cross(dir, transform->GetForward()));
						mat4 rot = lookAt(vec3(), dir, up);
						point->transform->rotation = quat(transpose(rot));
					}
					else
					{
						//quat rot = QuaternionUtil::LookRotation(dir, point->transform->GetUp()/*vec3(0.0f, 1.0f, 0.0f)*/);
						//point->transform->rotation = rot;
						//mat4 rot = orientation(dir, point->transform->GetUp());
						mat4 rot = lookAt(vec3(), dir, point->transform->GetUp());
						point->transform->rotation = quat(transpose(rot));
					}
				}
			}
			else
				point->UpdateHandles(moveDelta);

			DeformPoint(point);
		}
	}
}

void SelectionManager::CreatePathPoint()
{
	PathPoint* point = selectedEnt->GetComponent<PathPoint>();
	if (!point || point->IsIntermediate())
		return;

	PathPointMesh* pointMesh = selectedEnt->GetComponent<PathPointMesh>();
	PathPointShapes* pointShapes = selectedEnt->GetComponent<PathPointShapes>();
	bool isPointMesh = (pointMesh) ? true : false;

	vec3 rayPos = Camera::main->transform->position;
	vec3 rayDir = Physics::GetRayFromMouse();
	vec3 planePos = selectedEnt->transform->position;
	vec3 planeNorm = vec3(0.0f, 1.0f, 0.0f);
	RaycastHit hit;
	if (Physics::RaycastPlane(rayPos, rayDir, planePos, planeNorm, hit))
	{
		Entity* ent = EntityManager::CreateEntity();
		PathPoint* newPoint;
		if (isPointMesh)
			newPoint = ent->AddComponent<PathPointMesh>();
		else
			newPoint = ent->AddComponent<PathPointShapes>();

		vec3 dir;
		if (point->IsEnd()) // Link as end point
		{
			dir = normalize(hit.point - point->transform->position);

			point->next = newPoint;
			newPoint->prev = point;
		}
		else // Link as start point
		{
			dir = normalize(point->transform->position - hit.point);

			point->prev = newPoint;
			newPoint->next = point;
		}

		if (isPointMesh)
			((PathPointMesh*)newPoint)->Init(*pointMesh->lodMin, *pointMesh->lodMid, *pointMesh->lodMax, hit.point, dir);
		else
			((PathPointShapes*)newPoint)->Init(pointShapes->shapes, hit.point, dir);
		DeformPoint(newPoint);

		selectedEnt = newPoint->entity;
	}
}

void SelectionManager::DeformPoint(PathPoint* point)
{
	if (point->prev)
		point->prev->DeformPath();
	if (point->next)
		point->DeformPath();
}
