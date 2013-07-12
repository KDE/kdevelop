/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_PLUGIN_SVNREVISIONWIDGET_H
#define KDEVPLATFORM_PLUGIN_SVNREVISIONWIDGET_H

#include <QWidget>
#include "svnrevision.h"
#include "ui_revisionwidget.h"

class SvnRevisionWidget : public QWidget, public Ui::SvnRevisionWidget
{
    Q_OBJECT
public:
    explicit SvnRevisionWidget( QWidget *parent );
    virtual ~SvnRevisionWidget();

    /// Returns the revision currently represented by widget
    SvnRevision revision();

    /// Install revision types to combobox. Delete every previously registered keywords.
    void installKeys( const QList<SvnRevision::Keyword> &keylist );

    /// Set revision number to int number input widget
    void setNumber( long revnum );

    /// Set revision keyword to combobox widget
    void setKey( SvnRevision::Keyword keyword );

    /// Enables specified type only. Disable all other type
    void enableType( SvnRevision::Type type );
};

#endif
