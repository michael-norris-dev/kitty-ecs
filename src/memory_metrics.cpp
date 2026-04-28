#include "memory_metrics.hpp"
#include <cstdlib>
#include <iostream>

// This is the actual global instance in memory
AllocationMetrics g_Metrics;

void* operator new(size_t size) {
  g_Metrics.TotalAllocated += size;
  return malloc(size);
}

void operator delete(void* memory, size_t size) noexcept {
  g_Metrics.TotalFreed += size;
  free(memory);
}
