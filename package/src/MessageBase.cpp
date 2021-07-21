#include "MessageBase.h"

#include <cctype>
#include <map>

#include "Imonitor.h"
#include "registry.h"
#include "spdlog/spdlog.h"

bool invalid_level_option = false;
std::map<std::string, spdlog::level::level_enum> monitor_level;
spdlog::level::level_enum global_logging_level = spdlog::level::warn;

void processLevel(std::string s) {
  for (auto& ch : s) {
    ch = std::tolower(ch);
  }
  if (s.find(':') == std::string::npos) {
    // No ':' in option -> set global level
    spdlog::level::level_enum get_level_enum = spdlog::level::from_str(s);
    if (get_level_enum == 6) {
      // Invalid name
      spdlog::error("Invalid level name " + s);
      invalid_level_option = true;
    } else
      global_logging_level = get_level_enum;
  } else {
    size_t split_index = s.find(':');
    std::string monitor_name = s.substr(0, split_index);
    std::string level_name =
        s.substr(split_index + 1, s.size() - (split_index + 1));

    // Check validity of monitor name
    bool valid_monitor = false;
    auto monitors = registry::Registry<Imonitor>::list_registered();
    for (const auto& monitor : monitors) {
      if (monitor == monitor_name) {
        valid_monitor = true;
        break;
      }
    }
    if (!valid_monitor) {
      spdlog::error("Invalid monitor name " + monitor_name +
                    " for logging level");
      invalid_level_option = true;
      return;
    }
    // Check validity of level name
    spdlog::level::level_enum get_level_enum =
        spdlog::level::from_str(level_name);
    if (get_level_enum == 6) {
      // Invalid name
      spdlog::error("Invalid level name " + level_name);
      invalid_level_option = true;
      return;
    }

    monitor_level[monitor_name] = get_level_enum;
  }
}

void MessageBase::log_init(const std::string& classname,
                           const spdlog::level::level_enum& level) {
  // Initialise sink list
  spdlog::sinks_init_list s_list = {c_sink, f_sink};

  // Use the sink list to create multi sink logger
  logger =
      std::make_shared<spdlog::logger>(classname, s_list.begin(), s_list.end());
  if (monitor_level.find(classname) != monitor_level.end()) {
    log_level = monitor_level[classname];
  } else {
    log_level = level;
  }
  set_log_level(log_level);
  logger->flush_on(log_level);
  spdlog::register_logger(logger);
  info(classname + " logger initialised!");
}

void MessageBase::set_log_level(const spdlog::level::level_enum& level) {
  log_level = level;
  logger->set_level(level);
  logger->flush_on(level);
}
void MessageBase::debug(const std::string& message) { logger->debug(message); }
void MessageBase::info(const std::string& message) { logger->info(message); }
void MessageBase::warning(const std::string& message) { logger->warn(message); }
void MessageBase::error(const std::string& message) { logger->error(message); }
