/*
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/record/rcdtype.h>
#include <jive/util/buffer.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>

#include <string.h>

namespace jive {
namespace rcd {

/* type */

type::~type() noexcept {}

type::type(const jive::rcd::declaration * decl) noexcept
	: jive::value::type()
	, decl_(decl)
{}

void
type::label(jive_buffer & buffer) const
{
	jive_buffer_putstr(&buffer, "rcd");
}

bool
type::operator==(const jive_type & _other) const noexcept
{
	const jive::rcd::type * other = dynamic_cast<const jive::rcd::type*>(&_other);
	return other != nullptr && this->declaration() == other->declaration();
}

jive::rcd::type *
type::copy() const
{
	return new jive::rcd::type(this->declaration());
}

jive_input *
type::create_input(jive_node * node, size_t index, jive_output * origin) const
{
	return new jive::rcd::input(this->declaration(), node, index, origin);
}

jive_output *
type::create_output(jive_node * node, size_t index) const
{
	return new jive::rcd::output(this->declaration(), node, index);
}

jive_gate *
type::create_gate(jive_graph * graph, const char * name) const
{
	return new jive::rcd::gate(this->declaration(), graph, name);
}

/* input */

input::input(const jive::rcd::declaration * decl, struct jive_node * node, size_t index,
	jive_output * origin)
	: jive::value::input(node, index, origin)
	, type_(decl)
{}

input::~input() noexcept {}

/* output */

output::output(const jive::rcd::declaration * decl, struct jive_node * node, size_t index)
	: jive::value::output(node, index)
	, type_(decl)
{}

output::~output() noexcept {}

/* gate */

gate::gate(const jive::rcd::declaration * decl, jive_graph * graph, const char name[])
	: jive::value::gate(graph, name)
	, type_(decl)
{}

gate::~gate() noexcept {}

}
}
