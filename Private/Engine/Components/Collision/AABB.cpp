#include "Public/Engine/Components/Collision/AABB.h"

unsigned int AABB::staticVAO = 0;
unsigned int AABB::staticVBO = 0;
unsigned int AABB::staticEBO = 0;
bool AABB::buffersInitialized = false;