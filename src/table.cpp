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
 * @file table.cpp
 *
 * @date 2014-10-10
 *
 * @author Luciano Santos
 */

#include <sstream>
#include <iomanip>

#include "table.hpp"


namespace tac {
	SymbolTable::SymbolTable(uint size)
		: m_memmngr(size) { }

	SymbolTable::~SymbolTable() {
		for (auto p : m_labels)
			delete p.second;
	}

	bool SymbolTable::put(Symbol* symbol)
	{
		if (symbol->kind == Symbol::LABEL)
		{
			auto i = m_labels.find(*symbol->id);
			if (i == m_labels.end())
				m_labels[*symbol->id] = symbol;
			else
			{
				if (i->second != symbol)
					delete i->second;
				i->second = symbol;
			}
		}
		else
		{
			uint addr;
			map_t::iterator i = m_table.find(*symbol->id);
			if (i == m_table.end())
			{
				if (!m_memmngr.put(symbol, addr))
					return false;
				m_table[*symbol->id] = addr;
			}
			else
			{
				addr = i->second;
				const Symbol *old = m_memmngr.get_block(addr);
				if (old != symbol)
				{
					m_memmngr.free(addr);
					if (!m_memmngr.put(symbol, addr))
						return false;
				}
				i->second = addr;
			}
		}

		symbol->registered = true;
		return true;
	}

	const Symbol* SymbolTable::get(const std::string& id) const
	{
		auto i = m_labels.find(id);
		if (i != m_labels.end())
			return i->second;
		else
		{
			auto j = m_table.find(id);
			return (j == m_table.end()) ? nullptr : m_memmngr.get_block(j->second);
		}
	}

	uint SymbolTable::get_addr(const std::string& id) const
	{
		map_t::const_iterator i = m_table.find(id);
		return (i == m_table.end()) ? 0 : i->second;
	}

	const Symbol* SymbolTable::get(uint addr) const
	{
		return m_memmngr.get(addr);
	}

	uint SymbolTable::upper_bound() const
	{
		return m_memmngr.upper_bound();
	}

	void SymbolTable::show() const
	{
		for (auto s : m_memmngr.m_blocks)
		{
			std::cout << '[';
			std::cout << std::noshowbase << std::hex << std::setw(6) << std::setfill('0') << (int) s.base;
			std::cout << ':';
			std::cout << std::noshowbase << std::hex << std::setw(2) << std::setfill('0') << (int) s.size;
			std::cout << "] ";
			std::cout << s.symbol->to_str() << std::endl;
		}
	}

	std::string* SymbolTable::unique_id(float f)
	{
		std::ostringstream s;
		s << std::setprecision(9) << f;
		return new std::string(s.str());
	}

	std::string* SymbolTable::unique_id(int i)
	{
		std::ostringstream s;
		s << i;
		return new std::string(s.str());
	}
}
