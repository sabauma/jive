/*
 * Copyright 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 2018 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_IMMEDIATE_H
#define JIVE_ARCH_IMMEDIATE_H

#include <jive/rvsdg/nullary.h>
#include <jive/rvsdg/simple-node.h>
#include <jive/rvsdg/type.h>

typedef uint64_t jive_immediate_int;

namespace jive {

/* immediate type */

class immtype final : public jive::valuetype {
public:
	virtual
	~immtype() noexcept;

	inline constexpr
	immtype() noexcept
	: jive::valuetype()
	{}

	virtual std::string
	debug_string() const override;

	virtual bool
	operator==(const jive::type & other) const noexcept override;

	virtual std::unique_ptr<jive::type>
	copy() const override;
};

/* immediate */

class label;

class immediate final {
public:
	inline constexpr
	immediate(
		jive_immediate_int offset = 0,
		const jive::label * add_label = nullptr,
		const jive::label * sub_label = nullptr,
		const void * modifier = nullptr) noexcept
	: offset_(offset)
	, add_label_(add_label)
	, sub_label_(sub_label)
	, modifier_(modifier)
	{}

	constexpr
	immediate(const immediate & other)
	: offset_(other.offset_)
	, add_label_(other.add_label_)
	, sub_label_(other.sub_label_)
	, modifier_(other.modifier_)
	{}

	constexpr
	immediate(immediate && other)
	: offset_(std::move(other.offset_))
	, add_label_(std::move(other.add_label_))
	, sub_label_(std::move(other.sub_label_))
	, modifier_(std::move(other.modifier_))
	{}

	inline immediate &
	operator=(const immediate & other) noexcept
	{
		offset_ = other.offset_;
		add_label_ = other.add_label_;
		sub_label_ = other.sub_label_;
		modifier_ = other.modifier_;
		return *this;
	}

	immediate &
	operator=(immediate && other) noexcept
	{
		if (this == &other)
			return *this;

		offset_ = std::move(offset_);
		add_label_ = std::move(other.add_label_);
		sub_label_ = std::move(other.sub_label_);
		modifier_ = std::move(other.modifier_);
		return *this;
	}

	inline immediate &
	operator+(const immediate & other)
	{
		auto add1 = add_label_;
		auto add2 = other.add_label_;
		auto sub1 = sub_label_;
		auto sub2 = other.sub_label_;

		if (add1 == sub2) {
			add1 = nullptr;
			sub2 = nullptr;
		}

		if (add2 && sub1) {
			add2 = nullptr;
			sub1 = nullptr;
		}

		if ((add1 && add2) || (sub1 && sub2) || (modifier_ || other.modifier_))
			throw compiler_error("Cannot add immediates");

		offset_ += other.offset_;
		add_label_ = add1 ? add1 : add2;
		sub_label_ = sub1 ? sub1 : sub2;
		modifier_ = nullptr;

		return *this;
	}

	inline immediate &
	operator+(jive_immediate_int offset) noexcept
	{
		offset_ += offset;
		return *this;
	}

	inline immediate &
	operator+=(const immediate & other)
	{
		*this = *this + other;
		return *this;
	}

	inline immediate &
	operator+=(uint64_t offset)
	{
		*this = *this + offset;
		return *this;
	}

	inline immediate &
	operator-(const immediate & other)
	{
		auto add1 = add_label_;
		auto add2 = other.sub_label_;
		auto sub1 = sub_label_;
		auto sub2 = other.add_label_;

		if (add1 == sub2) {
			add1 = nullptr;
			sub2 = nullptr;
		}

		if (add2 && sub1) {
			add2 = nullptr;
			sub1 = nullptr;
		}

		if ((add1 && add2) || (sub1 && sub2) || (modifier_ || other.modifier_))
			throw compiler_error("Cannot subtract immediates");

		offset_ -= other.offset_;
		add_label_ = add1 ? add1 : add2;
		sub_label_ = sub1 ? sub1 : sub2;
		modifier_ = nullptr;

		return *this;
	}

	inline bool
	operator==(const immediate & other) const noexcept
	{
		return
			offset_ == other.offset_ &&
			add_label_ == other.add_label_ &&
			sub_label_ == other.sub_label_ &&
			modifier_ == other.modifier_;
	}

	inline bool
	operator==(jive_immediate_int offset) const noexcept
	{
		return
			offset_ == offset &&
			add_label_ == nullptr &&
			sub_label_ == nullptr &&
			modifier_ == nullptr;
	}

	inline bool
	operator!=(const immediate & other) const noexcept
	{
		return !(*this == other);
	}

	inline bool
	operator!=(jive_immediate_int offset) const noexcept
	{
		return !(*this == offset);
	}

	bool
	has_add_label() const noexcept
	{
		return add_label_ != nullptr;
	}

	bool
	has_sub_label() const noexcept
	{
		return sub_label_ != nullptr;
	}

	bool
	has_modifier() const noexcept
	{
		return modifier_ != nullptr;
	}

	bool
	has_symbols() const noexcept
	{
		return has_add_label() || has_sub_label() || has_modifier();
	}

	inline jive_immediate_int
	offset() const noexcept
	{
		return offset_;
	}

	void
	set_offset(uint64_t offset) noexcept
	{
		offset_ = offset;
	}

	const jive::label *
	add_label() const noexcept
	{
		return add_label_;
	}

	void
	set_add_label(const jive::label * add_label) noexcept
	{
		add_label_ = add_label;
	}

	const jive::label *
	sub_label() const noexcept
	{
		return sub_label_;
	}

	void
	set_sub_label(const jive::label * sub_label) noexcept
	{
		sub_label_ = sub_label;
	}

	const void *
	modifier() const noexcept
	{
		return modifier_;
	}

	void
	set_modifier(const void * modifier) noexcept
	{
		modifier_ = modifier;
	}

private:
	jive_immediate_int offset_;
	const jive::label * add_label_;
	const jive::label * sub_label_;
	const void * modifier_;
};

/* immediate operator */

class immediate_op final : public nullary_op {
public:
	virtual
	~immediate_op() noexcept;

	inline
	immediate_op(const jive::immediate & value) noexcept
	: nullary_op(immtype())
	, value_(value)
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual std::string
	debug_string() const override;

	inline const jive::immediate &
	value() const noexcept
	{
		return value_;
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;

	static inline jive::output *
	create(
		jive::region * region,
		const jive::immediate & value)
	{
		jive::immediate_op op(value);
		return simple_node::create_normalized(region, op, {})[0];
	}

private:
	jive::immediate value_;
};

static inline bool
is_immediate_op(const jive::operation & op) noexcept
{
	return dynamic_cast<const jive::immediate_op*>(&op);
}

static inline bool
is_immediate_node(const jive::node * node) noexcept
{
	return is<immediate_op>(node);
}

}

#endif
