/* This file is part of KDevelop
    Copyright (C) 2002-2005 Roberto Raggi <roberto@kdevelop.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "ast.h"
#include "tokens.h"
#include "lexer.h"
#include "parser.h"
#include "control.h"
#include "default_visitor.h"
#include "dumptree.h"
#include "binder.h"
#include "codemodel.h"

#include <QtCore/QFile>
#include <QtCore/qdebug.h>

bool parse_file(const QString &fileName, bool dump = false)
{
  QFile file(fileName);
  if (!file.open(QFile::ReadOnly))
    return false;

  QByteArray contents = file.readAll();
  file.close();

  Control control;
  Parser p(&control);
  pool __pool;

  TranslationUnitAST *ast = p.parse(contents, contents.size(), &__pool);
  if (ast && dump)
    {
      DumpTree dump;
      dump.dump(ast);
    }

  CodeModel model;
  Binder binder(&model, &p.token_stream);
  FileModelItem dom = binder.run(ast);

  if (p.problemCount() == 0)
    {
      QHash<QString, ClassModelItem> typeMap = dom->classMap();
      // ### do something
    }

  return p.problemCount() == 0;
}

int main(int, char *argv[])
{
  const char *filename = 0;
  bool dump = false;

  do
    {
      const char *arg = *++argv;
      if (!strcmp(arg, "-dump"))
	{
	  dump = true;
        }
      else
	{
	  filename = arg;
	  break;
        }
    }
  while (*argv);

  bool parsed = false;

  if (filename)
    {
      parsed = parse_file(filename, dump);
    }
  else
    {
      fprintf(stderr, "r++: no input file\n");
    }

  return parsed ? EXIT_SUCCESS : EXIT_FAILURE;
}

// kate: space-indent on; indent-width 2; replace-tabs on;
