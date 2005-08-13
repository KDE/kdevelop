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

#ifndef CVSFORM_H
#define CVSFORM_H

#include "cvsformbase.h"

/**
An instance of this class is used by the AppWizard to collect
information about setting up the cvs repository.
*/
class CvsForm : public CvsFormBase
{
    Q_OBJECT
public:
    CvsForm( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );
    virtual ~CvsForm();

    QString module() const;
    QString vendor() const;
    QString message() const;
    QString release() const;
    QString location() const;
    QString cvsRsh() const;
    bool mustInitRoot() const;
};

#endif
