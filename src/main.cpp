#include <iostream>
#include <memory>
#include <SDL2/SDL.h>
#include <spdlog/spdlog.h>
#include <cppunix/parallel_scheduler.h>
#include <fast-event-system/sync.h>
#include <OIS/OIS.h>
#include <GL/glew.h>
#ifdef _WIN32
// windows
#include <GL/wglew.h>
#endif
// OIS
#include <SDL2/SDL_config.h>
#include <SDL2/SDL_syswm.h>
//
#ifdef _WIN32
//
#elif defined(__APPLE__)
//
#elif defined(__linux__)
#include <X11/Xlib.h>
#endif
#include "GeometryArray.h"

namespace spd = spdlog;

const int SCREEN_WIDTH  = 800;
const int SCREEN_HEIGHT = 600;

class graphics_system
{
public:
	explicit graphics_system()
	{
		spd::get("console")->warn("Iniciando SDL2...");
		// if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
		if (SDL_Init(SDL_INIT_VIDEO) != 0)
		{
			spd::get("console")->error("SDL2: {}", SDL_GetError());
			throw std::exception();
		}
	}

	~graphics_system()
	{
		spd::get("console")->warn("Destruction SDL2 ...");
		SDL_Quit();
	}
protected:
};


class window
{
public:
	explicit window()
	{
		spd::get("console")->warn("Create Window...");

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

		_window = SDL_CreateWindow("helloworld", 8, 22 + 8, SCREEN_WIDTH, SCREEN_HEIGHT,  
																			SDL_WINDOW_RESIZABLE |
																			SDL_WINDOW_MOUSE_FOCUS |
																			SDL_WINDOW_OPENGL |
																			SDL_WINDOW_SHOWN);

		// _window = SDL_CreateWindow("Hello world", 100, 100, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (_window == nullptr)
		{
			spd::get("console")->error("SDL2: {}", SDL_GetError());
			throw std::exception();
		}
	}

	~window()
	{
		spd::get("console")->warn("Destruction window ...");
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



class input_system : public OIS::KeyListener, public OIS::MouseListener, public OIS::JoyStickListener
{
public:
	explicit input_system(SDL_Window* window);
	virtual ~input_system();
	input_system(const input_system&) = delete;
	input_system& operator=(const input_system&) = delete;

	void update();

	// events to notify
	bool keyPressed(const OIS::KeyEvent &arg) override;
	bool keyReleased(const OIS::KeyEvent &arg) override;
	bool mouseMoved(const OIS::MouseEvent &arg) override;
	bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id) override;
	bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id) override;
	bool buttonPressed(const OIS::JoyStickEvent &arg, int button) override;
	bool buttonReleased(const OIS::JoyStickEvent &arg, int button) override;
	bool axisMoved(const OIS::JoyStickEvent &arg, int axis) override;
	bool povMoved(const OIS::JoyStickEvent &arg, int pov) override;
	bool vector3Moved(const OIS::JoyStickEvent &arg, int index) override;
	
	// ctrl / alt / mayus
	int get_modifier_state();

public:
	fes::sync<OIS::KeyEvent> key_pressed;
	fes::sync<OIS::KeyEvent> key_release;
	fes::sync<OIS::MouseEvent> mouse_moved;
	fes::sync<OIS::MouseEvent, OIS::MouseButtonID> mouse_pressed;
	fes::sync<OIS::MouseEvent, OIS::MouseButtonID> mouse_released;
	fes::sync<OIS::JoyStickEvent, int> button_pressed;
	fes::sync<OIS::JoyStickEvent, int> _buttonReleased;
	fes::sync<OIS::JoyStickEvent, int> axis_moved;
	fes::sync<OIS::JoyStickEvent, int> pov_moved;
	fes::sync<OIS::JoyStickEvent, int> vector3_moved;
protected:
	SDL_Window* _window;
	OIS::InputManager* _input_manager = 0;
	OIS::Keyboard* _keyboard  = 0;
	OIS::Mouse* _mouse   = 0;
	OIS::JoyStick* _joystick = 0;
	
	int _width;
	int _height;
	long long _hwnd;
};



class texture;

class renderer
{
public:
	explicit renderer()
	{
		spd::get("console")->warn("Create renderer...");

		SDL_SysWMinfo system_info;
		SDL_VERSION(&system_info.version);
		SDL_GetWindowWMInfo(_w.get(), &system_info);
		Display* window_handler = system_info.info.x11.display;
		_context = SDL_GL_CreateContext(_w.get());

		// http://glew.sourceforge.net/basic.html
		glewExperimental = GL_TRUE;
		GLenum status = glewInit();
		if (status != GLEW_OK)
		{
			spd::get("console")->error("SDL2: {}", glewGetErrorString(status));
			throw std::exception();
		}

		LOGI("Status: Using GLEW %s", glewGetString(GLEW_VERSION));
		LOGI("OpenGL version %s supported", glGetString(GL_VERSION));

		// _renderer = SDL_CreateRenderer(_w.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
		// if (_renderer == nullptr) {
		// 	spd::get("console")->error("SDL2: {}", SDL_GetError());
		// 	throw std::exception();
		// }

		_input = std::make_unique<input_system>( _w.get() );
	}

	~renderer()
	{
		spd::get("console")->warn("Destruction renderer ...");
		// SDL_DestroyRenderer(_renderer);
	}

	// SDL_Renderer* get() const
	// {
	// 	return _renderer;
	// }

	// void clear()
	// {
	// 	// SDL_RenderClear(_renderer);
	// }

	void render(texture& tex, int x, int y);
	void render(texture& tex, int x, int y, int w, int h);

	void update()
	{
		_input->update();
		// SDL_RenderPresent(_renderer);
	}

	input_system& input()
	{
		return *_input;
	}

protected:
	// SDL_Renderer* _renderer;
	SDL_GLContext _context;
	window _w;
	std::unique_ptr<input_system> _input;
};

class texture
{
public:
	explicit texture(renderer& ren, const std::string& file)
	{
		spd::get("console")->warn("Load texture...");
		//Load the image
		// SDL_Surface *loadedImage = SDL_LoadBMP(file.c_str());
		// //If the loading went ok, convert to texture and return the texture
		// if (loadedImage != nullptr){
		// 	_image = SDL_CreateTextureFromSurface(ren.get(), loadedImage);
		// 	SDL_FreeSurface(loadedImage);
		// 	//Make sure converting went ok too
		// 	if (_image == nullptr){
		// 		spd::get("console")->error("SDL2: {}", SDL_GetError());
		// 		throw std::exception();
		// 	}
		// }
		// else {
		// 	spd::get("console")->error("SDL2: {}", SDL_GetError());
		// 	throw std::exception();
		// }
	}

	~texture()
	{
		spd::get("console")->warn("Destruction texture ...");
		// SDL_DestroyTexture(_image);
	}

	// SDL_Texture* get() const
	// {
	// 	return _image;
	// }
protected:
	// SDL_Texture* _image;
};


void renderer::render(texture& tex, int x, int y)
{
	// SDL_Rect dst;
	// dst.x = x;
	// dst.y = y;
	// SDL_QueryTexture(tex.get(), NULL, NULL, &dst.w, &dst.h);
	// SDL_RenderCopy(_renderer, tex.get(), NULL, &dst);
}

void renderer::render(texture& tex, int x, int y, int w, int h)
{
	// SDL_Rect dst;
	// dst.x = x;
	// dst.y = y;
	// dst.w = w;
	// dst.h = h;
	// SDL_RenderCopy(_renderer, tex.get(), NULL, &dst);
}


const char* g_DeviceType[] = {"OISUnknown", "OISKeyboard", "OISMouse", "OISJoyStick", "OISTablet", "OISOther"};

input_system::input_system(SDL_Window* window)
			: _window(window)
			, _input_manager(nullptr)
			, _keyboard(nullptr)
			, _mouse(nullptr)
			, _joystick(nullptr)
			, _width(SCREEN_WIDTH)
			, _height(SCREEN_HEIGHT)
{
	spd::get("console")->warn("Starting input manager ...");

	SDL_SysWMinfo system_info;
	SDL_VERSION(&system_info.version);
	SDL_GetWindowWMInfo(_window, &system_info);

	std::ostringstream wnd;
#ifdef _WIN32
 	wnd << system_info.info.win.window;
#else
	wnd << system_info.info.x11.window;
 #endif

	OIS::ParamList pl;
	pl.insert(std::make_pair( std::string("WINDOW"), wnd.str() ));
#ifdef _WIN32
	pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND" )));
	pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")));
	pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_FOREGROUND")));
	pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_NONEXCLUSIVE")))
#else
	pl.insert(std::make_pair(std::string("x11_mouse_grab"), std::string("false")));
	pl.insert(std::make_pair(std::string("x11_mouse_hide"), std::string("false")));
	pl.insert(std::make_pair(std::string("x11_keyboard_grab"), std::string("true")));
	pl.insert(std::make_pair(std::string("XAutoRepeatOn"), std::string("true")));
#endif

	_input_manager = OIS::InputManager::createInputSystem(pl);
	_input_manager->enableAddOnFactory(OIS::InputManager::AddOn_All);

	unsigned int v = _input_manager->getVersionNumber();
	std::stringstream ss;
	ss << "OIS Version: " << (v>>16) << "." << ((v>>8) & 0x000000FF) << "." << (v & 0x000000FF)
		<< "\nRelease Name: " << _input_manager->getVersionName()
		<< "\nManager: " << _input_manager->inputSystemName()
		<< "\nTotal Keyboards: " << _input_manager->getNumberOfDevices(OIS::OISKeyboard)
		<< "\nTotal Mice: " << _input_manager->getNumberOfDevices(OIS::OISMouse)
		<< "\nTotal JoySticks: " << _input_manager->getNumberOfDevices(OIS::OISJoyStick);
	LOGI(ss.str().c_str());

	OIS::DeviceList list = _input_manager->listFreeDevices();
	for( OIS::DeviceList::iterator i = list.begin(); i != list.end(); ++i )
	{
		std::cout << "\n\tDevice: " << g_DeviceType[i->first] << " Vendor: " << i->second;
	}
	std::cout << std::endl;

	_keyboard = (OIS::Keyboard*)_input_manager->createInputObject(OIS::OISKeyboard, true);
	_keyboard->setTextTranslation(OIS::Keyboard::Unicode);
	_keyboard->setEventCallback(this);

#ifdef _Win32
	// linux SDL2 ya coge el raton de X11
	// http://www.wreckedgames.com/forum/index.php?topic=1233.0
	_mouse = (OIS::Mouse*)_input_manager->createInputObject(OIS::OISMouse, true);
	_mouse->setEventCallback(this);
	const OIS::MouseState &ms = _mouse->getMouseState();
	ms.width = _width;
	ms.height = _height;
#endif

	try
	{
		int numSticks = std::min<int>(_input_manager->getNumberOfDevices(OIS::OISJoyStick), 1);
		if (numSticks > 0)
		{
			_joystick = (OIS::JoyStick*)_input_manager->createInputObject(OIS::OISJoyStick, true);
			_joystick->setEventCallback(this);
			std::stringstream ss2;
			ss2 << "\n\nCreating Joystick "
				<< "\n\tAxes: " << _joystick->getNumberOfComponents(OIS::OIS_Axis)
				<< "\n\tSliders: " << _joystick->getNumberOfComponents(OIS::OIS_Slider)
				<< "\n\tPOV/HATs: " << _joystick->getNumberOfComponents(OIS::OIS_POV)
				<< "\n\tButtons: " << _joystick->getNumberOfComponents(OIS::OIS_Button)
				<< "\n\tVector3: " << _joystick->getNumberOfComponents(OIS::OIS_Vector3)
				<< std::endl;
			LOGI(ss2.str().c_str());
		}
	}
	catch(OIS::Exception &ex)
	{
		spd::get("console")->error("Exception raised on joystick creation: {}", ex.eText);
	}
}

input_system::~input_system()
{
	spd::get("console")->warn("Destruction input manager ...");
	OIS::InputManager::destroyInputSystem(_input_manager);
}

void input_system::update()
{
	_keyboard->capture();
#ifdef _Win32
	_mouse->capture();
#endif
	if( _joystick )
	{
		_joystick->capture();
	}
}

bool input_system::keyPressed(const OIS::KeyEvent &arg)
{
	key_pressed(arg);
	return true;
}

bool input_system::keyReleased(const OIS::KeyEvent &arg)
{
	key_release(arg);
	return true;
}

bool input_system::mouseMoved(const OIS::MouseEvent &arg)
{
	mouse_moved(arg);
	return true;
}

bool input_system::mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
{
	mouse_pressed(arg, id);
	return true;
}

bool input_system::mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
{
	mouse_released(arg, id);
	return true;
}

bool input_system::buttonReleased(const OIS::JoyStickEvent &arg, int button)
{
	_buttonReleased(arg, button);
	return true;
}

bool input_system::buttonPressed(const OIS::JoyStickEvent &arg, int button)
{
	button_pressed(arg, button);
	return true;
}

bool input_system::axisMoved(const OIS::JoyStickEvent &arg, int axis)
{
	axis_moved(arg, axis);
	return true;
}

bool input_system::povMoved(const OIS::JoyStickEvent &arg, int pov)
{
	pov_moved(arg, pov);
	return true;
}

bool input_system::vector3Moved(const OIS::JoyStickEvent &arg, int index)
{
	vector3_moved(arg, index);
	return true;
}

int input_system::get_modifier_state()
{
	int modifier_state = 0;

	if (_keyboard->isModifierDown(OIS::Keyboard::Ctrl))
		std::cout << "down ctrl" << std::endl;
	else
		std::cout << "up ctrl" << std::endl;

	if (_keyboard->isModifierDown(OIS::Keyboard::Shift))
		std::cout << "down shift" << std::endl;
	else
		std::cout << "up shift" << std::endl;

	if (_keyboard->isModifierDown(OIS::Keyboard::Alt))
		std::cout << "down alt" << std::endl;
	else
		std::cout << "up alt" << std::endl;

#ifdef _WIN32

	if (GetKeyState(VK_CAPITAL) > 0)
		modifier_state |= Rocket::Core::Input::KM_CAPSLOCK;
	if (GetKeyState(VK_NUMLOCK) > 0)
		modifier_state |= Rocket::Core::Input::KM_NUMLOCK;
	if (GetKeyState(VK_SCROLL) > 0)
		modifier_state |= Rocket::Core::Input::KM_SCROLLLOCK;

#elif defined(__APPLE__)

	UInt32 key_modifiers = GetCurrentEventKeyModifiers();
	if (key_modifiers & (1 << alphaLockBit))
		modifier_state |= Rocket::Core::Input::KM_CAPSLOCK;

#elif defined(__linux__)

	SDL_SysWMinfo system_info;
	SDL_VERSION(&system_info.version);
	SDL_GetWindowWMInfo(_window, &system_info);

	Display* display = system_info.info.x11.display;
	XKeyboardState keyboard_state;
	XGetKeyboardControl(display, &keyboard_state);

	if (keyboard_state.led_mask & (1 << 0))
		LOGI("modifier_state |= Rocket::Core::Input::KM_CAPSLOCK;");
	if (keyboard_state.led_mask & (1 << 1))
		LOGI("modifier_state |= Rocket::Core::Input::KM_NUMLOCK;");
	if (keyboard_state.led_mask & (1 << 2))
		LOGI("modifier_state |= Rocket::Core::Input::KM_SCROLLLOCK;");

#endif

	return modifier_state;
}


int main(int argc, char const* argv[])
{
	auto console = spd::stdout_color_mt("console");
	spd::get("console")->warn("Starting ...");

	cu::parallel_scheduler sch;
	renderer ren;
	texture tex(ren, "pic.bmp");

	bool exit = false;
	ren.input().key_pressed.connect([&](auto& event){
		//
		spd::get("console")->error("key press: {}", event.key);
        //
	});
	ren.input().key_release.connect([&](auto& event){
		//
		spd::get("console")->error("key release: {}", event.key);
		//
		if (event.key == OIS::KC_ESCAPE)
		{
			exit = true;
		}
	});
		
	int x = 20;
	int x_inc = 2;
	sch.spawn([&](auto& yield) {
		while(!exit)
		{
			if(x > (SCREEN_WIDTH - 100) || x < 0)
			{
				x_inc = -x_inc;
				console->warn("Collision!.");
			}
			x += x_inc;
			yield( {} );
		}
	});
	sch.spawn([&](auto& yield) {

		glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

		while(!exit)
		{
			SDL_Event event;
			while(SDL_PollEvent(&event)) { ; }

			// glClearColor(230 / 255.0f, 249 / 255.0f, 255 / 255.0f, 1.0);
			glClearColor(230 / 255.0f, 19 / 255.0f, 15 / 255.0f, 1.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// ren.clear();
			// ren.render(tex, x, 20, 100, 100);
			// ren.render(tex, 100, y, 100, 100);
			ren.update();

			// SDL_GL_SwapWindow(_w.get());

			yield( {} );
		}
	});
	sch.run_until_complete();
	spd::get("console")->warn("Exiting ...");
	return 0;
}

