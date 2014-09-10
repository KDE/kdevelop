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
    diff.setBaseDiff( QUrl::fromLocalFile(process()->workingDirectory()) );

    diff.setDiff( output() );

    /// @todo check output of "cvs diff" if it reported binary files
    diff.setContentType( KDevelop::VcsDiff::Text );

    /// @todo hmmm, we always call "cvs diff" with it's -u option
    ///       but if this option would be omitted cvs would return an other format
    diff.setType( KDevelop::VcsDiff::DiffUnified );

    return qVariantFromValue( diff );
}

void CvsDiffJob::slotProcessError(QProcess::ProcessError error) {
    // Do not blindly raise an error on non-zero return code of "cvs diff".
    // If its output contains the "Index:" mark, the diff is probably intact,
    // and non-zero return code indicates just that there are changes.
    if (error == QProcess::UnknownError && output().contains("Index:"))
        return;

    KDevelop::DVcsJob::slotProcessError(error);
}

