/*
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_FLTTYPE_H
#define JIVE_TYPES_FLOAT_FLTTYPE_H

#include <jive/vsdg/valuetype.h>

namespace jive {
namespace flt {

/* float type */

class type final : public jive::value::type {
public:
	virtual ~type() noexcept;

	inline constexpr type() noexcept : jive::value::type() {};

	virtual void label(jive_buffer & buffer) const override;

	virtual bool operator==(const jive::base::type & other) const noexcept override;

	virtual jive::flt::type * copy() const override;

	virtual jive::input * create_input(jive_node * node, size_t index,
		jive::output * origin) const override;

	virtual jive::output * create_output(jive_node * node, size_t index) const override;

	virtual jive::gate * create_gate(jive_graph * graph, const char * name) const override;
};

/* float input */

class input final : public jive::value::input {
public:
	virtual ~input() noexcept;

	input(struct jive_node * node, size_t index, jive::output * origin);

	virtual const jive::flt::type & type() const noexcept { return type_; }

private:
	jive::flt::type type_;
};

/* float output */

class output final : public jive::value::output {
public:
	virtual ~output() noexcept;

	output(struct jive_node * node, size_t index);

	virtual const jive::flt::type & type() const noexcept { return type_; }

private:
	jive::flt::type type_;
};

/* float gate */

class gate final : public jive::value::gate {
public:
	virtual ~gate() noexcept;

	gate(jive_graph * graph, const char name[]);

	virtual const jive::flt::type & type() const noexcept { return type_; }

private:
	jive::flt::type type_;
};

}
}

#endif
