/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/region.h>

#include <jive/common.h>

#include <jive/util/list.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/graph-private.h>
#include <jive/vsdg/substitution.h>
#include <jive/vsdg/traverser.h>
#include <jive/vsdg/variable.h>

static inline void
jive_region_attrs_init(jive_region_attrs * attrs)
{
	attrs->align = 1;
	attrs->section = jive_region_section_inherit;
	attrs->is_looped = false;
}

jive_region::~jive_region()
{
	JIVE_DEBUG_ASSERT(nodes.first == nullptr && nodes.last == nullptr);
	JIVE_DEBUG_ASSERT(subregions.first == nullptr && subregions.last == nullptr);

	graph->on_region_destroy(this);

	if (parent)
		JIVE_LIST_REMOVE(parent->subregions, this, region_subregions_list);
	/* FIXME: destroy stackframe! */
	/* if (self->stackframe)
		jive_stackframe_destroy(self->stackframe); */
}

jive_region::jive_region(jive_region * _parent, jive_graph * _graph)
	: graph(_graph)
	, parent(_parent)
	, stackframe(nullptr)
	, top(nullptr)
	, bottom(nullptr)
	, anchor(nullptr)
	, depth_(0)
{
	nodes.first = nodes.last = nullptr;
	top_nodes.first = top_nodes.last = nullptr;
	subregions.first = subregions.last = nullptr;
	region_subregions_list.prev = region_subregions_list.next = nullptr;
	jive_region_attrs_init(&attrs);

	if (parent) {
		JIVE_LIST_PUSH_BACK(parent->subregions, this, region_subregions_list);
		depth_ = parent->depth() + 1;
	}

	graph->on_region_create(this);
}

struct jive_node *
jive_region_get_anchor(struct jive_region * self)
{
	if (self->anchor)
		return self->anchor->node();

	return nullptr;
}

void
jive_region::reparent(jive_region * new_parent) noexcept
{
	std::function<void (jive_region *, size_t)> set_depth_recursive = [&](
		jive_region * region,
		size_t new_depth)
	{
		region->depth_ = new_depth;
		jive_region * subregion;
		JIVE_LIST_ITERATE(region->subregions, subregion, region_subregions_list)
			set_depth_recursive(subregion, new_depth + 1);
	};

	JIVE_LIST_REMOVE(parent->subregions, this, region_subregions_list);

	parent = new_parent;
	size_t new_depth = new_parent->depth() + 1;
	if (new_depth != depth())
		set_depth_recursive(this, new_depth);

	JIVE_LIST_PUSH_BACK(parent->subregions, this, region_subregions_list);
}

bool
jive_region::contains(const jive_node * node) const noexcept
{
	const jive_region * tmp = node->region();
	while (tmp->depth() >= depth()) {
		if (tmp == this)
			return true;
		tmp = tmp->parent;
		if (!tmp)
			break;
	}
	return false;
}

void
jive_region_prune_subregions_(jive_region * self)
{
	jive_region * subregion;
	subregion = self->subregions.first;
	while(subregion) {
		jive_region * next = subregion->region_subregions_list.next;
		if (subregion->is_empty()) {
			JIVE_LIST_REMOVE(self->subregions, subregion, region_subregions_list);
			delete subregion;
		}
		subregion = next;
	}
}

void
jive_region_add_used_ssavar(jive_region * self, jive_ssavar * ssavar)
{
	if (ssavar->origin->node()->region()->depth() >= self->depth()) return;
	if (self->attrs.is_looped) {
		jive_region_ssavar_use * use = self->used_ssavars.find(ssavar).ptr();
		if (use)
			use->count ++;
		else {
			use = new jive_region_ssavar_use;
			use->region = self;
			use->ssavar = ssavar;
			use->count = 1;
			self->used_ssavars.insert(use);
			ssavar->assigned_regions.insert(use);
			
			self->graph->on_region_add_used_ssavar(self, ssavar);
		}
	}
	
	jive_region_add_used_ssavar(self->parent, ssavar);
}

void
jive_region_remove_used_ssavar(jive_region * self, jive_ssavar * ssavar)
{
	if (ssavar->origin->node()->region()->depth() >= self->depth()) return;
	if (self->attrs.is_looped) {
		jive_region_ssavar_use * use = self->used_ssavars.find(ssavar).ptr();
		use->count --;
		if (use->count == 0) {
			self->used_ssavars.erase(use);
			ssavar->assigned_regions.erase(use);

			self->graph->on_region_remove_used_ssavar(self, ssavar);
			delete use;
		}
	}
	
	jive_region_remove_used_ssavar(self->parent, ssavar);
}

typedef struct jive_copy_context jive_copy_context;
struct jive_copy_context {
	std::vector<std::vector<jive_node*>> depths;
};

static void
jive_copy_context_append(jive_copy_context * self, jive_node * node)
{
	if (node->depth_from_root >= self->depths.size())
		self->depths.resize(node->depth_from_root+1);

	self->depths[node->depth_from_root].push_back(node);
}

static void
pre_copy_region(
	jive_region * target_region,
	const jive_region * original_region,
	jive_copy_context * copy_context,
	jive::substitution_map & substitution,
	bool copy_top, bool copy_bottom)
{
	jive_node * node;
	JIVE_LIST_ITERATE(original_region->nodes, node, region_nodes_list) {
		if (!copy_top && node == original_region->top) continue;
		if (!copy_bottom && node == original_region->bottom) continue;
		jive_copy_context_append(copy_context, node);
	}
	
	jive_region * subregion;
	JIVE_LIST_ITERATE(original_region->subregions, subregion, region_subregions_list) {
		jive_region * target_subregion = new jive_region(target_region, target_region->graph);
		target_subregion->attrs = subregion->attrs;
		substitution.insert(subregion, target_subregion);
		pre_copy_region(target_subregion, subregion, copy_context, substitution, true, true);
	}
}

void
jive_region_copy_substitute(
	const jive_region * self,
	jive_region * target,
	jive::substitution_map & substitution,
	bool copy_top, bool copy_bottom)
{
	jive_copy_context copy_context;

	substitution.insert(self, target);
	pre_copy_region(target, self, &copy_context, substitution, copy_top, copy_bottom);
	
	for (size_t depth = 0; depth < copy_context.depths.size(); depth ++) {
		for (size_t n = 0; n < copy_context.depths[depth].size(); n++) {
			jive_node * node = copy_context.depths[depth][n];
			jive_region * target_subregion = substitution.lookup(node->region());
			jive_node * new_node = node->copy(target_subregion, substitution);
			if (node->region()->top == node)
				target_subregion->top = new_node;
			if (node->region()->bottom == node)
				target_subregion->bottom = new_node;
		}
	}
}

#ifdef JIVE_DEBUG
void
jive_region_verify_top_node_list(struct jive_region * region)
{
	jive_region * subregion;
	JIVE_LIST_ITERATE(region->subregions, subregion, region_subregions_list)
		jive_region_verify_top_node_list(subregion);

	/* check whether all nodes in the top_node_list are really nullary nodes */
	jive_node * node;
	JIVE_LIST_ITERATE(region->top_nodes, node, region_top_node_list)
		JIVE_DEBUG_ASSERT(node->ninputs == 0);

	/* check whether all nullary nodes from the region are in the top_node_list */
	JIVE_LIST_ITERATE(region->nodes, node, region_nodes_list) {
		if (node->ninputs != 0)
			continue;

		jive_node * top;
		JIVE_LIST_ITERATE(region->top_nodes, top, region_top_node_list) {
			if (top == node)
				break;
		}
		if (top == NULL)
			JIVE_DEBUG_ASSERT(0);
	}
}
#endif
