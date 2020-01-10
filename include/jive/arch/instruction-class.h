/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_INSTRUCTION_CLASS_H
#define JIVE_ARCH_INSTRUCTION_CLASS_H

#include <string.h>

#include <jive/arch/immediate.h>
#include <jive/arch/linker-symbol.h>
#include <jive/arch/registers.h>
#include <jive/rvsdg/label.h>
#include <jive/rvsdg/node.h>
#include <jive/types/bitstring/type.h>

struct jive_compilate;
struct jive_label;
struct jive_label_symbol_mapper;

typedef struct jive_asmgen_imm jive_asmgen_imm;
typedef struct jive_codegen_imm jive_codegen_imm;

/* immediates, as represented during code generation */

/** \brief Information about compile-time knowledge of value */

typedef struct jive_codegen_imm jive_codegen_imm;


enum jive_codegen_imm_info {
	/* the value is known at present time, and will be the same on
	each translation attempt; instruction encoding should use given value
	as is */
	jive_codegen_imm_info_static_known = 0,
	/* the value is known to be unknowable at translation time; instruction
	encoding must ensure that an arbitrary value can be substituted at link
	time */
	jive_codegen_imm_info_static_unknown = 1,
	/* the value is known at present time, but might change for future
	translation attempts; instruction encoding should use given value as
	is, but if the current value allows a "smaller" code representation
	as the one used in a previous translation attempt, it should stick
	with the larger representation (to ensure eventual convergence) */
	jive_codegen_imm_info_dynamic_known = 2,
	/* the value is not known at the present time, but a value will be
	supplied in subsequent translation attempts; instruction encoding
	should assume an arbitrary value at its own discretion and perform
	a dummy encoding */
	jive_codegen_imm_info_dynamic_unknown = 3
};

struct jive_codegen_imm {
	/** \brief Knowledge about immediate value */
	jive_codegen_imm_info info;
	/** \brief Numeric portion of immediate value */
	jive_immediate_int value;
	/** \brief Referenced symbol */
	jive_symref symref;
	/** \brief Whether the symbol is to be interpreted pc-relative */
	bool pc_relative;
};

/* immediates, as represented during asm generation */

struct jive_asmgen_imm {
	/** \brief Numeric portion of immediate value */
	jive_immediate_int value;
	/** \brief Symbol to be added to base value
	 
	String representation of the symbol to be added, or NULL if there
	is no symbol to be added. */
	const char * add_symbol;
	/** \brief Symbol to be subtracted from base value
	
	String representation of the symbol to be subtracted, or NULL if
	there is no symbol to be subtracted. */
	const char * sub_symbol;
};

/* instruction representation */

enum jive_instruction_encoding_flags {
	jive_instruction_encoding_flags_none = 0,
	/* instruction is a conditional branch, and its decision logic
	is to be inverted during codegen */
	jive_instruction_encoding_flags_jump_conditional_invert = 1,
	
	/* the following flags may be updated by instruction encoding itself
	
	instructions may (depending on label distance etc.) have to choose
	between different displacement sizes, depending on labels; the idea is
	to start out conservative, but allow the instruction to "expand" its
	encoded size in case it does not fit (but never shrink) */
	
	jive_instruction_encoding_flags_option0 = (1<<16),
	jive_instruction_encoding_flags_option1 = (1<<17),
	jive_instruction_encoding_flags_option2 = (1<<18),
	jive_instruction_encoding_flags_option3 = (1<<19),
	jive_instruction_encoding_flags_option4 = (1<<20),
	jive_instruction_encoding_flags_option5 = (1<<21),
	jive_instruction_encoding_flags_option6 = (1<<22),
	jive_instruction_encoding_flags_option7 = (1<<23),
} ;

static inline constexpr jive_instruction_encoding_flags
operator|(jive_instruction_encoding_flags a, jive_instruction_encoding_flags b)
{
	return static_cast<jive_instruction_encoding_flags>(
		static_cast<int>(a) | static_cast<int>(b));
}

static inline constexpr jive_instruction_encoding_flags
operator&(jive_instruction_encoding_flags a, jive_instruction_encoding_flags b)
{
	return static_cast<jive_instruction_encoding_flags>(
		static_cast<int>(a) & static_cast<int>(b));
}

namespace jive {

class buffer;
class section;

class instruction {
public:
	enum class flags {
		none = 0,
		/* instruction reuses first input register as output */
		write_input = 1,
		/* first two input operands are commutative */
		commutative = (1<<8),
		/* instruction is an (unconditional) jump */
		jump = (1<<12),
		/* instruction is a relative jump */
		jump_relative = (1<<13),
		/* instruction is a conditional jump and there is a matching "inverse" instruction */
		jump_conditional_invertible = (1<<14)
	};

	virtual
	~instruction()
	{}

	inline
	instruction(
		const std::string & name,
		int code,
		const std::string & mnemonic,
		const std::vector<const jive::register_class*> & inputs,
		const std::vector<const jive::register_class*> & outputs,
		size_t nimmediates,
		enum instruction::flags flags,
		const jive::instruction * inverse_jump)
	: code_(code)
	, name_(name)
	, nimmediates_(nimmediates)
	, mnemonic_(mnemonic)
	, flags_(flags)
	, inverse_jump_(inverse_jump)
	, inputs_(inputs)
	, outputs_(outputs)
	{}

	/** \brief Descriptive name of instruction */
	inline const std::string &
	name() const noexcept
	{
		return name_;
	}

	/** \brief Opcode of instruction */
	inline int
	code() const noexcept
	{
		return code_;
	}

	/** \brief Mnemonic name of instruction */
	inline const std::string &
	mnemonic() const noexcept
	{
		return mnemonic_;
	}

	inline instruction::flags
	flags() const noexcept
	{
		return flags_;
	}

	bool
	is_commutative() const noexcept;

	inline size_t
	nimmediates() const noexcept
	{
		return nimmediates_;
	}

	/** \brief Inverse jump class (only meaningful if flag set accordingly) */
	inline const jive::instruction *
	inverse_jump() const noexcept
	{
		return inverse_jump_;
	}

	inline size_t
	ninputs() const noexcept
	{
		return inputs_.size();
	}

	inline const jive::register_class *
	input(size_t n) const noexcept
	{
		JIVE_DEBUG_ASSERT(n < ninputs());
		return inputs_[n];
	}

	inline size_t
	noutputs() const noexcept
	{
		return outputs_.size();
	}

	inline const jive::register_class *
	output(size_t n) const noexcept
	{
		JIVE_DEBUG_ASSERT(n < noutputs());
		return outputs_[n];
	}

	/**
		\brief Generate code
		\param target Target buffer to put encoded instructions into
	*/
	virtual void
	encode(
		jive::section * target,
		const jive::registers * inputs[],
		const jive::registers * outputs[],
		const jive_codegen_imm immediates[],
		jive_instruction_encoding_flags * flags) const = 0;

	/**
		\brief Generate mnemonic
		\param target Target buffer to put mnemonic instruction into
		\param instruction Instruction to encode
	*/
	virtual void
	write_asm(
		jive::buffer * target,
		const jive::registers * inputs[],
		const jive::registers * outputs[],
		const jive_asmgen_imm immediates[],
		jive_instruction_encoding_flags * flags) const = 0;

	virtual std::unique_ptr<jive::instruction>
	copy() const = 0;

private:
	int code_;
	std::string name_;
	size_t nimmediates_;
	std::string mnemonic_;
	enum instruction::flags flags_;
	const jive::instruction * inverse_jump_;
	std::vector<const jive::register_class*> inputs_;
	std::vector<const jive::register_class*> outputs_;
};

static inline constexpr enum instruction::flags
operator|(enum instruction::flags a, enum instruction::flags b)
{
	return static_cast<enum instruction::flags>(static_cast<int>(a) | static_cast<int>(b));
}

static inline constexpr enum instruction::flags
operator&(enum instruction::flags a, enum instruction::flags b)
{
	return static_cast<enum instruction::flags>(static_cast<int>(a) & static_cast<int>(b));
}

/* instruction methods */

inline bool
instruction::is_commutative() const noexcept
{
	return static_cast<int>(flags() & flags::commutative);
}

}

#endif
