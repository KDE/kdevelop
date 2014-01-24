/* This file is part of KDevelop
    Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef CONTROL_H
#define CONTROL_H

#include "symbol.h"
#include <language/duchain/problem.h>
#include <cppparserexport.h>

#include <QtCore/QHash>

struct Declarator;
struct Type;

/**Parser control information.
Provides problem tracking, context(visibility)-aware symbol name tables,
typedef table and other information about the parsing process.*/
class KDEVCPPPARSER_EXPORT Control
{
public:
  Control();
  ~Control();

  const QList<KDevelop::ProblemPointer>& problems() const;

  /**Adds a problem to the list of problems.*/
  void reportProblem(const KDevelop::ProblemPointer &problem);

  /**Returns whether there is a problem from the given source */
  bool hasProblem(KDevelop::ProblemData::Source source) const;
  
private:
  QList<KDevelop::ProblemPointer> m_problems;
};

#endif // CONTROL_H

