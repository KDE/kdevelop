/* KDevelop xUnit plugin
 *
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

// "0 root_suite"
// "0 0 foo_test"
// "0 0 0 foo_cmd1"
// "0 0 1 foo_cmd2"
// "0 0 2 x"
// "0 1 bar_test"
// "0 1 0 bar_cmd1"
// "0 1 1 bar_cmd2"
// "0 1 2 x"
// "0 2 baz_test"
// "0 2 0 baz_cmd1"
// "0 2 1 x"
// "0 3 x"
// "1 x"

START_TEST( foo_cmd1 )
{}
END_TEST

START_TEST( foo_cmd2 )
{}
END_TEST

START_TEST( bar_cmd1 )
{}
END_TEST

START_TEST( bar_cmd2 )
{
    fail_unless(0);
}
END_TEST

START_TEST( baz_cmd1 )
{
    sleep(1);
}
END_TEST

Suite* suite(void)
{
    Suite *s = suite_create("root_suite");

    /* foo test case */
    TCase *tc_foo = tcase_create("foo_test");
    tcase_add_test(tc_foo, foo_cmd1);
    tcase_add_test(tc_foo, foo_cmd2);
    suite_add_tcase(s, tc_foo);

    /* bar test case */
    TCase *tc_bar = tcase_create("bar_test");
    tcase_add_test(tc_bar, bar_cmd1);
    tcase_add_test(tc_bar, bar_cmd2);
    suite_add_tcase(s, tc_bar);

    /* baz test case */
    TCase *tc_baz = tcase_create("baz_test");
    tcase_add_test(tc_baz, baz_cmd1);
    suite_add_tcase(s, tc_baz);

    return s;
}

CHECK_XTEST_MAIN( suite() )
