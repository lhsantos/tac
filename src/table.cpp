/**
 * @file table.cpp
 *
 * @date 10/10/2014
 * @author Luciano Santos
 */

#include <cstdio>
#include <cstring>
#include <sstream>
#include <iomanip>
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
					s << value.arrval->at(i)->to_str() << ", ";
				s << value.arrval->back()->to_str();
				s << ']';
			}
			else
			{
				switch (type->kind)
				{
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

				case Type::ADDR:
					s << value.addrval;
					break;
				}
			}
		}

		if ((kind == PARAM) || (kind == TEMP))
			s << " " << value.addrval;

		return s.str();
	}



	SymbolTable::AddressedElement::AddressedElement(const Symbol* symbol, uint base, size_t size)
		: symbol(symbol), base(base), size(size) { }

	uint SymbolTable::AddressedElement::next_addr() const
	{
		return base + size;
	}

	SymbolTable::SymbolTable()
		: m_last_var_index(-1) { }

	SymbolTable::~SymbolTable()
	{
		addr_list_t::iterator i;
		for (i = m_list.begin(); i != m_list.end(); ++i)
			delete i->symbol;
	}

	void SymbolTable::end_var_section() {
		m_last_var_index = m_list.size() - 1;
	}

	void SymbolTable::put(Symbol* symbol)
	{
		size_t size = ((symbol->type && symbol->type->array_size) ? symbol->type->array_size : 1);
		if (symbol->kind == Symbol::LABEL)
			size = 0;

		map_t::iterator i = m_table.find(*symbol->id);
		if (i == m_table.end())
		{
			i = m_table.insert(pair_t(*symbol->id, m_list.size())).first;
			m_list.push_back(AddressedElement(symbol, m_list.empty() ? 0 : m_list.back().next_addr(), size));
		}
		else
		{
			AddressedElement e = m_list[i->second];
			if (e.symbol != symbol)
			{
				delete e.symbol;
				e.symbol = symbol;
			}
			e.size = size;
		}

		for (uint k = i->second + 1; k < m_list.size(); ++k)
			m_list[k].base = m_list[k - 1].next_addr();

		symbol->registered = true;
	}

	const Symbol* SymbolTable::get(const std::string& id) const
	{
		map_t::const_iterator i = m_table.find(id);
		return (i == m_table.end()) ? 0 : m_list[i->second].symbol;
	}

	uint SymbolTable::get_addr(const std::string& id) const
	{
		map_t::const_iterator i = m_table.find(id);
		return (i == m_table.end()) ? 0 : m_list[i->second].base;
	}

	const Symbol* SymbolTable::get(uint addr) const
	{
		int first = 0;
		int last = m_last_var_index;
		while (first <= last)
		{
			int m = (first + last) / 2;
			if (addr < m_list[m].base)
				last = m - 1;
			else if (addr >= m_list[m].next_addr())
				first = m + 1;
			else
			{
				const Symbol *s = m_list[m].symbol;
				if ((s->type) && (s->type->array_size))
					return s->value.arrval->at(addr - m_list[m].base);
				else if (addr == m_list[m].base)
					return s;
				return 0;
			}
		}
		return 0;
	}

	uint SymbolTable::next_addr() const
	{
		return m_list.empty() ? 0 : m_list.back().next_addr();
	}

	void SymbolTable::show() const
	{
		for (addr_list_t::const_iterator i = m_list.begin(); i != m_list.end(); ++i)
		{
			std::cout << '[';
			std::cout << std::noshowbase << std::hex << std::setw(6) << std::setfill('0') << (int) i->base;
			std::cout << ':';
			std::cout << std::noshowbase << std::hex << std::setw(2) << std::setfill('0') << (int) i->size;
			std::cout << "] ";
			std::cout << i->symbol->to_str() << std::endl;
		}
	}

	std::string* SymbolTable::unique_id(float f)
	{
		char temp[22];
		sprintf(temp, "%.9f", f);
		return new std::string(temp);
	}

	std::string* SymbolTable::unique_id(int i)
	{
		char temp[12];
		sprintf(temp, "%u", i);
		return new std::string(temp);
	}
}
