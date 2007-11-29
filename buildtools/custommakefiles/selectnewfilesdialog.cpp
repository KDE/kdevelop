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


#include "selectnewfilesdialog.h"

#include <qlistview.h>
#include <klistview.h>
#include <qheader.h>
#include <qstringlist.h>
#include <klocale.h>
#include <kdebug.h>
#include "selectnewfilesdialogbase.h"

SelectNewFilesDialog::SelectNewFilesDialog( QStringList paths, QWidget* parent, const char* name )
        : KDialogBase( parent, name, true, i18n("Add newly created files to project"), KDialogBase::Ok|KDialogBase::Cancel )
{
    m_widget = new SelectNewFilesDialogBase(this);
    m_widget->fileView->header()->hide();
    m_widget->fileView->addColumn(i18n("Path") );
    for( QStringList::const_iterator it = paths.begin(); it != paths.end(); ++it)
    {
        addPath(0, *it);
    }
    setMainWidget( m_widget );
    resize( 300,400 );
}

SelectNewFilesDialog::~SelectNewFilesDialog()
{}

void SelectNewFilesDialog::slotCancel()
{
    excludePaths.clear();
    includePaths.clear();
    KDialogBase::slotCancel();
}

void SelectNewFilesDialog::checkItem( QCheckListItem* item, const QString& curpath )
{
    if( !item )
        return;

    QString path = curpath + item->text();
    if( item->state() != QCheckListItem::Off )
        includePaths << path;
    else
        excludePaths << path;
    if( item->firstChild() )
    {
        checkItem( static_cast<QCheckListItem*>(item->firstChild()), path+"/" );
    }
    if( item->nextSibling() )
    {
        checkItem( static_cast<QCheckListItem*>(item->nextSibling()), curpath );
    }
}

void SelectNewFilesDialog::slotOk()
{
    QCheckListItem* item = static_cast<QCheckListItem*> (m_widget->fileView->firstChild());
    checkItem( item, "" );
    kdDebug(9025) << "Inc List:" << includePaths << endl;
    kdDebug(9025) << "Exc List:" << excludePaths << endl;
    KDialogBase::slotOk();
}

void SelectNewFilesDialog::addPath( QCheckListItem* item, const QString& path )
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

QCheckListItem* SelectNewFilesDialog::createItem( QCheckListItem* parent, const QString& name, int count )
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

QStringList SelectNewFilesDialog::excludedPaths() const
{
    return excludePaths;
}

QStringList SelectNewFilesDialog::includedPaths() const
{
    return includePaths;
}

#include "selectnewfilesdialog.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
