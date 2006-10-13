/***************************************************************************
*   Copyright (C) 2006 by Andreas Pakulat                                 *
*   apaku@gmx.de                                                          *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "createscopedlg.h"

#include <klineedit.h>
#include <kurlrequester.h>
#include <qcheckbox.h>
#include <kfile.h>
#include <kcombobox.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kurlcompletion.h>

#include "pathutil.h"
#include "qmakescopeitem.h"
#include "scope.h"
#include "trollprojectwidget.h"

CreateScopeDlg::CreateScopeDlg( QMakeScopeItem* item, QWidget* parent, const char* name, bool modal, WFlags fl )
        : CreateScopeDlgBase( parent, name, modal, fl ), m_item( item )
{
    incUrl->setMode( KFile::File | KFile::LocalOnly );
    incUrl->setCaption( i18n( "Choose existing .pri file or give a new filename for creation" ) );
    incUrl->setURL( QString::null );
    incUrl->completionObject() ->setDir( item->scope->projectDir() );
}

CreateScopeDlg::~CreateScopeDlg()
{}

void CreateScopeDlg::accept()
{
    Scope * s = 0;
    switch ( comboScopeType->currentItem() )
    {
        case 0:
            if ( !editScopeName->text().isEmpty() )
                s = m_item->scope->createSimpleScope( editScopeName->text() );
            break;
        case 1:
            if ( !editFunction->text().isEmpty() && !editArguments->text().isEmpty() )
                s = m_item->scope->createFunctionScope( editFunction->text(), editArguments->text() );
            break;
        case 2:
            if ( !incUrl->url().isEmpty() )
            {
                QString file = incUrl->url();
                if ( !incUrl->url().endsWith( ".pri" ) )
                    file += ".pri";

                // We need to create the file, because getRelativePath checks for existent paths
                if( !QFile::exists(file) )
                {
                    QFile temp(file);
                    if( temp.open(IO_WriteOnly) )
                        temp.close();
                }
                file = getRelativePath( m_item->scope->projectDir(), file );
                s = m_item->scope->createIncludeScope( file );
            }
            break;
    }
    if ( s )
    {
        if ( !m_item->firstChild() )
            new QMakeScopeItem( m_item, s->scopeName(), s );
        else
        {
            QListViewItem* item = m_item->firstChild();
            while( item->nextSibling() )
                item = item->nextSibling();
            QMakeScopeItem* newitem = new QMakeScopeItem( m_item, s->scopeName(), s );
            newitem->moveItem( item );
        }
        QDialog::accept();
    }
    else
    {
        if ( KMessageBox::warningYesNo( this, i18n( "You did not specify all needed information. "
                                        "The scope will not be created.<br>Do you want to abort the scope creation?" ),
                                        i18n( "Missing information" ) ) == KMessageBox:: Yes )
            QDialog::reject();
    }
}

#include "createscopedlg.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
