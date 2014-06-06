/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/addresstype.h>

#include <string.h>

#include <jive/util/buffer.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>

namespace jive {
namespace addr {

/* type */

type::~type() noexcept {}

void
type::label(jive_buffer & buffer) const
{
	jive_buffer_putstr(&buffer, "addr");
}

bool
type::operator==(const jive_type & other) const noexcept
{
	return dynamic_cast<const jive::addr::type*>(&other) != nullptr;
}

jive::addr::type *
type::copy() const
{
	return new jive::addr::type();
}

jive_input *
type::create_input(jive_node * node, size_t index, jive_output * origin) const
{
	return new jive::addr::input(node, index, origin);
}

jive_output *
type::create_output(jive_node * node, size_t index) const
{
	return new jive::addr::output(node, index);
}

jive_gate *
type::create_gate(jive_graph * graph, const char * name) const
{
	return new jive::addr::gate(graph, name);
}

/* input */

input::~input() noexcept {}

input::input(struct jive_node * node, size_t index, jive_output * origin)
	: jive::value::input(node, index, origin)
{}

/* output */

output::~output() noexcept {}

output::output(jive_node * node, size_t index)
	: jive::value::output(node, index)
{}

/* gate */

gate::~gate() noexcept {}

gate::gate(jive_graph * graph, const char name[])
	: jive::value::gate(graph, name)
{}

}
}
