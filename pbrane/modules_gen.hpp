#ifndef MODULES_GEN_HPP
#define MODULES_GEN_HPP

#include <vector>
#include <functional>
#include "variable.hpp"
#include "modules.hpp"

namespace modules {
	template<typename Ret, typename... Args>
	struct IFWrapper {
		IFWrapper(std::function<Ret(Args...)> func);

		Variable operator()(std::vector<Variable> args);

		private:
			std::function<Ret(Args...)> _func;
	};
	template<typename... Args>
	struct IFWrapper<void, Args...> {
		IFWrapper(std::function<void(Args...)> func);

		Variable operator()(std::vector<Variable> args);

		private:
			std::function<void(Args...)> _func;
	};

	template<typename Context, typename Ret, typename... Args>
			IFWrapper<Ret, Args...> make_wrapper(Context *ctx, Ret (*func)(Args...));
	template<typename Context, typename Ret, typename... Args>
			IFWrapper<Ret, Args...> make_wrapper(Context *ctx, Ret (*func)(Context &, Args...));

	template<typename Context> bool init(Context *context);
}

#include "modules_gen.inc.hpp"

#endif // MODULES_GEN_HPP
