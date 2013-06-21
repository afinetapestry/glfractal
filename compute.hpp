#define __CL_ENABLE_EXCEPTIONS

#include <exception>
#include <iostream>
#include <string>

#include "clerr.h"
#include "cl.hpp"

namespace Compute {
	using namespace std;

	string LoadFile(const string & filename) {
		ifstream file;
		file.exceptions(std::ifstream::failbit);
		file.open(filename.c_str());
		string text(istreambuf_iterator<char>(file), (istreambuf_iterator<char>()));
		return text;
	}

	class Kernel {
		public:
			cl::Kernel * _kernel;

			Kernel(cl::Program & program, const string & name) : _kernel(NULL) {
				_kernel = new cl::Kernel(program, name.c_str(), NULL);
			}
			~Kernel() {
				if (_kernel != NULL) {delete _kernel;}
			}

			/*bool operator()(cl::CommandQueue & queue, cl::NDRange & range, cl::detail::NullType arg) {

			}*/
	};

	class Program {
		public:
			cl::Context * _context;
			cl::Program * _program;
			vector<string> _files;
			unsigned int _device;

			Program(cl::Context * context) :	_context(context),
												_device(0)	{
			}

			void init() {
				vector<cl::Device> devices = _context->getInfo<CL_CONTEXT_DEVICES>();

				cl::Program::Sources sources;
				for (vector<string>::iterator i = _files.begin(); i != _files.end(); ++i) {
					string source = LoadFile(*i);
					sources.push_back(make_pair(source.c_str(), source.size()));
				}
				_program = new cl::Program(*_context, sources);
				try {
					string options = "";
#ifdef _CLDEBUG
					options.append("-Werror ");
					//options.append("-g ");
#ifdef __NVIDIA_CL__
					options.append("-cl-nv-verbose ");
#endif
#endif
					_program->build(devices, options.c_str());
				} catch (cl::Error & e) { 
					cerr << "Caught Exception: " << e.what() << endl << _clGetErrorMessage(e.err()) << endl;
					string str = _program->getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[_device]);
					cerr << str.c_str() << endl;
					throw exception();
				}
			}

			template <class T>
				T getKernel(const string & name) {
					return T(*_program, name);
				}
	};

	class Context {
		public:
			vector<cl_context_properties> _contextProperties;
			cl_device_type _type;
			vector<cl::Platform> _platforms;
			cl::Context * _context;
			vector<cl::Device> _devices;
			map<const string, Program *> _programs;
			map<const unsigned int, cl::CommandQueue *> _queues;
			unsigned int _platform;

			Context() : _context(NULL),
						_type(CL_DEVICE_TYPE_GPU),
						_platform(0) {
			}
			~Context() {
				for (map<const unsigned int, cl::CommandQueue *>::iterator i = _queues.begin(); i != _queues.end(); ++i) {
					delete i->second;
				}

				for (map<const string, Program *>::iterator i = _programs.begin(); i != _programs.end(); ++i) {
					delete i->second;
				}
				if (_context != NULL) {delete _context;}
			}

			void init() {
				cl::Platform::get(&_platforms);

				_contextProperties.push_back(CL_CONTEXT_PLATFORM); _contextProperties.push_back((cl_context_properties)(_platforms[0]()));
				_contextProperties.push_back(0);
				_context = new cl::Context(_type, &_contextProperties[0]);
				_devices = _context->getInfo<CL_CONTEXT_DEVICES>();
			}

			Program & getProgram(const string & name) {
				map<const string, Program *>::iterator i;
				Program * program;
				if ((i = _programs.find(name)) == _programs.end()) {
					program = _programs[name] = new Program(_context);
				} else {
					program = i->second;
				}
				return *program;
			}

			cl::CommandQueue & getQueue(const unsigned int device = 0, const cl_command_queue_properties props = 0) {
				map<const unsigned int, cl::CommandQueue *>::iterator i;
				cl::CommandQueue * queue;
				if ((i = _queues.find(device)) == _queues.end()) {
					queue = _queues[device] = new cl::CommandQueue(*_context, _devices[device], props);
				} else {
					queue = i->second;
				}
				return *queue;
			}
	};
};
