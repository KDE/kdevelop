/*
 *  Copyright (C) 2003 Roberto Raggi (roberto@kdevelop.org)
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
 *  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#include <kdevproject.h>
#include <kdevpartcontroller.h>


#include <klistbox.h>
#include <klocale.h>
#include <kdebug.h>
#include <kcompletion.h>
#include <klineedit.h>

#include <qregexp.h>
#include <qlabel.h>

#include "quickopenclassdialog.h"
#include "quickopen_part.h"

QuickOpenClassDialog::QuickOpenClassDialog(QuickOpenPart* part, QWidget* parent, const char* name, bool modal, WFlags fl)
    : QuickOpenDialog( part, parent, name, modal, fl )
{
    nameLabel->setText( i18n("Class &name:") );
    itemListLabel->setText( i18n("Class &list:") );

    findAllClasses( m_classList );
    qHeapSort( m_classList );

    m_completion = new KCompletion();
    m_completion->insertItems( m_classList );
    m_completion->setIgnoreCase( true );

    nameEdit->setFocus();

    itemList->insertStringList( m_classList );
    itemList->setCurrentItem(0);
}

QuickOpenClassDialog::~QuickOpenClassDialog()
{
    delete( m_completion );
    m_completion = 0;
}

void QuickOpenClassDialog::slotExecuted( QListBoxItem* /*item*/ )
{
    accept();
}

void QuickOpenClassDialog::accept()
{
    if( QListBoxItem* item = itemList->selectedItem() )
    {
        ClassDom klass = findClass( item->text() );
        if( klass )
        {
            int startLine, startColumn;
            klass->getStartPosition( &startLine, &startColumn );
            m_part->partController()->editDocument( KURL( klass->fileName() ), startLine );
        }
    }

    QDialog::accept();
}

void QuickOpenClassDialog::slotReturnPressed( )
{
    accept();
}

void QuickOpenClassDialog::findAllClasses( QStringList& lst )
{
    findAllClasses( lst, m_part->codeModel()->globalNamespace() );
}

void QuickOpenClassDialog::findAllClasses( QStringList& lst, const ClassDom klass )
{
    QStringList fullName = klass->scope();
    fullName << klass->name();
    lst << fullName.join( "::" );

    const ClassList classList = klass->classList();
    for( ClassList::ConstIterator it=classList.begin(); it!=classList.end(); ++it )
        findAllClasses( lst, *it );
}

void QuickOpenClassDialog::findAllClasses( QStringList& lst, const NamespaceDom ns )
{
    const NamespaceList namespaceList = ns->namespaceList();
    for( NamespaceList::ConstIterator it=namespaceList.begin(); it!=namespaceList.end(); ++it )
        findAllClasses( lst, *it );

    const ClassList classList = ns->classList();
    for( ClassList::ConstIterator it=classList.begin(); it!=classList.end(); ++it )
        findAllClasses( lst, *it );
}

ClassDom QuickOpenClassDialog::findClass( const QString& name )
{
    QStringList path = QStringList::split( "::", name );
    return findClass( path, m_part->codeModel()->globalNamespace() );
}

ClassDom QuickOpenClassDialog::findClass( QStringList& path, const NamespaceDom ns )
{
    if( path.isEmpty() )
        return ClassDom();

    QString current = path.front();
    if( ns->hasNamespace(current) )
    {
        path.pop_front();
        if( ClassDom klass = findClass( path, ns->namespaceByName(current) ) )
            return klass;
        path.push_front( current );
    }

    if( ns->hasClass(current) )
    {
        path.pop_front();
        return findClass( path, ns->classByName(current)[0] );
    }

    return ClassDom();
}

ClassDom QuickOpenClassDialog::findClass( QStringList& path, const ClassDom klass )
{
    if( path.isEmpty() )
        return klass;

    QString current = path.front();
    if( klass->hasClass(current) )
    {
        path.pop_front();
        return findClass( path, klass->classByName(current)[0] );
    }

    return ClassDom();
}

#include "quickopenclassdialog.moc"

