/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_REGVALUE_H
#define JIVE_ARCH_REGVALUE_H

#include <stdint.h>

#include <jive/arch/registers.h>
#include <jive/vsdg/node.h>

extern const jive_node_class JIVE_REGVALUE_NODE;

namespace jive {

class regvalue_operation final : public jive_node_attrs {
public:
	inline explicit constexpr
	regvalue_operation(const jive_register_class * regcls) noexcept
		: regcls_(regcls)
	{
	}

	inline const jive_register_class * regcls() const { return regcls_; }
private:
	const jive_register_class * regcls_;
};

}

typedef jive::operation_node<jive::regvalue_operation> jive_regvalue_node;

/**
	\brief Create register constant
	\param ctl Control of region entry where value is alive
	\param regcls Register class
	\param value Value to be represented
	\returns Bitstring value representing constant, constrained to register class
	
	Convenience function that either creates a new constant or
	returns the output handle of an existing constant.
*/
jive::output *
jive_regvalue(jive::output * ctl, const jive_register_class * regcls, jive::output * value);

JIVE_EXPORTED_INLINE jive_regvalue_node *
jive_regvalue_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_REGVALUE_NODE))
		return (jive_regvalue_node *) node;
	else
		return 0;
}

#endif
