/*
 * Copyright 2012 2013 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_COMPILATE_H
#define JIVE_ARCH_COMPILATE_H

#include <stdint.h>

#include <jive/arch/linker-symbol.h>
#include <jive/rvsdg/label.h>
#include <jive/rvsdg/section.h>
#include <jive/util/buffer.h>

struct jive_linker_symbol_resolver;

typedef struct jive_relocation_type jive_relocation_type;

/**
	\brief Architecture-specific representation of relocation type
*/
struct jive_relocation_type {
	uint32_t arch_code;
};

namespace jive {

/**
	\brief Relocation table entry
*/
class relocation_entry {
public:
	inline constexpr
	relocation_entry(
		jive_offset offset,
		const jive_relocation_type & type,
		jive_symref target,
		jive_offset value)
	: offset_(offset)
	, type_(type)
	, target_(target)
	, value_(value)
	{}

	inline jive_offset
	offset() const noexcept
	{
		return offset_;
	}

	inline const jive_relocation_type &
	type() const noexcept
	{
		return type_;
	}

	inline jive_symref
	target() const noexcept
	{
		return target_;
	}

	inline jive_offset
	value() const noexcept
	{
		return value_;
	}

private:
	/** \brief Offset within section of relocation record */
	jive_offset offset_;

	/** \brief Type of relocation to be applied */
	jive_relocation_type type_;

	/** \brief Target address of relocation operation */
	jive_symref target_;

	/** \brief Additional offset to be applied to symbol */
	jive_offset value_;
};

}

/**
	\brief Apply relocation processing in a single location
	\param where Location of the data item
	\param max_size Maximum allowed size of data item
	\param offset Assumed offset of data item
	\param type Relocation type
	\param symoffset Offset of the symbol being referenced
	\param value Additional value from the relocation record
	\returns True if relocation could be applied
*/
typedef bool (*jive_process_relocation_function)(
	void * where, size_t max_size, jive_offset offset,
	jive_relocation_type type, jive_offset target, jive_offset value);

namespace jive {

/**
	\brief Section of a compilate
*/
class section {
public:
	inline
	~section()
	{
		clear();
	}

	inline
	section(jive_stdsectionid id)
	: id_(id)
	{}

	section(const section &) = delete;

	section(section &&) = delete;

	section &
	operator=(const section &) = delete;

	section &
	operator=(section &&) = delete;

	inline jive_stdsectionid
	id() const noexcept
	{
		return id_;
	}


	inline const uint8_t *
	data()
	{
		return data_.data();
	}

	inline size_t
	size() const noexcept
	{
		return data_.size();
	}

	inline void
	put(const void * data, size_t size)
	{
		data_.push_back(data, size);
	}

	inline void
	putbyte(uint8_t byte)
	{
		data_.push_back(byte);
	}

	inline void
	clear()
	{
		data_.clear();
		for (const auto & relocation : relocations)
			delete relocation;
		relocations.clear();
	}

	void
	add_relocation(
		const void * data,
		size_t size,
		jive_relocation_type type,
		jive_symref target,
		jive_offset value);

	std::unordered_set<relocation_entry*> relocations;

private:
	jive::buffer data_;
	jive_stdsectionid id_;
};

/* compilate */

class compilate_map;

class compilate final {
public:
	~compilate();

	inline
	compilate()
	{}

	compilate(const compilate &) = delete;

	compilate(compilate &&) = delete;

	compilate &
	operator=(const compilate &) = delete;

	compilate &
	operator=(compilate &&) = delete;

	/**
		\brief Clear compilation object

		Clears the contents of the given compilation object, i.e. subsequently
		it behaves as if it were newly allocated (actual buffers allocated
		might be reused as an optimization, though).
	*/
	void
	clear();

	jive::section *
	section(jive_stdsectionid sectionid);

	/**
		\brief Load a compilate into process' address space

		\returns A description of where sections have been mapped

		Maps all of the sections contained in the compilate into the process'
		address space. Returns a structure describing the mapping of the
		sections to the address space.
	*/
	std::unique_ptr<jive::compilate_map>
	load(
		const struct jive_linker_symbol_resolver * sym_resolver,
		jive_process_relocation_function relocate);

private:
	std::vector<std::unique_ptr<jive::section>> sections_;
};

}

class jive_compilate_section {
public:
	inline
	jive_compilate_section(const jive::section * _section, void * _base, size_t _size)
		: section(_section)
		, base(_base)
		, size(_size)
	{}

	/** \brief Section descriptor from compilate */
	const jive::section * section;
	/** \brief Base address in process' address space */
	void * base;
	/** \brief Size of loaded section */
	size_t size;
};

/** \brief Represent a compilate loaded into process' address space */

namespace jive {

class compilate_map final {
public:
	~compilate_map();

	inline
	compilate_map()
	{}

	compilate_map(const compilate_map &) = delete;

	compilate_map(compilate_map &&) = delete;

	compilate_map &
	operator=(const compilate_map &) = delete;

	compilate_map &
	operator=(compilate_map &&) = delete;

	inline void *
	section(jive_stdsectionid id)
	{
		for (const auto & cs : sections) {
			if (cs.section->id() == id)
				return cs.base;
		}

		return nullptr;
	}

	std::vector<jive_compilate_section> sections;
};

}

#endif
