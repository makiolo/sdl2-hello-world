/**
@file GeometryUI.h

OpenGL VAO abstraction for GUI

@author Ricardo Marmolejo García
@date 04/01/15
*/

#ifndef GEOMETRYARRAY_H
#define GEOMETRYARRAY_H

#include <memory>
#include <GL/glew.h>
#include <GL/gl.h>

typedef enum {
	AttribPosition,
	AttribNormal,
	AttribCoord,
	AttribColor,
	AttribBones,
	AttribWeights
} GLKVertexAttrib;

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

namespace dune {

struct GeometryBuffer
{
	float position[3]; //12 bytes
	float coord[2];// 8 bytes
	float color[4]; // 4 bytes

	static void build(GLsizei size)
	{
		glEnableVertexAttribArray(AttribPosition);
		glVertexAttribPointer(AttribPosition, 3, GL_FLOAT, GL_FALSE, size, BUFFER_OFFSET(0));

		glEnableVertexAttribArray(AttribCoord);
		glVertexAttribPointer(AttribCoord, 2, GL_FLOAT, GL_FALSE, size, BUFFER_OFFSET(sizeof(float) * 3));

		glEnableVertexAttribArray(AttribColor);
		glVertexAttribPointer(AttribColor, 4, GL_FLOAT, GL_FALSE, size, BUFFER_OFFSET(sizeof(float) * 5));
	}
};

class IRenderable
{
public:
	virtual void render(GLenum mode = GL_TRIANGLES) = 0;
};

template <typename V>
class StaticGeometryArray
{
public:
	StaticGeometryArray(unsigned int vert_max)
	{
		// gen vao
		glGenVertexArrays(1, &_vao);

		// This VAO is for the Axis
		glBindVertexArray(_vao);

		// Generate two slots for the vertex and color buffers
		glGenBuffers(1, &_vao_buffer);

		// crear un buffer
		glBindBuffer(GL_ARRAY_BUFFER, _vao_buffer);
		//glBufferData(GL_ARRAY_BUFFER, sizeof(V) * vert_max, NULL, GL_STATIC_DRAW);
		glBufferData(GL_ARRAY_BUFFER, sizeof(V) * vert_max, NULL, GL_DYNAMIC_DRAW);

		// build opengl convention
		V::build((GLsizei)(sizeof(V)));

		// unbind buffers
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	~StaticGeometryArray()
	{
		glDeleteBuffers(1, &_vao_buffer);
		glDeleteVertexArrays(1, &_vao);
	}

	unsigned int getHandler() { return _vao; }
	inline void upload_data(const std::vector<V>& vertices, unsigned int vert_num)
	{
		if (vert_num > 0)
		{
			bind();
			glBindBuffer(GL_ARRAY_BUFFER, _vao_buffer);
			glBufferSubData(GL_ARRAY_BUFFER, 0, vert_num * sizeof(V), &(vertices[0]));
		}
	}
	inline void render(GLsizei vert_num, GLenum mode = GL_TRIANGLES)
	{
		bind();
		glDrawArrays(mode, 0, vert_num);
	}
protected:
	inline void bind()
	{
		glBindVertexArray(_vao);
	}
protected:
	// VAO del gui
	unsigned int _vao;
	// buffer del gui
	unsigned int _vao_buffer;
};

template <typename V>
class DynamicGeometryArray : public IRenderable
{
public:
	explicit DynamicGeometryArray()
		: _dirty(false)
		, _vert_num(0)
		, _userdata(0)
		, _vert_max(0)
		, _handler(_counter_geometries++)
	{

	}

	virtual ~DynamicGeometryArray()
	{

	}

    DynamicGeometryArray(const DynamicGeometryArray&) = delete;
    DynamicGeometryArray& operator=(const DynamicGeometryArray&) = delete;

	inline unsigned int getHandler()
	{
		return _handler;
	}

	void AddVert(const V& ver)
	{
		_vertexs.emplace_back(ver);
		_dirty = true;
		_vert_num = (int)_vertexs.size();
	}

	void flush()
	{
		if (_vert_num > _vert_max)
		{
			_vert_max = _vert_num;
			_vao = std::make_shared<StaticGeometryArray<V> >(_vert_max);
		}

		if (_dirty)
		{
			// reset
			_dirty = false;

			// cpu to gpu
			_vao->upload_data(_vertexs, _vert_num);
		}
	}

	virtual void render(GLenum mode = GL_TRIANGLES) override
	{
		// render
		if (_vert_num > 0)
		{
			_vao->render((GLsizei)_vert_num, mode);
		}
	}

	void clear_vertices()
	{
		_vertexs.clear();
	}

public:
	unsigned int _userdata;

protected:
	// handler id
	int _handler;
	// array vertex
	std::vector<V> _vertexs;
	// dirty
	bool _dirty;
	// num vertices actives
	unsigned int _vert_num;
	// reverva activa;
	unsigned int _vert_max;
	// geometry active
	std::shared_ptr<StaticGeometryArray<V> > _vao;
	// count compiled geometries
	static int _counter_geometries;
};

template <typename V>
int DynamicGeometryArray<V>::_counter_geometries = 1;

} // end namespace dune

#endif // GEOMETRYARRAY_H

