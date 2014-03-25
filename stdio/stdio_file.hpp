#ifndef STDIO_FILE_HPP
#define STDIO_FILE_HPP

#include <cstdio>
#include <cstdlib>
#include <cstdarg>

class stdio_file {
private:
	FILE *F;

public:
	// Ctors/dtor
	// Assignment/move
	FILE *get_file() {
		return F;
	}
	int getc() {
		// Don't use getc because it might be a macro and we want to
		// qualify the call.
		return fgetc();
	}
	int putc(int c) {
		return ::putc(c, get_file());
	}

	void clearerr() {
		::clearerr(get_file());
	}
	int feof() {
		return ::feof(get_file());
	}
	int ferror() {
		return ::ferror(get_file());
	}
	int fflush() {
		return ::fflush(get_file());
	}
	int fgetc() {
		return ::fgetc(get_file());
	}
	int fputc(int c) {
		return ::fputc(c, get_file());
	}

	size_t fread(void *ptr, size_t size, size_t n) {
		return ::fread(ptr, size, n, get_file());
	}
	size_t fwrite(const void *ptr, size_t size, size_t n) {
		return ::fwrite(ptr, size, n, get_file());
	}

	char *fgets(char *s, int n) {
		return ::fgets(s, n, get_file());
	}
	int fputs(const char *s) {
		return ::fputs(s, get_file());
	}

	int fseek(long offset, int whence) {
		return ::fseek(get_file(), offset, whence);
	}
	long ftell() {
		return ::ftell(get_file());
	}
	void rewind() {
		::rewind(get_file());
	}
	int fgetpos(fpos_t *pos) {
		return ::fgetpos(get_file(), pos);
	}
	int fsetpos(fpos_t *pos) {
		return ::fsetpos(get_file(), pos);
	}

	void setbuf(char *buf) {
		::setbuf(get_file(), buf);
	}
	void setbuffer(char *buf, size_t size) {
		::setbuffer(get_file(), buf, size);
	}
	void setlinebuf() {
		::setlinebuf(get_file());
	}
	int setvbuf(char *buf, int mode, size_t size) {
		return ::setvbuf(get_file(), buf, mode, size);
	}

	int printf(const char *fmt, ...) {
		va_list ap;
		va_start(ap, fmt);
		int ret = fprintf(get_file(), fmt, ap);
		va_end(ap);
		return ret;
	}
	int vprintf(const char *fmt, va_list ap) {
		return vfprintf(get_file(), fmt, ap);
	}

	int scanf(const char *fmt, ...) {
		va_list ap;
		va_start(ap, fmt);
		int ret = fscanf(get_file(), fmt, ap);
		va_end(ap);
		return ret;
	}
	int vscanf(const char *fmt, va_list ap) {
		return vfscanf(get_file(), fmt, ap);
	}

	stdio_file *freopen(const char *path, const char *mode, stdio_file *f);

	int ungetc(int c) {
		return ::ungetc(c, get_file());
	}

#if _POSIX_C_SOURCE >= 1 || _XOPEN_SOURCE || _POSIX_SOURCE
	int fileno() {
		return ::fileno(get_file());
	}
#endif
};
#endif
