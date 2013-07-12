/***************************************************************************
 *   Copyright 2008 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_PLUGIN_CVSANNOTATEJOB_H
#define KDEVPLATFORM_PLUGIN_CVSANNOTATEJOB_H

#include <vcs/vcsannotation.h>

#include "cvsjob.h"

/**
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class CvsAnnotateJob : public CvsJob
{
    Q_OBJECT
public:
    CvsAnnotateJob(KDevelop::IPlugin* parent, KDevelop::OutputJob::OutputJobVerbosity verbosity = KDevelop::OutputJob::Verbose);
    virtual ~CvsAnnotateJob();

    // Begin:  KDevelop::VcsJob
    virtual QVariant fetchResults();
    // End:  KDevelop::VcsJob

private:
    void parseOutput(const QString& jobOutput, const QString& workingDirectory, KDevelop::VcsAnnotation& annotateInfo);
};

#endif
