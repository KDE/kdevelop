/***************************************************************************
 *   Copyright (C) 2003 by Mario Scalas                                    *
 *   mario.scalas@libero.it                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TAGDIALOG_H
#define TAGDIALOG_H

#include "tagdialogbase.h"

/**
* Implementation for a dialog collecting data for tagging / branching
* CVS repositories.
*
* @author Mario Scalas
*/
class TagDialog : public TagDialogBase
{
    Q_OBJECT
public:
    TagDialog( const QString &caption, QWidget *parent = 0, const char *name = 0 );
    virtual ~TagDialog();

    /**
    * @return the tag name selected by the user
    */
    QString tagName() const;
    /**
    * @return the branch name selected by the user
    */
    QString branchName() const;
    /**
    * @return true if the user want to branch the selected files
    */
    bool isBranch() const;
    /**
    * @return true if operation must be enforced
    */
    bool force() const;

protected slots:
    virtual void accept();
};

#endif
