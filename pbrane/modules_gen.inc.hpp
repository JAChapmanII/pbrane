#include "sekisa/err.hpp"
#include "sekisa/util.hpp"
#include <iostream>

namespace modules {
	namespace IFHelper {
		template<int Arity> struct ArityBinder {
			template<typename Func, typename Arg>
			decltype(auto) bind(Func func, Arg arg);
		};
		template<> struct ArityBinder<1> {
			template<typename Func, typename Arg>
			decltype(auto) operator()(Func func, Arg arg) {
				return std::bind(func, arg);
			}
		};
		template<> struct ArityBinder<2> {
			template<typename Func, typename Arg>
			decltype(auto) operator()(Func func, Arg arg) {
				return std::bind(func, arg, std::placeholders::_1);
			}
		};
		template<> struct ArityBinder<3> {
			template<typename Func, typename Arg>
			decltype(auto) operator()(Func func, Arg arg) {
				return std::bind(func, arg, std::placeholders::_1, std::placeholders::_2);
			}
		};

		template<typename Arg> Arg coerce(std::vector<Variable> &vars) {
			throw make_except("unexpected coerce type");
		}
		template<> std::string coerce(std::vector<Variable> &vars) {
			//if(vars.empty())
				//throw std::string{"coerce: wanted std::string but has nothing"};
			auto res = util::join(vars, " ");
			vars.clear();
			return res;
		}
		template<> Word coerce(std::vector<Variable> &vars) {
			if(vars.empty())
				throw std::string{"coerce: wanted word but has nothing"};
			auto res = vars.front().toString();
			vars.erase(vars.begin());
			return Word{res.begin(), res.end()};
		}
		template<> long coerce(std::vector<Variable> &vars) {
			if(vars.empty())
				throw std::string{"coerce: wanted int but has nothing"};
			long var = (long)vars.front().asNumber().toNumber();
			vars.erase(vars.begin());
			return var;
		}
		template<> double coerce(std::vector<Variable> &vars) {
			if(vars.empty())
				throw std::string{"coerce: wanted int but has nothing"};
			double var = (double)vars.front().asNumber().toNumber();
			vars.erase(vars.begin());
			return var;
		}
		template<> std::vector<Variable> coerce(std::vector<Variable> &vars) {
			auto copy = vars;
			vars.clear();
			return copy;
		}
		template<> Variable coerce(std::vector<Variable> &vars) {
			if(vars.empty())
				throw std::string{"coerce: wanted Variable but has nothing"};
			Variable var = vars.front();
			vars.erase(vars.begin());
			return var;
		}

		template<typename Ret> Variable makeVariable(Ret ret) {
			return Variable(ret);
		}
		template<> Variable makeVariable(Variable var) { return var; }
		template<> Variable makeVariable(sqlite_int64 var) {
			// TODO: this is terrible
			return Variable((long int)var);
		}


		template<typename F>
				decltype(auto) if_bind(F func, std::vector<Variable> &pargs) {
			if(!pargs.empty()) throw pargs.size();
			return func;
		}
		/*template<typename F, typename Arg>
				decltype(auto) if_bind(F func, std::vector<Variable> &pargs) {
			return std::bind(func, coerce<Arg>(pargs));
		}*/
		template<typename F, typename Arg, typename... Args>
				decltype(auto) if_bind(F func, std::vector<Variable> &pargs) {
			auto binder = ArityBinder<sizeof...(Args) + 1>{};
			auto fNext = binder(func, coerce<Arg>(pargs));
			return if_bind<decltype(fNext), Args...>(fNext, pargs);
		}
		template<typename Ret, typename... Args> decltype(auto)
				bind(std::function<Ret(Args...)> func, std::vector<Variable> pargs) {
			return if_bind<std::function<Ret(Args...)>, Args...>(func, pargs);
		}
	}

	template<typename Ret, typename... Args>
			IFWrapper<Ret, Args...>::IFWrapper(std::function<Ret(Args...)> func)
				: _func{func} { }
	template<typename Ret, typename... Args>
			Variable IFWrapper<Ret, Args...>::operator()(std::vector<Variable> args) {
		auto f = IFHelper::bind(_func, args);
		auto ret = f();
		return IFHelper::makeVariable(ret);
	}


	template<typename... Args>
			IFWrapper<void, Args...>::IFWrapper(std::function<void(Args...)> func)
				: _func{func} { }
	template<typename... Args>
			Variable IFWrapper<void, Args...>::operator()(std::vector<Variable> args) {
		auto f = IFHelper::bind(_func, args);
		f();
		return IFHelper::makeVariable("");
	}

	template<typename Context, typename Ret, typename... Args> IFWrapper<Ret, Args...>
			make_wrapper(Context *, Ret (*func)(Args...)) {
		return IFWrapper<Ret, Args...>{func};
	}
	template<typename Context, typename Ret, typename... Args> IFWrapper<Ret, Args...>
			make_wrapper(Context *ctx, Ret (*func)(Context *, Args...)) {
		auto binder = IFHelper::ArityBinder<sizeof...(Args) + 1>{};
		auto fNext = binder(func, ctx);
		return IFWrapper<Ret, Args...>{fNext};
	}

	static bool modules_inited{false};

	void defineModules();
	template<typename Context> void setupFunctions(Context *context);

	template<typename Context>
	bool init(Context *context) {
		if(modules_inited)
			return true;

		std::cerr << "moudles::init: " << std::endl;
		defineModules();

		setupFunctions(context);
		return true;
	}
}

// this is not jank, I swear!
#include "modules.cpp.gen"

