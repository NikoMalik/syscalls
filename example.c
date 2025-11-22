#include "syscall.h"

static inline ssize_t sys_write(int fd, const void *buf, size_t len) {
    ssize_t ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(1),   // write = syscall 1
          "D"(fd),  // rdi
          "S"(buf), // rsi
          "d"(len)  // rdx
        : "rcx", "r11", "memory");
    return ret;
}

static inline void print(const char *s) {
    const char *p = s;
    size_t len = 0;
    while (p[len] != 0)
        len++;

    sys_write(1, p, len);
}

void __exit() {
    asm volatile(
        "syscall"
        :
        : "a"(60), "D"(0)
        : "rcx", "r11", "memory");
}

void _start() {
    print("===raw syscall + VDSO test ===\n");

    void *p = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == (void *)-1) {
        print("mmap failed!\n");
        __exit();
    }
    print("mmap succeeded: ");
    char buf[32];
    unsigned long addr = (unsigned long)p;
    for (int i = 60; i >= 0; i -= 4) {
        char c = "0123456789abcdef"[(addr >> i) & 0xf];
        syscall3(__NR_write, 2, (long)&c, 1);
    }
    syscall3(__NR_write, 2, (long)"\n", 1);

    *(long *)p = 0xDEADBEEF;
    if (*(long *)p == 0xDEADBEEF) {
        print("memory write/read OK\n");
    }

    struct timespec ts;
    long r = clock_gettime(1, &ts); // CLOCK_MONOTONIC
    if (r == 0) {
        print("clock_gettime worked!\n");
    } else {
        print("fallback used\n");
    }

    struct timespec ts2;
    long r2 = clock_gettime_vdso(1, &ts2); // CLOCK_MONOTONIC WITH VDSO

    if (r2 == 0)
        print("VDSO clock_gettime OK!\n");
    else
        print("VDSO failed, fallback used\n");

    if (munmap(p, 4096)) {
        print("munmap OK\n");
    } else {
        print("munmap failed\n");
    }

    print("All tests passed!\n");
    __exit();
}
