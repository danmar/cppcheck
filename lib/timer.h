/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2011 Daniel Marjamäki and Cppcheck team.
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

#ifndef TIMER_H
#define TIMER_H

#include <string>
#include <map>
#include <ctime>

enum {
    SHOWTIME_NONE = 0,
    SHOWTIME_FILE,
    SHOWTIME_SUMMARY,
    SHOWTIME_TOP5
};

class TimerResultsIntf {
public:
    virtual ~TimerResultsIntf() { }

    virtual void AddResults(const std::string& str, std::clock_t clocks) = 0;
};

struct TimerResultsData {
    std::clock_t _clocks;
    long _numberOfResults;

    TimerResultsData()
        : _clocks(0)
        , _numberOfResults(0) {
    }
};

class TimerResults : public TimerResultsIntf {
public:
    TimerResults() {
    }

    void ShowResults();
    virtual void AddResults(const std::string& str, std::clock_t clocks);

private:
    std::map<std::string, struct TimerResultsData> _results;
};

class Timer {
public:
    Timer(const std::string& str, unsigned int showtimeMode, TimerResultsIntf* timerResults = NULL);
    ~Timer();
    void Stop();

private:
    Timer& operator=(const Timer&); // disallow assignments

    const std::string _str;
    const unsigned int _showtimeMode;
    std::clock_t _start;
    bool _stopped;
    TimerResultsIntf* _timerResults;
};


#endif // TIMER_H
