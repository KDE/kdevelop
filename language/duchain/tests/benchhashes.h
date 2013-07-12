/*
 * This file is part of KDevelop
 * Copyright 2012 Milian Wolff <mail@milianw.de>
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

#ifndef KDEVPLATFORM_BENCHHASHES_H
#define KDEVPLATFORM_BENCHHASHES_H

#include <QObject>

class BenchHashes : public QObject
{
  Q_OBJECT

private:
  void feedData();

private slots:
  void initTestCase();
  void cleanupTestCase();

  void insert();
  void insert_data();
  void find();
  void find_data();
  void constFind();
  void constFind_data();
  void remove();
  void remove_data();
  void typeRepo();
  void typeRepo_data();
};

#endif // KDEVPLATFORM_BENCHHASHES_H
