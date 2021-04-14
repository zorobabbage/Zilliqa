#ifndef ZILLIQA_SRC_LIBUTILS_MEMORY_STATS_H_
#define ZILLIQA_SRC_LIBUTILS_MEMORY_STATS_H_
#include <string>
void DisplayVirtualMemoryStats();
uint64_t DisplayPhysicalMemoryStats(const std::string& str, const std::uint64_t& epochNo, uint64_t  startMem = 0);

#endif