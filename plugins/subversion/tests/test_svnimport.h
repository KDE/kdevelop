/*
    SPDX-FileCopyrightText: 2009 Fabian Wiesel <fabian.wiesel@googlemail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_SVNIMPORT_H
#define KDEVPLATFORM_PLUGIN_SVNIMPORT_H

#include <QObject>

class QTemporaryDir;
namespace KDevelop
{
class ICentralizedVersionControl;
}

class TestSvnImport
            : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testBasic();
    void testImportIntoDir();
    void testImportWithMissingDirs();
private:
    void validateImport( const QString& repo, QTemporaryDir& checkout, const QString& origcontent );
    KDevelop::ICentralizedVersionControl* vcs = nullptr;
};

#endif // KDEVPLATFORM_PLUGIN_SVNRECURSIVEADD_H
