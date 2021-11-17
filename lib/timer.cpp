/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2021 Cppcheck team.
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

#include <algorithm>
#include <iostream>
#include <utility>
#include <vector>
/*
    TODO:
    - rename "file" to "single"
    - synchronise map access in multithreaded mode or disable timing
    - add unit tests
        - for --showtime (needs input file)
        - for Timer* classes
 */

namespace {
    using dataElementType = std::pair<std::string, struct TimerResultsData>;
    bool more_second_sec(const dataElementType& lhs, const dataElementType& rhs)
    {
        return lhs.second.seconds() > rhs.second.seconds();
    }
}

void TimerResults::showResults(SHOWTIME_MODES mode) const
{
    if (mode == SHOWTIME_MODES::SHOWTIME_NONE)
        return;

    std::cout << std::endl;
    TimerResultsData overallData;

    std::vector<dataElementType> data(mResults.begin(), mResults.end());
    std::sort(data.begin(), data.end(), more_second_sec);

    size_t ordinal = 1; // maybe it would be nice to have an ordinal in output later!
    for (std::vector<dataElementType>::const_iterator iter=data.begin(); iter!=data.end(); ++iter) {
        const double sec = iter->second.seconds();
        const double secAverage = sec / (double)(iter->second.mNumberOfResults);
        overallData.mClocks += iter->second.mClocks;
        if ((mode != SHOWTIME_MODES::SHOWTIME_TOP5) || (ordinal<=5)) {
            std::cout << iter->first << ": " << sec << "s (avg. " << secAverage << "s - " << iter->second.mNumberOfResults  << " result(s))" << std::endl;
        }
        ++ordinal;
    }

    const double secOverall = overallData.seconds();
    std::cout << "Overall time: " << secOverall << "s" << std::endl;
}

void TimerResults::addResults(const std::string& str, std::clock_t clocks)
{
    mResults[str].mClocks += clocks;
    mResults[str].mNumberOfResults++;
}

Timer::Timer(const std::string& str, SHOWTIME_MODES showtimeMode, TimerResultsIntf* timerResults)
    : mStr(str)
    , mTimerResults(timerResults)
    , mStart(0)
    , mShowTimeMode(showtimeMode)
    , mStopped(false)
{
    if (showtimeMode != SHOWTIME_MODES::SHOWTIME_NONE)
        mStart = std::clock();
}

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
            const double sec = (double)diff / CLOCKS_PER_SEC;
            std::cout << mStr << ": " << sec << "s" << std::endl;
        } else {
            if (mTimerResults)
                mTimerResults->addResults(mStr, diff);
        }
    }

    mStopped = true;
}
