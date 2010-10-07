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

#include <QProcess>

#include <kurl.h>

#include <outputview/outputjob.h>

namespace KDevelop
{
    class IOutputView;
    class IProject;
    class ProcessLineMaker;
}

class GrepOutputModel;
class GrepOutputDelegate;
class KProcess;

class GrepJob : public KDevelop::OutputJob
{
    Q_OBJECT

public:
    enum ErrorTypes {
        TemporaryFileError = UserDefinedError
    };

    GrepJob( QObject *parent = 0 );

    QString patternString() const;

    void setPatternString(const QString& patternString);
    void setTemplateString(const QString &templateString);
    void setFilesString(const QString &filesString);
    void setExcludeString(const QString &excludeString);
    void setDirectory(const KUrl &directory);
    void setRecursive(bool recursive);
    void setRegexpFlag(bool regexpFlag);
    void setCaseSensitive(bool caseSensitive);
    void setProjectFilesFlag(bool projectFilesFlag);

    virtual void start();

protected:
    virtual bool doKill();

    GrepOutputModel* model() const;

private Q_SLOTS:
    void slotFinished();
    void slotError(QProcess::ProcessError error);

private:
    static QString escape(const QString &str);

    KDevelop::ProcessLineMaker* m_lineMaker;
    QList<KProcess*> m_processes;

    QString m_patternString;

    QString m_templateString;
    QString m_filesString;
    QString m_excludeString;
    KUrl m_directory;

    bool m_useProjectFilesFlag;
    bool m_regexpFlag;
    bool m_recursiveFlag;
    bool m_caseSensitiveFlag;
};

#endif
