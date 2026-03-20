#include <iostream>
#include <sys/mman.h>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <cstdlib>

void* allocate_memory(size_t bytes, bool use_hugepages) {
    int flags = MAP_ANONYMOUS | MAP_PRIVATE;

    if (use_hugepages) {
        flags |= MAP_HUGETLB; // Use default huge page size (2MB usually)
        // Align size to 2MB
        size_t huge_size = 2 * 1024 * 1024;
        bytes = ((bytes + huge_size - 1) / huge_size) * huge_size;
    }

    void* ptr = mmap(nullptr, bytes, PROT_READ | PROT_WRITE, flags, -1, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap failed (check hugepages reservation)");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void execute_workload(size_t total_mb, bool is_random, bool use_hugepages) {
    size_t bytes = total_mb * 1024 * 1024;
    char* buffer = static_cast<char*>(allocate_memory(bytes, use_hugepages));

    // Determine stride: standard page vs huge page
    size_t stride = use_hugepages ? (2 * 1024 * 1024) : 4096;
    size_t num_touches = bytes / stride;

    // Generate page offsets
    std::vector<size_t> offsets(num_touches);
    for (size_t i = 0; i < num_touches; ++i) offsets[i] = i * stride;

    if (is_random) {
        std::shuffle(offsets.begin(), offsets.end(), std::mt19937{std::random_device{}()});
    }

    auto start = std::chrono::high_resolution_clock::now();
    for (size_t offset : offsets) {
        buffer[offset] = 'A'; // Trigger the page fault
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << "Workload complete: "
              << total_mb << " MB, "
              << (is_random ? "Random" : "Sequential") << ", "
              << (use_hugepages ? "HugePages" : "Normal") << "\n";
    std::cout << "Elapsed time: " << elapsed.count() << " seconds\n";

    if (munmap(buffer, bytes) != 0) {
        perror("munmap failed");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "Usage: ./workload <MB> <random(0/1)> <hugepages(0/1)>\n";
        return EXIT_FAILURE;
    }

    size_t mb = std::stoull(argv[1]);
    bool is_random = std::stoi(argv[2]) == 1;
    bool use_hugepages = std::stoi(argv[3]) == 1;

    execute_workload(mb, is_random, use_hugepages);
    return EXIT_SUCCESS;
}
