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

/*! KDevelop4 gcov coverage tool example.
usage:
1/ Boot up KDevelop4
2/ Project -> Open -> prime.kdev4
3/ Build. Notice the gcov data file `main.gcno'
   in the project's directory
4/ Run `is_prime'. Notice `main.gcda'
5/ Add the coverage toolview. View -> Add Tool View -> Coverage
6/ Enter the prime-projects root directory under 'Build Path:'
7/ Hit the run coverage button, next to the breadcrumb widget
8/ Use left and right arrow to navigate the report
9/ Close this file
10/ Double click 'main.cpp' to view coverage annotations
*/

#include <stdio.h>
bool is_prime(long number);

int main(int,char**)
{
    if (is_prime(2111*4111)) {
        printf("not covered, not prime.\n");
    }
    if (is_prime(29)) {
        printf("covered, 29 is prime.\n");
    }
    return 0;
}

const bool g_primes[10] = {0, 0, 1, 1, 0, 1, 0, 1, 0, 0};
bool is_prime(long number)
{
    if (number < 10) {
        return g_primes[number];
    }
    if (number % 2 == 0) {
        return false;
    }
    for(long m = 3; m*m <= number; m += 2) {
        if (number % m == 0) {
            return false;
        }
    }
    return true;
}
