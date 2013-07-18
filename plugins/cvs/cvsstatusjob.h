/***************************************************************************
 *   Copyright 2008 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_PLUGIN_CVSSTATUSJOB_H
#define KDEVPLATFORM_PLUGIN_CVSSTATUSJOB_H

#include "cvsjob.h"
#include <vcs/vcsstatusinfo.h>

/**
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class CvsStatusJob : public CvsJob
{
    Q_OBJECT
public:
    CvsStatusJob(KDevelop::IPlugin* parent, KDevelop::OutputJob::OutputJobVerbosity verbosity = KDevelop::OutputJob::Verbose);
    virtual ~CvsStatusJob();

    // Begin:  KDevelop::VcsJob
    virtual QVariant fetchResults();
    // End:  KDevelop::VcsJob

private:
    void parseOutput(const QString& jobOutput, QList<QVariant>& infos);
    void addInfoToList(QList<QVariant>& infos, 
            const QString& currentDir, const QString& filename, 
            const QString& statusString);

    KDevelop::VcsStatusInfo::State String2EnumState(const QString& stateAsString);

};

#endif
