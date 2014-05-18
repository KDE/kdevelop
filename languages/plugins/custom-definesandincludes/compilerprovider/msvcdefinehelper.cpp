/*
* This file is part of KDevelop
*
* Copyright 2010 Patrick Spendrin <ps_ml@gmx.de>
*
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

/* 
*  This helper program is needed for the Visual Studio compiler 
*  to find out the standard macros, there is no "gcc -dM -E -".
*  A trick is used to find out the standard macros. You can 
*  exchange the compiler. The standard macros are written in
*  a enviroment variable. This helper programm analysis this
*  variable and prints the output to stdout.
*/

#include <cstdio>
#include <cstdlib>
#include <cstring>

int main(int argc, char**argv)
{
    char *next, *token, *tmp, *equal;
    int i = 0, nextlen, currentlen;
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