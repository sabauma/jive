/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_BASETYPE_H
#define JIVE_VSDG_BASETYPE_H

#include <memory>
#include <stdbool.h>
#include <stdlib.h>

#include <jive/common.h>
#include <jive/vsdg/gate-interference.h>

class jive_gate;
class jive_input;
class jive_output;

struct jive_buffer;
struct jive_cpureg;
struct jive_graph;
struct jive_node;
struct jive_regcls;
struct jive_region;
struct jive_resource_class;
struct jive_ssavar;
struct jive_variable;

/**
        \defgroup jive_type Types
        Types
        @{
*/

class jive_type {
public:
	virtual ~jive_type() noexcept;

protected:
	inline constexpr jive_type() noexcept {};

public:
	virtual void label(jive_buffer & buffer) const = 0;

	virtual bool operator==(const jive_type & other) const noexcept = 0;

	inline bool operator!=(const jive_type & other) const noexcept { return !(*this == other); }

	virtual jive_type * copy() const = 0;

	/*
		FIXME: change return type to std::unique_ptr<jive_input>
	*/
	virtual jive_input * create_input(jive_node * node, size_t index, jive_output * origin) const = 0;

	/*
		FIXME: change return type to std::unique_ptr<jive_output>
	*/
	virtual jive_output * create_output(jive_node * node, size_t index) const = 0;

	/*
		FIXME: change return type to std::unique_ptr<jive_gate>
	*/
	virtual jive_gate * create_gate(jive_graph * graph, const char * name) const = 0;
};

struct jive_input *
jive_type_create_input(const jive_type * self, struct jive_node * node, size_t index,
	jive_output * initial_operand);

/**
        \defgroup jive_input Inputs
        Inputs
        @{
*/

class jive_input {
public:
	virtual ~jive_input() noexcept;

protected:
	jive_input(struct jive_node * node, size_t index, jive_output * origin);

public:
	virtual const jive_type & type() const noexcept = 0;

	virtual void label(jive_buffer & buffer) const;

	/*
		FIXME: Try to merge internal_divert_origin and divert_origin methods.
	*/
	void internal_divert_origin(jive_output * new_origin) noexcept;

	void divert_origin(jive_output * new_origin) noexcept;
	
	/*
		FIXME: This function is only used two times in src/regalloc/fixup.c. See whether we can
		actually remove it and add a replacement in the register allocator.
	*/
	void swap(jive_input * other) noexcept;

	inline jive_output * origin() const noexcept { return origin_; }

	inline jive_node * producer() const noexcept;

	struct jive_node * node;
	size_t index;

	struct {
		jive_input * prev;
		jive_input * next;
	} output_users_list;
	
	jive_gate * gate;
	struct {
		jive_input * prev;
		jive_input * next;
	} gate_inputs_list;
	
	struct jive_ssavar * ssavar;
	struct {
		jive_input * prev;
		jive_input * next;
	} ssavar_input_list;

	struct {
		struct jive_region_hull_entry * first;
		struct jive_region_hull_entry * last;
	} hull;
	
	const struct jive_resource_class * required_rescls;

private:
	jive_output * origin_;
};

JIVE_EXPORTED_INLINE void
jive_input_get_label(const jive_input * self, struct jive_buffer * buffer)
{
	self->label(*buffer);
}

JIVE_EXPORTED_INLINE const jive_type *
jive_input_get_type(const jive_input * self)
{
	return &self->type();
}

struct jive_variable *
jive_input_get_constraint(const jive_input * self);

void
jive_input_unassign_ssavar(jive_input * self);

struct jive_ssavar *
jive_input_auto_assign_variable(jive_input * self);

struct jive_ssavar *
jive_input_auto_merge_variable(jive_input * self);

void
jive_input_destroy(jive_input * self);

/**	@}	*/

/**
        \defgroup jive_output Outputs
        Outputs
        @{
*/

class jive_output {
public:
	virtual ~jive_output() noexcept;

protected:
	jive_output(struct jive_node * node, size_t index);

public:
	virtual const jive_type & type() const noexcept = 0;

	virtual void label(jive_buffer & buffer) const;

	inline jive_node * node() const noexcept { return node_; }

	size_t index;
	
	struct {
		jive_input * first;
		jive_input * last;
	} users;
	
	jive_gate * gate;
	struct {
		jive_output * prev;
		jive_output * next;
	} gate_outputs_list;
	
	struct jive_ssavar * ssavar;
	
	struct {
		struct jive_ssavar * first;
		struct jive_ssavar * last;
	} originating_ssavars;
	
	const struct jive_resource_class * required_rescls;
private:
	jive_node * node_;
};

JIVE_EXPORTED_INLINE bool
jive_output_has_no_user(const struct jive_output * self)
{
	return self->users.first == NULL;
}

JIVE_EXPORTED_INLINE bool
jive_output_has_single_user(const struct jive_output * self)
{
	return (self->users.first != NULL) && (self->users.first == self->users.last);
}

JIVE_EXPORTED_INLINE void
jive_output_get_label(const jive_output * self, struct jive_buffer * buffer)
{
	self->label(*buffer);
}

JIVE_EXPORTED_INLINE const jive_type *
jive_output_get_type(const jive_output * self)
{
	return &self->type();
}

struct jive_variable *
jive_output_get_constraint(const jive_output * self);

void
jive_output_replace(jive_output * self, jive_output * other);

struct jive_ssavar *
jive_output_auto_assign_variable(jive_output * self);

struct jive_ssavar *
jive_output_auto_merge_variable(jive_output * self);

void
jive_output_destroy(jive_output * self);

JIVE_EXPORTED_INLINE struct jive_node *
jive_output_node(const jive_output * output)
{
	return output->node();
}

/**	@}	*/

/**
        \defgroup jive_gate Gates
        Gates
        @{
*/

class jive_gate {
public:
	virtual ~jive_gate() noexcept;

protected:
	jive_gate(struct jive_graph * graph, const char name[]);

public:
	virtual const jive_type & type() const noexcept = 0;

	virtual void label(jive_buffer & buffer) const;
	
	struct jive_graph * graph;
	struct {
		jive_gate * prev;
		jive_gate * next;
	} graph_gate_list;
	
	char * name;
	
	struct {
		jive_input * first;
		jive_input * last;
	} inputs;
	
	struct {
		jive_output * first;
		jive_output * last;
	} outputs;
	
	bool may_spill;
	jive_gate_interference_hash interference;
	
	struct jive_variable * variable;
	struct {
		jive_gate * prev;
		jive_gate * next;
	} variable_gate_list;
	
	const struct jive_resource_class * required_rescls;
};

JIVE_EXPORTED_INLINE void
jive_gate_get_label(const jive_gate * self, struct jive_buffer * buffer)
{
	self->label(*buffer);
}

JIVE_EXPORTED_INLINE const jive_type *
jive_gate_get_type(const jive_gate * self)
{
	return &self->type();
}

struct jive_variable *
jive_gate_get_constraint(jive_gate * self);

JIVE_EXPORTED_INLINE jive_input *
jive_gate_create_input(const jive_gate * self, struct jive_node * node, size_t index,
	jive_output * initial_operand)
{
	jive_input * input = jive_type_create_input(jive_gate_get_type(self), node, index,
		initial_operand);
	input->required_rescls = self->required_rescls;
	return input;
}

JIVE_EXPORTED_INLINE jive_output *
jive_gate_create_output(const jive_gate * self, struct jive_node * node, size_t index)
{
	jive_output * output = jive_gate_get_type(self)->create_output(node, index);
	output->required_rescls = self->required_rescls;
	return output;
}

size_t
jive_gate_interferes_with(const jive_gate * self, const jive_gate * other);

void
jive_gate_merge(jive_gate * self, jive_gate * other);

void
jive_gate_split(jive_gate * self);

void
jive_gate_auto_merge_variable(jive_gate * self);

void
jive_gate_destroy(jive_gate * self);

/**	@}	*/

/**	@}	*/

void
jive_raise_type_error(const jive_type * self, const jive_type * other,
	struct jive_context * context);

inline jive_node *
jive_input::producer() const noexcept
{
	return origin_->node();
}

#endif
