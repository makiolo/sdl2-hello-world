#include "Engine.h"
//

Shader::Shader()
	: _vertex_program_file("")
	, _geometry_program_file("")
	, _fragment_program_file("")
{
	;
}

Shader::~Shader()
{
	;
}

bool Shader::compile()
{
	char* bufferV = Sys_TextFileRead( _vertex_program_file.c_str() );
	char* bufferF = Sys_TextFileRead( _fragment_program_file.c_str() );

	LOGI("Compiling %s / %s", _vertex_program_file.c_str(),  _fragment_program_file.c_str()  );

    _program = createProgram(bufferV, bufferF);

	// liberar memoria
	free(bufferV);
	free(bufferF);

    if(_program <= 0)
	{
        LOGE("Could not create program.");
        return false;
    }
	else
	{
		return true;
	}
}

bool Shader::linking()
{
    if(_program)
    {
        glLinkProgram(_program);
		GLint success;
		char error_log[BUFSIZ];
		glGetProgramiv(_program, GL_LINK_STATUS, &success);
		if (success == 0) {
			glGetProgramInfoLog(_program, sizeof(error_log), NULL, error_log);
			LOGE("Error linking shader program: '%s'", error_log);
		}

		glValidateProgram(_program);
		glGetProgramiv(_program, GL_VALIDATE_STATUS, &success);
        printProgramInfoLog(_program);

        return true;
    }
	else
	{
		return false;
	}
}

void Shader::Destroy()
{
    glDetachShader(_program, _vertex_shader_id);
    glDeleteShader(_vertex_shader_id);

    glDetachShader(_program, _fragment_shader_id);
    glDeleteShader(_fragment_shader_id);
    
	glDeleteProgram(_program);
}

void Shader::bind_attrib(GLint numSlot, const char* attribName)
{
    // bindea antes del linkado en opengl es 2.0
    glBindAttribLocation(_program, numSlot, attribName);
    CHECK_GL_ERRORS;
}

int Shader::getParameter(const std::string& parmName)
{
	if(_parameters.count(parmName) <= 0)
	{
		// no hsy parametro, se busca en el shader
		int idLocation = glGetUniformLocation(_program, parmName.c_str());
		if(idLocation > -1)
		{
			_parameters[parmName] = idLocation;
			return idLocation;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		// se tira de cache
		return _parameters[parmName];
	}
}

unsigned int Shader::loadShader(unsigned int shaderType, const char* pSource)
{
	// crear vertex o frament sahder
    GLuint shader = glCreateShader(shaderType);
    if (shader)
	{
        glShaderSource(shader, 1, &pSource, NULL);
        glCompileShader(shader);

		printShaderInfoLog(shader);
    }
    return shader;
}

unsigned int Shader::createProgram(const char* pVertexSource, const char* pFragmentSource)
{
	_vertex_shader_id = loadShader(GL_VERTEX_SHADER, pVertexSource);
	if (!_vertex_shader_id)
	{
		LOGE("Fallo la creacion del vertex shader\n");
		return 0;
	}

	_fragment_shader_id = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
	if (!_fragment_shader_id)
	{
		LOGE("Fallo la creacion del pixel shader\n");
		return 0;
	}
	
	unsigned int program = glCreateProgram();
	if (program)
	{
		glAttachShader(program, _vertex_shader_id);
		CHECK_GL_ERRORS;
		glAttachShader(program, _fragment_shader_id);
		CHECK_GL_ERRORS;
		glBindFragDataLocation(program, 0, "FragColor");
		CHECK_GL_ERRORS;
	}
	return program;
}

void Shader::printShaderInfoLog(unsigned int obj)
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;
 
    glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);
 
    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
        printf("%s",infoLog);
        free(infoLog);
    }
}
 
void Shader::printProgramInfoLog(unsigned int obj)
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;
 
    glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength);
 
	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
		if(charsWritten > 0)
		{
			infoLog[charsWritten] = '\0';
			printf("%s\n", infoLog);
		}
        free(infoLog);
    }
}

