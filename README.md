---

# Linux Memory Performance & Page Fault Analysis

A systems-level engineering project to quantify the overhead of the Linux Virtual Memory subsystem. This suite measures the latency impact of **Page Faults** (Minor vs. Major) and **TLB (Translation Lookaside Buffer) efficiency** using hardware performance counters.

## 🎯 Project Objective
To move beyond high-level memory allocation (`malloc`) and investigate the mechanical cost of address translation, kernel-side fault handling, and the performance benefits of **Huge Pages** (2MB) over standard pages (4KB).

## 🛠️ Tech Stack & Tools
* **Language:** C++17 (Low-level `mmap` system calls)
* **Tracing:** `perf` (Linux profiling with hardware counters)
* **Stress Testing:** `stress-ng` (Simulating memory pressure/swap)
* **Build System:** Makefile
* **Environment:** Linux Kernel 5.x+ (Ubuntu/Debian)

---

## 📊 Experimental Results (512MB Workload)

The following data was captured on a live system comparing **Standard Pages** vs. **Huge Pages** under a sequential write workload.

| Metric | Standard (4KB) | Huge Pages (2MB) | Improvement |
| :--- | :--- | :--- | :--- |
| **Execution Time** | 0.2649s | 0.0758s | **~71% Faster** |
| **Minor Faults** | 131,461 | 390 | **99.7% Reduction** |
| **TLB Load Misses**| 687,023 | 14,507 | **97.8% Reduction** |
| **Kernel Time (sys)**| 0.278s | 0.077s | **~72% Reduction** |

### 🔍 Key Observations
1. **The "Fault Wall":** With 4KB pages, the kernel was interrupted **131,461 times** to map physical RAM. With Huge Pages, this dropped to **390**, shifting almost all processing time from "Kernel Space" back to "User Space."
2. **TLB Optimization:** Huge Pages allow a single TLB entry to cover 512x more memory, effectively eliminating the bottleneck of address translation.
3. **Memory Pressure:** Under simulated load (`stress-ng`), TLB misses spiked from **687k to 3.9M**, demonstrating how background processes compete for the CPU's MMU caches.

---

## 🚀 Getting Started

### 1. Build the Project
```bash
make
```

### 2. Configure the Kernel (Reserve Huge Pages)
```bash
make reserve-huge
```

### 3. Run a Baseline Test (4KB Pages)
```bash
perf stat -e dTLB-load-misses,minor-faults ./bin/workload 512 0 0
```

### 4. Run the Optimized Test (Huge Pages)
```bash
perf stat -e dTLB-load-misses,minor-faults ./bin/workload 512 0 1
```

---

## 📂 Project Structure
* `src/workload.cpp`: Core engine using `mmap` and `MAP_HUGETLB`.
* `scripts/run_suite.sh`: Automation script for consistent data collection.
* `Makefile`: Handles compilation and system cache clearing.
* `data/`: Stores `.csv` outputs from experimental runs.

---

## ⏭️ Future Phases
* [ ] **Phase 2:** Quantifying Major Faults (Disk I/O latency) during swap-heavy conditions.
* [ ] **Phase 3:** Analyzing Spatial Locality by comparing Sequential vs. Random access patterns.

---