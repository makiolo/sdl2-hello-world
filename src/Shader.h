/**
@author Ricardo Marmolejo García
@date 2013
*/
#ifndef _SHADER_H_
#define _SHADER_H_

#include <myMath/h/myMath.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include "gl_errors.h"

class Shader
{
public:
	Shader();
	~Shader();

	bool compile();
    bool linking();
	void Destroy();

	inline void activate()
	{
		glUseProgram(_program);
	}

	inline void deactivate()
	{
		glUseProgram(0);
	}

	inline void uniform_matrix4(int location, const Matrix4& matrix)
	{
		glUniformMatrix4fv(location, 1, GL_FALSE, matrix._m); CHECK_GL_ERRORS;
	}

	inline void uniform_vec3(int location, const Vector3& vec3)
	{
		glUniform3f(location, vec3.x, vec3.y, vec3.z); CHECK_GL_ERRORS;
	}

    void bind_attrib(GLint numSlot, const char* attribName);

	inline void SetPass(unsigned int iNumPass)
	{
		glUniformSubroutinesuiv( GL_FRAGMENT_SHADER, 1, &iNumPass);
	}

	unsigned int getIDProgram() const {return _program;}
	int getParameter(const std::string& parmName);

	void set_vertex_program_file(const std::string& vertexProgramFile) {_vertex_program_file = vertexProgramFile;}
	void setGeometryProgramFile(const std::string& geometryProgramFile) {_geometry_program_file = geometryProgramFile;}
	void set_fragment_program_file(const std::string& fragmentProgramFile) {_fragment_program_file = fragmentProgramFile;}

protected:

	unsigned int loadShader(unsigned int shaderType, const char* pSource);
	unsigned int createProgram(const char* pVertexSource, const char* pFragmentSource);
	void printShaderInfoLog(unsigned int obj);
	void printProgramInfoLog(unsigned int obj);

protected:
	unsigned int _program;
	unsigned int _vertex_shader_id;
	unsigned int _fragment_shader_id;

	std::string _vertex_program_file;
	std::string _geometry_program_file;
	std::string _fragment_program_file;

	std::map<std::string, int> _parameters;
};

template <typename T>
class Shader2 : public Shader
{
public:
	T* operator->()
	{
		return &_data;
	}

	void binding()
	{
		_data.build(this);
	}

protected:
	T _data;
};

#endif
