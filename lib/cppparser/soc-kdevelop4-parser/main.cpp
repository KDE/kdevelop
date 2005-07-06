/* This file is part of KDevelop
    Copyright (C) 2002,2003,2004 Roberto Raggi <roberto@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "ast.h"
#include "tokens.h"
#include "lexer.h"
#include "parser.h"
#include "treewalker.h"
#include "treedump.h"
#include "control.h"

#include <list>
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>


inline void parse_file(const std::string &fileName, bool dump = false)
{
    char *contents;
    std::size_t size;

    struct stat st;
    stat(fileName.c_str(), &st);
    size = st.st_size + 1;

    int fd = open(fileName.c_str(), O_RDONLY);
    assert(fd != -1);

    contents = (char *) mmap(contents, size, PROT_READ, MAP_SHARED, fd, 0);
    assert(contents != (void*) -1);
    close(fd);

    Control control;
    Parser p(&control);
    pool __pool;

    TranslationUnitAST *ast = p.parse(contents, size, &__pool);
    if (dump) {
        TreeDump dump(&p.token_stream);
        dump.dumpNode(ast);
    }

    munmap(contents, size);
}

int main(int, char *argv[])
{
    const char *filename = 0;
    bool dump = false;

    do {
        const char *arg = *++argv;
        if (!strcmp(arg, "-dump")) {
            dump = true;
        } else {
            filename = arg;
            break;
        }
    } while (*argv);

    if (filename)
        parse_file(filename, dump);
    else
        fprintf(stderr, "r++: no input file\n");

    return EXIT_SUCCESS;
}
