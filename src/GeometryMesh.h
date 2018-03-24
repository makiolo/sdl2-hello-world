/**
@file GeometryMesh.h

Geometry of mesh model

@author Ricardo Marmolejo García
@date 06/01/15
*/

#ifndef GEOMETRYMESH_H
#define GEOMETRYMESH_H

#define MAX_BONES_PER_VERTICES 4
#define MAX_BONES_PER_SKELETON 61

namespace dune {

struct ENGINE_API MeshBuffer
{
	float position[3];
	float normal[3];
	float coord[2];
	float color[4];

	static void build(GLsizei size)
	{
		glEnableVertexAttribArray(AttribPosition);
		glVertexAttribPointer(AttribPosition, 3, GL_FLOAT, GL_FALSE, size, BUFFER_OFFSET(0));

		glEnableVertexAttribArray(AttribNormal);
		glVertexAttribPointer(AttribNormal, 3, GL_FLOAT, GL_FALSE, size, BUFFER_OFFSET(sizeof(float) * 3));

		glEnableVertexAttribArray(AttribCoord);
		glVertexAttribPointer(AttribCoord, 2, GL_FLOAT, GL_FALSE, size, BUFFER_OFFSET(sizeof(float) * 6));

		glEnableVertexAttribArray(AttribColor);
		glVertexAttribPointer(AttribColor, 4, GL_FLOAT, GL_FALSE, size, BUFFER_OFFSET(sizeof(float) * 8));
	}
};

struct ENGINE_API MeshLocation
{
    GLint transform;
    GLint world;
    GLint position_camera;
	GLint texture0;

	void build(Shader* shader);
};

struct ENGINE_API MeshSkinnedBuffer : MeshBuffer
{
	float bone_index[MAX_BONES_PER_VERTICES];
	float weight[MAX_BONES_PER_VERTICES];

	static void build(GLsizei size)
	{
		DUNE_ASSERT(sizeof(MeshBuffer) == ((sizeof(float) * 12) + (sizeof(unsigned char) * 0)));

		MeshBuffer::build(size);

		glEnableVertexAttribArray(AttribBones);
		glVertexAttribPointer(AttribBones, MAX_BONES_PER_VERTICES, GL_FLOAT, GL_FALSE, size, BUFFER_OFFSET((sizeof(float) * 12)));

		glEnableVertexAttribArray(AttribWeights);
		glVertexAttribPointer(AttribWeights, MAX_BONES_PER_VERTICES, GL_FLOAT, GL_FALSE, size, BUFFER_OFFSET((sizeof(float) * (12 + MAX_BONES_PER_VERTICES))));
	}
};

struct ENGINE_API MeshSkinnedLocation
{
	GLint transform;
	GLint position_camera;
	GLint texture0;
	GLint boneWorldMatrix;
	GLint diffuseColor;

	void build(Shader* shader);
};

} // end namespace dune

#endif // GEOMETRYMESH_H

