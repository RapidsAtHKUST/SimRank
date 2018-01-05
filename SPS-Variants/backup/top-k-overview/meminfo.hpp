#include <sys/time.h>
#include <sys/resource.h>

void print_mem_info() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    printf("%.1lfM\n", usage.ru_maxrss / 1024.0);
}

double get_mem_info() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_maxrss / 1024.0;
}
