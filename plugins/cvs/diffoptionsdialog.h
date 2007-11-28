/***************************************************************************
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DIFFOPTIONSDIALOG_H
#define DIFFOPTIONSDIALOG_H

#include <QDialog>
#include <KUrl>

#include "vcsrevision.h"
#include "ui_diffoptionsdialog.h"

/**
 * Allows the user to define which revisions to pass to @code cvs diff @endcode
 * Calling revA() and revB() returns the revisions the user wants to diff.
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class DiffOptionsDialog : public QDialog, private Ui::DiffOptionsDialogBase
{
    Q_OBJECT
public:
    DiffOptionsDialog(QWidget *parent, const KUrl& url);
    virtual ~DiffOptionsDialog();

    /**
     * @return The first diffing revision, branchname or tag
     * @note Can also be QString() if the user requested a diff agains BASE
     */
    KDevelop::VcsRevision revA() const;
    /**
     * @return The second diffing revision, branchname or tag
     * @note Can also be QString() if the user requested a diff agains a single revision
     */
    KDevelop::VcsRevision revB() const;

private:
    enum DiffType { diffLocalBASE, diffLocalHEAD, diffLocalOther, diffArbitrary };

    KUrl m_url;

    DiffType requestedDiff() const;
};

#endif
