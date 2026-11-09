/* Stands in for nothing: libHUI.so reaches libcef.so transitively, so libcef.so ends up last in the
   link map and every dlsym(RTLD_NEXT, "X") libcef.so does to find *libc's* X returns NULL (close
   crashes with brk, localtime_r deadlocks, ...). This library is a DT_NEEDED of libHUI.so placed
   after libcef.so, so those lookups land here and forward to the real libc implementation.
   Plain POSIX on purpose: no libc/compiler/arch ifdefs, no glibc version tokens. */

#include <dlfcn.h>
#include <malloc.h>
#include <netdb.h>
#include <stddef.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

extern "C" {

static int (*hui_libc_close)(int) = 0;
static int (*hui_libc_getaddrinfo)(const char*, const char*, const struct addrinfo*, struct addrinfo**) = 0;
static struct tm* (*hui_libc_localtime)(const time_t*) = 0;
static struct tm* (*hui_libc_localtime_r)(const time_t*, struct tm*) = 0;
static struct tm* (*hui_libc_localtime64)(const time_t*) = 0;
static struct tm* (*hui_libc_localtime64_r)(const time_t*, struct tm*) = 0;
static size_t (*hui_libc_malloc_usable_size)(void*) = 0;
static int (*hui_libc_memfd_create)(const char*, unsigned int) = 0;

__attribute__((constructor)) static void hui_localtime_shim_init(void) {
  hui_libc_close = (int (*)(int)) dlsym(RTLD_DEFAULT, "close");
  hui_libc_getaddrinfo = (int (*)(const char*, const char*, const struct addrinfo*, struct addrinfo**)) dlsym(RTLD_DEFAULT, "getaddrinfo");
  hui_libc_localtime = (struct tm* (*)(const time_t*)) dlsym(RTLD_DEFAULT, "localtime");
  hui_libc_localtime_r = (struct tm* (*)(const time_t*, struct tm*)) dlsym(RTLD_DEFAULT, "localtime_r");
  hui_libc_localtime64 = (struct tm* (*)(const time_t*)) dlsym(RTLD_DEFAULT, "localtime64");
  hui_libc_localtime64_r = (struct tm* (*)(const time_t*, struct tm*)) dlsym(RTLD_DEFAULT, "localtime64_r");
  hui_libc_malloc_usable_size = (size_t (*)(void*)) dlsym(RTLD_DEFAULT, "malloc_usable_size");
  hui_libc_memfd_create = (int (*)(const char*, unsigned int)) dlsym(RTLD_DEFAULT, "memfd_create");
}

int close(int fd) {
  return hui_libc_close ? hui_libc_close(fd) : (int) syscall(SYS_close, fd);
}

int getaddrinfo(const char* node, const char* service, const struct addrinfo* req, struct addrinfo** res) {
  return hui_libc_getaddrinfo ? hui_libc_getaddrinfo(node, service, req, res) : EAI_SYSTEM;
}

struct tm* localtime(const time_t* t) { return hui_libc_localtime ? hui_libc_localtime(t) : 0; }
struct tm* localtime_r(const time_t* t, struct tm* r) { return hui_libc_localtime_r ? hui_libc_localtime_r(t, r) : 0; }
struct tm* localtime64(const time_t* t) { return hui_libc_localtime64 ? hui_libc_localtime64(t) : 0; }
struct tm* localtime64_r(const time_t* t, struct tm* r) { return hui_libc_localtime64_r ? hui_libc_localtime64_r(t, r) : 0; }
size_t malloc_usable_size(void* p) { return hui_libc_malloc_usable_size ? hui_libc_malloc_usable_size(p) : 0; }
int memfd_create(const char* name, unsigned int flags) {
  return hui_libc_memfd_create ? hui_libc_memfd_create(name, flags) : (int) syscall(SYS_memfd_create, name, flags);
}

}
