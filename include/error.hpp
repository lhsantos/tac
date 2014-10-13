/**
 * @file error.hpp
 *
 * @date 10/10/2014
 * @author  Luciano Santos
 */

#ifndef ERROR_HPP_
#define ERROR_HPP_ 1

#include <ostream>

namespace tac {
	enum ErrorLevel {
		INFO,
		WARNING,
		ERROR
	};

	class Error {
	public:
		Error(
			ErrorLevel level,
			const std::string& msg,
			const std::string& file = "",
			int line = 0,
			int col = 0);

		~Error();

		ErrorLevel errorLevel() const;

		void print(std::ostream& out) const;

	private:
		ErrorLevel level;
		std::string msg;
		std::string file;
		int line;
		int col;
	};
}

#endif /* ERROR_HPP_ */
