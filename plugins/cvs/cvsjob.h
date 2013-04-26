/***************************************************************************
 *   This file was partly taken from cervisia's cvsservice                 *
 *   Copyright 2002-2003 Christian Loose <christian.loose@hamburg.de>      *
 *                                                                         *
 *   Adapted for KDevelop                                                  *
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_PLUGIN_CVSJOB_H
#define KDEVPLATFORM_PLUGIN_CVSJOB_H


#include <QStringList>
#include <KProcess>

#include <vcs/dvcs/dvcsjob.h>

/**
 * This class is capable of running our cvs commands
 * Connect to Kjob::result(KJob*) to be notified when the job finished.
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class CvsJob : public KDevelop::DVcsJob
{
    Q_OBJECT
public:
    CvsJob(const QDir& workingDir, KDevelop::IPlugin* parent=0, KDevelop::OutputJob::OutputJobVerbosity verbosity = KDevelop::OutputJob::Verbose);

    KDE_DEPRECATED CvsJob(KDevelop::IPlugin* parent = 0, KDevelop::OutputJob::OutputJobVerbosity verbosity = KDevelop::OutputJob::Verbose);

    /**
     * @return The command that is executed when calling start()
     */
    QString cvsCommand();
    void clear();
    void setDirectory(const QString& directory);
    KDE_DEPRECATED QString getDirectory();

    void setRSH(const QString& rsh);
    void setServer(const QString& server);

    /**
     * Call this method to start this job.
     * @note Default communication mode is KProcess::AllOutput.
     * @see Use setCommunicationMode() to override the default communication mode.
     */
    virtual void start();

private:
    class CvsJobPrivate* const d;
};

#endif
