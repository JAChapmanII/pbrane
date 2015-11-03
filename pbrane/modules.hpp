#ifndef MODULES_HPP
#define MODULES_HPP

#include <map>
#include <vector>
#include <string>
#include "pvm.hpp"

namespace modules {
	struct Word : std::string { using std::string::string; };

	struct Module {
		std::string name;
		std::string desc;
		bool loaded;
	};

	extern std::map<std::string, InjectedFunction> hfmap;
	extern std::vector<Module> modules;

}

#endif // MODULES_HPP
