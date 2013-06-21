#include <cmath>
#include <cstdio>
#include <exception>
#include <iostream>

#include <engine.hpp>
#include <sprite.hpp>
#include <texture.hpp>

#include "compute.hpp"

using namespace std;

class Fractal : public Compute::Kernel, public Eventable, public Updateable, public Renderable {
	public:
		Compute::Context & _context;
		cl::CommandQueue & _queue;
		Texture _texture;
		cl::ImageGL * _image;
		vector<cl::Memory> _glObjects;
		int2 _size;
		float2 _center;
		float _scale;
		int2 _mouse;

		Fractal(Compute::Context & context, cl::CommandQueue & queue, cl::Program & clProgram, const string & name, Program & glProgram) :	Compute::Kernel(clProgram, name),
																													Renderable(glProgram),
																													_context(context),
																													_queue(queue),
																													_image(NULL),
																													_size(0, 0),
																													_center(0.0f, 0.0f),
																													_scale(1.0f),
																													_mouse(0, 0) {
			glBindVertexArray(_vao);
			_glException();
			glBindBuffer(GL_ARRAY_BUFFER, _vbo);
			_glException();
			GLfloat data[] = {
				// X	Y		Z			U		V
				-1.0f,	-1.0f,	0.0f,		0.0f,	0.0f,
				1.0f,	-1.0f,	0.0f,		1.0f,	0.0f,
				-1.0f,	1.0f,	0.0f,		0.0f,	1.0f,

				1.0f,	1.0f,	0.0f,		1.0f,	1.0f,
				-1.0f,	1.0f,	0.0f,		0.0f,	1.0f,
				1.0f,	-1.0f,	0.0f,		1.0f,	0.0f
			};
			glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
			_glException();
			glEnableVertexAttribArray(_program.attrib("vert"));
			_glException();
			glVertexAttribPointer(_program.attrib("vert"), 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), NULL);
			_glException();
			glEnableVertexAttribArray(_program.attrib("vertTexCoord"));
			_glException();
			glVertexAttribPointer(_program.attrib("vertTexCoord"), 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (const GLvoid *)(3 * sizeof(GLfloat)));
			_glException();
			_first = 0;
			_count = 6;
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			_glException();
			glBindVertexArray(0);
			_glException();	
		}
		~Fractal() {
			glDeleteTextures(1, &_texture._texture);
			_glException();
			if (_image != NULL) {delete _image;}
		}

		void event(SDL_Event & event) {
			switch (event.type) {
				case SDL_MOUSEMOTION: {
					if (event.motion.state & SDL_BUTTON_LMASK) {
						int2 delta(event.motion.x - _mouse.x, event.motion.y - _mouse.y);
						_center.x -= (float)delta.x / ((float)_size.x / _scale);
						_center.y += (float)delta.y / ((float)_size.y / _scale);
						Engine::RaiseRenderEvent();
					}
					_mouse.x = event.motion.x;
					_mouse.y = event.motion.y;
				} break;
				case SDL_MOUSEWHEEL: {
					float delta = pow(2.0f, (float)event.wheel.y);
					_scale /= delta;
					Engine::RaiseRenderEvent();
				} break;
				case SDL_WINDOWEVENT: {
					switch (event.window.event) {
						case SDL_WINDOWEVENT_RESIZED: {
							_size.x = event.window.data1;
							_size.y = event.window.data2;
							createTexture();
							render();
						} break;
					}
				} break;
			}
		}

		void render() {
			cl::Event event;

			glFinish();
			_queue.enqueueAcquireGLObjects(&_glObjects, NULL, &event);
			event.wait();

			execute();	

			glFinish();
			_queue.enqueueReleaseGLObjects(&_glObjects, NULL, &event);
			event.wait();

			glActiveTexture(GL_TEXTURE0);
			_glException();
			_texture.bind();
			Renderable::render();
			_texture.unbind();
		}

		void createTexture() {
			glActiveTexture(GL_TEXTURE0);
			_glException();
			_program.uniform("tex", 0);

			_glObjects.clear();
			if (_image != NULL) {delete _image;}

			_texture.setParameters();
			_texture.bind();
			glTexImage2D(_texture._target, 0, GL_RGBA, _size.x, _size.y, 0, GL_RGBA, GL_FLOAT, NULL);
			_glException();
			_image = new cl::ImageGL(*_context._context, CL_MEM_READ_WRITE, _texture._target, 0, _texture._texture, NULL);
			_glObjects.push_back(*_image);
			_texture.unbind();
		}

		void execute() {
			cl::Event event;

			float aspect = (float)_size.x / (float)_size.y;

			_kernel->setArg(0, *_image);
			_kernel->setArg(1, _center.x - (_scale / 2.0f));
			_kernel->setArg(2, (_center.y - (_scale / 2.0f)) / aspect);
			_kernel->setArg(3, _center.x + (_scale / 2.0f));
			_kernel->setArg(4, (_center.y + (_scale / 2.0f)) / aspect);
			//_kernel->setArg(5, max((int)(200.0f / _scale), 50));
			_kernel->setArg(5, 200);
			_queue.enqueueNDRangeKernel(*_kernel, cl::NullRange, cl::NDRange(_size.x, _size.y), cl::NullRange, NULL, &event);
			event.wait();

			//double nanosecs = event.getProfilingInfo<CL_PROFILING_COMMAND_END>() - event.getProfilingInfo<CL_PROFILING_COMMAND_START>();
			//printf("%fs\n", nanosecs / 1000000000.0f);
		}
};

int main(int argc, char *argv[]) {
	try {
		Engine & engine = Engine::GetInstance();
		engine._title = "GLFractal";
		engine._size = int2(1280, 720);
		engine.init();

		Program texture("texture.vert", "texture.frag");

		CGLContextObj kCGLContext = CGLGetCurrentContext(); 
		CGLShareGroupObj kCGLShareGroup = CGLGetShareGroup(kCGLContext);

		Compute::Context context;
		context._contextProperties.push_back(CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE); context._contextProperties.push_back((cl_context_properties)kCGLShareGroup);
		context.init();

		cl::CommandQueue & queue = context.getQueue(0, CL_QUEUE_PROFILING_ENABLE);

		Compute::Program & program = context.getProgram("program");
		program._files.push_back("kernel.cl");
		program.init();

		Fractal fractal(context, queue, *program._program, "fractal", texture);
		fractal._size = engine._size;
		fractal.createTexture();
		engine.addRenderHandler(&fractal);
		engine.addEventHandler(&fractal);

		engine.runEvent();
		return EXIT_SUCCESS;
	}
	catch (cl::Error & e) {
		cerr << e.what() << endl << _clGetErrorMessage(e.err()) << endl; 
	}
	catch (exception & e) {
		cerr << e.what() << endl;
	}
}

