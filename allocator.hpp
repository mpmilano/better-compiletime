#pragma once
#include <cassert>
#include <utility>
#include <ostream>
#include "array.hpp"
#include "allocated_ref.hpp"
#include "utils.hpp"

namespace compile_time {

template<std::size_t size, typename T> struct SingleAllocator{
	const std::size_t allocator_index;
	array<T,size> data;
	bool open_slots[size];

	template<typename Allocator>
	constexpr SingleAllocator(SingleAllocator&& o, Allocator& )
		:allocator_index(Allocator::template get_allocator_index<SingleAllocator>()),
		data(std::move(o.data)),
		open_slots{false} {
		//initialize data as copy (if that's a thing)
		for (auto i = 0u; i < size; ++i){
			open_slots[i] = o.open_slots[i];
		}
	}
	template<typename Allocator>
	constexpr SingleAllocator(const Allocator&)
	:allocator_index(Allocator::template get_allocator_index<SingleAllocator>()),
	open_slots{true} {
		for (auto& b : open_slots){
			b = true;
		}
	}

private:
	constexpr std::size_t _allocate(){
		for (auto i = 0u; i < size; ++i){
			if (open_slots[i]){
				open_slots[i] = false;
				return i;
			}
		}
		//this will always assert false;
		//gcc just gets upset if I do that directly,
		//and clang gets upset if I don't. 
		if (!open_slots[3]) {assert(false && "out of memory"); return 0;}
		else return 0;
	}
public:
	constexpr auto allocate(){
		return allocated_ref<T> {*this};
	}

	constexpr void free (std::size_t i){
		open_slots[i] = true;
	}

	constexpr void free (allocated_ref<T>& o){
		return o.free(*this);
	}

	friend struct allocated_ref<T>;
};

template<typename T> template<std::size_t s>
constexpr allocated_ref<T>::allocated_ref(SingleAllocator<s,T>& sa)
:indx(sa._allocate() + 1)
{}

template<typename T> template<std::size_t s>
constexpr void allocated_ref<T>::free(SingleAllocator<s,T>& sa)
{
	sa.free(indx);
}

template<typename T, std::size_t s>
constexpr erased_ref::erased_ref(SingleAllocator<s,T>& sa)
:indx(sa._allocate() + 1)
{}

template<typename T, std::size_t s>
constexpr void erased_ref::free(SingleAllocator<s,T>& sa)
{
	sa.free(indx);
}

template<std::size_t s, typename Top, typename... Subs> struct Allocator
	: public SingleAllocator<s,Subs>...{

	Top top;
	static constexpr const std::size_t size = s;

	constexpr Allocator():SingleAllocator<s,Subs>(*this)...{}

	constexpr Allocator(Allocator&& o)
		:SingleAllocator<s,Subs>(std::move(o),*this)...,
		top(std::move(o.top)){}
	
	template<typename SA>
	static constexpr std::size_t get_allocator_index(){
		return index_of<SA, SingleAllocator<s,Subs>...>;
	}

	template<typename T>
		constexpr SingleAllocator<s,T>& get(){
		return *this;
	}

	template<typename sub>
	constexpr SingleAllocator<s,sub> & as_single_allocator() {
		return *this;
	}

	template<typename sub>
	constexpr const SingleAllocator<s,sub> & as_single_allocator() const {
		return *this;
	}

	template<typename T>
		constexpr const SingleAllocator<s,T>& get() const {
		return *this;
	}

	template<typename T> constexpr allocated_ref<T> allocate(){
		return get<T>().allocate();
	}

	template<typename T> constexpr void free(std::size_t index){
		get<T>().free(index);
	}

	template<typename T> constexpr void free(allocated_ref<T> ptr){
		get<T>().free(ptr);
	}
	
};

#define $(a) a.get(allocator)

template<typename T, std::size_t s, typename Top, typename... Subs>
std::ostream& print(std::ostream& o, const allocated_ref<T>& ptr, const Allocator<s,Top,Subs...> &_allocator){
	const SingleAllocator<s,T> &allocator = _allocator;
	o << "&(";
	print(o,ptr.get(allocator),allocator);
	return o << ")";
}

template<typename T, std::size_t s>
std::ostream& print(std::ostream& o, const allocated_ref<T>& ptr, const SingleAllocator<s,T> &allocator){
	o << "&(";
	print(o,ptr.get(allocator),allocator);
	return o << ")";
}

template<typename T, std::size_t s, typename Top, typename... Subs>
std::ostream& pretty_print(std::ostream& o, const allocated_ref<T>& ptr, const Allocator<s,Top,Subs...> &_allocator){
	const SingleAllocator<s,T> &allocator = _allocator;
	pretty_print(o,ptr.get(allocator),allocator);
	return o;
}

template<typename T, std::size_t s>
std::ostream& pretty_print(std::ostream& o, const allocated_ref<T>& ptr, const SingleAllocator<s,T> &allocator){
	pretty_print(o,ptr.get(allocator),allocator);
	return o;
}
}