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

#include <kdevproject.h>
#include <kdevpartcontroller.h>
#include <kdevlanguagesupport.h>

#include <klistbox.h>
#include <klocale.h>
#include <kdebug.h>
#include <kcompletion.h>
#include <klineedit.h>

#include <qregexp.h>
#include <qlabel.h>

#include "quickopenclassdialog.h"
#include "quickopen_part.h"
#include "quickopenfunctionchooseform.h"

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
        ClassList klasses = findClass( item->text() );
        if( klasses.count() == 1 )
        {
            ClassDom klass = klasses.first();
            int startLine, startColumn;
            klass->getStartPosition( &startLine, &startColumn );
            m_part->partController()->editDocument( KURL( klass->fileName() ), startLine );
            selectClassViewItem( ItemDom(&(*klass)) );
        }
        else if (klasses.count() > 1 )
        {
            //several classes with the same name found
            QString fileStr;

            QuickOpenFunctionChooseForm fdlg( this, "" );
            fdlg.setCaption(i18n("Select The Location of Class %1").arg(klasses.first()->name()));
            fdlg.textLabel2->setText(i18n("Class name:"));

            for( ClassList::const_iterator it = klasses.constBegin(); it != klasses.constEnd() ; ++it )
            {
                ClassDom klass = *it;
                //assemble class name to display (maybe with scope info and specialization)
                QString classStr = m_part->languageSupport()->formatModelItem(klass);
                if(klass->hasSpecializationDeclaration())
                  classStr += klass->getSpecializationDeclaration();
                if(!klass->scope().isEmpty())
                  classStr += "   (in " + klass->scope().join("::") + ")";
                fdlg.argBox->insertItem(classStr);

                fileStr = KURL( klass->fileName() ).fileName();
                KURL full_url( klass->fileName() );
                KURL base_url( m_part->project()->projectDirectory()+"/" );
                fdlg.fileBox->insertItem(fileStr);
                fdlg.setRelativePath(fdlg.fileBox->count()-1,
                    KURL::relativeURL( base_url, full_url ));
            }
            if( fdlg.exec() ){
                int id = fdlg.argBox->currentItem();
                if( id>-1 && id < (int) klasses.count() ){
                    ClassDom model = klasses[id];
                    int line, col;
                    model->getStartPosition( &line, &col );
                    selectClassViewItem( ItemDom(&(*model)) );
                    QString fileNameStr = model->fileName();
                    m_part->partController()->editDocument( KURL(fileNameStr), line );
                }
            }
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

ClassList QuickOpenClassDialog::findClass( const QString& name )
{
    QStringList path = QStringList::split( "::", name );
    return findClass( path, m_part->codeModel()->globalNamespace() );
}

ClassList QuickOpenClassDialog::findClass( QStringList& path, const NamespaceDom ns )
{
    ClassList list;
    if( path.isEmpty() )
        return list;

    QString current = path.front();
    if( ns->hasNamespace(current) )
    {
        path.pop_front();
        list += findClass( path, ns->namespaceByName(current) );
        path.push_front( current );
    }

    if( ns->hasClass(current) )
    {
        path.pop_front();
        list += findClass( path, ns->classByName(current) );
    }

    return list;
}

ClassList QuickOpenClassDialog::findClass( QStringList& path, const ClassList klasses )
{
    ClassList list;
    if( path.isEmpty() )
    {
        list += klasses;
        return list;
    }

    for (ClassList::const_iterator it = klasses.constBegin(); it != klasses.constEnd(); ++it)
    {
        list += findClass(path, *it);
    }

    return list;
}

ClassList QuickOpenClassDialog::findClass( QStringList &path, const ClassDom klass )
{
    ClassList list;
    if( path.isEmpty() )
    {
        list << klass;
        return list;
    }

    QString current = path.front();
    if( klass->hasClass(current) )
    {
        path.pop_front();
        list += findClass( path, klass->classByName(current) );
        path.push_front(current);
    }

    return list;
}

#include "quickopenclassdialog.moc"

