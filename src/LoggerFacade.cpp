
#include "LoggerFacade.hpp"

#include <atomic>
#include <mutex>
#include <filesystem>
#include <string>
#include <hyprland/src/debug/log/Logger.hpp>
namespace {
    // Real HU logger instance (internally thread-safe)
    CLI::CLogger gHuLogger;

    // Wrapper's state
    std::string             gDefaultTag;
    std::atomic<AppLog::LogLevel> gMinLevel{AppLog::LogLevel::Info}; // default: Info
    std::once_flag          gInitOnce;

    constexpr size_t BRACKETS_OVERHEAD = 3; // '[' + ']' + space

    void appendBracketed(std::string& target, const std::string_view& value) {
        target.push_back('[');
        target.append(value);
        target.append("] ");
    }

    std::string buildDecoratedMessage(const std::string_view& msg,
                                      std::optional<std::string_view> tag,
                                      std::optional<std::string_view> category) {
        const bool hasCategory = category && !category->empty();
        const bool hasOverrideTag = tag && !tag->empty();
        const bool hasAnyTag = hasOverrideTag || !gDefaultTag.empty();

        size_t reserve = msg.size();
        if (hasCategory)
            reserve += category->size() + BRACKETS_OVERHEAD;
        if (hasAnyTag)
            reserve += (hasOverrideTag ? tag->size() : gDefaultTag.size()) + BRACKETS_OVERHEAD;

        std::string decorated;
        decorated.reserve(reserve);

        if (hasCategory)
            appendBracketed(decorated, *category);

        if (hasOverrideTag)
            appendBracketed(decorated, *tag);
        else if (hasAnyTag)
            appendBracketed(decorated, std::string_view{gDefaultTag});

        decorated.append(msg);
        return decorated;
    }

    void logErrorMessage(std::string message) {
        gHuLogger.log(AppLog::LVL_ERR, message);
    }

    // Utility: enable file sink only if path is non-empty; create directories as needed
    static void enableFileSinkIfValid(const char* filePath) {
        if (!filePath || !*filePath)
            return;

        std::filesystem::path desired{filePath};
        if (!desired.is_absolute()) {
            std::error_code cwdEc;
            const auto cwd = std::filesystem::current_path(cwdEc);
            if (cwdEc) {
                std::string msg = "Failed to resolve working directory for log file '";
                msg.append(filePath);
                msg.append("': ");
                msg.append(cwdEc.message());
                logErrorMessage(std::move(msg));
                return;
            }
            desired = cwd / desired;
        }

        const auto parent = desired.parent_path();
        if (!parent.empty()) {
            std::error_code dirEc;
            const bool exists = std::filesystem::exists(parent, dirEc);
            if (dirEc) {
                std::string msg = "Failed to check if log directory '";
                msg.append(parent.string());
                msg.append("' exists: ");
                msg.append(dirEc.message());
                logErrorMessage(std::move(msg));
                return;
            }

            if (!exists) {
                dirEc.clear();
                if (!std::filesystem::create_directories(parent, dirEc)) {
                    if (dirEc) {
                        std::string msg = "Failed to create log directory '";
                        msg.append(parent.string());
                        msg.append("': ");
                        msg.append(dirEc.message());
                        logErrorMessage(std::move(msg));
                        return;
                    }
                }
            }
        }
        const std::string target = desired.string();

        if (auto status = gHuLogger.setOutputFile(target); !status) {
            std::string msg = "Failed to open log file '";
            msg.append(target);
            msg.append("': ");
            msg.append(status.error());
            logErrorMessage(std::move(msg));
        }
    }
}

namespace AppLog {

    Logger& getLogger() {
        static Logger L;
        return L;
    }

    void Logger::log(UnderlyingLevel level,
                    const std::string_view& msg,
                    std::optional<std::string_view> tagOverride,
                    std::optional<std::string_view> category)
    {
        const bool hasCategory = category && !category->empty();
        const bool hasOverrideTag = tagOverride && !tagOverride->empty();
        const bool hasAnyTag = hasOverrideTag || !gDefaultTag.empty();

        if (!hasCategory && !hasAnyTag) {
            gHuLogger.log(level, msg);
            return;
        }
        
        const std::string finalMsg = buildDecoratedMessage(msg, tagOverride, category);
        gHuLogger.log(level, finalMsg);
        gHuLogger.log(CLI::eLogLevel::LOG_TRACE, (std::string_view)"sono entrato nel log function");
    }

    void initLogging(bool toStdout, bool colored, const char* filePath, const std::string_view& tag) {
        std::call_once(gInitOnce, [&]{
            gHuLogger.setEnableRolling(true);
            if (toStdout) {
                gHuLogger.setEnableStdout(toStdout);
                gHuLogger.setEnableColor(colored);
            }
            enableFileSinkIfValid(filePath);
            gHuLogger.log(LVL_INFO, "Logging initialized");        
        });
        gDefaultTag = std::string(tag);
    }

    void setMinLevel(LogLevel lvl) { gMinLevel.store(lvl, std::memory_order_relaxed); }
    LogLevel getMinLevel()         { return gMinLevel.load(std::memory_order_relaxed); }

} // namespace AppLog
