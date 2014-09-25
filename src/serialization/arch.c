/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/address.h>
#include <jive/arch/immediate-node.h>
#include <jive/arch/instruction.h>
#include <jive/arch/memorytype.h>
#include <jive/arch/registers.h>
#include <jive/arch/stackslot.h>
#include <jive/arch/subroutine.h>
#include <jive/arch/subroutine/nodes.h>
#include <jive/serialization/driver.h>
#include <jive/serialization/grammar.h>
#include <jive/serialization/instrcls-registry.h>
#include <jive/serialization/nodecls-registry.h>
#include <jive/serialization/rescls-registry.h>
#include <jive/serialization/token-stream.h>
#include <jive/serialization/typecls-registry.h>

static void
jive_serialization_stackslot_serialize(
	const jive_serialization_rescls * self,
	jive_serialization_driver * driver,
	const jive_resource_class * rescls,
	jive_token_ostream * os)
{
	const jive_stackslot_size_class * cls = (const jive_stackslot_size_class *) rescls;
	
	jive_serialize_uint(driver, cls->size, os);
	jive_serialize_char_token(driver, ',', os);
	jive_serialize_uint(driver, cls->alignment, os);
}

static bool
jive_serialization_stackslot_deserialize(
	const jive_serialization_rescls * self,
	jive_serialization_driver * driver,
	jive_token_istream * is,
	const jive_resource_class ** rescls)
{
	uint64_t size, alignment;
	
	if (!jive_deserialize_uint(driver, is, &size))
		return false;
	if (!jive_deserialize_char_token(driver, is, ','))
		return false;
	if (!jive_deserialize_uint(driver, is, &alignment))
		return false;
	
	*rescls = jive_stackslot_size_class_get(size, alignment);
	
	return *rescls != 0;
}

static void
jive_serialization_frameslot_serialize(
	const jive_serialization_rescls * self,
	jive_serialization_driver * driver,
	const jive_resource_class * rescls,
	jive_token_ostream * os)
{
	const jive_fixed_stackslot_class * cls = (const jive_fixed_stackslot_class *) rescls;
	const jive_stackslot * slot = (const jive_stackslot *) cls->slot;
	
	jive_serialize_uint(driver, cls->base.size, os);
	jive_serialize_char_token(driver, ',', os);
	jive_serialize_uint(driver, cls->base.alignment, os);
	jive_serialize_char_token(driver, ',', os);
	jive_serialize_int(driver, slot->offset, os);
}


static bool
jive_serialization_frameslot_deserialize(
	const jive_serialization_rescls * self,
	jive_serialization_driver * driver,
	jive_token_istream * is,
	const jive_resource_class ** rescls)
{
	uint64_t size, alignment;
	int64_t offset;
	
	if (!jive_deserialize_uint(driver, is, &size))
		return false;
	if (!jive_deserialize_char_token(driver, is, ','))
		return false;
	if (!jive_deserialize_uint(driver, is, &alignment))
		return false;
	if (!jive_deserialize_char_token(driver, is, ','))
		return false;
	if (!jive_deserialize_int(driver, is, &offset))
		return false;
	
	*rescls = jive_fixed_stackslot_class_get(size, alignment, offset);
	
	return *rescls != 0;
}

static void
jive_serialization_callslot_serialize(
	const jive_serialization_rescls * self,
	jive_serialization_driver * driver,
	const jive_resource_class * rescls,
	jive_token_ostream * os)
{
	const jive_callslot_class * cls = (const jive_callslot_class *) rescls;
	const jive_callslot * slot = (const jive_callslot *) cls->slot;
	
	jive_serialize_uint(driver, cls->base.size, os);
	jive_serialize_char_token(driver, ',', os);
	jive_serialize_uint(driver, cls->base.alignment, os);
	jive_serialize_char_token(driver, ',', os);
	jive_serialize_int(driver, slot->offset, os);
}

static bool
jive_serialization_callslot_deserialize(
	const jive_serialization_rescls * self,
	jive_serialization_driver * driver,
	jive_token_istream * is,
	const jive_resource_class ** rescls)
{
	uint64_t size, alignment;
	int64_t offset;
	
	if (!jive_deserialize_uint(driver, is, &size))
		return false;
	if (!jive_deserialize_char_token(driver, is, ','))
		return false;
	if (!jive_deserialize_uint(driver, is, &alignment))
		return false;
	if (!jive_deserialize_char_token(driver, is, ','))
		return false;
	if (!jive_deserialize_int(driver, is, &offset))
		return false;
	
	*rescls = jive_callslot_class_get(size, alignment, offset);
	
	return *rescls != 0;
}

static void
jive_memory_type_serialize(
	const jive_serialization_typecls * self,
	jive_serialization_driver * driver,
	const jive::base::type * type,
	jive_token_ostream * os)
{
	/* no attributes */
}

static bool
jive_memory_type_deserialize(
	const jive_serialization_typecls * self,
	jive_serialization_driver * driver,
	jive_token_istream * is,
	jive::base::type ** type)
{
	*type = new jive::mem::type();
	return true;
}

static void
jive_instruction_serialize(
	const jive_serialization_nodecls * self,
	struct jive_serialization_driver * driver,
	const jive_node_attrs * attrs_, jive_token_ostream * os)
{
	const jive::instruction_op * attrs = (const jive::instruction_op *) attrs_;
	const jive_serialization_instrcls_registry * reg = driver->instrcls_registry;
	const jive_serialization_instrcls * sercls =
		jive_serialization_instrcls_lookup_by_cls(reg, attrs->icls());
	
	jive_token_ostream_identifier(os, sercls->tag);
}

static bool
jive_instruction_deserialize(
	const jive_serialization_nodecls * self,
	struct jive_serialization_driver * driver,
	jive_region * region, size_t narguments,
	jive::output * const arguments[], jive_token_istream * is,
	jive_node ** node)
{
	const jive_token * token = jive_token_istream_current(is);
	const jive_instruction_class * icls = 0;
	if (token->type == jive_token_identifier) {
		const jive_serialization_instrcls * sercls;
		sercls = jive_serialization_instrcls_lookup_by_tag(
			driver->instrcls_registry, token->v.identifier);
		if (sercls)
			icls = sercls->cls;
	}
	if (!icls) {
		driver->error(driver, "Expected instruction class identifier");
		return false;
	}
	jive_token_istream_advance(is);
	
	jive::instruction_op op(icls);
	*node = op.create_node(region, narguments, arguments);
	
	return true;
}

static void
jive_label_to_bitstring_serialize(
	const jive_serialization_nodecls * self,
	struct jive_serialization_driver * driver,
	const jive_node_attrs * attrs_, jive_token_ostream * os)
{
	const jive::address::label_to_bitstring_operation * attrs =
		(const jive::address::label_to_bitstring_operation *) attrs_;
	
	jive_serialize_label(driver, attrs->label(), os);
	jive_serialize_char_token(driver, ',', os);
	jive_serialize_uint(driver, attrs->nbits(), os);
}

static bool
jive_label_to_bitstring_deserialize(
	const jive_serialization_nodecls * self,
	struct jive_serialization_driver * driver,
	jive_region * region, size_t noperands,
	jive::output * const operands[], jive_token_istream * is,
	jive_node ** node)
{
	const jive_label * label;
	uint64_t nbits;
	if (!jive_deserialize_label(driver, is, &label))
		return false;
	if (!jive_deserialize_char_token(driver, is, ','))
		return false;
	if (!jive_deserialize_uint(driver, is, &nbits))
		return false;
	
	jive::address::label_to_bitstring_operation attrs(label, nbits);
	
	*node = JIVE_LABEL_TO_BITSTRING_NODE.create(region, &attrs,
		0, NULL);
	
	return true;
}

static void
jive_label_to_address_serialize(
	const jive_serialization_nodecls * self,
	struct jive_serialization_driver * driver,
	const jive_node_attrs * attrs_, jive_token_ostream * os)
{
	const jive::address::label_to_address_operation * attrs =
		(const jive::address::label_to_address_operation *) attrs_;
	
	jive_serialize_label(driver, attrs->label(), os);
}

static bool
jive_label_to_address_deserialize(
	const jive_serialization_nodecls * self,
	struct jive_serialization_driver * driver,
	jive_region * region, size_t noperands,
	jive::output * const operands[], jive_token_istream * is,
	jive_node ** node)
{
	const jive_label * label;
	if (!jive_deserialize_label(driver, is, &label))
		return false;
	
	jive::address::label_to_address_operation attrs(label);
	
	*node = JIVE_LABEL_TO_ADDRESS_NODE.create(region, &attrs,
		0, NULL);
	
	return true;
}

#if 0
#include <jive/backend/i386/subroutine.h>

static void
jive_subroutine_serialize(
	const jive_serialization_nodecls * self,
	struct jive_serialization_driver * driver,
	const jive_node_attrs * attrs_, jive_token_ostream * os)
{
	const jive::subroutine_op * attrs = (const jive::subroutine_op *) attrs_;
	jive_subroutine_deprecated * subroutine = attrs->subroutine();
	
	if (subroutine->class_ == &JIVE_I386_SUBROUTINE)
		jive_token_ostream_identifier(os, "i386");
	else
		abort();
	
	size_t n;
	for (n = 0; n < subroutine->nparameters; ++n) {
		jive::gate * gate = subroutine->parameters[n];
		jive_serialize_defined_gate(driver, gate, os);
	}
	jive_serialize_char_token(driver, ';', os);
	for (n = 0; n < subroutine->nreturns; ++n) {
		jive::gate * gate = subroutine->returns[n];
		jive_serialize_defined_gate(driver, gate, os);
	}
	jive_serialize_char_token(driver, ';', os);
	for (n = 0; n < subroutine->npassthroughs; ++n) {
		jive::gate * gate = subroutine->passthroughs[n].gate;
		jive_serialize_defined_gate(driver, gate, os);
	}
	jive_serialize_char_token(driver, ';', os);
}

static bool
jive_deserialize_gatelist(jive_serialization_driver * self,
	jive_token_istream * is,
	size_t * ngates, jive::gate *** gates)
{
	*ngates = 0;
	*gates = 0;
	const jive_token * token = jive_token_istream_current(is);
	while (token->type == jive_token_identifier) {
		jive::gate * gate;
		if (!jive_deserialize_defined_gate(self, is, &gate)) {
			jive_context_free(self->context, *gates);
			return false;
		}
		*gates = jive_context_realloc(self->context, *gates,
			(1 + *ngates) * sizeof(jive::gate *));
		(*gates)[*ngates] = gate;
		(*ngates) ++;
	}
	
	return true;
}

static bool
jive_subroutine_deserialize(
	const jive_serialization_nodecls * self,
	struct jive_serialization_driver * driver,
	jive_region * region, size_t noperands,
	jive::output * const operands[], jive_token_istream * is,
	jive_node ** node)
{
	*node = 0;
	if (noperands != 1)
		return false;
	
	jive_node * leave = operands[0]->node();
	if (leave->region->bottom != leave)
		return false;
	jive_node * enter = leave->region->top;
	if (!enter)
		return false;
	
	const jive_token * token = jive_token_istream_current(is);
	if (token->type != jive_token_identifier)
		return false;
	if (strcmp(token->v.identifier, "i386") != 0)
		return false;
	jive_token_istream_advance(is);
	
	size_t nparameters;
	jive::gate ** parameters;
	if (!jive_deserialize_gatelist(driver, is, &nparameters, &parameters))
		return false;
	if (!jive_deserialize_char_token(driver, is, ';'))
		return false;
	
	size_t nreturns;
	jive::gate ** returns;
	if (!jive_deserialize_gatelist(driver, is, &nreturns, &returns)) {
		jive_context_free(driver->context, parameters);
		return false;
	}
	if (!jive_deserialize_char_token(driver, is, ';')) {
		jive_context_free(driver->context, parameters);
		return false;
	}
	
	size_t npassthroughs;
	jive::gate ** passthrough_gates;
	if (!jive_deserialize_gatelist(driver, is, &npassthroughs, &passthrough_gates)) {
		jive_context_free(driver->context, returns);
		jive_context_free(driver->context, parameters);
		return false;
	}
	if (!jive_deserialize_char_token(driver, is, ';')) {
		jive_context_free(driver->context, returns);
		jive_context_free(driver->context, parameters);
		return false;
	}
	
	std::vector<jive_subroutine_passthrough> passthroughs(npassthroughs);
	for (size_t n = 0; n < npassthroughs; ++n) {
		jive::gate * gate = passthrough_gates[n];
		passthroughs[n].gate = gate;
		passthroughs[n].output = jive_node_get_gate_output(enter, gate);
		passthroughs[n].input = jive_node_get_gate_input(leave, gate);
	}
	
	jive_subroutine_deprecated * subroutine = jive_subroutine_create_takeover(
		driver->context, &JIVE_I386_SUBROUTINE,
		nparameters, parameters,
		nreturns, returns,
		passthroughs.size(), &passthroughs[0]);
	*node = jive_subroutine_node_create(enter->region, subroutine);
	
	jive_context_free(driver->context, passthrough_gates);
	jive_context_free(driver->context, returns);
	jive_context_free(driver->context, parameters);
	
	return *node != NULL;
}
#endif

static void
jive_immediate_serialize(
	const jive_serialization_nodecls * self,
	struct jive_serialization_driver * driver,
	const jive_node_attrs * attrs_, jive_token_ostream * os)
{
	const jive::immediate_op * attrs = (const jive::immediate_op *) attrs_;
	jive_serialize_immediate(driver, &attrs->value(), os);
}

static bool
jive_immediate_deserialize(
	const jive_serialization_nodecls * self,
	struct jive_serialization_driver * driver,
	jive_region * region, size_t narguments,
	jive::output * const arguments[], jive_token_istream * is,
	jive_node ** node)
{
	jive_immediate imm;
	if (!jive_deserialize_immediate(driver, is, &imm))
		return false;
	
	jive::immediate_op op(imm);
	
	*node = op.create_node(region, narguments, arguments);
	
	return true;
}

JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(
	JIVE_GRAPH_TAIL_NODE, "graph_tail");
JIVE_SERIALIZATION_RESCLS_REGISTER(jive_root_register_class, "register");
JIVE_SERIALIZATION_META_RESCLS_REGISTER(JIVE_STACK_RESOURCE, "stackslot",
	jive_serialization_stackslot_serialize,
	jive_serialization_stackslot_deserialize);
JIVE_SERIALIZATION_META_RESCLS_REGISTER(JIVE_STACK_FRAMESLOT_RESOURCE, "stack_frameslot",
	jive_serialization_frameslot_serialize,
	jive_serialization_frameslot_deserialize);
JIVE_SERIALIZATION_META_RESCLS_REGISTER(JIVE_STACK_CALLSLOT_RESOURCE, "stack_callslot",
	jive_serialization_callslot_serialize,
	jive_serialization_callslot_deserialize);
JIVE_SERIALIZATION_TYPECLS_REGISTER(jive::mem::type, jive_memory_type, "memory",
	jive_memory_type_serialize,
	jive_memory_type_deserialize);
JIVE_SERIALIZATION_NODECLS_REGISTER(
	JIVE_INSTRUCTION_NODE, "instr",
	jive_instruction_serialize,
	jive_instruction_deserialize);
JIVE_SERIALIZATION_NODECLS_REGISTER(
	JIVE_LABEL_TO_ADDRESS_NODE, "label2addr",
	jive_label_to_address_serialize,
	jive_label_to_address_deserialize);
JIVE_SERIALIZATION_NODECLS_REGISTER(
	JIVE_LABEL_TO_BITSTRING_NODE, "label2bits",
	jive_label_to_bitstring_serialize,
	jive_label_to_bitstring_deserialize);
#if 0
JIVE_SERIALIZATION_OPNODE_REGISTER_SIMPLE(
	JIVE_SUBROUTINE_ENTER_NODE,
	jive::subroutine_head_op,
	"subroutine_enter");
JIVE_SERIALIZATION_OPNODE_REGISTER_SIMPLE(
	JIVE_SUBROUTINE_LEAVE_NODE,
	jive::subroutine_tail_op,
	"subroutine_leave");
JIVE_SERIALIZATION_NODECLS_REGISTER(
	JIVE_SUBROUTINE_NODE, "subroutine",
	jive_subroutine_serialize,
	jive_subroutine_deserialize);
#endif
JIVE_SERIALIZATION_NODECLS_REGISTER(
	JIVE_IMMEDIATE_NODE, "immediate",
	jive_immediate_serialize,
	jive_immediate_deserialize);
