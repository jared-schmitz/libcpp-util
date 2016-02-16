#include <exception>

// Pull the simple boolean logic into a base class to avoid template bloat.
class scope_guard_base {
private:
	scope_guard_base(const scope_guard_base& other) = delete;
	scope_guard_base& operator=(const scope_guard_base& other) = delete;

protected:
	bool should_fire;

	~scope_guard_base() = default;
	explicit scope_guard_base() : should_fire(true) {
	}
	explicit scope_guard_base(scope_guard_base&& other)
		: should_fire(other.should_fire) {
		other.disarm();
	}

public:
	void disarm() {
		should_fire = false;
	}
};

// If not explicitly dismissed, the passed function will be called if the
// supplied predicate is true.
template <typename ExitFn, typename WhenPred>
class scope_guard_policy_base : scope_guard_base {
private:
	ExitFn exitFn;

public:
	// Optionally allow a function to be called on entry too.
	template <typename EnterFn>
	explicit scope_guard_policy_base(EnterFn enterFn, ExitFn exitFn)
		: exitFn(exitFn) {
		enterFn();
	}

	explicit scope_guard_policy_base(ExitFn exitFn) : exitFn(exitFn) {
	}

	~scope_guard_policy_base() {
		if (should_fire && WhenPred())
			exitFn();
	}
};

// Tiny policy predicates to build the different scope guard variants.
namespace detail {
struct CallAlways {
	bool operator()() const {
		return true;
	}
};
struct CallOnUnwind {
	bool operator()() const {
		return std::uncaught_exception();
	}
};
struct CallOnEndOfScope {
	bool operator()() const {
		return !std::uncaught_exception();
	}
};
}

// For when the function is to be unconditionally called.
template <typename ExitFn>
using scope_guard = scope_guard_policy_base<ExitFn, detail::CallAlways>;

// For when the function is to be called on normal end of scope.
template <typename ExitFn>
using success_scope_guard =
    scope_guard_policy_base<ExitFn, detail::CallOnEndOfScope>;

// For when the function is only to be called if the scope is exited
// exceptionally, during stack unwinding.
template <typename ExitFn>
using unwind_scope_guard =
    scope_guard_policy_base<ExitFn, detail::CallOnUnwind>;

// Convenience (sort of?) class that allows for one scope guard that will call
// one of two functions depending on whether the scope was exited normally or by
// exception unwinding.
template <typename NormalFn, typename ExceptFn>
class success_unwind_scope_guard : success_scope_guard<NormalFn>,
				   unwind_scope_guard<ExceptFn> {
private:
	// Hide disarm because it has become ambiguous, because we have created
	// the multiple inheritance diamond. Note that we do want two booleans
	// though.
	using normal = success_scope_guard<NormalFn>;
	using unwind = unwind_scope_guard<ExceptFn>;
	using success_scope_guard<NormalFn>::disarm;
	using unwind_scope_guard<ExceptFn>::disarm;
public:
	template <typename EnterFn>
	success_unwind_scope_guard(EnterFn enterFn, NormalFn normalFn,
				   ExceptFn exceptFn) : normal(normalFn), unwind(exceptFn) {
		enterFn();
	}
	success_unwind_scope_guard(NormalFn normalFn, ExceptFn exceptFn)
		: normal(normalFn), unwind(exceptFn) {}

	void disarm_success() {
		success_scope_guard<NormalFn>::disarm();
	}
	void disarm_exceptional() {
		unwind_scope_guard<ExceptFn>::disarm();
	}

	void disarm() {
		disarm_success();
		disarm_exceptional();
	}
};
