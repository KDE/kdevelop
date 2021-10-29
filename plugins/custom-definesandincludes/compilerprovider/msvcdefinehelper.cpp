/*
    SPDX-FileCopyrightText: 2010 Patrick Spendrin <ps_ml@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

/* 
*  This helper program is needed for the Visual Studio compiler 
*  to find out the standard macros, there is no "gcc -dM -E -".
*  A trick is used to find out the standard macros. You can 
*  exchange the compiler. The standard macros are written in
*  a environment variable. This helper program analysis this
*  variable and prints the output to stdout.
*/

#include <cstdio>
#include <cstdlib>
#include <cstring>

int main(int argc, char**argv)
{
    char *next, *token, *tmp, *equal;
    int nextlen, currentlen;
    next = getenv("MSC_CMD_FLAGS");

    while (next != NULL)
    {
        token = next;
        next = strstr(next + 1, " -");

        if(strncmp(token, " -D", 3) == 0) {
            if(next != NULL) 
                nextlen = strlen(next);
            else
                nextlen = 0;

            currentlen = strlen(token);
            tmp = new char[(currentlen - nextlen + 1)];
            strncpy(tmp, token, currentlen - nextlen);
            tmp[currentlen - nextlen] = '\0';
            equal = strchr(tmp, '=');
            if(equal != NULL) *equal = ' ';
            printf("#define %s\n", tmp + 3);
            delete [] tmp;
        }
    }
    // return an error so that the compiler doesn't check for the output
    return 1;
}
