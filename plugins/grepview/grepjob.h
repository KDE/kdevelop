/***************************************************************************
 *   Copyright 1999-2001 by Bernd Gehrmann                                 *
 *   bernd@kdevelop.org                                                    *
 *   Copyright 2008 by Hamish Rodda                                        *
 *   rodda@kde.org                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef GREPJOB_H
#define GREPJOB_H

#include <iplugin.h>
#include <outputjob.h>

#include <QProcess>

namespace KDevelop
{
    class IOutputView;
    class IProject;
    class ProcessLineMaker;
}

class GrepOutputModel;
class GrepOutputDelegate;
class GrepViewPlugin;
class KProcess;

class GrepJob : public KDevelop::OutputJob
{
    Q_OBJECT

public:   
    enum ErrorTypes {
        TemporaryFileError = UserDefinedError
    };

    GrepJob( GrepViewPlugin *parent );

    QString patternString;
    QString templateString;
    QString filesString;
    QString excludeString;
    KUrl directory;

    bool useProjectFilesFlag;
    bool regexpFlag;
    bool recursiveFlag;
    bool noFindErrorsFlag;
    bool caseSensitiveFlag;
    
    KDevelop::IProject *project;

    virtual void start();
    
protected:
    virtual bool doKill();

    GrepOutputModel* model() const;
    
private Q_SLOTS:
    void slotFinished();
    void slotError(QProcess::ProcessError error);

private:
    static QString escape(const QString &str);
    GrepViewPlugin* plugin() const;
    
    KDevelop::IOutputView *m_view;
    KDevelop::ProcessLineMaker* m_lineMaker;
    QList<KProcess*> m_processes;
};

#endif
