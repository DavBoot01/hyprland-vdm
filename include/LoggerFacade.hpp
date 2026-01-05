
#pragma once
#include <string_view>
#include <utility>
#include <optional>
#include <hyprutils/cli/Logger.hpp>
namespace CLI = Hyprutils::CLI;

#define APPLOG_USE_SCOPED_ENUM
#include <hyprland/src/debug/log/Logger.hpp>

namespace AppLog {

    // ---- Selettore grafia dei livelli ----
    // Definisci una di queste macro da CMake (vedi snippet più sotto):
    //  - APPLOG_USE_LOG_PREFIX    -> livelli come CLI::LOG_INFO
    //  - APPLOG_USE_SCOPED_ENUM   -> livelli come CLI::eLogLevel::INFO
    #if defined(APPLOG_USE_LOG_PREFIX)
        constexpr auto LVL_TRACE = CLI::LOG_TRACE;
        constexpr auto LVL_INFO  = CLI::LOG_INFO;
        constexpr auto LVL_WARN  = CLI::LOG_WARN;
        constexpr auto LVL_ERR   = CLI::LOG_ERR;
    #elif defined(APPLOG_USE_SCOPED_ENUM)
        constexpr auto LVL_TRACE = CLI::eLogLevel::LOG_TRACE;
        constexpr auto LVL_INFO  = CLI::eLogLevel::LOG_DEBUG;
        constexpr auto LVL_WARN  = CLI::eLogLevel::LOG_WARN;
        constexpr auto LVL_ERR   = CLI::eLogLevel::LOG_ERR;
    #else
        // Fallback: non-scoped senza LOG_
        constexpr auto LVL_TRACE = CLI::TRACE;
        constexpr auto LVL_INFO  = CLI::INFO;
        constexpr auto LVL_WARN  = CLI::WARN;
        constexpr auto LVL_ERR   = CLI::ERR;
    #endif

    using UnderlyingLevel = decltype(LVL_INFO);

       
    // ---- Levels wrapper ---
    enum class LogLevel { Trace, Info, Warn, Error };

    // Mapping -> HU levels
    inline CLI::eLogLevel toUnderlying(LogLevel lvl) noexcept {
        switch (lvl) {
            case LogLevel::Trace: return LVL_TRACE;
            case LogLevel::Info:  return LVL_INFO;
            case LogLevel::Warn:  return LVL_WARN;
            case LogLevel::Error: return LVL_ERR;
        }
        return CLI::LOG_DEBUG;
    }

    // ---- Logger facade ----
    struct Logger {
        void log(CLI::eLogLevel level, 
             const std::string_view& msg, 
                 std::optional<std::string_view> tagOverride = std::nullopt,
                 std::optional<std::string_view> category     = std::nullopt
        );
    };

    Logger& getLogger();

    // ---- Initialization and configuration ----
    void initLogging(bool toStdout = true, bool colored = true, const char* filePath = nullptr, const std::string_view& tag = "");

    // ---- Minimum level configuration ----
    void setMinLevel(LogLevel lvl);
    LogLevel getMinLevel();

    inline void log(LogLevel lvl, const std::string_view& msg, const std::string_view& tag = "") {
        if (lvl < getMinLevel()) return;
        const auto tagOpt = tag.empty() ? std::optional<std::string_view>{} : std::make_optional(tag);
        getLogger().log(toUnderlying(lvl), msg, tagOpt);
    }

    inline void logWithCategory(LogLevel lvl, const std::string_view& category, const std::string_view& msg, const std::string_view& tag = "") {
        if (lvl < getMinLevel()) return;
        const auto tagOpt = tag.empty() ? std::optional<std::string_view>{} : std::make_optional(tag);
        const auto categoryOpt = category.empty() ? std::optional<std::string_view>{} : std::make_optional(category);
        getLogger().log(toUnderlying(lvl), msg, tagOpt, categoryOpt);

    }

    // Convenience helpers per level
    inline void logTrace(const std::string_view& msg, const std::string_view& tag = "") { log(LogLevel::Trace, msg, tag); }
    inline void logInfo(const std::string_view& msg, const std::string_view& tag = "") { log(LogLevel::Info, msg, tag); }
    inline void logWarn(const std::string_view& msg, const std::string_view& tag = "") { log(LogLevel::Warn, msg, tag); }
    inline void logError(const std::string_view& msg, const std::string_view& tag = "") { log(LogLevel::Error, msg, tag); }

    inline void logTrace(const std::string_view& category, const std::string_view& msg, const std::string_view& tag = "") { logWithCategory(LogLevel::Trace, category, msg, tag); }
    inline void logInfo(const std::string_view& category, const std::string_view& msg, const std::string_view& tag = "") { logWithCategory(LogLevel::Info, category, msg, tag); }
    inline void logWarn(const std::string_view& category, const std::string_view& msg, const std::string_view& tag = "") { logWithCategory(LogLevel::Warn, category, msg, tag); }
    inline void logError(const std::string_view& category, const std::string_view& msg, const std::string_view& tag = "") { logWithCategory(LogLevel::Error, category, msg, tag); }


    // ---- Macro con contesto (opzionale: usa std::source_location) ----
    // inline void logCtx(LogLevel lvl, std::string_view msg,
    //                std::string_view tag = "",
    //                const std::source_location& loc = std::source_location::current()) {
    // if (lvl < getMinLevel()) return;
    // const auto ctx = std::format("{}:{} {}", loc.file_name(), loc.line(), msg);
    // getLogger().log(toUnderlying(lvl),
    //                 ctx,
    //                 tag.empty() ? std::nullopt : std::make_optional(tag));
    // }

} // Namespace AppLog


// With compile-time context
#define APPLOG_WITH_CTX(LVL, MSG) \
    AppLog::getLogger().log( \
        AppLog::toUnderlying(LVL), \
        std::string_view{MSG} \
    )

// More detailed - file/line/function in the message
#define APPLOG_CTX(LVL, MSG) \
    AppLog::getLogger().log( \
        AppLog::toUnderlying(LVL), \
        std::string_view{ \
            MSG \
        } \
    )
