/*
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

#include "timer.h"

#include "utils.h"

#include <algorithm>
#include <iostream>
#include <utility>
#include <vector>

namespace {
    using dataElementType = std::pair<std::string, TimerResultsData>;
    bool more_second_sec(const dataElementType& lhs, const dataElementType& rhs)
    {
        return lhs.second.seconds() > rhs.second.seconds();
    }

    // TODO: remove and print through (synchronized) ErrorLogger instead
    std::mutex stdCoutLock;
}

// TODO: this does not include any file context when SHOWTIME_FILE thus rendering it useless - should we include the logging with the progress logging?
// that could also get rid of the broader locking
void TimerResults::showResults(SHOWTIME_MODES mode) const
{
    if (mode == SHOWTIME_MODES::SHOWTIME_NONE || mode == SHOWTIME_MODES::SHOWTIME_FILE_TOTAL)
        return;

    TimerResultsData overallData;
    std::vector<dataElementType> data;

    {
        std::lock_guard<std::mutex> l(mResultsSync);

        data.reserve(mResults.size());
        data.insert(data.begin(), mResults.cbegin(), mResults.cend());
    }
    std::sort(data.begin(), data.end(), more_second_sec);

    // lock the whole logging operation to avoid multiple threads printing their results at the same time
    std::lock_guard<std::mutex> l(stdCoutLock);

    std::cout << std::endl;

    size_t ordinal = 1; // maybe it would be nice to have an ordinal in output later!
    for (auto iter=data.cbegin(); iter!=data.cend(); ++iter) {
        const double sec = iter->second.seconds();
        const double secAverage = sec / static_cast<double>(iter->second.mNumberOfResults);
        bool hasParent = false;
        {
            // Do not use valueFlow.. in "Overall time" because those are included in Tokenizer already
            if (startsWith(iter->first,"valueFlow"))
                hasParent = true;

            // Do not use inner timers in "Overall time"
            const std::string::size_type pos = iter->first.rfind("::");
            if (pos != std::string::npos)
                hasParent = std::any_of(data.cbegin(), data.cend(), [iter,pos](const dataElementType& d) {
                    return d.first.size() == pos && iter->first.compare(0, d.first.size(), d.first) == 0;
                });
        }
        if (!hasParent)
            overallData.mClocks += iter->second.mClocks;
        if ((mode != SHOWTIME_MODES::SHOWTIME_TOP5_FILE && mode != SHOWTIME_MODES::SHOWTIME_TOP5_SUMMARY) || (ordinal<=5)) {
            std::cout << iter->first << ": " << sec << "s (avg. " << secAverage << "s - " << iter->second.mNumberOfResults  << " result(s))" << std::endl;
        }
        ++ordinal;
    }
}

void TimerResults::addResults(const std::string& str, std::clock_t clocks)
{
    std::lock_guard<std::mutex> l(mResultsSync);

    mResults[str].mClocks += clocks;
    mResults[str].mNumberOfResults++;
}

void TimerResults::reset()
{
    std::lock_guard<std::mutex> l(mResultsSync);
    mResults.clear();
}

Timer::Timer(std::string str, SHOWTIME_MODES showtimeMode, TimerResultsIntf* timerResults)
    : mStr(std::move(str))
    , mTimerResults(timerResults)
    , mStart(std::clock())
    , mShowTimeMode(showtimeMode)
    , mStopped(showtimeMode == SHOWTIME_MODES::SHOWTIME_NONE || showtimeMode == SHOWTIME_MODES::SHOWTIME_FILE_TOTAL)
    , mStartTimePoint(Clock::now())
{}

Timer::Timer(std::string str)
    : mStr(std::move(str))
    , mShowTimeMode(SHOWTIME_MODES::SHOWTIME_FILE_TOTAL)
    , mStartTimePoint(Clock::now())
{}

Timer::~Timer()
{
    stop();
}

void Timer::stop()
{
    if ((mShowTimeMode != SHOWTIME_MODES::SHOWTIME_NONE) && !mStopped) {
        const std::clock_t end = std::clock();
        const std::clock_t diff = end - mStart;

        if (mShowTimeMode == SHOWTIME_MODES::SHOWTIME_FILE) {
            const double sec = static_cast<double>(diff) / CLOCKS_PER_SEC;
            std::lock_guard<std::mutex> l(stdCoutLock);
            std::cout << mStr << ": " << sec << "s" << std::endl;
        } else if (mShowTimeMode == SHOWTIME_MODES::SHOWTIME_FILE_TOTAL && mStartTimePoint != TimePoint{}) {
            std::lock_guard<std::mutex> l(stdCoutLock);
            std::cout << "Check time: " << mStr << ": " << getRealTimePassed() << std::endl;
        } else {
            if (mTimerResults)
                mTimerResults->addResults(mStr, diff);
            else if (mStr.empty() && mStartTimePoint != TimePoint{}) { // Get real time
                std::lock_guard<std::mutex> l(stdCoutLock);
                std::cout << "Overall time: " << getRealTimePassed() << std::endl;
            }
        }
    }

    mStopped = true;
}

std::string Timer::getRealTimePassed()
{
    auto diff = std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - mStartTimePoint);

    // Extract hours
    auto hours = std::chrono::duration_cast<std::chrono::hours>(diff);
    diff -= hours; // Subtract the extracted hours

    // Extract minutes
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(diff);
    diff -= minutes; // Subtract the extracted minutes

    // Extract seconds
    auto seconds = static_cast<double>(diff.count()) / std::chrono::microseconds::period::den;

    std::string ellapsedTime;
    if (hours.count() > 0)
        ellapsedTime += std::to_string(hours.count()) + "h ";
    if (minutes.count() > 0)
        ellapsedTime += std::to_string(minutes.count()) + "m ";
    return (ellapsedTime + std::to_string(seconds) + "s ");
}
