/*
 * This file is part of KDevelop
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <checkwrapper.h>
#include <unistd.h>

// "0 foo"
// "0 0 foo_test"
// "0 0 0 foo_cmd"
// "0 0 1 x"
// "0 bar"
// "1 0 bar_test"
// "1 0 0 bar_cmd"
// "1 1 1 x"
// "1 1 x"
// "2 x"

START_TEST( foo_cmd )
{}
END_TEST

START_TEST( bar_cmd )
{}
END_TEST

Suite* ss[2];

Suite** suites(void)
{
    Suite *fs = suite_create("foo");
    TCase *tc_foo = tcase_create("foo_test");
    tcase_add_test(tc_foo, foo_cmd);
    suite_add_tcase(fs, tc_foo);

    Suite *bs = suite_create("bar");
    TCase *tc_bar = tcase_create("bar_test");
    tcase_add_test(tc_bar, bar_cmd);
    suite_add_tcase(bs, tc_bar);

    ss[0] = fs;
    ss[1] = bs;
    return ss;
}

CHECK_VERITAS_MAIN( suites(), 2 )
