/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_NEGOTIATOR_H
#define JIVE_VSDG_NEGOTIATOR_H

#include <stdbool.h>

#include <jive/util/hash.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/operators/unary.h>

namespace jive {
	class gate;
	class output;
}

struct jive_context;
struct jive_graph;
struct jive_node;
struct jive_region;

struct jive_negotiator;
struct jive_negotiator_connection;
struct jive_negotiator_constraint;
struct jive_negotiator_constraint_class;

/* options */

class jive_negotiator_option {
public:
	virtual
	~jive_negotiator_option() noexcept;

	inline constexpr jive_negotiator_option() {}

	/* test two options for equality */
	virtual bool
	operator==(const jive_negotiator_option & other) const noexcept = 0;

	inline bool
	operator!=(const jive_negotiator_option & other) noexcept
	{
		return !(*this == other);
	}

	/* specialize option, return true if changed */
	virtual bool
	specialize() noexcept = 0;

	/* try to compute intersection; return true if changed, return false if
	 * it would be empty (and is therefore unchanged) */
	virtual bool
	intersect(const jive_negotiator_option & other) noexcept = 0;

	/* assign new value to option, return true if changed */
	virtual bool
	assign(const jive_negotiator_option & other) noexcept = 0;

	/* make copy of current option */
	virtual jive_negotiator_option *
	copy() const = 0;
};

/* split node */

extern const jive_node_class JIVE_NEGOTIATOR_SPLIT_NODE;

namespace jive {

class negotiator_split_operation final : public unary_operation {
public:
	inline
	negotiator_split_operation(
		jive_negotiator * negotiator,
		const jive::base::type & input_type,
		const jive_negotiator_option & input_option,
		const jive::base::type & output_type,
		const jive_negotiator_option & output_option)
		: negotiator_(negotiator)
		, input_type_(input_type.copy())
		, input_option_(input_option.copy())
		, output_type_(output_type.copy())
		, output_option_(output_option.copy())
	{
	}

	inline
	negotiator_split_operation(
		const negotiator_split_operation & other)
		: negotiator_(other.negotiator())
		, input_type_(other.input_type().copy())
		, input_option_(other.input_option().copy())
		, output_type_(other.output_type().copy())
		, output_option_(other.output_option().copy())
	{
	}

	virtual bool
	operator==(const operation& other) const noexcept override;

	virtual const base::type &
	argument_type(size_t index) const noexcept override;

	virtual const base::type &
	result_type(size_t index) const noexcept override;

	virtual jive_node *
	create_node(
		jive_region * region,
		size_t narguments,
		jive::output * const arguments[]) const override;

	virtual std::string
	debug_string() const override;

	inline jive_negotiator *
	negotiator() const noexcept { return negotiator_; }

	inline const jive::base::type &
	input_type() const noexcept { return *input_type_; }

	inline const jive_negotiator_option &
	input_option() const noexcept { return *input_option_; }

	inline const jive::base::type &
	output_type() const noexcept { return *output_type_; }

	inline const jive_negotiator_option &
	output_option() const noexcept { return *output_option_; }

private:
	jive_negotiator * negotiator_;
	std::unique_ptr<jive::base::type> input_type_;
	std::unique_ptr<jive_negotiator_option> input_option_;
	std::unique_ptr<jive::base::type> output_type_;
	std::unique_ptr<jive_negotiator_option> output_option_;
};

class negotiator_split_node : public jive_node {
public:
	virtual
	~negotiator_split_node() noexcept;

	inline negotiator_split_node(
		const negotiator_split_operation & op)
		: op_(op)
	{
	}

	virtual const negotiator_split_operation &
	operation() const noexcept override;

	void detach() noexcept;

	struct {
		negotiator_split_node * prev;
		negotiator_split_node * next;
	} split_node_list;

	jive_negotiator * negotiator;

private:
	negotiator_split_operation op_;
};

}

/* ports */

typedef enum {
	jive_negotiator_port_attach_none = 0,
	jive_negotiator_port_attach_input = 1,
	jive_negotiator_port_attach_output = 2
} jive_negotiator_port_attach;

struct jive_negotiator_port {
	jive_negotiator_constraint * constraint;
	struct {
		jive_negotiator_port * prev;
		jive_negotiator_port * next;
	} constraint_port_list;
	
	jive_negotiator_connection * connection;
	struct {
		jive_negotiator_port * prev;
		jive_negotiator_port * next;
	} connection_port_list;
	
	jive_negotiator_option * option;
	
	jive_negotiator_port_attach attach;
	struct {
		jive_negotiator_port * prev;
		jive_negotiator_port * next;
	} hash_chain;
	union {
		jive::input * input;
		jive::output * output;
	} hash_key;
	
	bool specialized;
	struct {
		jive_negotiator_port * prev;
		jive_negotiator_port * next;
	} specialized_list;
};

/* connections */

struct jive_negotiator_connection {
	jive_negotiator * negotiator;
	struct {
		jive_negotiator_port * first;
		jive_negotiator_port * last;
	} ports;
	
	struct {
		jive_negotiator_connection * prev;
		jive_negotiator_connection * next;
	} negotiator_connection_list;
	bool validated;
};

jive_negotiator_connection *
jive_negotiator_connection_create(jive_negotiator * negotiator);

void
jive_negotiator_connection_invalidate(jive_negotiator_connection * self);

/* constraints */

struct jive_negotiator_constraint {
	const jive_negotiator_constraint_class * class_;
	jive_negotiator * negotiator;
	struct {
		jive_negotiator_port * first;
		jive_negotiator_port * last;
	} ports;
	
	struct {
		jive_negotiator_constraint * prev;
		jive_negotiator_constraint * next;
	} negotiator_constraint_list;
	
	struct {
		jive_negotiator_constraint * prev;
		jive_negotiator_constraint * next;
	} hash_chain;
	union {
		struct jive_node * node;
		jive::gate * gate;
	} hash_key;
};

struct jive_negotiator_constraint_class {
	void (*fini)(jive_negotiator_constraint * self);
	void (*revalidate)(jive_negotiator_constraint * self, jive_negotiator_port * port);
};

jive_negotiator_constraint *
jive_negotiator_identity_constraint_create(jive_negotiator * self);

JIVE_DECLARE_HASH_TYPE(
	jive_negotiator_node_hash,
	jive_negotiator_constraint,
	jive_node *,
	hash_key.node,
	hash_chain);
JIVE_DECLARE_HASH_TYPE(jive_negotiator_gate_hash, jive_negotiator_constraint, jive::gate *,
	hash_key.gate, hash_chain);
JIVE_DECLARE_HASH_TYPE(jive_negotiator_input_hash, jive_negotiator_port, jive::input *,
	hash_key.input, hash_chain);
JIVE_DECLARE_HASH_TYPE(jive_negotiator_output_hash, jive_negotiator_port, jive::output *,
	hash_key.output, hash_chain);

struct jive_negotiator_class {
	/* create empty option (probably invalid) */
	jive_negotiator_option * (*option_create)(const jive_negotiator * self);
	
	/* store suitable default options for this type/resource class pair */
	bool (*option_gate_default)(const jive_negotiator * self, jive_negotiator_option * dst,
		const jive::gate * gate);
	
	/* annotate non-gate ports of node */
	void (*annotate_node_proper)(jive_negotiator * self, struct jive_node * node);
	
	/* annotate ports of node */
	void (*annotate_node)(jive_negotiator * self, struct jive_node * node);
	
	/* process region */
	void (*process_region)(jive_negotiator * self, struct jive_region * region);
};

struct jive_negotiator {
	const jive_negotiator_class * class_;
	
	struct jive_graph * graph;
	struct jive_context * context;
	jive_negotiator_input_hash input_map;
	jive_negotiator_output_hash output_map;
	jive_negotiator_node_hash node_map;
	jive_negotiator_gate_hash gate_map;
	
	struct {
		jive_negotiator_connection * first;
		jive_negotiator_connection * last;
	} validated_connections;
	
	struct {
		jive_negotiator_connection * first;
		jive_negotiator_connection * last;
	} invalidated_connections;
	
	struct {
		jive_negotiator_constraint * first;
		jive_negotiator_constraint * last;
	} constraints;
	
	struct {
		jive::negotiator_split_node * first;
		jive::negotiator_split_node * last;
	} split_nodes;
	
	struct {
		jive_negotiator_port * first;
		jive_negotiator_port * last;
	} unspecialized_ports;
	
	struct {
		jive_negotiator_port * first;
		jive_negotiator_port * last;
	} specialized_ports;
	
	jive_negotiator_option * tmp_option;
};

jive_negotiator_port *
jive_negotiator_port_create(
	jive_negotiator_constraint * constraint,
	jive_negotiator_connection * connection,
	const jive_negotiator_option * option);

void
jive_negotiator_port_divert(
	jive_negotiator_port * self,
	jive_negotiator_connection * new_connection);

void
jive_negotiator_port_split(jive_negotiator_port * self);

/* inheritable initializer for constraint */
void
jive_negotiator_constraint_init_(
	jive_negotiator_constraint * self,
	jive_negotiator * negotiator,
	const jive_negotiator_constraint_class * class_);

/* inheritable finalizer for constraint */
void
jive_negotiator_constraint_fini_(jive_negotiator_constraint * self);

/* inheritable default node annotator */
void
jive_negotiator_annotate_node_(jive_negotiator * self, struct jive_node * node);

/* inheritable default proper node annotator */
void
jive_negotiator_annotate_node_proper_(jive_negotiator * self, struct jive_node * node);

/* inheritable default gate annotator */
bool
jive_negotiator_option_gate_default_(const jive_negotiator * self, jive_negotiator_option * dst,
	const jive::gate * gate);

void
jive_negotiator_process_region_(jive_negotiator * self, struct jive_region * region);

void
jive_negotiator_init_(
	jive_negotiator * self,
	const jive_negotiator_class * class_,
	jive_graph * graph);

void
jive_negotiator_fini_(jive_negotiator * self);

void
jive_negotiator_process(jive_negotiator * self);

void
jive_negotiator_insert_split_nodes(jive_negotiator * self);

void
jive_negotiator_remove_split_nodes(jive_negotiator * self);

jive_negotiator_constraint *
jive_negotiator_annotate_identity(jive_negotiator * self,
	size_t ninputs, jive::input * const inputs[],
	size_t noutputs, jive::output * const outputs[],
	const jive_negotiator_option * option);

jive_negotiator_constraint *
jive_negotiator_annotate_identity_node(
	jive_negotiator * self,
	jive_node * node,
	const jive_negotiator_option * option);

void
jive_negotiator_fully_specialize(jive_negotiator * self);

jive_negotiator_port *
jive_negotiator_map_output(const jive_negotiator * self, jive::output * output);

jive_negotiator_port *
jive_negotiator_map_input(const jive_negotiator * self, jive::input * input);

/* protected functions that allow to manipulate negotiator state */
void
jive_negotiator_port_destroy(jive_negotiator_port * self);

jive_negotiator_port *
jive_negotiator_annotate_simple_input(jive_negotiator * self, jive::input * input,
	const jive_negotiator_option * option);

jive_negotiator_port *
jive_negotiator_annotate_simple_output(jive_negotiator * self, jive::output * output,
	const jive_negotiator_option * option);

#endif
