
#pragma once
/*------------------------------------------------------------------------------
-- This file is a part of the CDFpp library
-- Copyright (C) 2022, Plasma Physics Laboratory - CNRS
--
-- This program is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 2 of the License, or
-- (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program; if not, write to the Free Software
-- Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
-------------------------------------------------------------------------------*/
/*-- Author : Alexis Jeandet
-- Mail : alexis.jeandet@member.fsf.org
----------------------------------------------------------------------------*/
#include "cdf-debug.hpp"
#include <chrono>
#include <utility>
#include <array>
#include "cdf-chrono-constants.hpp"
namespace cdf::chrono::leap_seconds
{    
using namespace std::chrono;
constexpr std::array leap_seconds_tt2000 = 
    {// ('1', 'Jan', '1972')
    std::pair{ 63072000000000000  , 10000000000},
// ('1', 'Jul', '1972')
    std::pair{ 78796800000000000  , 11000000000},
// ('1', 'Jan', '1973')
    std::pair{ 94694400000000000  , 12000000000},
// ('1', 'Jan', '1974')
    std::pair{ 126230400000000000  , 13000000000},
// ('1', 'Jan', '1975')
    std::pair{ 157766400000000000  , 14000000000},
// ('1', 'Jan', '1976')
    std::pair{ 189302400000000000  , 15000000000},
// ('1', 'Jan', '1977')
    std::pair{ 220924800000000000  , 16000000000},
// ('1', 'Jan', '1978')
    std::pair{ 252460800000000000  , 17000000000},
// ('1', 'Jan', '1979')
    std::pair{ 283996800000000000  , 18000000000},
// ('1', 'Jan', '1980')
    std::pair{ 315532800000000000  , 19000000000},
// ('1', 'Jul', '1981')
    std::pair{ 362793600000000000  , 20000000000},
// ('1', 'Jul', '1982')
    std::pair{ 394329600000000000  , 21000000000},
// ('1', 'Jul', '1983')
    std::pair{ 425865600000000000  , 22000000000},
// ('1', 'Jul', '1985')
    std::pair{ 489024000000000000  , 23000000000},
// ('1', 'Jan', '1988')
    std::pair{ 567993600000000000  , 24000000000},
// ('1', 'Jan', '1990')
    std::pair{ 631152000000000000  , 25000000000},
// ('1', 'Jan', '1991')
    std::pair{ 662688000000000000  , 26000000000},
// ('1', 'Jul', '1992')
    std::pair{ 709948800000000000  , 27000000000},
// ('1', 'Jul', '1993')
    std::pair{ 741484800000000000  , 28000000000},
// ('1', 'Jul', '1994')
    std::pair{ 773020800000000000  , 29000000000},
// ('1', 'Jan', '1996')
    std::pair{ 820454400000000000  , 30000000000},
// ('1', 'Jul', '1997')
    std::pair{ 867715200000000000  , 31000000000},
// ('1', 'Jan', '1999')
    std::pair{ 915148800000000000  , 32000000000},
// ('1', 'Jan', '2006')
    std::pair{ 1136073600000000000  , 33000000000},
// ('1', 'Jan', '2009')
    std::pair{ 1230768000000000000  , 34000000000},
// ('1', 'Jul', '2012')
    std::pair{ 1341100800000000000  , 35000000000},
// ('1', 'Jul', '2015')
    std::pair{ 1435708800000000000  , 36000000000},
// ('1', 'Jan', '2017')
    std::pair{ 1483228800000000000  , 37000000000},
};
constexpr std::array leap_seconds_tt2000_reverse = 
    {// ('1', 'Jan', '1972')
    std::pair{ -883655957816000000  , 10000000000},
// ('1', 'Jul', '1972')
    std::pair{ -867931156816000000  , 11000000000},
// ('1', 'Jan', '1973')
    std::pair{ -852033555816000000  , 12000000000},
// ('1', 'Jan', '1974')
    std::pair{ -820497554816000000  , 13000000000},
// ('1', 'Jan', '1975')
    std::pair{ -788961553816000000  , 14000000000},
// ('1', 'Jan', '1976')
    std::pair{ -757425552816000000  , 15000000000},
// ('1', 'Jan', '1977')
    std::pair{ -725803151816000000  , 16000000000},
// ('1', 'Jan', '1978')
    std::pair{ -694267150816000000  , 17000000000},
// ('1', 'Jan', '1979')
    std::pair{ -662731149816000000  , 18000000000},
// ('1', 'Jan', '1980')
    std::pair{ -631195148816000000  , 19000000000},
// ('1', 'Jul', '1981')
    std::pair{ -583934347816000000  , 20000000000},
// ('1', 'Jul', '1982')
    std::pair{ -552398346816000000  , 21000000000},
// ('1', 'Jul', '1983')
    std::pair{ -520862345816000000  , 22000000000},
// ('1', 'Jul', '1985')
    std::pair{ -457703944816000000  , 23000000000},
// ('1', 'Jan', '1988')
    std::pair{ -378734343816000000  , 24000000000},
// ('1', 'Jan', '1990')
    std::pair{ -315575942816000000  , 25000000000},
// ('1', 'Jan', '1991')
    std::pair{ -284039941816000000  , 26000000000},
// ('1', 'Jul', '1992')
    std::pair{ -236779140816000000  , 27000000000},
// ('1', 'Jul', '1993')
    std::pair{ -205243139816000000  , 28000000000},
// ('1', 'Jul', '1994')
    std::pair{ -173707138816000000  , 29000000000},
// ('1', 'Jan', '1996')
    std::pair{ -126273537816000000  , 30000000000},
// ('1', 'Jul', '1997')
    std::pair{ -79012736816000000  , 31000000000},
// ('1', 'Jan', '1999')
    std::pair{ -31579135816000000  , 32000000000},
// ('1', 'Jan', '2006')
    std::pair{ 189345665184000000  , 33000000000},
// ('1', 'Jan', '2009')
    std::pair{ 284040066184000000  , 34000000000},
// ('1', 'Jul', '2012')
    std::pair{ 394372867184000000  , 35000000000},
// ('1', 'Jul', '2015')
    std::pair{ 488980868184000000  , 36000000000},
// ('1', 'Jan', '2017')
    std::pair{ 536500869184000000  , 37000000000},
};

} //namespace cdf
    