/***************************************************************************
 *   Copyright 2008 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_PLUGIN_CVSDIFFJOB_H
#define KDEVPLATFORM_PLUGIN_CVSDIFFJOB_H

#include "cvsjob.h"

#include <vcs/vcsdiff.h>

/**
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class CvsDiffJob : public CvsJob
{
    Q_OBJECT
public:
    CvsDiffJob(KDevelop::IPlugin* parent, KDevelop::OutputJob::OutputJobVerbosity verbosity = KDevelop::OutputJob::Verbose);
    virtual ~CvsDiffJob();

    // Begin:  KDevelop::VcsJob
    virtual QVariant fetchResults();
    // End:  KDevelop::VcsJob

private:
    // Begin: KDevelop::DVcsJob
    virtual void slotProcessError(QProcess::ProcessError error);
    // End: KDevelop::DVcsJob
};

#endif
