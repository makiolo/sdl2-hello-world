## Como empezar un repositorio vacio
- git clone git@github.com:makiolo/sdl2-hello-world.git
- npm init
- create CMakeLists.txt minimal:
```cmake
project(sdl2-hello-world CXX)
cmake_minimum_required(VERSION 3.0)
include(cmaki)
cmaki_find_package(sdl2)
cmaki_executable(test1 src/main.cpp)
```
- mkdir src
- create minimal src/main.cpp
```cpp
#include <iostream>

int main(int argc, char const* argv[])
{
	std::cout << "test" << std::endl;
	return 0;
}
```
- npm install -D https://github.com/makiolo/npm-mas-mas
- In package.json replace scripts with this:
```json
"scripts": {
	"ci": "curl -s https://raw.githubusercontent.com/makiolo/cmaki_scripts/master/ci.sh | bash",
	"docker": "curl -s https://raw.githubusercontent.com/makiolo/cmaki_scripts/master/docker.sh | bash",
	"clean": "cmaki clean",
	"setup": "cmaki setup",
	"compile": "cmaki compile",
	"install": "cmaki setup && cmaki compile",
	"test": "cmaki test",
	"upload": "cmaki upload"
},
```

## Iteración durante el desarrollo (compilar + ejecutar)
- npm install && (cd ./bin/Debug/ && LD_LIBRARY_PATH=$(pwd) ./test1)

## Añadir log al proyecto
- Añadir en el CMakeLists.txt: cmaki_find_package(spdlog)

## Añadir controles
- Añadir en el CMakeLists.txt: cmaki_find_package(ois)

## SDL2
- http://www.willusher.io/pages/sdl2/
