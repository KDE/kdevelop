/* $Id$
 *
 *  Copyright (C) 2002 Roberto Raggi (raggi@cli.di.unipi.it)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */
#include "ccodetemplateopts.h"
#include "ccodetemplateopts.moc"
#include "caddtemplatedialog.h"
#include "kdevcodetemplate.h"

#include <kdebug.h>

#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qlistview.h>

CCodeTemplateOpts::CCodeTemplateOpts( QWidget *parent, const char *name )
    : CodeTemplateOptsDlg(parent,name)
{
    QAsciiDictIterator<Template> it( KDevCodeTemplate::self()->templates() );
    while( it.current() ){
        Template* templ = it.current();
        new QListViewItem( listTemplates,
                           QString::fromLatin1(it.currentKey()),
                           templ->description,
                           templ->code );
        ++it;
    }
}

CCodeTemplateOpts::~CCodeTemplateOpts()
{
}

void CCodeTemplateOpts::slotAddTemplate()
{
    CAddTemplateDialog dlg( this );
    if( dlg.exec() ){
        QString templ = dlg.templ();
        QString description = dlg.description();
        if( !(templ.isEmpty() || description.isEmpty()) ){
            QListViewItem* item = new QListViewItem( listTemplates, templ, description );
            listTemplates->setSelected( item, true );
            editCode->setFocus();
        }
    }
}

void CCodeTemplateOpts::slotRemoveTemplate()
{
    if( listTemplates->selectedItem() ){
        delete( listTemplates->selectedItem() );
    }
}

void CCodeTemplateOpts::slotSettingsChanged()
{
    QListViewItem* item = listTemplates->firstChild();
    while( item ){
        KDevCodeTemplate::self()->addTemplate( item->text(0),
                                               item->text(1),
                                               item->text(2) );
        item = item->nextSibling();
    }
    KDevCodeTemplate::self()->save();
}

void CCodeTemplateOpts::slotSelectionChanged()
{
    QListViewItem* item = listTemplates->selectedItem();
    if( item ){
        editCode->setText( item->text(2) );
    }
}

void CCodeTemplateOpts::slotTextChanged()
{
    QListViewItem* item = listTemplates->selectedItem();
    if( item ){
        item->setText( 2, editCode->text() );
    }
}

