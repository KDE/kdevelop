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

/*!
Constructs a compatible check executable for check Suite(s)
through main macro's. See CHECK_VERITAS_MAIN.
This header is intended for users of the runner, not a part of the
check plugin.
*/

#ifndef CHECK_WRAPPER_H
#define CHECK_WRAPPER_H

#include <stdio.h>
#include <check.h>
#include "check_list.h"
#include "check_impl.h"

void qxcheck_print_suite(Suite* suite)
{
    printf("<suite name=\"%s\">\n", suite->name);
    List *cases = suite->tclst;
    list_front(cases);
    while (!list_at_end(cases)) {
        TCase* caze = (struct TCase*)list_val(cases);
        printf("  <case name=\"%s\">\n", caze->name);
        List *cmds = caze->tflst;
        list_front(cmds);
        while (!list_at_end(cmds)) {
            TF* cmd = (struct TF*)list_val(cmds);
            printf("    <command name=\"%s\"/>\n", cmd->name);
            list_advance(cmds);
        }
        printf("  </case>\n");
        list_advance(cases);
    }
    printf("</suite>\n");
}

int qxcheck_print_tests(Suite** suites, int num)
{
    int i;
    printf("<?xml version=\"1.0\"?>\n"
            "<testsuites>\n");
    for (i=0; i<num; i++) {
        Suite* s = suites[i];
        qxcheck_print_suite(s);
    }
    printf("</testsuites>\n");
    return 0;
}

int qxcheck_run_tests(Suite** suites, int num)
{
    int number_failed, i;

    SRunner *sr = srunner_create(suites[0]);
    for (i=1; i<num; i++) {
        srunner_add_suite(sr, suites[i]);
    }
    srunner_set_xml(sr, "checklog.xml");
    srunner_run_all(sr, CK_SILENT);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? 0 : 1;
}

#define CHECK_VERITAS_MAIN_( root ) \
int main(int argc, char** argv)\
{\
    Suite* s = root; \
    Suite* suites[1] = {s}; \
    if (argc == 1 || argv[1][0] != '-') { \
        return qxcheck_run_tests(suites,1); \
    } else { \
        return qxcheck_print_tests(suites,1); \
    }\
}

#define CHECK_VERITAS_MAIN( multi, num ) \
int main(int argc, char** argv)\
{\
    if (argc == 1 || argv[1][0] != '-') {\
        return qxcheck_run_tests(multi, num); \
    } else { \
       return qxcheck_print_tests(multi, num); \
    }\
}

#endif // CHECK_WRAPPER_H
