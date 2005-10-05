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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "problem.h"

Problem::Problem()
  : d(new Data())
{
}

Problem::Problem(const Problem &other)
  : d(other.d)
{
}

Problem &Problem::operator = (const Problem &other)
{
  d = other.d;
  return *this;
}

QString Problem::message() const
{
  return d->message;
}

void Problem::setMessage(const QString &message)
{
  d->message = message;
}

QString Problem::fileName() const
{
  return d->fileName;
}

void Problem::setFileName(const QString &fileName)
{
  d->fileName = fileName;
}

int Problem::line() const
{
  return d->line;
}

void Problem::setLine(int line)
{
  d->line = line;
}

int Problem::column() const
{
  return d->column;
}

void Problem::setColumn(int column)
{
  d->column = column;
}

// kate: space-indent on; indent-width 2; replace-tabs on;
