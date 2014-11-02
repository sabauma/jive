/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>
#include <stdio.h>

#include <jive/arch/subroutine/nodes.h>
#include <jive/regalloc.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/view.h>
#include <jive/vsdg.h>

#include "testarch.h"

static jive_graph *
create_testgraph_postop_xfer()
{
	/* requires post-op transfer to satisfy register constraints */
	jive_graph * graph = jive_graph_create();
	const jive_argument_type tmparray0[] = { jive_argument_int, jive_argument_int };
	
	jive_subroutine subroutine = jive_testarch_subroutine_begin(graph,
		2, tmparray0,
		0, NULL
	);
	jive::output * memstate = jive_subroutine_simple_get_global_state(subroutine);
	const jive::base::type * memtype = &memstate->type();
	jive_node * enter_mux = jive_state_split(memtype, memstate, 1)[0]->node();
	jive_node * leave_mux = jive_state_merge(memtype, 1, &enter_mux->outputs[0])->node();
	jive_subroutine_simple_set_global_state(subroutine, leave_mux->outputs[0]);
	
	jive::gate * retval_gate = jive_register_class_create_gate(
		&jive_testarch_regcls_r3, graph, "retval");
	
	jive::output * arg1 = jive_subroutine_simple_get_argument(subroutine, 0);
	jive::output * arg2 = jive_subroutine_simple_get_argument(subroutine, 1);
	jive::output * tmparray1[] = {arg1, arg2};
	jive::output * retval = jive_instruction_node_create(
		subroutine.region,
		&jive_testarch_instr_sub_gpr,
		tmparray1, NULL)->outputs[0];
	jive_node_gate_input(leave_mux, retval_gate, retval);
	
	jive_graph_export(graph, jive_subroutine_end(subroutine)->outputs[0]);
	
	return graph;
}

static jive_graph *
create_testgraph_preop_xfer()
{
	/* requires pre-op transfer to satisfy register constraints */
	jive_graph * graph = jive_graph_create();
	const jive_argument_type tmparray2[] = { jive_argument_int, jive_argument_int };
	
	jive_subroutine subroutine = jive_testarch_subroutine_begin(graph,
		2, tmparray2,
		0, NULL
	);
	jive::output * memstate = jive_subroutine_simple_get_global_state(subroutine);
	const jive::base::type * memtype = &memstate->type();
	jive_node * enter_mux = jive_state_split(memtype, memstate, 1)[0]->node();
	jive_node * leave_mux = jive_state_merge(memtype, 1, &enter_mux->outputs[0])->node();
	jive_subroutine_simple_set_global_state(subroutine, leave_mux->outputs[0]);
	
	jive::gate * retval_gate = jive_register_class_create_gate(
		&jive_testarch_regcls_r3, graph, "retval");
	jive::gate * retval2_gate = jive_register_class_create_gate(
		&jive_testarch_regcls_r1, graph, "retval2");
	
	jive::output * arg1 = jive_subroutine_simple_get_argument(subroutine, 0);
	jive::output * arg2 = jive_subroutine_simple_get_argument(subroutine, 1);
	jive::output * tmparray3[] = {arg1, arg2};
	jive::output * retval = jive_instruction_node_create(
		subroutine.region,
		&jive_testarch_instr_sub_gpr,
		tmparray3, NULL)->outputs[0];
	jive_node_gate_input(leave_mux, retval_gate, retval);
	jive_node_gate_input(leave_mux, retval2_gate, arg1);
	
	jive_graph_export(graph, jive_subroutine_end(subroutine)->outputs[0]);
	
	return graph;
}

static jive_graph *
create_testgraph_preop_aux_xfer()
{
	/* requires pre-op transfer to auxiliary register to satisfy register constraints */
	jive_graph * graph = jive_graph_create();
	const jive_argument_type tmparray4[] = { jive_argument_int, jive_argument_int };
	
	jive_subroutine subroutine = jive_testarch_subroutine_begin(graph,
		2, tmparray4,
		0, NULL
	);
	jive::output * memstate = jive_subroutine_simple_get_global_state(subroutine);
	const jive::base::type * memtype = &memstate->type();
	jive_node * enter_mux = jive_state_split(memtype, memstate, 1)[0]->node();
	jive_node * leave_mux = jive_state_merge(memtype, 1, &enter_mux->outputs[0])->node();
	jive_subroutine_simple_set_global_state(subroutine, leave_mux->outputs[0]);
	
	jive::gate * retval_gate = jive_register_class_create_gate(
		&jive_testarch_regcls_r2, graph, "retval");
	jive::gate * retval2_gate = jive_register_class_create_gate(
		&jive_testarch_regcls_r1, graph, "retval2");
	
	jive::output * arg1 = jive_subroutine_simple_get_argument(subroutine, 0);
	jive::output * arg2 = jive_subroutine_simple_get_argument(subroutine, 1);
	jive::output * tmparray5[] = {arg1, arg2};
	jive::output * retval = jive_instruction_node_create(
		subroutine.region,
		&jive_testarch_instr_sub_gpr,
		tmparray5, NULL)->outputs[0];
	jive_node_gate_input(leave_mux, retval_gate, retval);
	jive_node_gate_input(leave_mux, retval2_gate, arg1);
	
	jive_graph_export(graph, jive_subroutine_end(subroutine)->outputs[0]);
	
	return graph;
}

static jive_graph *
create_testgraph_preop_aux_xfer_shaper()
{
	/* requires pre-op transfer to auxiliary register to satisfy register constraints
	additionally, the auxiliary register must be reserved by the shaper */
	jive_graph * graph = jive_graph_create();
	const jive_argument_type tmparray6[] = { jive_argument_int, jive_argument_int };
	
	jive_subroutine subroutine = jive_testarch_subroutine_begin(graph,
		2, tmparray6,
		0, NULL
	);
	jive::output * memstate = jive_subroutine_simple_get_global_state(subroutine);
	const jive::base::type * memtype = &memstate->type();
	jive_node * enter_mux = jive_state_split(memtype, memstate, 1)[0]->node();
	jive_node * leave_mux = jive_state_merge(memtype, 1, &enter_mux->outputs[0])->node();
	jive_subroutine_simple_set_global_state(subroutine, leave_mux->outputs[0]);
	
	jive::gate * save_r3_gate = jive_register_class_create_gate(
		&jive_testarch_regcls_r3, graph, "save_r3");
	jive::gate * retval_gate = jive_register_class_create_gate(
		&jive_testarch_regcls_r2, graph, "retval");
	jive::gate * retval2_gate = jive_register_class_create_gate(
		&jive_testarch_regcls_r1, graph, "retval2");
	
	jive::output * arg1 = jive_subroutine_simple_get_argument(subroutine, 0);
	jive::output * arg2 = jive_subroutine_simple_get_argument(subroutine, 1);
	jive::output * tmparray7[] = {arg1, arg2};
	jive::output * retval = jive_instruction_node_create(
		subroutine.region,
		&jive_testarch_instr_sub_gpr,
		tmparray7, NULL)->outputs[0];
	jive_node_gate_input(leave_mux, retval_gate, retval);
	jive_node_gate_input(leave_mux, retval2_gate, arg1);
	
	jive_node_gate_input(leave_mux, save_r3_gate, jive_node_gate_output(enter_mux, save_r3_gate));
	
	jive_graph_export(graph, jive_subroutine_end(subroutine)->outputs[0]);
	
	return graph;
}

typedef jive_graph * (*creator_function_t)();

static const creator_function_t tests[] = {
	create_testgraph_postop_xfer,
	create_testgraph_preop_xfer,
	create_testgraph_preop_aux_xfer,
	create_testgraph_preop_aux_xfer_shaper,
};

static int test_main(void)
{
	setlocale(LC_ALL, "");
	
	size_t n;
	for (n = 0; n < sizeof(tests)/sizeof(tests[0]); n++) {
		fprintf(stderr, "%zd\n", n);
		jive_graph * graph = tests[n]();
		jive_view(graph, stdout);
		jive_shaped_graph * shaped_graph = jive_regalloc(graph);
		jive_view(graph, stdout);
		jive_shaped_graph_destroy(shaped_graph);
		jive_graph_destroy(graph);
	}

	return 0;
}

JIVE_UNIT_TEST_REGISTER("regalloc/test-twoop", test_main);
