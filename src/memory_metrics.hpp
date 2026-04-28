#pragma once
#include <cstddef>
#include <cstdint>

struct AllocationMetrics {
  uint32_t TotalAllocated = 0;
  uint32_t TotalFreed = 0;

  uint32_t CurrentUsage() const { return TotalAllocated - TotalFreed; }
};

// 'extern' tells the compiler: "This variable exists somewhere else, just trust me."
extern AllocationMetrics g_Metrics; 

// You don't actually need to declare the global new/delete here 
// for them to work, but it's good practice for clarity.
void* operator new(size_t size);
void operator delete(void* memory, size_t size) noexcept;
