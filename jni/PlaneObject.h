#ifndef _QCAR_PLANE_OBJECT_H_
#define _QCAR_PLANE_OBJECT_H_


#define NUM_PLANE_OBJECT_VERTEX 4
#define NUM_PLANE_OBJECT_INDEX 6

static const float planeVertices[NUM_PLANE_OBJECT_VERTEX * 3] =
{
	50.f, 80.f, 0.0f,
	50.f, -80.f, 0.0f,
	-50.f, -80.f, 0.0f,
	-50.f, 80.f, 0.0f
};

static const float planeNormals[NUM_PLANE_OBJECT_VERTEX * 3] =
{
	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 1.0f
};

static const float planeTexCoords[NUM_PLANE_OBJECT_VERTEX * 2] =
{
	1.0f, 1.0f,
	1.0f, 0.0f,
	0.0f, 0.0f,
	0.0f, 1.0f
};

static const unsigned short planeIndices[NUM_PLANE_OBJECT_INDEX] =
{
	0, 3, 2,
	0, 2, 1
};

#endif
