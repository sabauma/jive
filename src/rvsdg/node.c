/*
 * Copyright 2010 2011 2012 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <string.h>

#include <jive/common.h>

#include <jive/rvsdg/control.h>
#include <jive/rvsdg/node-normal-form.h>
#include <jive/rvsdg/notifiers.h>
#include <jive/rvsdg/region.h>
#include <jive/rvsdg/resource.h>
#include <jive/rvsdg/simple-node.h>
#include <jive/rvsdg/substitution.h>
#include <jive/rvsdg/theta.h>

namespace jive {

/* input */

input::~input() noexcept
{
	origin()->remove_user(this);
}

input::input(
	size_t index,
	jive::output * origin,
	jive::region * region,
	const jive::port & port)
: index_(index)
, origin_(origin)
, region_(region)
, port_(port.copy())
{
	if (region != origin->region())
		throw jive::compiler_error("Invalid operand region.");

	if (port.type() != origin->type())
		throw jive::type_error(port.type().debug_string(), origin->type().debug_string());

	origin->add_user(this);
}

std::string
input::debug_string() const
{
	return detail::strfmt(index());
}

void
input::divert_to(jive::output * new_origin)
{
	if (origin() == new_origin)
		return;

	if (type() != new_origin->type())
		throw jive::type_error(type().debug_string(), new_origin->type().debug_string());

	if (region() != new_origin->region())
		throw jive::compiler_error("Invalid operand region.");

	auto old_origin = origin();
	old_origin->remove_user(this);
	this->origin_ = new_origin;
	new_origin->add_user(this);

	if (node()) node()->recompute_depth();
	region()->graph()->mark_denormalized();
	on_input_change(this, old_origin, new_origin);
}

/* output */

output::~output() noexcept
{
	JIVE_DEBUG_ASSERT(nusers() == 0);
}

output::output(
	size_t index,
	jive::region * region,
	const jive::port & port)
: index_(index)
, region_(region)
, port_(port.copy())
{}

std::string
output::debug_string() const
{
	return detail::strfmt(index());
}

void
output::remove_user(jive::input * user)
{
	JIVE_DEBUG_ASSERT(users_.find(user) != users_.end());

	users_.erase(user);
	if (node() && !node()->has_users())
		region()->bottom_nodes.push_back(node());
}

void
output::add_user(jive::input * user)
{
	JIVE_DEBUG_ASSERT(users_.find(user) == users_.end());

	if (node() && !node()->has_users())
		region()->bottom_nodes.erase(node());
	users_.insert(user);
}

}	//jive namespace

jive::node_normal_form *
jive_node_get_default_normal_form_(
	const std::type_info & operator_class,
	jive::node_normal_form * parent,
	jive::graph * graph)
{
	jive::node_normal_form * normal_form = new jive::node_normal_form(
		operator_class, parent, graph);
	return normal_form;
}

static void  __attribute__((constructor))
register_node_normal_form(void)
{
	jive::node_normal_form::register_factory(
		typeid(jive::operation), jive_node_get_default_normal_form_);
}

namespace jive {

node::node(std::unique_ptr<jive::operation> op, jive::region * region)
	: depth_(0)
	, graph_(region->graph())
	, region_(region)
	, operation_(std::move(op))
{
	region->bottom_nodes.push_back(this);
	region->top_nodes.push_back(this);
	region->nodes.push_back(this);
}

node::~node()
{
	outputs_.clear();
	region()->bottom_nodes.erase(this);

	if (ninputs() == 0)
		region()->top_nodes.erase(this);
	inputs_.clear();

	region()->nodes.erase(this);
}

void
node::add_input(std::unique_ptr<jive::input> input)
{
	if (ninputs() == 0) {
		JIVE_DEBUG_ASSERT(depth() == 0);
		region()->top_nodes.erase(this);
	}

	inputs_.push_back(std::move(input));

	auto producer = inputs_.back().get()->origin()->node();
	auto new_depth = producer ? producer->depth()+1 : 0;
	if (new_depth > depth())
		recompute_depth();
}

void
node::remove_input(size_t index)
{
	JIVE_DEBUG_ASSERT(index < ninputs());
	auto producer = input(index)->origin()->node();

	/* remove input */
	for (size_t n = index; n < ninputs()-1; n++) {
		inputs_[n] = std::move(inputs_[n+1]);
		inputs_[n]->index_ = n;
	}
	inputs_.pop_back();

	/* recompute depth */
	if (producer) {
		auto pdepth = producer->depth();
		JIVE_DEBUG_ASSERT(pdepth < depth());
		if (pdepth != depth()-1)
			return;
	}
	recompute_depth();

	/* add to region's top nodes */
	if (ninputs() == 0) {
		JIVE_DEBUG_ASSERT(depth() == 0);
		region()->top_nodes.push_back(this);
	}
}

void
node::remove_output(size_t index)
{
	JIVE_DEBUG_ASSERT(index < noutputs());

	for (size_t n = index; n < noutputs()-1; n++) {
		outputs_[n] = std::move(outputs_[n+1]);
		outputs_[n]->index_ = n;
	}
	outputs_.pop_back();
}

void
node::recompute_depth() noexcept
{
	/*
		FIXME: This function is inefficient, as it can visit the
		node's successors multiple times. Optimally, we would like
		to visit the node's successors in top down order to ensure
		that each node is only visited once.
	*/
	size_t new_depth = 0;
	for (size_t n = 0; n < ninputs(); n++) {
		auto producer = input(n)->origin()->node();
		new_depth = std::max(new_depth, producer ? producer->depth()+1 : 0);
	}
	if (new_depth == depth())
		return;

	size_t old_depth = depth();
	depth_ = new_depth;
	on_node_depth_change(this, old_depth);

	for (size_t n = 0; n < noutputs(); n++) {
		for (auto user : *(output(n))) {
			if (user->node())
				user->node()->recompute_depth();
		}
	}
}

jive::node *
producer(const jive::output * output) noexcept
{
	if (auto node = output->node())
		return node;

	JIVE_DEBUG_ASSERT(dynamic_cast<const jive::argument*>(output));
	auto argument = static_cast<const jive::argument*>(output);

	if (!argument->input())
		return nullptr;

	if (is<theta_op>(argument->region()->node())
	&& (argument->region()->result(argument->index()+1)->origin() != argument))
		return nullptr;

	return producer(argument->input()->origin());
}

bool
normalize(jive::node * node)
{
	auto nf = node->graph()->node_normal_form(typeid(node->operation()));
	return nf->normalize_node(node);
}

}
