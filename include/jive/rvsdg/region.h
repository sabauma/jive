/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_RVSDG_REGION_H
#define JIVE_RVSDG_REGION_H

#include <stdbool.h>
#include <stddef.h>

#include <jive/common.h>
#include <jive/rvsdg/node.h>
#include <jive/rvsdg/section.h>

namespace jive {

class node;
class simple_node;
class simple_op;
class structural_input;
class structural_node;
class structural_op;
class structural_output;
class substitution_map;

class argument final : public output {
	jive::detail::intrusive_list_anchor<
		jive::argument
	> structural_input_anchor_;

public:
	typedef jive::detail::intrusive_list_accessor<
		jive::argument,
		&jive::argument::structural_input_anchor_
	> structural_input_accessor;

	virtual
	~argument() noexcept;

	argument(
		jive::region * region,
		size_t index,
		jive::structural_input * input,
		const jive::port & port);

	argument(const argument &) = delete;

	argument(argument &&) = delete;

	argument &
	operator=(const argument &) = delete;

	argument &
	operator=(argument &&) = delete;

	virtual jive::node *
	node() const noexcept override;

	inline jive::structural_input *
	input() const noexcept
	{
		return input_;
	}

private:
	jive::structural_input * input_;
};

class result final : public input {
	jive::detail::intrusive_list_anchor<
		jive::result
	> structural_output_anchor_;

public:
	typedef jive::detail::intrusive_list_accessor<
		jive::result,
		&jive::result::structural_output_anchor_
	> structural_output_accessor;

	virtual
	~result() noexcept;

	result(
		jive::region * region,
		size_t index,
		jive::output * origin,
		jive::structural_output * output,
		const jive::port & port);

	result(const result &) = delete;

	result(result &&) = delete;

	result &
	operator=(const result &) = delete;

	result &
	operator=(result &&) = delete;

	virtual jive::node *
	node() const noexcept override;

	inline jive::structural_output *
	output() const noexcept
	{
		return output_;
	}

private:
	jive::structural_output * output_;
};

class region {
	typedef jive::detail::intrusive_list<
		jive::node,
		jive::node::region_node_list_accessor
	> region_nodes_list;

	typedef jive::detail::intrusive_list<
		jive::node,
		jive::node::region_top_node_list_accessor
	> region_top_node_list;

	typedef jive::detail::intrusive_list<
		jive::node,
		jive::node::region_bottom_node_list_accessor
	> region_bottom_node_list;

public:
	~region();

	region(jive::region * parent, jive::graph * graph);

	region(jive::structural_node * node);

	inline region_nodes_list::iterator
	begin()
	{
		return nodes.begin();
	}

	inline region_nodes_list::const_iterator
	begin() const
	{
		return nodes.begin();
	}

	inline region_nodes_list::iterator
	end()
	{
		return nodes.end();
	}

	inline region_nodes_list::const_iterator
	end() const
	{
		return nodes.end();
	}

	inline jive::graph *
	graph() const noexcept
	{
		return graph_;
	}

	inline jive::structural_node *
	node() const noexcept
	{
		return node_;
	}

	jive::argument *
	add_argument(jive::structural_input * input, const jive::port & port);

	void
	remove_argument(size_t index);

	inline size_t
	narguments() const noexcept
	{
		return arguments_.size();
	}

	inline jive::argument *
	argument(size_t index) const noexcept
	{
		JIVE_DEBUG_ASSERT(index < narguments());
		return arguments_[index];
	}

	jive::result *
	add_result(jive::output * origin, structural_output * output, const jive::port & port);

	void
	remove_result(size_t index);

	inline size_t
	nresults() const noexcept
	{
		return results_.size();
	}

	inline jive::result *
	result(size_t index) const noexcept
	{
		JIVE_DEBUG_ASSERT(index < nresults());
		return results_[index];
	}

	inline size_t
	nnodes() const noexcept
	{
		return nodes.size();
	}

	void
	remove_node(jive::node * node);

	/**
		\brief Copy a region with substitutions
		\param target Target region to create nodes in
		\param substitution Operand substitutions
		\param copy_arguments Copy region arguments
		\param copy_results Copy region results

		Copies all nodes of the specified region and its
		subregions into the target region. Substitutions
		will be performed as specified, and the substitution
		map will be updated as nodes are copied.
	*/
	void
	copy(
		region * target,
		substitution_map & smap,
		bool copy_arguments,
		bool copy_results) const;

	void
	prune(bool recursive);

	void
	normalize(bool recursive);

	region_nodes_list nodes;

	region_top_node_list top_nodes;

	region_bottom_node_list bottom_nodes;

private:
	jive::graph * graph_;
	jive::structural_node * node_;
	std::vector<jive::result*> results_;
	std::vector<jive::argument*> arguments_;
};

static inline void
remove(jive::node * node)
{
	return node->region()->remove_node(node);
}

size_t
nnodes(const jive::region * region) noexcept;

size_t
nstructnodes(const jive::region * region) noexcept;

size_t
nsimpnodes(const jive::region * region) noexcept;

size_t
ninputs(const jive::region * region) noexcept;

} //namespace

#endif
