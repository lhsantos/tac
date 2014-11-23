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
 * @file symbol.hpp
 *
 * @date 2014-11-23
 *
 * @author Luciano Santos
 */

#ifndef INCLUDE_SYMBOL_HPP_
#define INCLUDE_SYMBOL_HPP_

#include <string>
#include <vector>

#include "location.hh"


namespace tac
{
	struct Type
	{
	public:
		enum Kind
		{
			CHAR  = 0,
			INT   = 1,
			FLOAT = 2,
			ADDR  = 3
		};

		Kind kind; // the language type
		size_t array_size;

		/**
		 * @brief Creates a copy of a type.
		 *
		 * The pointee will be duplicated, i.e., a newly created object will be associated to this type,
		 * so both objects can be destroyed independently.
		 *
		 * @param other
		 */
		Type(const Type& other);

		/**
		 * @brief Creates a new type descriptor, with given values.
		 *
		 * The name string will not be destroyed, the pointee will, even if it was allocated outside this class.
		 *
		 * @param kind
		 * @param constant
		 * @param pointee
		 * @param array_size
		 */
		Type(Kind kind, size_t array_size = 0);

		/**
		 * @brief Destroys this type, and recursively destroys its pointee.
		 */
		~Type();

		std::string to_str() const;

	private:
		void init(const Type& other);
	};


	struct Symbol
	{
	public:
		enum Kind
		{
			LABEL,
			VAR,
			CONST,
			TEMP,
			PARAM
		};

		union Value
		{
			uint addrval;
			int ival;
			char cval;
			float fval;
			std::vector<Symbol*>* arrval;
		};

		const std::string *id;
		Kind kind; // table entry type of this symbol
		Type *type; // type of this symbol
		Value value; // value of this symbol
		bool registered;
		location loc;


		/**
		 * @brief Creates a new symbol.
		 *
		 * Note that id and type are created elsewhere, but will be destroyed when this symbol is destroyed.
		 *
		 * @param id
		 * @param kind
		 * @param type
		 * @param args
		 */
		Symbol(const std::string*, const location&, Kind, Type* type = 0);

		explicit Symbol(const Symbol&);

		~Symbol();

		std::string to_str() const;
	};
}

#endif /* INCLUDE_SYMBOL_HPP_ */
