/***************************************************************************
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef UPDATEOPTIONSDIALOG_H
#define UPDATEOPTIONSDIALOG_H

#include <QDialog>

#include "vcsrevision.h"

#include "ui_updateoptionsdialog.h"

/**
 * Allows the user to define the options to pass to @code cvs update @endcode
 * Calling options() returns a QString with the user specified options.
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class UpdateOptionsDialog : public QDialog, private Ui::UpdateOptionsDialogBase
{
  Q_OBJECT

public:
    UpdateOptionsDialog(QWidget* parent = 0);
    virtual ~UpdateOptionsDialog();

    /**
     * @return The revision to update the file to
     */
    KDevelop::VcsRevision revision();
};

#endif
