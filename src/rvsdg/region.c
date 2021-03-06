/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/common.h>

#include <jive/rvsdg/graph.h>
#include <jive/rvsdg/notifiers.h>
#include <jive/rvsdg/simple-node.h>
#include <jive/rvsdg/region.h>
#include <jive/rvsdg/structural-node.h>
#include <jive/rvsdg/substitution.h>
#include <jive/rvsdg/traverser.h>

namespace jive {

/* argument */

argument::~argument() noexcept
{
	on_output_destroy(this);

	if (input())
		input()->arguments.erase(this);
}

argument::argument(
	jive::region * region,
	size_t index,
	jive::structural_input * input,
	const jive::port & port)
	: output(index, region, port)
	, input_(input)
{
	if (input)
		input->arguments.push_back(this);
}

jive::node *
argument::node() const noexcept
{
	return nullptr;
}

/* result */

result::~result() noexcept
{
	on_input_destroy(this);

	if (output())
		output()->results.erase(this);
}

result::result(
	jive::region * region,
	size_t index,
	jive::output * origin,
	jive::structural_output * output,
	const jive::port & port)
	: input(index, origin, region, port)
	, output_(output)
{
	if (output)
		output->results.push_back(this);
}

jive::node *
result::node() const noexcept
{
	return nullptr;
}

/* region */

region::~region()
{
	on_region_destroy(this);

	while (results_.size())
		remove_result(results_.size()-1);

	prune(false);
	JIVE_DEBUG_ASSERT(nodes.empty());
	JIVE_DEBUG_ASSERT(top_nodes.empty());
	JIVE_DEBUG_ASSERT(bottom_nodes.empty());

	while (arguments_.size())
		remove_argument(arguments_.size()-1);
}

region::region(jive::region * parent, jive::graph * graph)
	: graph_(graph)
	, node_(nullptr)
{
	on_region_create(this);
}

region::region(jive::structural_node * node)
	: graph_(node->graph())
	, node_(node)
{
	on_region_create(this);
}

jive::argument *
region::add_argument(jive::structural_input * input, const jive::port & port)
{
	jive::argument * argument = new jive::argument(this, narguments(), input, port);
	arguments_.push_back(argument);

	on_output_create(argument);

	return argument;
}

void
region::remove_argument(size_t index)
{
	JIVE_DEBUG_ASSERT(index < narguments());
	jive::argument * argument = arguments_[index];

	delete argument;
	for (size_t n = index; n < arguments_.size()-1; n++) {
		arguments_[n] = arguments_[n+1];
		arguments_[n]->index_ = n;
	}
	arguments_.pop_back();
}

jive::result *
region::add_result(jive::output * origin, structural_output * output, const jive::port & port)
{
	jive::result * result = new jive::result(this, nresults(), origin, output, port);
	results_.push_back(result);

	if (origin->region() != this)
		throw jive::compiler_error("Invalid region result");

	on_input_create(result);

	return result;
}

void
region::remove_result(size_t index)
{
	JIVE_DEBUG_ASSERT(index < results_.size());
	jive::result * result = results_[index];

	delete result;
	for (size_t n = index; n < results_.size()-1; n++) {
		results_[n] = results_[n+1];
		results_[n]->index_ = n;
	}
	results_.pop_back();
}

void
region::remove_node(jive::node * node)
{
	delete node;
}

void
region::copy(
	region * target,
	substitution_map & smap,
	bool copy_arguments,
	bool copy_results) const
{
	smap.insert(this, target);

	/* order nodes top-down */
	std::vector<std::vector<const jive::node*>> context(nnodes());
	for (const auto & node : nodes) {
		JIVE_DEBUG_ASSERT(node.depth() < context.size());
		context[node.depth()].push_back(&node);
	}

	/* copy arguments */
	if (copy_arguments) {
		for (size_t n = 0; n < narguments(); n++) {
			auto narg = target->add_argument(smap.lookup(argument(n)->input()), argument(n)->type());
			smap.insert(argument(n), narg);
		}
	}

	/* copy nodes */
	for (size_t n = 0; n < context.size(); n++) {
		for (const auto node : context[n]) {
			target = smap.lookup(node->region());
			node->copy(target, smap);
		}
	}

	/* copy results */
	if (copy_results) {
		for (size_t n = 0; n < nresults(); n++) {
			auto origin = smap.lookup(result(n)->origin());
			if (!origin) origin = result(n)->origin();

			auto output = dynamic_cast<jive::structural_output*>(smap.lookup(result(n)->output()));
			target->add_result(origin, output, result(n)->type());
		}
	}
}

void
region::prune(bool recursive)
{
	while (bottom_nodes.first())
		remove_node(bottom_nodes.first());

	if (!recursive)
		return;

	for (const auto & node : nodes) {
		if (auto snode = dynamic_cast<const jive::structural_node*>(&node)) {
			for (size_t n = 0; n < snode->nsubregions(); n++)
				snode->subregion(n)->prune(recursive);
		}
	}
}

void
region::normalize(bool recursive)
{
	for (auto node : jive::topdown_traverser(this)) {
		if (auto structnode = dynamic_cast<const jive::structural_node*>(node)) {
			for (size_t n = 0; n < structnode->nsubregions(); n++)
				structnode->subregion(n)->normalize(recursive);
		}

		graph()->node_normal_form(typeid(node->operation()))->normalize_node(node);
	}
}

size_t
nnodes(const jive::region * region) noexcept
{
	size_t n = region->nnodes();
	for (const auto & node : region->nodes) {
		if (auto snode = dynamic_cast<const jive::structural_node*>(&node)) {
			for (size_t r = 0; r < snode->nsubregions(); r++)
				n += nnodes(snode->subregion(r));
		}
	}

	return n;
}

size_t
nstructnodes(const jive::region * region) noexcept
{
	size_t n = 0;
	for (const auto & node : region->nodes) {
		if (auto snode = dynamic_cast<const jive::structural_node*>(&node)) {
			for (size_t r = 0; r < snode->nsubregions(); r++)
				n += nstructnodes(snode->subregion(r));
			n += 1;
		}
	}

	return n;
}

size_t
nsimpnodes(const jive::region * region) noexcept
{
	size_t n = 0;
	for (const auto & node : region->nodes) {
		if (auto snode = dynamic_cast<const jive::structural_node*>(&node)) {
			for (size_t r = 0; r < snode->nsubregions(); r++)
				n += nsimpnodes(snode->subregion(r));
		} else {
			n += 1;
		}
	}

	return n;
}

size_t
ninputs(const jive::region * region) noexcept
{
	size_t n = region->nresults();
	for (const auto & node : region->nodes) {
		if (auto snode = dynamic_cast<const jive::structural_node*>(&node)) {
			for (size_t r = 0; r < snode->nsubregions(); r++)
				n += ninputs(snode->subregion(r));
		}
		n += node.ninputs();
	}

	return n;
}

}	//namespace
