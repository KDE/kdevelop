/***************************************************************************
 *   Copyright (C) 2003 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CLOSER_DIALOGIMPL_H
#define CLOSER_DIALOGIMPL_H

#include <qstringlist.h>

#include "closer_dialog.h"

class CloserDialogImpl : public CloserDialog
{
    Q_OBJECT

public:
    CloserDialogImpl( QStringList const & fileList );
    ~CloserDialogImpl();

    QStringList getCheckedFiles();

};

#endif

