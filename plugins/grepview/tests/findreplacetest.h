/***************************************************************************
*   Copyright 1999-2001 Bernd Gehrmann and the KDevelop Team              *
*   bernd@kdevelop.org                                                    *
*   Copyright 2010 Julien Desgats <julien.desgats@gmail.com>              *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/


#ifndef KDEVPLATFORM_PLUGIN_REPLACETEST_H
#define KDEVPLATFORM_PLUGIN_REPLACETEST_H

#include <QtCore/QObject>
#include <QList>
#include <QMetaType>
#include <QPair>

namespace KDevelop
{
    class TestCore;
}
class GrepViewPlugin;

class FindReplaceTest : public QObject
{
    Q_OBJECT
public:
    struct Match {
        Match() {}
        Match(int l,int s,int e) : line(l), start(s), end(e) {}
        int line;
        int start;
        int end;
    };
    typedef QList<Match> MatchList;
    
    typedef QPair<QString, QString> File;  /// Represent a file with name => content
    typedef QList<File> FileList;
    
private:
    GrepViewPlugin* m_plugin;
    
private slots:
    void initTestCase();
    void cleanupTestCase();
    
    void testFind();
    void testFind_data();
    
    void testReplace();
    void testReplace_data();
};

Q_DECLARE_METATYPE(FindReplaceTest::MatchList)
Q_DECLARE_METATYPE(FindReplaceTest::FileList);

#endif // KDEVPLATFORM_PLUGIN_REPLACETEST_H
