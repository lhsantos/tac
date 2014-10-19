/**
 * @file error.hpp
 *
 * @date 10/10/2014
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
