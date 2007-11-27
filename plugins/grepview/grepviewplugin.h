/***************************************************************************
 *   Copyright 1999-2001 by Bernd Gehrmann                                 *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _GREPVIEWPART_H_
#define _GREPVIEWPART_H_

#include <iplugin.h>
#include <QtCore/QVariant>

class QStringList;
class GrepDialog;
class KDialog;

namespace KDevelop
{
    class IOutputView;
    class IProject;
    class ProcessLineMaker;
}

class GrepOutputModel;
class GrepOutputDelegate;
class KProcess;
class QSignalMapper;

class GrepViewPlugin : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    GrepViewPlugin( QObject *parent, const QVariantList & = QVariantList() );
    ~GrepViewPlugin();

private Q_SLOTS:
    void slotGrep();
    void showDialogWithPattern(const QString& pattern);
    void searchActivated();
    void procFailed(int);
    void procFinished(int);
    void cleanupForView(int);

private:
    static QString escape(const QString &str);
    QString currentWord();
    QString currentSelectedWord();

    GrepDialog* m_grepdlg;
    QString m_lastPattern;

    KDevelop::IProject *m_projectForActiveFile;

    KDevelop::IOutputView *m_view;
    QMap<int, GrepOutputModel*> models;
    QMap<int, GrepOutputDelegate*> delegates;
    QMap<int, KDevelop::ProcessLineMaker*> lineMakers;
    QMap<int, QList<KProcess*> > processes;
    QSignalMapper* finishedmapper;
    QSignalMapper* failedmapper;
};

#endif
