/*
 * Copyright 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/anchor-private.h>
#include <jive/vsdg/anchor.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>

namespace jive {

bool
region_head_op::operator==(const operation & other) const noexcept
{
	return typeid(*this) == typeid(other);
}

size_t
region_head_op::narguments() const noexcept
{
	return 0;
}

const jive::base::type &
region_head_op::argument_type(size_t index) const noexcept
{
	throw std::logic_error("region_head_op has no arguments");
}

size_t
region_head_op::nresults() const noexcept
{
	return 0;
}

const jive::base::type &
region_head_op::result_type(size_t index) const noexcept
{
	throw std::logic_error("region_head_op has no results");
}


bool
region_tail_op::operator==(const operation & other) const noexcept
{
	return typeid(*this) == typeid(other);
}

size_t
region_tail_op::narguments() const noexcept
{
	return 0;
}

const jive::base::type &
region_tail_op::argument_type(size_t index) const noexcept
{
	throw std::logic_error("region_tail_op has no arguments");
}

size_t
region_tail_op::nresults() const noexcept
{
	return 1;
}

const jive::base::type &
region_tail_op::result_type(size_t index) const noexcept
{
	static const achr::type type;
	return type;
}


bool
region_anchor_op::operator==(const operation & other) const noexcept
{
	return typeid(*this) == typeid(other);
}

size_t
region_anchor_op::narguments() const noexcept
{
	return 1;
}

const jive::base::type &
region_anchor_op::argument_type(size_t index) const noexcept
{
	static const achr::type type;
	return type;
}

size_t
region_anchor_op::nresults() const noexcept
{
	return 0;
}

const jive::base::type &
region_anchor_op::result_type(size_t index) const noexcept
{
	throw std::logic_error("region_anchor_op has no results");
}

}

/* node class */

const jive_node_class JIVE_ANCHOR_NODE = {
	parent : &JIVE_NODE,
	name : "ANCHOR",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_anchor_node_get_default_normal_form_, /* override */
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr,
};

/* node class inhertiable methods */

jive_node_normal_form *
jive_anchor_node_get_default_normal_form_(const jive_node_class * cls,
	jive_node_normal_form * parent_, struct jive_graph * graph)
{
	jive_anchor_node_normal_form * nf = new jive_anchor_node_normal_form;
	nf->base.class_ = &JIVE_ANCHOR_NODE_NORMAL_FORM;

	jive_anchor_node_normal_form_init_(nf, cls, parent_, graph);

	return &nf->base;
}

/* normal form class */

const jive_anchor_node_normal_form_class JIVE_ANCHOR_NODE_NORMAL_FORM_ = {
	base : { /* jive_node_normal_form_class */
		parent : &JIVE_NODE_NORMAL_FORM,
		fini : jive_node_normal_form_fini_, /* inherit */
		normalize_node : jive_node_normal_form_normalize_node_, /* inherit */
		operands_are_normalized : jive_node_normal_form_operands_are_normalized_, /* inherit */
		normalized_create : NULL,
		set_mutable : jive_node_normal_form_set_mutable_, /* inherit */
		set_cse : jive_node_normal_form_set_cse_, /* inherit */
	},
	set_reducible : jive_anchor_node_normal_form_set_reducible_
};

void
jive_anchor_node_normal_form_init_(jive_anchor_node_normal_form * self,
	const jive_node_class * cls, jive_node_normal_form * parent_, struct jive_graph * graph)
{
	jive_node_normal_form_init_(&self->base, cls, parent_, graph);
	
	jive_anchor_node_normal_form * parent = jive_anchor_node_normal_form_cast(parent_);

	if (parent)
		self->enable_reducible = parent->enable_reducible;
	else
		self->enable_reducible = true;
}

void
jive_anchor_node_normal_form_set_reducible_(jive_anchor_node_normal_form * self, bool enable)
{
	if (self->enable_reducible == enable)
		return;

	jive_node_normal_form * child;
	JIVE_LIST_ITERATE(self->base.subclasses, child, normal_form_subclass_list)
		jive_anchor_node_normal_form_set_reducible_((jive_anchor_node_normal_form *)child, enable);
	
	self->enable_reducible = enable;
	if (self->base.enable_mutable && self->enable_reducible)
		jive_graph_mark_denormalized(self->base.graph);
}
