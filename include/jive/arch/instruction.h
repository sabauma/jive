/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_INSTRUCTION_H
#define JIVE_ARCH_INSTRUCTION_H

#include <algorithm>
#include <string.h>

#include <jive/arch/immediate-node.h>
#include <jive/arch/instruction-class.h>
#include <jive/arch/linker-symbol.h>
#include <jive/arch/registers.h>
#include <jive/types/bitstring/type.h>
#include <jive/util/ptr-collection.h>
#include <jive/vsdg/label.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/operators/nullary.h>
#include <jive/vsdg/statetype.h>

namespace jive {

class instruction_op final : public simple_op {
public:
	virtual
	~instruction_op() noexcept;

	inline
	instruction_op(
		const jive::instruction_class * icls,
		const std::vector<const jive::state::type*> & istates,
		const std::vector<const jive::state::type*> & ostates)
		: icls_(icls)
		, istates_(istates.size())
		, ostates_(ostates.size())
	{
		std::transform(istates.begin(), istates.end(), istates_.begin(),
			[](const auto & t){ return t->copy(); });
		std::transform(ostates.begin(), ostates.end(), ostates_.begin(),
			[](const auto & t){ return t->copy(); });
	}

	inline
	instruction_op(const instruction_op & other)
		: icls_(other.icls_)
		, istates_(detail::unique_ptr_vector_copy(other.istates_))
		, ostates_(detail::unique_ptr_vector_copy(other.ostates_))
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual size_t
	narguments() const noexcept override;

	virtual const jive::base::type &
	argument_type(size_t index) const noexcept override;

	virtual const jive_resource_class *
	argument_cls(size_t index) const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual const jive::base::type &
	result_type(size_t index) const noexcept override;

	virtual const jive_resource_class *
	result_cls(size_t index) const noexcept override;
	virtual std::string
	debug_string() const override;

	inline const jive::instruction_class *
	icls() const noexcept
	{
		return icls_;
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	const jive::instruction_class * icls_;
	std::vector<std::unique_ptr<base::type>> istates_;
	std::vector<std::unique_ptr<base::type>> ostates_;
};

}

jive::node *
jive_instruction_node_create_simple(
	struct jive::region * region,
	const jive::instruction_class * icls,
	jive::oport * const * operands,
	const int64_t * immediates);

jive::node *
jive_instruction_node_create_extended(
	struct jive::region * region,
	const jive::instruction_class * icls,
	jive::oport * const * operands,
	const jive::immediate immediates[]);

jive::node *
jive_instruction_node_create(
	struct jive::region * region,
	const jive::instruction_class * icls,
	const std::vector<jive::oport*> & operands,
	const std::vector<jive::immediate> & immediates,
	const std::vector<const jive::state::type*> & itypes,
	const std::vector<jive::oport*> & istates,
	const std::vector<const jive::state::type*> & otypes);

JIVE_EXPORTED_INLINE jive::node *
jive_instruction_node_create(
	struct jive::region * region,
	const jive::instruction_class * icls,
	jive::oport * const * operands,
	const int64_t * immediates)
{
	return jive_instruction_node_create_simple(region, icls, operands, immediates);
}

#endif
