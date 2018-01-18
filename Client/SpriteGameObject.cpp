#include "SpriteGameObject.h"
#include "Gizmos.h"
using aie::Gizmos;


SpriteGameObject::SpriteGameObject()
{
}


SpriteGameObject::~SpriteGameObject()
{
}

void SpriteGameObject::Draw()
{
	Gizmos::addAABB(localPosition, glm::vec3(0.2f, 1.0f, 0.2f), data.colour);
	//GameObject::Draw();
}
