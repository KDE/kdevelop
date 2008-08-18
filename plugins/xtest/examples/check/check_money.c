/*
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

/*! Example Check testcase intended to showcase the KDevelop4 xTest runner */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "money.h"

#include <kdevelop/veritas/checkwrapper.h>

Money *five_dollars;

void
setup (void)
{
  five_dollars = money_create (5, "USD");
}

void
teardown (void)
{
  money_free (five_dollars);
}

START_TEST (test_money_create)
{
  fail_unless (money_amount (five_dollars) == 5,
           "Amount not set correctly on creation");
  fail_unless (strcmp (money_currency (five_dollars), "USD") == 0,
           "Currency not set correctly on creation");
}
END_TEST

START_TEST (test_money_create_neg)
{
  Money *m = money_create (-1, "USD");
  fail_unless (m == NULL,
           "NULL should be returned on attempt to create with "
           "a negative amount");
}
END_TEST

START_TEST (test_money_create_zero)
{
  Money *m = money_create (0, "USD");
  fail_unless (money_amount (m) == 0, 
           "Zero is a valid amount of money");
}
END_TEST

Suite *
money_suite (void)
{
  Suite *s = suite_create ("Money");

  /* Core test case */
  TCase *tc_core = tcase_create ("Core");
  tcase_add_checked_fixture (tc_core, setup, teardown);
  tcase_add_test (tc_core, test_money_create);
  suite_add_tcase (s, tc_core);

  /* Limits test case */
  TCase *tc_limits = tcase_create ("Limits");
  tcase_add_test (tc_limits, test_money_create_neg);
  tcase_add_test (tc_limits, test_money_create_zero);
  suite_add_tcase (s, tc_limits);

  return s;
}

CHECK_VERITAS_MAIN_( money_suite() )
