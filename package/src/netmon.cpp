// Copyright (C) CERN, 2018

#include "netmon.h"

#include <dirent.h>
#include <cstring>
#include <memory>

const static std::vector<std::string> default_if_params{
    "rx_bytes", "rx_packets", "tx_bytes", "tx_packets"};

// Constructor; uses RAII pattern to open all monitored
// network device streams and to take initial values
// to the monitor relative differences
netmon::netmon(std::vector<std::string> netdevs)
    : interface_params{default_if_params}, network_if_streams{} {
  if (netdevs.size() == 0) {
    monitored_netdevs = get_all_network_devs();
  } else {
    monitored_netdevs = netdevs;
  }
  open_interface_streams();

  read_raw_network_stats(network_stats_start);
}

// Get all available network devices
// This is all a bit yuk, using C style directory
// parsing. From C++17 we should use the filesystem
// library, but only when we decide it's reasonable
// to no longer support older compilers.
std::vector<std::string> const netmon::get_all_network_devs() {
  std::vector<std::string> devices{};
  DIR* d;
  struct dirent* dir;
  const char* netdir = "/sys/class/net";
  d = opendir(netdir);
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      if (!(!std::strcmp(dir->d_name, ".") || !std::strcmp(dir->d_name, "..")))
        devices.push_back(dir->d_name);
    }
    closedir(d);
  } else {
    std::cerr << "Failed to open " << netdir
              << " to get list of network devices. "
              << "No network data will be available" << std::endl;
  }
  return devices;
}

// Opens an ifstream for all monitored network parameters
// Stored as unique_ptrs to ensure they are closed when
// the instance is destroyed
void netmon::open_interface_streams() {
  for (const auto& if_param : interface_params) {
    for (const auto& device : monitored_netdevs) {
      // make_unique would be better, but mandates C++14
      std::unique_ptr<std::ifstream> u_strm_ptr =
          std::unique_ptr<std::ifstream>(
              new std::ifstream(get_sys_filename(device, if_param)));
      network_if_streams[if_param][device] = std::move(u_strm_ptr);
    }
  }
}

// Read raw stat values
void netmon::read_raw_network_stats(
    std::unordered_map<std::string, unsigned long long>& stats) {
  for (const auto& if_param : interface_params) {
    unsigned long long value_read{};
    stats[if_param] = 0;
    for (const auto& device : monitored_netdevs) {
      network_if_streams[if_param][device]->seekg(0);
      *network_if_streams[if_param][device] >> value_read;
      stats[if_param] += value_read;
    }
  }
}

// Read stats and return relative increases in counters
void netmon::read_network_stats(
    std::unordered_map<std::string, unsigned long long>& stats) {
  read_raw_network_stats(stats);
  for (const auto& if_param : interface_params)
    stats[if_param] -= network_stats_start[if_param];
}
