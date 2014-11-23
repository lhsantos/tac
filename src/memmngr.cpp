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
 * @file memmngr.cpp
 *
 * @brief
 *
 * @date 2014-11-19
 *
 * @author Luciano Santos
 */

#include <algorithm>

#include "memmngr.hpp"


namespace tac
{
	uint MemoryManager::Block::upper_bound() const
	{
		return base + size;
	}

	bool MemoryManager::Block::operator<(const Block& other) const
	{
		return operator<(other.base);
	}

	bool MemoryManager::Block::operator<(uint addr) const
	{
		return base < addr;
	}



	MemoryManager::MemoryBlock::MemoryBlock(uint base, size_t size, const Symbol *s) :
		Block{base, size}, symbol(s) { }



	MemoryManager::MemoryManager(uint max_size) :
			m_max_size(max_size),
			m_size(0)
	{
		m_stalls.push_back(Block( {0, max_size} ));
	}

	MemoryManager::~MemoryManager()
	{
		clear();
	}

	bool MemoryManager::alloc(size_t size, uint& addr)
	{
		return put(0, size, addr);
	}

	bool MemoryManager::put(const Symbol *s, uint& addr)
	{
		return put(s, 0, addr);
	}

	const Symbol* MemoryManager::get(uint addr) const
	{
		// Let's do a binary search...
		int p = 0, q = m_blocks.size() - 1;
		while (p <= q)
		{
			int i = (p + q) >> 1;
			// If address is before middle element's base address, check lower partition.
			if (addr < m_blocks[i].base)
				q = i - 1;
			else
			{
				// The address is geq to middle element's base address, but is it within
				// this block's boundaries?
				if (addr < m_blocks[i].upper_bound())
				{
					auto s = m_blocks[i].symbol;
					return (s->type->array_size) ?
							s->value.arrval->at(addr - m_blocks[i].base) : s;
				}
				else
				{
					// OK, it's not in the block boundaries, but is it at least in the
					// next block? If not, it's in a stall.
					p = i + 1;
					if ((p > q) || (addr < m_blocks[p].base))
						return nullptr;
				}
			}
		}

		return nullptr;
	}

	const Symbol* MemoryManager::get_block(uint addr) const
	{
		// Let's do a binary search...
		int p = 0, q = m_blocks.size() - 1;
		while (p <= q)
		{
			int i = (p + q) >> 1;
			if (addr < m_blocks[i].base)
				q = i - 1;
			else if (addr > m_blocks[i].base)
				p = i + 1;
			else
				return m_blocks[i].symbol;
		}

		return nullptr;
	}

	bool MemoryManager::free(uint addr)
	{
		// Looks for the block.
		auto block = std::lower_bound(m_blocks.begin(), m_blocks.end(), addr);
		if ((block == m_blocks.end()) || (addr != block->base))
			return false;

		// Creates (or merges) a stall.
		auto i = m_stalls.begin();
		while ((i != m_stalls.end()) && (addr > i->upper_bound()))
			++i;
		if (i == m_stalls.end())
			m_stalls.push_back(Block{addr, block->size});
		else
		{
			if (addr == i->upper_bound())
				i->size += block->size;
			else // here addr must be less than i->base
			{
				if ((addr + block->size) == i->base)
				{
					i->base = addr;
					i->size += block->size;
				}
				else
					m_stalls.insert(i, Block{addr, block->size});
			}
		}

		// Removes the block from list.
		delete block->symbol;
		m_blocks.erase(block);

		return true;
	}

	void MemoryManager::clear()
	{
		for (auto block : m_blocks)
			delete block.symbol;

		m_blocks.clear();
		m_stalls.clear();
		m_stalls.push_back( Block{0, m_max_size} );
	}

	size_t MemoryManager::available() const
	{
		return m_max_size - m_size;
	}

	uint MemoryManager::upper_bound() const
	{
		return m_blocks.empty() ? 0 : m_blocks.back().upper_bound();
	}

	bool MemoryManager::put(const Symbol *symbol, size_t size, uint& addr)
	{
		// Was a symbol provided?
		if (symbol)
			size = (symbol->type && symbol->type->array_size) ? symbol->type->array_size : 1;

		// Is size valid?
		if ((!size) || (size > available()))
			return false;

		// Finds the largest stall that fits the block size.
		auto stall = std::max_element(m_stalls.begin(), m_stalls.end(),
				[] (const Block& a, const Block& b)
				{
					return a.size < b.size;
				});
		if ((stall == m_stalls.end()) || (size > stall->size))
			return false;

		// If necessary allocates the new symbol.
		if (!symbol)
		{
			Symbol *s = new Symbol(0, location(), Symbol::TEMP, new Type(Type::CHAR));
			if (size > 1)
			{
				s->type->array_size = size;
				auto v = new std::vector<Symbol*>(size);
				for (auto i = v->begin(); i != v->end(); ++i)
					*i = new Symbol(0, location(), Symbol::TEMP, new Type(Type::CHAR));
				s->value.arrval = v;
			}
			symbol = s;
		}

		// Inserts the symbol.
		addr = stall->base;
		m_blocks.push_back(MemoryBlock(stall->base, size, symbol));
		uint i = m_blocks.size() - 1;
		while ((i > 0) && (m_blocks[i].base < m_blocks[i - 1].base))
		{
			std::swap(m_blocks[i], m_blocks[i - 1]);
			++i;
		}

		// Updates stall list.
		if (size < stall->size)
		{
			stall->base += size;
			stall->size -= size;
		}
		else
			m_stalls.erase(stall);

		return true;
	}
}
