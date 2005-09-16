/* This file is part of KDevelop
    Copyright (C) 2005 Roberto Raggi <roberto@kdevelop.org>

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

#ifndef PROBLEM_H
#define PROBLEM_H

#include <QtCore/QSharedData>
#include <QtCore/QSharedDataPointer>
#include <QtCore/QString>

class Problem
{
private:
  Problem();
  Problem(const Problem &other);
  Problem &operator = (const Problem &other);

  QString message() const;
  void setMessage(const QString &message);

  QString fileName() const;
  void setFileName(const QString &fileName);

  int line() const;
  void setLine(int line);

  int column() const;
  void setColumn(int column);

private:
  struct Data: public QSharedData
  {
    QString message;
    QString fileName;
    int line;
    int column;

    Data():
      line(0), column(0) {}
  };
  QSharedDataPointer<Data> d;
};

#endif // PROBLEM_H

// kate: space-indent on; indent-width 2; replace-tabs on;
