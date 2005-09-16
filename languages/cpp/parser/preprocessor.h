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

#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QByteArray>

class Preprocessor: public QObject
{
  Q_OBJECT
public:
  Preprocessor(QObject *parent = 0);
  virtual ~Preprocessor();

  QByteArray run(const QByteArray &contents);
};

#endif // PREPROCESSOR_H

// kate: space-indent on; indent-width 2; replace-tabs on;
