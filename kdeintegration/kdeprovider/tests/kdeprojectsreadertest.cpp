/* This file is part of KDevelop
    Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

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

#include "kdeprojectsreadertest.h"
#include <QTest>
#include <QDebug>
#include <QSignalSpy>
#include "../kdeprojectsreader.h"

QTEST_MAIN( KDEProjectsReaderTest )

#define WAIT_FOR_SIGNAL(a,b) \
{\
    bool gotSignal = QSignalSpy((a), SIGNAL(b)).wait(30000);\
    QVERIFY2(gotSignal, "Timeout while waiting for opened signal");\
} void(0)

void KDEProjectsReaderTest::testsProperParse()
{
    KDEProjectsModel m;
    KDEProjectsReader reader(&m, 0);
    
    if(reader.hasErrors())
        qDebug() << "errors:" << reader.errors();
    
    QVERIFY(!reader.hasErrors());
    
    WAIT_FOR_SIGNAL(&reader,downloadDone());
    
    for(int i=0; i<m.rowCount(); i++) {
        QStandardItem* item = m.item(i,0);
        qDebug() << ":::::" << item->text() << item->icon() << item->data(KDEProjectsModel::VcsLocationRole);
        
        QVERIFY(item);
        QVERIFY(!item->text().isEmpty());
        
        QVariant urls = item->data(KDEProjectsModel::VcsLocationRole);
        QVERIFY(urls.isValid());
        QVERIFY(urls.canConvert(QVariant::Map));
        QVariantMap mapurls=urls.toMap();
        for(QVariantMap::const_iterator it=mapurls.constBegin(), itEnd=mapurls.constEnd(); it!=itEnd; ++it) {
            QVERIFY(!it.key().isEmpty());
            QVERIFY(!it.value().toString().isEmpty());
        }
        
        QVERIFY(!item->data(KDEProjectsModel::VcsLocationRole).toMap().isEmpty());
    }
}
