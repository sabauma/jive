/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/record.h>

#include <stdio.h>
#include <string.h>

#include <jive/arch/address-transform.h>
#include <jive/arch/address.h>
#include <jive/arch/load.h>
#include <jive/types/bitstring/type.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>

static constexpr jive_unop_reduction_path_t jive_select_reduction_load = 128;

namespace jive {
namespace rcd {

select_operation::~select_operation() noexcept
{
}

bool
select_operation::operator==(const operation & other) const noexcept
{
	const select_operation * op =
		dynamic_cast<const select_operation *>(&other);
	return op && type_ == op->type_ && element_ == op->element_;
}

jive_node *
select_operation::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	jive_select_node * node = new jive_select_node(*this);
	node->class_ = &JIVE_SELECT_NODE;

	const jive::base::type * argtypes[1] = { &argument_type(0) };
	const jive::base::type * restypes[1] = { &result_type(0) };

	jive_node_init_(node, region,
		1, argtypes, arguments,
		1, restypes);

	return node;
}

std::string
select_operation::debug_string() const
{
	char tmp[32];
	snprintf(tmp, sizeof(tmp), "SELECT(%zd)", element());
	return tmp;
}

const jive::base::type &
select_operation::argument_type(size_t index) const noexcept
{
	return type_;
}

const jive::base::type &
select_operation::result_type(size_t index) const noexcept
{
	return *type_.declaration()->elements[element()];
}

jive_unop_reduction_path_t
select_operation::can_reduce_operand(
	const jive::output * arg) const noexcept
{
	if (dynamic_cast<const group_op *>(&arg->node()->operation())) {
		return jive_unop_reduction_inverse;
	}

	if (dynamic_cast<const load_op *>(&arg->node()->operation())) {
		return jive_select_reduction_load;
	}

	return jive_unop_reduction_none;
}

jive::output *
select_operation::reduce_operand(
	jive_unop_reduction_path_t path,
	jive::output * arg) const
{
	if (path == jive_unop_reduction_inverse) {
		return arg->node()->inputs[element()]->origin();
	}

	if (path == jive_select_reduction_load) {
		const load_op & op =
			static_cast<const load_op &>(arg->node()->operation());
		jive::output * address = arg->node()->inputs[0]->origin();

		size_t nbits = 0;
		if (dynamic_cast<jive::bits::output*>(address)) {
			nbits = static_cast<const jive::bits::output*>(address)->nbits();
			address = jive_bitstring_to_address_create(address, nbits, &address->type());
		}
		
		const jive::rcd::declaration * decl = static_cast<const jive::rcd::output*>(
			arg->node()->outputs[0])->declaration();

		size_t nstates = arg->node()->ninputs-1;
		jive::output * states[nstates];
		for (size_t n = 0; n < nstates; n++) {
			states[n] = arg->node()->inputs[n+1]->origin();
		}

		jive::output * element_address = jive_memberof(address, decl, element());
		if (dynamic_cast<jive::addr::output*>(address)) {
			return jive_load_by_address_create(element_address, decl->elements[element()],
				nstates, states);
		} else {
			return jive_load_by_bitstring_create(element_address, nbits, decl->elements[element()],
				nstates, states);
		}
	}

	return nullptr;
}

std::unique_ptr<jive::operation>
select_operation::copy() const
{
	return std::unique_ptr<jive::operation>(new select_operation(*this));
}

}
}

const jive_node_class JIVE_SELECT_NODE = {
	parent : &JIVE_UNARY_OPERATION,
	name : "SELECT",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : nullptr,
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};


jive::output *
jive_select_create(size_t member, jive::output * argument)
{
	const jive::rcd::type & rcd_type =
		dynamic_cast<const jive::rcd::type &>(argument->type());
	jive::rcd::select_operation op(rcd_type, member);

	return jive_unary_operation_create_normalized(
		&JIVE_SELECT_NODE, argument->node()->graph, &op, argument);
}

