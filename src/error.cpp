/**
 * @file error.cpp
 *
 * @date 10/10/2014
 * @author Luciano Santos
 */


#include <iostream>

#include "error.hpp"


namespace tac {
	Error::Error(ErrorLevel level, const std::string& msg, const std::string& file, int line, int col)
			: level(level), msg(msg), file(file), line(line), col(col) {}

	Error::~Error() {}

	void Error::print(std::ostream& out) const {
		/* Prints the level of the error. */
		switch (level) {
			case INFO:
				out << "info:";
				break;

			case WARNING:
				out << "warning:";
				break;

			default:
				out << "error:";
				break;
		}

		if (file.size() > 0) {
			out << " " << file.c_str();
			if (line > 0) {
				out << "(" << line;
				if (col > 0)
					out << "," << col;
				out << ")";
			}
			out << ":";
		}

		out << " " << msg.c_str();
	}

	ErrorLevel Error::errorLevel() const {
		return level;
	}
}
