#include <iostream>
#include <SDL2/SDL.h>
#include <spdlog/spdlog.h>

namespace spd = spdlog;

const int SCREEN_WIDTH  = 800;
const int SCREEN_HEIGHT = 600;

class graphics_system
{
public:
	explicit graphics_system()
	{
		spd::get("console")->info("Iniciando SDL2...");
		if (SDL_Init(SDL_INIT_EVERYTHING) != 0){
			spd::get("console")->error("SDL2: {}", SDL_GetError());
			throw std::exception();
		}
	}

	~graphics_system()
	{
		SDL_Quit();
	}
protected:
};


class window
{
public:
	explicit window()
	{
		spd::get("console")->info("Create Window...");
		_window = SDL_CreateWindow("Hello world", 100, 100, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (_window == nullptr){
			spd::get("console")->error("SDL2: {}", SDL_GetError());
			throw std::exception();
		}
	}

	~window()
	{
		SDL_DestroyWindow(_window);
	}

	SDL_Window* get() const
	{
		return _window;
	}
protected:
	SDL_Window* _window;
	graphics_system _g;
};

class texture;

class renderer
{
public:
	explicit renderer()
	{
		spd::get("console")->info("Create renderer...");
		_renderer = SDL_CreateRenderer(_w.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
		if (_renderer == nullptr) {
			spd::get("console")->error("SDL2: {}", SDL_GetError());
			throw std::exception();
		}
	}

	~renderer()
	{
		SDL_DestroyRenderer(_renderer);
	}

	SDL_Renderer* get() const
	{
		return _renderer;
	}

	void clear()
	{
		SDL_RenderClear(_renderer);
	}

	void render(texture& tex, int x, int y);
	void render(texture& tex, int x, int y, int w, int h);

	void render()
	{
		SDL_RenderPresent(_renderer);
	}
protected:
	SDL_Renderer* _renderer;
	window _w;
};

class texture
{
public:
	explicit texture(renderer& ren, const std::string& file)
	{
		spd::get("console")->info("Load texture...");
		///////////////////////////////////////////////
		// Need SDL_image
		// _image = IMG_LoadTexture(ren.get(), file.c_str());
		// if(_image != nullptr)
		// {
		// 	spd::get("console")->error("SDL2: {}", SDL_GetError());
		// 	throw std::exception();
		// }
		////////////////////////////////////////////////
		//Load the image
		SDL_Surface *loadedImage = SDL_LoadBMP(file.c_str());
		//If the loading went ok, convert to texture and return the texture
		if (loadedImage != nullptr){
			_image = SDL_CreateTextureFromSurface(ren.get(), loadedImage);
			SDL_FreeSurface(loadedImage);
			//Make sure converting went ok too
			if (_image == nullptr){
				spd::get("console")->error("SDL2: {}", SDL_GetError());
				throw std::exception();
			}
		}
		else {
			spd::get("console")->error("SDL2: {}", SDL_GetError());
			throw std::exception();
		}
	}

	~texture()
	{
		SDL_DestroyTexture(_image);
	}

	SDL_Texture* get() const
	{
		return _image;
	}
protected:
	SDL_Texture* _image;
};


void renderer::render(texture& tex, int x, int y)
{
	SDL_Rect dst;
	dst.x = x;
	dst.y = y;
	SDL_QueryTexture(tex.get(), NULL, NULL, &dst.w, &dst.h);
	SDL_RenderCopy(_renderer, tex.get(), NULL, &dst);
}

void renderer::render(texture& tex, int x, int y, int w, int h)
{
	SDL_Rect dst;
	dst.x = x;
	dst.y = y;
	dst.w = w;
	dst.h = h;
	SDL_RenderCopy(_renderer, tex.get(), NULL, &dst);
}

int main(int argc, char const* argv[])
{
	auto console = spd::stdout_color_mt("console");

	renderer ren;
	texture tex(ren, "pic.bmp");
	texture tex2(ren, "pic.bmp");
	
	for (int i = 0; i < 8; ++i)
	{
		ren.clear();
		ren.render(tex, 20, 20, 100, 100);
		ren.render(tex, 100, 200, 100, 100);
		ren.render();
		SDL_Delay(1000);
	}
	return 0;
}

