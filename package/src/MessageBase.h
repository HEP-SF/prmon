#ifndef PRMON_MESSAGEBASE_H
#define PRMON_MESSAGEBASE_H 1

#include <map>

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

// Global sinks

static const std::shared_ptr<spdlog::sinks::stdout_color_sink_st> c_sink{
    std::make_shared<spdlog::sinks::stdout_color_sink_st>()};
static const std::shared_ptr<spdlog::sinks::basic_file_sink_st> f_sink{
    std::make_shared<spdlog::sinks::basic_file_sink_st>("prmon.log", true)};

// Map from monitor to logging level

extern bool invalid_level_option;
extern std::map<std::string, spdlog::level::level_enum> monitor_level;

// Processing command line switches

extern spdlog::level::level_enum global_logging_level;
void processLevel(std::string s);

class MessageBase {
  std::shared_ptr<spdlog::logger> logger;

 protected:
  spdlog::level::level_enum log_level;
  void log_init(const std::string& classname,
                const spdlog::level::level_enum& level = global_logging_level);

 public:
  void set_log_level(const spdlog::level::level_enum& level);
  void debug(const std::string& message);
  void info(const std::string& message);
  void warning(const std::string& message);
  void error(const std::string& message);
};

#endif  // PRMON_MESSAGEBASE_H
