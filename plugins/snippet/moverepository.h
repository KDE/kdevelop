/***************************************************************************
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __MOVEREPOSITORY_H__
#define __MOVEREPOSITORY_H__

#include "ui_addrepository.h"
#include <kdialog.h>

class SnippetRepository;

/**
 * A dialog that allows to edit a repository.
 * The user can change the location of the repository.
 * The name of the repository can only be changed if it's a toplevel repository.
 * Otherwise the repo's name is identically to the directory name.
 * @see moverepository.ui
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class MoveRepository : public KDialog, private Ui::AddRepository
{
  Q_OBJECT

public:
  explicit MoveRepository(SnippetRepository* repo, QWidget* parent = 0);
  ~MoveRepository();

private:
    SnippetRepository* repo_;

public slots:
    void accept();
};

#endif

