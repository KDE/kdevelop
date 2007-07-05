/***************************************************************************
 *   Copyright (C) 2007 by Dukju Ahn                                       *
 *   dukjuahn@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SVNREVISIONWIDGET_H
#define SVNREVISIONWIDGET_H

#include <QWidget>
#include "subversion_utils.h"
#include "ui_svnrevisionwidget.h"

class SvnRevisionWidget : public QWidget, public Ui::SvnRevisionWidget
{
    Q_OBJECT
public:
    explicit SvnRevisionWidget( QWidget *parent );
    virtual ~SvnRevisionWidget();

    /// Returns the revision currently represented by widget
    SvnUtils::SvnRevision revision();

    /// Install revision types to combobox. Delete every previously registered keywords.
    void installKeys( const QList<SvnUtils::SvnRevision::RevKeyword> &keylist );

    /// Set revision number to int number input widget
    void setNumber( long revnum );

    /// Set revision keyword to combobox widget
    void setKey( SvnUtils::SvnRevision::RevKeyword keyword );

    /// Enables specified type only. Disable all other type
    void enableType( SvnUtils::SvnRevision::RevType type );
};

#endif
