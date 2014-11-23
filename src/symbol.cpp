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
 * @file symbol.cpp
 *
 * @date 2014-11-24
 *
 * @author Luciano Santos
 */

#include <cstring>
#include <sstream>
#include <iomanip>

#include "symbol.hpp"


namespace tac {
	Type::Type(const Type& other)
			: kind(other.kind),
			  array_size(other.array_size) { }

	Type::Type(Type::Kind kind, size_t array_size)
			: kind(kind),
			  array_size(array_size) { }

	Type::~Type() { }

	std::string Type::to_str() const
	{
		std::ostringstream sstream;

		switch (kind)
		{
		case INT:
			sstream << "int";
			break;

		case CHAR:
			sstream << "char";
			break;

		case FLOAT:
			sstream << "float";
			break;

		case ADDR:
			sstream << "addr";
			break;
		}

		if (array_size)
		{
			sstream << "[";
			sstream << array_size;
			sstream << "]";
		}

		return sstream.str();
	}



	Symbol::Symbol(const std::string* id, const location &loc, Kind kind, Type* type)
			: id(id),
			  kind(kind),
			  type(type), registered(false),
			  loc(loc)
	{
		memset(&value, 0, sizeof(value));
	}

	Symbol::Symbol(const Symbol& other)
			: id(0),
			  kind(other.kind),
			  type(0),
			  value(other.value),
			  registered(other.registered),
			  loc(other.loc)
	{
		if (other.id)
			this->id = new std::string(*other.id);

		if (other.type)
		{
			this->type = new Type(*other.type);
			if (other.type->array_size)
			{
				this->value.arrval = new std::vector<Symbol*>();
				this->value.arrval->reserve(other.value.arrval->size());
				for (std::vector<Symbol*>::iterator i = this->value.arrval->begin(); i != this->value.arrval->end(); ++i)
					this->value.arrval->push_back(new Symbol(**i));
			}
		}
	}

	Symbol::~Symbol()
	{
		delete id;
		if ((type) && (type->array_size) && (value.arrval))
		{
			for (std::vector<Symbol*>::iterator i = value.arrval->begin(); i != value.arrval->end(); ++i)
				delete (*i);
		}
		delete type;
	}

	static void printval(std::ostringstream &s, Symbol::Value v, Type *t) {
		switch (t->kind)
		{
		case Type::INT:
			s << v.ival;
			break;

		case Type::CHAR:
			s << "'";
			if (v.cval < 32)
				s << "\\" << (int) v.cval;
			else
				s << v.cval;
			s << "'";
			break;

		case Type::FLOAT:
			s << v.fval;
			break;

		case Type::ADDR:
			s << v.addrval;
			break;
		}
	}

	std::string Symbol::to_str() const
	{
		std::ostringstream s;

		switch (kind)
		{
		case LABEL:
			s << "label";
			break;

		case VAR:
			s << "var";
			break;

		case CONST:
			s << "const";
			break;

		case TEMP:
			s << "temp";
			break;

		case PARAM:
			s << "param";
			break;
		}

		if (id)
			s << " " << *id;

		if (type)
		{
			s << ": " << type->to_str() << " ";

			if (type->array_size)
			{
				s << '[';
				for (size_t i = 0; i < type->array_size - 1; ++i)
				{
					printval(s, value.arrval->at(i)->value, value.arrval->at(i)->type);
					s << ", ";
				}
				printval(s, value.arrval->back()->value, value.arrval->back()->type);
				s << ']';
			}
			else
				printval(s, value, type);
		}

		if ((kind == PARAM) || (kind == TEMP))
			s << " " << value.addrval;

		return s.str();
	}
}
