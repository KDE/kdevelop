/***************************************************************************
 *   Copyright (C) 2007 by Andreas Pakulat                                 *
 *   apaku@gmx.de                                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "addfilesdialog.h"

#include <qlistview.h>
#include <klistview.h>
#include <qheader.h>
#include <qstringlist.h>
#include <klocale.h>

#include "addfilesdialogbase.h"

AddFilesDialog::AddFilesDialog( QStringList paths, QWidget* parent, const char* name )
        : KDialogBase( parent, name, true, i18n("Add newly created files to project"), KDialogBase::Ok|KDialogBase::Cancel )
{
    m_widget = new AddFilesDialogBase(this);
    m_widget->fileView->header()->hide();
    m_widget->fileView->addColumn(i18n("Path") );
    for( QStringList::const_iterator it = paths.begin(); it != paths.end(); ++it)
    {
        addPath(0, *it);
    }
    setMainWidget( m_widget );
    resize( 300,400 );
}

AddFilesDialog::~AddFilesDialog()
{}

/*$SPECIALIZATION$*/
void AddFilesDialog::reject()
{
    KDialogBase::reject();
}

void AddFilesDialog::accept()
{
    KDialogBase::accept();
}

void AddFilesDialog::addPath( QCheckListItem* item, const QString& path )
{
    if( path.isEmpty() )
        return;

    QStringList parts = QStringList::split("/", path );
    QString name = parts.first();
    parts.pop_front();
    QCheckListItem* i = createItem( item, name, parts.size() );
    i->setState( QCheckListItem::On );
    i->setTristate( true );
    addPath(i, parts.join("/") );
}

QCheckListItem* AddFilesDialog::createItem( QCheckListItem* parent, const QString& name, int count )
{
    QCheckListItem::Type t = QCheckListItem::CheckBox;
    if( count > 0 )
        t = QCheckListItem::CheckBoxController;

    if( parent == 0 )
    {
        QListViewItem* item = m_widget->fileView->firstChild();
        while( item )
        {
            if( item->text( 0 ) == name )
                return static_cast<QCheckListItem*>(item);
            item = item->nextSibling();
        }
        return new QCheckListItem( m_widget->fileView, name, t );
    }else
    {
        QListViewItem* item = parent->firstChild();
        while( item )
        {
            if( item->text( 0 ) == name )
                return static_cast<QCheckListItem*>(item);
            item = item->nextSibling();
        }
        return new QCheckListItem( parent, name, t );
    }
}

QStringList AddFilesDialog::excludedPaths() const
{
    return QStringList();
}

QStringList AddFilesDialog::includedPaths() const
{
    return QStringList();
}

#include "addfilesdialog.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
