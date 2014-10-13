/**
 * @file table.cpp
 *
 * @date 10/10/2014
 * @author Luciano Santos
 */

#include <cstdio>
#include <cstring>
#include <sstream>
#include <set>

#include "table.hpp"

namespace tac {
	Type::Type(const Type& other)
			: kind(other.kind),
			  array_size(other.array_size) { }

	Type::Type(Type::Kind kind, size_t array_size)
			: kind(kind),
			  array_size(array_size) { }

	Type::~Type() { }

	std::string Type::to_str() const {
		std::ostringstream sstream;

		switch (kind) {
		case INT:
			sstream << "int";
			break;

		case CHAR:
			sstream << "char";
			break;

		case FLOAT:
			sstream << "float";
			break;
		}

		if (array_size) {
			sstream << "[";
			sstream << array_size;
			sstream << "]";
		}

		return sstream.str();
	}



	Symbol::Symbol(const std::string* id, const location &loc, Kind kind, Type* type)
			: id(id), kind(kind), type(type), registered(false), loc(loc) {
		memset(&value, 0, sizeof(value));
	}

	Symbol::Symbol(const Symbol& other)
			: id(0),
			  kind(other.kind),
			  type(0),
			  value(other.value),
			  registered(other.registered),
			  loc(other.loc) {

		if (other.id)
			this->id = new std::string(*other.id);

		if (other.type) {
			this->type = new Type(*other.type);
			if (other.type->array_size) {
				this->value.arrval = new std::vector<Symbol*>();
				this->value.arrval->reserve(other.value.arrval->size());
				for (std::vector<Symbol*>::iterator i = this->value.arrval->begin(); i != this->value.arrval->end(); ++i)
					this->value.arrval->push_back(new Symbol(**i));
			}
		}
	}

	Symbol::~Symbol() {
		delete id;
		if ((type) && (type->array_size) && (value.arrval)) {
			for (std::vector<Symbol*>::iterator i = value.arrval->begin(); i != value.arrval->end(); ++i)
				delete (*i);
		}
		delete type;
	}

	std::string Symbol::to_str() const {
		std::ostringstream s;

		switch (kind) {
		case LABEL:
			s << "LABEL";
			break;

		case VAR:
			s << "VAR";
			break;

		case CONST:
			s << "CONST";
			break;

		case TEMP:
			s << "TEMP";
			break;

		case PARAM:
			s << "PARAM";
			break;
		}

		if (id) {
			s << " " << *id;
		}

		if (type) {
			s << ": " << type->to_str() << " ";

			if (type->array_size) {
				s << '[';
				for (size_t i = 0; i < type->array_size - 1; ++i)
					s << value.arrval->at(i)->to_str() << ", ";
				s << value.arrval->back()->to_str();
				s << ']';
			}
			else {
				switch (type->kind) {
				case Type::INT:
					s << value.ival;
					break;

				case Type::CHAR:
					s << "'";
					if (value.cval < 32)
						s << "\\" << (int) value.cval;
					else
						s << value.cval;
					s << "'";
					break;

				case Type::FLOAT:
					s << value.fval;
					break;
				}
			}
		}

		if ((kind == LABEL) || (kind == PARAM) || (kind == TEMP))
			s << " " << value.addrval;

		return s.str();
	}



	SymbolTable::SymbolTable() {}

	SymbolTable::~SymbolTable() {
		std::set<std::string> deleted;
		for (map_t::iterator i = m_table.begin(); i != m_table.end(); ++i) {
			if (deleted.find(i->first) == deleted.end()) {
				deleted.insert(i->first);
				delete i->second;
			}
		}
	}

	void SymbolTable::put(Symbol* symbol) {
		put(m_table, symbol);
		symbol->registered = true;

		std::cout << "declared: " << symbol->to_str() << std::endl;
	}

	const Symbol* SymbolTable::get(const std::string& id) const {
		map_t::const_iterator i = m_table.find(id);
		return (i == m_table.end()) ? 0 : i->second;
	}

	std::string* SymbolTable::unique_id(float f) {
		char temp[22];
		sprintf(temp, "%.9f", f);
		return new std::string(temp);
	}

	std::string* SymbolTable::unique_id(int i) {
		char temp[12];
		sprintf(temp, "%u", i);
		return new std::string(temp);
	}

	void SymbolTable::put(map_t& map, Symbol* symbol) {
		map_t::iterator i = map.find(*symbol->id);
		if (i == map.end())
			map.insert(pair_t(*symbol->id, symbol));
		else {
			if (i->second != symbol) {
				delete i->second;
				i->second = symbol;
			}
		}
	}
}
