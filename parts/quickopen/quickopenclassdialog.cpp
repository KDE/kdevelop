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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */

#include "quickopenclassdialog.h"
#include "quickopen_part.h"

#include <kdevproject.h>
#include <kdevpartcontroller.h>

#include "doclineedit.h"

#include <klistbox.h>
#include <klocale.h>
#include <kdebug.h>

#include <qregexp.h>
#include <qlabel.h>

#if QT_VERSION < 0x030100
/* original source from qt-3.2.1/src/widgets/qlistbox.cpp
QListBoxItem* QListBox::selectedItem() const
{
    if ( d->selectionMode != Single )
	return 0;
    if ( isSelected( currentItem() ) )
	return  d->current;
    return 0;
}
*/
QListBoxItem* QListBox_selectedItem(QListBox* cpQListBox)
{
    if ( cpQListBox->selectionMode() != QListBox::Single )
    return 0;
    if ( cpQListBox->isSelected( cpQListBox->currentItem() ) )
    return  cpQListBox->item(cpQListBox->currentItem());
    return 0;
}
#endif


QuickOpenClassDialog::QuickOpenClassDialog(QuickOpenPart* part, QWidget* parent, const char* name, bool modal, WFlags fl)
    : QuickOpenDialogBase( parent, name, modal, fl ), m_part( part )
{
    nameLabel->setText( i18n("Class name:") );
    itemListLabel->setText( i18n("Class list:") );

    findAllClasses( m_classList );
    qHeapSort( m_classList );

    m_completion = new KCompletion();
    m_completion->insertItems( m_classList );
    m_completion->setIgnoreCase( true );

    nameEdit->setFocus();

    itemList->insertStringList( m_classList );
    itemList->setCurrentItem(0);

    connect(nameEdit, SIGNAL(upPressed()), this, SLOT(moveUpInList()));
    connect(nameEdit, SIGNAL(downPressed()), this, SLOT(moveDownInList()));
    connect(nameEdit, SIGNAL(pgupPressed()), this, SLOT(scrollUpInList()));
    connect(nameEdit, SIGNAL(pgdownPressed()), this, SLOT(scrollDownInList()));
    connect(nameEdit, SIGNAL(homePressed()), this, SLOT(goToBegin()));
    connect(nameEdit, SIGNAL(endPressed()), this, SLOT(goToEnd()));
}

QuickOpenClassDialog::~QuickOpenClassDialog()
{
    delete( m_completion );
    m_completion = 0;
}

/*$SPECIALIZATION$*/
void QuickOpenClassDialog::slotExecuted( QListBoxItem* /*item*/ )
{
    accept();
}

void QuickOpenClassDialog::reject()
{
    QDialog::reject();
}

void QuickOpenClassDialog::accept()
{
#if QT_VERSION >= 0x030100
    if( QListBoxItem* item = itemList->selectedItem() )
#else
    if( QListBoxItem* item = QListBox_selectedItem(itemList) )
#endif
    {
        ClassDom klass = findClass( item->text() );
        if( klass )
        {
            int startLine, startColumn;
            klass->getStartPosition( &startLine, &startColumn );
            m_part->partController()->editDocument( klass->fileName(), startLine );
        }
    }

    QDialog::accept();
}

void QuickOpenClassDialog::slotReturnPressed( )
{
    accept();
}

void QuickOpenClassDialog::slotTextChanged( const QString & text )
{
    itemList->clear();
    itemList->insertStringList( m_completion->substringCompletion(text) );
    itemList->setCurrentItem(0);
}

void QuickOpenClassDialog::moveUpInList( )
{
    if (itemList->currentItem() == -1)
        itemList->setCurrentItem(itemList->count() - 1);
    else
        itemList->setCurrentItem(itemList->currentItem() - 1);
    itemList->ensureCurrentVisible();
}

void QuickOpenClassDialog::moveDownInList( )
{
    if (itemList->currentItem() == -1)
        itemList->setCurrentItem(0);
    else
        itemList->setCurrentItem(itemList->currentItem() + 1);
    itemList->ensureCurrentVisible();
}

void QuickOpenClassDialog::scrollUpInList( )
{
    if (itemList->currentItem() == -1)
        itemList->setCurrentItem(itemList->count() - 1);
    else
        itemList->setCurrentItem(itemList->currentItem() - (itemList->numItemsVisible()-1));
    itemList->ensureCurrentVisible();
}

void QuickOpenClassDialog::scrollDownInList( )
{
    if (itemList->currentItem() == -1)
        itemList->setCurrentItem(0);
    else
        itemList->setCurrentItem(itemList->currentItem() + (itemList->numItemsVisible()-1));
    itemList->ensureCurrentVisible();
}

void QuickOpenClassDialog::goToBegin( )
{
    itemList->setCurrentItem(0);
}

void QuickOpenClassDialog::goToEnd( )
{
    itemList->setCurrentItem(itemList->count()-1);
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

