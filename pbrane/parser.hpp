#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <memory>

struct Expression;

struct OpInfo {
	int precedence;
	bool leftAssociative{true};

	OpInfo() : precedence(-1) { }
	OpInfo(int p) : precedence(p) { }
	OpInfo(int p, bool lA) : precedence(p), leftAssociative(lA) { }
};

struct ParseException {
	std::string str;
	std::string msg;
	size_t idx;
	ParseException(std::string s, std::string m, size_t i) : str(s), msg(m), idx(i) { }

	std::string pretty() {
		return msg + "\n" + "   " + str + "\n" +
			std::string(idx, ' ') + "~~ ^ ~~";
	}
};

struct Parser {
	static std::shared_ptr<Expression> parse(std::string str);
	static std::shared_ptr<Expression> parseCanonical(std::string str);

	Parser(std::string s) : _str(s), _len(_str.length()) { }

	std::shared_ptr<Expression> parse();
	std::shared_ptr<Expression> parseCanonical();

	void ignoreWhiteSpace();
	bool is(std::string needle) const;
	bool atEnd() const;
	void expect(std::string needle);
	void except(std::string err);

	private:
		// returns the next chunk of characters until hitting whitespace note:
		// if there is an ${} expr embedded in the chunk, it is folded into the
		// output
		std::shared_ptr<Expression> parseDefaultContextValue();

		std::shared_ptr<Expression> parseVariableName();

		// TODO: comment these
		std::shared_ptr<Expression> parseExpression(int cLevel = 0);
		std::shared_ptr<Expression> parseSingleExpression();
		// parses !func arg1 arg2 arg3 etc...., use ${} to enter parseExpression
		std::shared_ptr<Expression> parseDefaultContext();
		std::shared_ptr<Expression> parseDefaultContextFunction();

		std::shared_ptr<Expression> parseFunctionCall();
		std::shared_ptr<Expression> parseBinaryExpression(int prec = 0);

		std::shared_ptr<Expression> parseNumber();
		std::shared_ptr<Expression> parseString();

		std::shared_ptr<Expression> parseVariableAccess();

		// returns the next string of consecutive digit characters
		std::string grabNumber();

		std::string nextOp();
		int nextPrecedence();
		bool leftAssociative(std::string op) const;
		int precedence(std::string op) const;
		std::shared_ptr<Expression> getBinaryOp();

	private:
		std::string _str;
		size_t _idx{0}, _len{0};
};

std::shared_ptr<Expression> parse(std::string str);


#endif // PARSER_HPP
