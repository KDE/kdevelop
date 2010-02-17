/***************************************************************************
 *   Copyright 2008 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cvsdiffjob.h"

#include <KDebug>
#include <QRegExp>

CvsDiffJob::CvsDiffJob(KDevelop::IPlugin* parent, KDevelop::OutputJob::OutputJobVerbosity verbosity)
    : CvsJob(parent, verbosity)
{
}

CvsDiffJob::~CvsDiffJob()
{
}

QVariant CvsDiffJob::fetchResults()
{
    KDevelop::VcsDiff diff;
    diff.setBaseDiff(KUrl("/"));

    diff.setDiff( output() );

    /// @todo check output of "cvs diff" if it reported binary files
    diff.setContentType( KDevelop::VcsDiff::Text );

    /// @todo hmmm, we always call "cvs diff" with it's -u option
    ///       but if this option would be omitted cvs would return an other format
    diff.setType( KDevelop::VcsDiff::DiffUnified );

    return qVariantFromValue( diff );
}

KDevelop::VcsJob::JobStatus CvsDiffJob::status() const
{
    KDevelop::VcsJob::JobStatus rv = CvsJob::status();

    // CVS has a bit of a weird return value handling.
    // Although cvs diff went ok it still returns non-zero
    if (rv == KDevelop::VcsJob::JobFailed) {
        // so if the output contains the "Index:" mark the diff seams to be ok 
        // -> change the return value according to this
        if (output().contains("Index:")) {
            rv = KDevelop::VcsJob::JobSucceeded;
        }
    }

    return rv;
}

#include "cvsdiffjob.moc"
