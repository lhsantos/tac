/*
Copyright 2014 Luciano Henrique de Oliveira Santos

This file is part of TAC project.

TAC project is licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/**
 * @file error.cpp
 *
 * @date 2014-10-10
 *
 * @author Luciano Santos
 */


#include <iostream>

#include "error.hpp"


namespace tac {
	Error::Error(ErrorLevel level, const std::string &msg, const std::string &file, int line, int col)
			: level(level), msg(msg), file(file), line(line), col(col) { }

	Error::~Error() { }

	void Error::print(std::ostream& out) const
	{
		/* Prints the level of the error. */
		switch (level)
		{
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

		if (file.size() > 0)
		{
			out << " " << file.c_str();
			if (line > 0)
			{
				out << "(" << line;
				if (col > 0)
					out << "," << col;
				out << ")";
			}
			out << ":";
		}

		out << " " << msg.c_str();
	}

	ErrorLevel Error::errorLevel() const
	{
		return level;
	}




	TACException::TACException(const position& pos) throw()
			: pos(pos) { }

	TACException::~TACException() throw() { }

	TACException::operator Error() const {
		return Error(ERROR, msg(), *pos.filename, pos.line, pos.column);
	}
}
