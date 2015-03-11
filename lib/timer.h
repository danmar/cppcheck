/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
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

#include <string>
#include <map>
#include <ctime>
#include "config.h"

enum SHOWTIME_MODES {
    SHOWTIME_NONE = 0,
    SHOWTIME_FILE,
    SHOWTIME_SUMMARY,
    SHOWTIME_TOP5
};

class CPPCHECKLIB TimerResultsIntf {
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

    double seconds() const {
        double ret = (double)((unsigned long)_clocks) / (double)CLOCKS_PER_SEC;
        return ret;
    }
};

class CPPCHECKLIB TimerResults : public TimerResultsIntf {
public:
    TimerResults() {
    }

    void ShowResults(SHOWTIME_MODES mode) const;
    virtual void AddResults(const std::string& str, std::clock_t clocks);

private:
    std::map<std::string, struct TimerResultsData> _results;
};

class CPPCHECKLIB Timer {
public:
    Timer(const std::string& str, unsigned int showtimeMode, TimerResultsIntf* timerResults = NULL);
    ~Timer();
    void Stop();

private:
    Timer& operator=(const Timer&); // disallow assignments

    const std::string _str;
    TimerResultsIntf* _timerResults;
    std::clock_t _start;
    const unsigned int _showtimeMode;
    bool _stopped;
};
//---------------------------------------------------------------------------
#endif // timerH
