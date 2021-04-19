#include <malloc.h>
#include <chrono>

#include "libUtils/Logger.h"
#include "libUtils/CommonUtils.h"

using namespace std;

void CommonUtils ::ReleaseSTLMemoryCache() {
  auto startTime = chrono::high_resolution_clock::now();
  malloc_trim(0);
  auto endTime = chrono::high_resolution_clock::now();
  auto ms_int = chrono::duration_cast<chrono::milliseconds>(endTime - startTime);
  LOG_GENERAL(INFO, "Time diff malloc_trim()" << ms_int.count() << "ms");
}
