/***************************************************************************
 *   Copyright 2007 Rober Gruber <rgruber@users.sourceforge.net>                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CVSTEST_H
#define CVSTEST_H

#include <QtCore/QObject>

class CvsProxy;

class CvsTest: public QObject
{
    Q_OBJECT

private:
    void repoInit();
    void importTestData();
    void checkoutTestData();

private slots:
    void initTestCase();
    void testInitAndImport();
    void cleanupTestCase();

private:
    CvsProxy* m_proxy;
};

#endif
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
