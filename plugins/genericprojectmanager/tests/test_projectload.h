/*
    SPDX-FileCopyrightText: 2010 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_PROJECTLOADTEST_H
#define KDEVPLATFORM_PLUGIN_PROJECTLOADTEST_H

#include <QObject>

class TestProjectLoad : public QObject
{
Q_OBJECT

private Q_SLOTS:
  void initTestCase();
  void cleanupTestCase();
  void init();

  void addRemoveFiles();
  void removeDirRecursive();
  void addLotsOfFiles();
  void addMultipleJobs();

  void raceJob();

  void addDuringImport();
};

#endif
