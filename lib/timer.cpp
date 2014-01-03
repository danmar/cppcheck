/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2013 Daniel Marjamäki and Cppcheck team.
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
#include <iostream>
#include "timer.h"

/*
    TODO:
    - sort list by time
    - do not sort the results alphabetically
    - rename "file" to "single"
    - synchronise map access in multithreaded mode or disable timing
    - add unit tests
        - for --showtime (needs input file)
        - for Timer* classes
*/


void TimerResults::ShowResults(SHOWTIME_MODES mode) const
{
    if (mode == SHOWTIME_NONE)
        return;

    std::cout << std::endl;
    TimerResultsData overallData;

    std::map<std::string, struct TimerResultsData>::const_iterator I = _results.begin();
    const std::map<std::string, struct TimerResultsData>::const_iterator E = _results.end();
    size_t item = 0;
    while (I != E) {
        const double sec = I->second.seconds();
        const double secAverage = sec / (double)(I->second._numberOfResults);
        std::cout << I->first << ": " << sec << "s (avg. " << secAverage << "s - " << I->second._numberOfResults  << " result(s))" << std::endl;

        overallData._clocks += I->second._clocks;

        ++I;
        ++item;
        if ((mode == SHOWTIME_TOP5) && (item>=5))
            break;
    }

    const double secOverall = overallData.seconds();
    std::cout << "Overall time: " << secOverall << "s" << std::endl;
}

void TimerResults::AddResults(const std::string& str, std::clock_t clocks)
{
    _results[str]._clocks += clocks;
    _results[str]._numberOfResults++;
}

Timer::Timer(const std::string& str, unsigned int showtimeMode, TimerResultsIntf* timerResults)
    : _str(str)
    , _timerResults(timerResults)
    , _start(0)
    , _showtimeMode(showtimeMode)
    , _stopped(false)
{
    if (showtimeMode != SHOWTIME_NONE)
        _start = std::clock();
}

Timer::~Timer()
{
    Stop();
}

void Timer::Stop()
{
    if ((_showtimeMode != SHOWTIME_NONE) && !_stopped) {
        const std::clock_t end = std::clock();
        const std::clock_t diff = end - _start;

        if (_showtimeMode == SHOWTIME_FILE) {
            double sec = (double)diff / CLOCKS_PER_SEC;
            std::cout << _str << ": " << sec << "s" << std::endl;
        } else {
            if (_timerResults)
                _timerResults->AddResults(_str, diff);
        }
    }

    _stopped = true;
}
