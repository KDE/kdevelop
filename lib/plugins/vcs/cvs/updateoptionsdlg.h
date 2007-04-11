/***************************************************************************
 *   Copyright (C) 2007 by Robert Gruber                                   *
 *   rgruber@users.sourceforge.net                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef UPDATEOPTIONSDLG_H
#define UPDATEOPTIONSDLG_H

#include <QDialog>

#include "ui_updateoptionsdlg.h"

/**
 * Allows the user to define the options to pass to @code cvs update @endcode
 * Calling options() returns a QString with the user specified options.
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class UpdateOptionsDlg : public QDialog, private Ui::UpdateOptionsDlgBase
{
  Q_OBJECT

public:
    UpdateOptionsDlg(QWidget* parent = 0);
    virtual ~UpdateOptionsDlg();

    /**
     * @return The options set by the use
     */
    QString options() const;
};

#endif
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
