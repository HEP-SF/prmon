// Generic header for prmon utilities
//

#ifndef PRMON_UTILS_H
#define PRMON_UTILS_H 1

#include <unistd.h>

namespace prmon {
  // For conversion from second <-> clock ticks
  const static unsigned long clock_ticks = sysconf(_SC_CLK_TCK);
  const static float inv_clock_ticks = 1. / sysconf(_SC_CLK_TCK);

  // These constants define where in the stat entry from proc
  // we find the parameters of interest
  const size_t utime_pos = 13;
  const size_t stime_pos = 14;
  const size_t cutime_pos = 15;
  const size_t cstime_pos = 16;
  const size_t stat_cpu_read_limit = 16;
  const size_t uptime_pos = 21;

  // Default parameter lists for monitor classes
  const static std::vector<std::string> default_cpu_params{"utime", "stime"};
  const static std::vector<std::string> default_network_if_params{
      "rx_bytes", "rx_packets", "tx_bytes", "tx_packets"};
  const static std::vector<std::string> default_wall_params{"wtime"};
  const static std::vector<std::string> default_memory_params{"vmem", "pss", "rss", "swap"};
}

#endif  // PRMON_UTILS_H
