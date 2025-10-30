/* -*- C++ -*-
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2025 Cppcheck team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
//---------------------------------------------------------------------------
#ifndef timerH
#define timerH
//---------------------------------------------------------------------------

#include "config.h"

#include <chrono>
#include <cstdint>
#include <ctime>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <utility>

enum class ShowTime : std::uint8_t {
    NONE,
    FILE,
    FILE_TOTAL,
    SUMMARY,
    TOP5_SUMMARY,
    TOP5_FILE
};

class CPPCHECKLIB TimerResultsIntf {
public:
    virtual ~TimerResultsIntf() = default;

    virtual void addResults(const std::string& timerName, std::chrono::milliseconds duation) = 0;
};

struct TimerResultsData {
    std::chrono::milliseconds mDuration;
    long mNumberOfResults{};

    std::chrono::duration<double> getSeconds() const {
        return std::chrono::duration_cast<std::chrono::duration<double>>(mDuration);
    }

    static std::string durationToString(std::chrono::milliseconds duration);
};

class CPPCHECKLIB TimerResults : public TimerResultsIntf {
public:
    TimerResults() = default;

    void showResults(ShowTime mode) const;
    void addResults(const std::string& str, std::chrono::milliseconds duration) override;

    void reset();

private:
    std::map<std::string, TimerResultsData> mResults;
    mutable std::mutex mResultsSync;
};

class CPPCHECKLIB Timer {
public:
    static constexpr char OVERALL[] = "Summary";
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    Timer(std::string str, ShowTime showtimeMode, TimerResultsIntf* timerResults = nullptr);
    ~Timer();

    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

    void stop();

    static void run(std::string str, ShowTime showtimeMode, TimerResultsIntf* timerResults, const std::function<void()>& f) {
        Timer t(std::move(str), showtimeMode, timerResults);
        f();
    }

private:
    const std::string mStr;
    TimerResultsIntf* mTimerResults{};
    ShowTime mShowTimeMode = ShowTime::FILE_TOTAL;
    TimePoint mStartTimePoint;
};

//---------------------------------------------------------------------------
#endif // timerH
