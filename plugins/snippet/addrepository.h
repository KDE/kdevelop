/***************************************************************************
 *   Copyright 2007 by Robert Gruber                                   *
 *   rgruber@users.sourceforge.net                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __ADDREPOSITORY_H__
#define __ADDREPOSITORY_H__


#include "ui_addrepository.h"

/**
 * A dialog that allows to input a name and select a directory.
 * The directory will be added as a repository to the SnippetStore
 * @see addrepository.ui
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class AddRepository : public QDialog, public Ui::AddRepositoryBase
{
    Q_OBJECT

public:
    AddRepository(QWidget* parent = 0);
    virtual ~AddRepository();

private slots:
    void slotOpenDir();
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; indent-mode cstyle; replace-tabs on; auto-insert-doxygen on;
