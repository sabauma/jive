#include <jive/serialization/driver.h>

#include <stdio.h>

#include <jive/serialization/grammar.h>
#include <jive/serialization/instrcls-registry.h>
#include <jive/serialization/nodecls-registry.h>
#include <jive/serialization/rescls-registry.h>
#include <jive/serialization/typecls-registry.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>

void
jive_serialization_driver_init(
	jive_serialization_driver * self,
	jive_context * context)
{
	self->context = context;
	self->instrcls_registry =  jive_serialization_instrcls_registry_get();
	self->nodecls_registry = jive_serialization_nodecls_registry_get();
	self->typecls_registry = jive_serialization_typecls_registry_get();
	self->rescls_registry = jive_serialization_rescls_registry_get();
	jive_serialization_symtab_init(&self->symtab, context);
}

void
jive_serialization_driver_fini(
	jive_serialization_driver * self)
{
	jive_serialization_instrcls_registry_put(self->instrcls_registry);
	jive_serialization_nodecls_registry_put(self->nodecls_registry);
	jive_serialization_typecls_registry_put(self->typecls_registry);
	jive_serialization_rescls_registry_put(self->rescls_registry);
	jive_serialization_symtab_fini(&self->symtab);
}

typedef struct jive_serialization_simple_namegen jive_serialization_simple_namegen;
struct jive_serialization_simple_namegen {
	jive_serialization_namegen base;
	size_t gate_id;
	size_t node_id;
	size_t output_id;
};

static void
simple_name_gate(
	jive_serialization_namegen * self_,
	jive_serialization_symtab * symtab,
	jive_gate * gate)
{
	if (jive_serialization_symtab_gate_to_name(symtab, gate) != 0)
		return;
	jive_serialization_simple_namegen * self = (jive_serialization_simple_namegen *) self_;
	char tmp[20];
	snprintf(tmp, sizeof(tmp), "g%zd", self->gate_id ++);
	jive_serialization_symtab_insert_gatesym(symtab, gate,
		jive_serialization_symtab_strdup(symtab, tmp));
}

static void
simple_name_node(
	jive_serialization_namegen * self_,
	jive_serialization_symtab * symtab,
	jive_node * node)
{
	if (jive_serialization_symtab_node_to_name(symtab, node) != 0)
		return;
	jive_serialization_simple_namegen * self = (jive_serialization_simple_namegen *) self_;
	char tmp[20];
	snprintf(tmp, sizeof(tmp), "n%zd", self->node_id ++);
	jive_serialization_symtab_insert_nodesym(symtab, node,
		jive_serialization_symtab_strdup(symtab, tmp));
}

static void
simple_name_output(
	jive_serialization_namegen * self_,
	jive_serialization_symtab * symtab,
	jive_output * output)
{
	if (jive_serialization_symtab_output_to_name(symtab, output) != 0)
		return;
	jive_serialization_simple_namegen * self = (jive_serialization_simple_namegen *) self_;
	char tmp[20];
	snprintf(tmp, sizeof(tmp), "o%zd", self->output_id ++);
	jive_serialization_symtab_insert_outputsym(symtab, output,
		jive_serialization_symtab_strdup(symtab, tmp));
}

static void
jive_serialization_simple_namegen_init(
	jive_serialization_simple_namegen * self)
{
	self->base.name_gate = simple_name_gate;
	self->base.name_node = simple_name_node;
	self->base.name_output = simple_name_output;
	self->gate_id = 0;
	self->node_id = 0;
	self->output_id = 0;
}

static void
jive_serialization_simple_namegen_fini(
	jive_serialization_simple_namegen * self)
{
}

void
jive_serialize_graph(
	jive_serialization_driver * self,
	struct jive_graph * graph,
	struct jive_token_ostream * os)
{
	jive_serialization_simple_namegen namegen;
	jive_serialization_simple_namegen_init(&namegen);
	jive_serialize_regionbody(self, &namegen.base, graph->root_region, os);
	jive_serialization_simple_namegen_fini(&namegen);
}

bool
jive_deserialize_graph(
	jive_serialization_driver * self,
	struct jive_token_istream * is,
	struct jive_graph * graph)
{
	return jive_deserialize_regionbody(self, is, graph->root_region);
}
