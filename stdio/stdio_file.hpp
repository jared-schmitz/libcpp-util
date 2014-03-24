#ifndef STDIO_FILE_HPP
#define STDIO_FILE_HPP

#include <cstdio>
#include <cstdlib>

class stdio_file {
private:
	FILE *F;
public:
	int getc();
	int putc(int c);

	void clearerr();
	int feof();
	int ferror();
	int fflush();
	int fgetc();
	int fputc(int c);

	size_t fread(void *ptr, size_t size, size_t n);
	size_t fwrite(const void *ptr, size_t size, size_t n);

	char *fgets(char *s, int n);
	int fputs(const char *s);

	int fseek(long offset, int whence);
	long ftell();
	void rewind();
	int fgetpos(fpos_t *pos);
	int fsetpos(fpos_t *pos);

	void setbuf(char *buf);
	void setbuffer(char *buf, size_t size);
	void setlinebuf();
	int setvbuf(char *buf, int mode, size_t size);

	int printf(const char* fmt, ...);
	int vprintf(const char* fmt, va_list ap);

	int scanf(const char *fmt, ...);
	int vscanf(const char *fmt, va_list ap);

	stdio_file *freopen(const char *path, const char *mode, stdio_file* f);

	int ungetc(int c);

#if _POSIX_C_SOURCE >= 1 || _XOPEN_SOURCE || _POSIX_SOURCE
	int fileno();
#endif
};
#endif
