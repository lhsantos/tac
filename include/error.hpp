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
 * @file error.hpp
 *
 * @date 2014-10-10
 *
 * @author  Luciano Santos
 */

#ifndef ERROR_HPP_
#define ERROR_HPP_ 1

#include <ostream>
#include <exception>
#include "location.hh"


namespace tac
{
	enum ErrorLevel
	{
		INFO,
		WARNING,
		ERROR
	};

	class Error
	{
	public:
		Error(
			ErrorLevel level,
			const std::string& msg,
			const std::string& file = "",
			int line = 0,
			int col = 0);

		~Error();

		ErrorLevel errorLevel() const;

		void print(std::ostream&) const;

	private:
		ErrorLevel level;
		std::string msg;
		std::string file;
		int line;
		int col;
	};


	class TACException : public std::exception
	{
	public:
		TACException(const position&) throw();

		virtual ~TACException() throw();

		operator Error() const;

		virtual const std::string msg() const = 0;


	protected:
		const position pos;
	};
}

#endif /* ERROR_HPP_ */
