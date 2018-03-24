/**
@file GeometryUI.h

OpenGL VAO abstraction for GUI

@author Ricardo Marmolejo García
@date 04/01/15
*/

#ifndef GEOMETRYELEMENT_H
#define GEOMETRYELEMENT_H

#include "GeometryArray.h"

namespace dune {

struct ElementsBuffer
{
	float position[3]; //12 bytes
	float coord[2];// 8 bytes
	unsigned char color[4]; // 4 bytes

	static void build(GLsizei size)
	{
		glEnableVertexAttribArray(AttribPosition);
		glVertexAttribPointer(AttribPosition, 3, GL_FLOAT, GL_FALSE, size, BUFFER_OFFSET(0));

		glEnableVertexAttribArray(AttribCoord);
		glVertexAttribPointer(AttribCoord, 2, GL_FLOAT, GL_FALSE, size, BUFFER_OFFSET(sizeof(float) * 3));

		glEnableVertexAttribArray(AttribColor);
		glVertexAttribPointer(AttribColor, 4, GL_UNSIGNED_BYTE, GL_FALSE, size, BUFFER_OFFSET(sizeof(float) * 5));
	}
};

template <typename V>
class StaticGeometryElement
{
public:
	StaticGeometryElement(unsigned int vert_max, unsigned int indexes_max)
	{
		// gen vao
		glGenVertexArrays(1, &_vao);

		// This VAO is for the Axis
		glBindVertexArray(_vao);

		// Generate two slots for the vertex and color buffers
		glGenBuffers(2, _vao_buffer);

		// crear un buffer
		glBindBuffer(GL_ARRAY_BUFFER, _vao_buffer[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(V) * vert_max, NULL, GL_DYNAMIC_DRAW);

		// crear un buffer de indices
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vao_buffer[1]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indexes_max, NULL, GL_DYNAMIC_DRAW);
		
		// build opengl convention
		V::build((GLsizei)(sizeof(V)));

		// unbind buffers
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	~StaticGeometryElement()
	{
		glDeleteBuffers(2, _vao_buffer);
		glDeleteVertexArrays(1, &_vao);
	}

	inline unsigned int getHandler() { return _vao; }
	inline void upload_data(const std::vector<V>& vertices, unsigned int vert_num)
	{
		if (vert_num)
		{
			bind();
			glBindBuffer(GL_ARRAY_BUFFER, _vao_buffer[0]);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(V) * vert_num, &(vertices[0]));
		}
	}
	inline void upload_indexes(const std::vector<GLuint>& indexes, unsigned int indexes_num)
	{
		if (indexes_num > 0)
		{
			bind();
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vao_buffer[1]);
			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(GLuint) * indexes_num, &(indexes[0]));
		}
	}
	inline void render(GLsizei indexes_num, GLenum mode = GL_TRIANGLES)
	{
		bind();
		glDrawElements(mode, indexes_num, GL_UNSIGNED_INT, 0);
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
	unsigned int _vao_buffer[2];
};

template <typename V>
class DynamicGeometryElement : public IRenderable
{
public:
	explicit DynamicGeometryElement()
		: _dirty(false)
		, _dirty_indexes(false)
		, _vert_num(0)
		, _indexes_num(0)
		, _userdata(0)
		, _vert_max(0)
		, _handler(_counter_geometries++)
	{

	}

	virtual ~DynamicGeometryElement()
	{

	}

	DynamicGeometryElement(const DynamicGeometryElement&) = delete;
	DynamicGeometryElement& operator=(const DynamicGeometryElement&) = delete;

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

	void AddIndex(GLuint index)
	{
		_indexes.push_back(index);
		_dirty_indexes = true;
		_indexes_num = (int)_indexes.size();
	}

	void flush()
	{
		if (_vert_num > _vert_max)
		{
			_vert_max = _vert_num;
			_vao = std::make_shared<StaticGeometryElement<V> >(_vert_max, _indexes_num);
		}

		if (_dirty)
		{
			_dirty = false;
			_vao->upload_data(_vertexs, _vert_num);
		}

		if (_dirty_indexes)
		{
			_dirty_indexes = false;
			_vao->upload_indexes(_indexes, _indexes_num);
		}
	}

	virtual void render(GLenum mode = GL_TRIANGLES) override
	{
		// render
		if (_indexes_num> 0)
		{
			_vao->render((GLsizei)_indexes_num, mode);
		}
	}

	void clear_vertices()
	{
		_vertexs.clear();
		_indexes.clear();
	}

public:
	unsigned int _userdata;

protected:
	// handler id
	int _handler;
	// array vertex
	std::vector<V> _vertexs;
	std::vector<unsigned int> _indexes;
	// dirty
	bool _dirty;
	bool _dirty_indexes;
	// num vertices actives
	unsigned int _vert_num;
	unsigned int _indexes_num;
	// reverva activa;
	unsigned int _vert_max;
	// geometry active
	std::shared_ptr<StaticGeometryElement<V> > _vao;
	// count compiled geometries
	static int _counter_geometries;
};

template <typename V>
int DynamicGeometryElement<V>::_counter_geometries = 1;

} // end namespace dune

#endif // GEOMETRYELEMENT_H
