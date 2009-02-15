/*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU Library General Public License as
* published by the Free Software Foundation; either version 2 of the
* License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public
* License along with this program; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

/*! Test - Unit Under Test code navigation example

Purpose is to navigate quickly from a test to it's unit.

KDevelop4 will read the switch target from the class'
documentation string. If this is not present a heuristic
based on naming conventions will try to resolve a switch
candidate.

First open both switch.h and switchtest.h in KDevelop4 to make
sure the symbols are parsed. Next configure the switch shortcut
in Settings -> Configure Shortcuts -> Switch Test/Unit Under Test.
Hit this key combination to jump between switch.h and switchtest.h
*/

/*! @unittest SwitchExampleTest */
class SwitchExample
{
public:
    void foo();
};
