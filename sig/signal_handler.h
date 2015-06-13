//============================================================================
//                                  libcpp-util
//                   A simple odds-n-ends library for C++11
//
//         Licensed under modified BSD license. See LICENSE for details.
//============================================================================

#ifndef LIBCPP_UTIL_SIGNAL_HANDLER
#define LIBCPP_UTIL_SIGNAL_HANDLER

#ifdef __GNU__
// FIXME: No way to know if this header exists.
#include <execinfo.h>
#define HAVE_BACKTRACE
#endif

#include <cstring>
#include <cstdio>
#include <signal.h>
#include <unistd.h>
#include <cstdint>
#include <cassert>

namespace {
int ascii_to_hex(uintptr_t addr, char *out, int out_len) {
	// TODO: Replace with guaranteed async-safe impl
	return snprintf(out, out_len, "0x%016tX", addr);
}

char *strcpy_end(char *dest, const char *src) {
	while ((*dest++ = *src++))
		/* Nada */;
	return dest;
}

char *strcat_end(char *dest, const char *src) {
	// Go to end of string
	while (*dest)
		dest++;
	return strcpy_end(dest, src);
}
}

// Simple class to gather up a bunch of information that the signal handler will
// need, but can't safely during do during the handler execution. So this does
// it upon signal registration. The obvious consequence is that we cannot use it
// to register new signals inside signal handlers.
class lazy_sig_info {
private:
	static constexpr int max_signal = 64;
	static constexpr int max_name_len = 64;

public:
	using signal_name = char[max_name_len];
	static signal_name &get(int i) {
		static signal_name arr[max_signal];
		assert(i < max_signal && "Signal number too large");
		return arr[i];
	}

	template <int i>
	static void put(const char *name) {
		static_assert(i < max_signal, "Signal number too large");
		put(i, name);
	}
	static void put(int i, const char *name) {
		assert(i < max_signal && "Signal number too large");
		strncpy(get(i), name, max_name_len);
	}
	static void register_signal(int signo) {
		put(signo, strsignal(signo));
	}
};

// TODO: Sweep this up. Use a sane method to build up the string. Remove the
// async-unsafe functions. Easy since most of them can be replaced with a simple
// while loop and don't need to be fast.
inline void god_signal_handler(int signo, siginfo_t *info, void *uctx) {
#if defined SIGSTKSZ && defined MINSIGSTKSZ
	char print_scratch_space[SIGSTKSZ - MINSIGSTKSZ + 128];
#else
	char print_scratch_space[512];
#endif
	char *write_ptr = print_scratch_space;

	// Grab pretty-printed signal information.
	write_ptr = strcpy_end(write_ptr, lazy_sig_info::get(signo));
	*write_ptr++ = ' ';

	// Try to write to stderr.
	int fd = fileno(stderr);
	if (fd == -1)
		fd = STDERR_FILENO;

	// For SIGSEGV, print the memory address we tried to access, then the
	// address of the instruction which referenced that address.
	// For SIGABRT, just prints the same value twice. ...FIXME? =)
	switch (signo) {
	case SIGABRT:
	case SIGSEGV:
		ascii_to_hex((uintptr_t)info->si_addr, write_ptr,
			     print_scratch_space + sizeof(print_scratch_space) -
				 write_ptr);
#ifdef __x86_64__
		// Dump the instruction that caused the signal
		struct ucontext *uc = (struct ucontext *)uctx;
		long insn_addr = uc->uc_mcontext.gregs[REG_RIP];
		write_ptr = print_scratch_space + strlen(print_scratch_space);
		write_ptr = strcat_end(write_ptr, " at instruction ");
		ascii_to_hex(insn_addr, write_ptr,
			     print_scratch_space + sizeof(print_scratch_space) -
				 write_ptr);
#endif
	}

	strcat(write_ptr, "\n");

	// FIXME: We'd want this to be non-blocking, in case we get a stop
	// signal if we don't have a terminal to print to.
	write(fd, print_scratch_space, strlen(print_scratch_space));

#ifdef HAVE_BACKTRACE
	// FIXME: The backtrace is helpful but uses more stack space than the
	// average signal handler, so at least don't do it if we're not on the
	// program stack. Also, ...don't do it at all because it's not
	// async-safe.
	stack_t old_stack;
	sigaltstack(nullptr, &old_stack);
	if (old_stack.ss_flags != SS_ONSTACK) {
		void *stack_frames[64];
		int nframes = backtrace(stack_frames, 64);
		backtrace_symbols_fd(stack_frames, nframes, fd);
	}
#endif
}

inline void god_reraise_handler(int signo, siginfo_t *info, void *uctx) {
	god_signal_handler(signo, info, uctx);

	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SIG_DFL;
	sigaction(signo, &sa, nullptr);
	raise(signo);
}

inline int register_to_god_handler(int signo, struct sigaction *new_sa,
			    bool reraise = true) {

	// Modify the signal action to call the god handler, but preserving the
	// other flags and whatnot.
	new_sa->sa_sigaction =
	    reraise ? god_reraise_handler : god_signal_handler;
	new_sa->sa_flags |= SA_SIGINFO;

	// Make sure that the string for the signal is available to us.
	lazy_sig_info::register_signal(signo);

	// FIXME: This needs error checking.
	if (signo == SIGSEGV) {
		// If there's no alternate signal stack, set
		stack_t new_stack, old_stack;
		sigaltstack(nullptr, &old_stack);
		if (old_stack.ss_flags == SS_DISABLE) {
			static char alt_stack[SIGSTKSZ * 2];
			new_stack.ss_sp = alt_stack;
			new_stack.ss_size = sizeof(alt_stack);
			new_stack.ss_flags = 0;
			sigaltstack(&new_stack, nullptr);
		}
		new_sa->sa_flags |= SA_ONSTACK;
	}

	if (sigaction(signo, new_sa, nullptr) == -1)
		return -1;

	return 0;
}
#endif
