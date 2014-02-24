
template <typename T, class... Fallbacks>
class allocator_chain : allocator_chain<T, Fallbacks...> {
	
};

template <typename T>
class allocator_chain<T> : public out_of_luck_allocator<T> {};
