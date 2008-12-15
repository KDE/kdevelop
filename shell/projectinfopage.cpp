/***************************************************************************
 *   Copyright (C) 2008 by Andreas Pakulat <apaku@gmx.de                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "projectinfopage.h"
#include "ui_projectinfopage.h"

namespace KDevelop
{

ProjectInfoPage::ProjectInfoPage( QWidget* parent )
        : QWidget( parent )
{
    page_ui = new Ui::ProjectInfoPage;
    page_ui->setupUi( this );
}

void ProjectInfoPage::setProjectDir( const KUrl& url )
{
}

}

#include "projectinfopage.moc"
