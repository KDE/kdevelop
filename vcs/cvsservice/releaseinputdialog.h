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

#ifndef RELEASEINPUTDIALOG_H
#define RELEASEINPUTDIALOG_H

#include "releaseinputdialogbase.h"

/**
* Every time an operation needs to prompt the user about a release name,
* it can use this class: just customize the message to display
*/
class ReleaseInputDialog : public ReleaseInputDialogBase
{
  Q_OBJECT

public:
    /**
    * C-tor
    * @param parent
    */
    ReleaseInputDialog( QWidget* parent = 0 );
    /**
    * Destructor
    */
    virtual ~ReleaseInputDialog();

    /**
    * @return a QString formatted as "-r <RELEASE-TAG> " or "-D <RELEASE-DATE> "
    *         so it can be embedded in the command line.
    */
    QString release() const;

    /**
    * @return true if the user has checked "rever": enforce operation then even
    * if the files have been locally modified.
    */
    bool isRevert() const;

private:
    enum ReleaseType { byHead, byDate, byRevision };

    ReleaseType type() const;
};

#endif

