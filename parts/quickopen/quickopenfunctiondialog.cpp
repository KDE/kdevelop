/*
 *  Copyright (C) 2004 Ahn, Duk J.(adjj22@kornet.net) (adjj1@hanmail.net)
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

#include <klocale.h>
#include <qlabel.h>
#include <qvaluelist.h>

#include <kcompletion.h>
#include <kdebug.h>
#include <klistbox.h>
#include <kmessagebox.h>

#include <kdevplugin.h>
#include <codemodel_utils.h>
#include <kdevpartcontroller.h>
#include <kdevproject.h>
#include <kdevlanguagesupport.h>


#include "quickopenfunctionchooseform.h"
#include "quickopenfunctiondialog.h"


void QuickOpenFunctionDialog::fillFunctions() {
    m_functionStrList->clear();
    for( FunctionDefinitionList::ConstIterator it = m_functionDefList->begin() ; it!=m_functionDefList->end(); ++it ){
        const FunctionDefinitionModel *fmodel = (*it).data();
        m_functionStrList->append( fmodel->name() );
    }
    m_completion->setItems( *m_functionStrList );
    itemList->clear();
    itemList->insertStringList( m_completion->items() );
}

QuickOpenFunctionDialog::QuickOpenFunctionDialog( QuickOpenPart *part, QWidget* parent, const char* name, bool modal, WFlags fl)
: QuickOpenDialog(part, parent, name, modal, fl), spaces(0)
{
        nameLabel->setText( i18n("Function &name:") );
        itemListLabel->setText( i18n("Function &list:") );

        FileList fileList = m_part->codeModel()->fileList();
        m_functionDefList = new FunctionDefinitionList();
        m_functionStrList = new QStringList();

        // for each one file, get all function list
        FileDom fileDom;
        for( FileList::Iterator it = fileList.begin() ; it!=fileList.end() ; ++it ){
                fileDom = *it;
                *m_functionDefList += CodeModelUtils::allFunctionDefinitionsDetailed( fileDom ).functionList;
        }
        m_completion = new KCompletion();
        //m_functionStrList->sort();
        m_completion->setOrder( KCompletion::Sorted );

        fillFunctions();

        itemList->setCurrentItem( 0 );
}

QuickOpenFunctionDialog::~QuickOpenFunctionDialog()
{
        delete m_completion;
        delete m_functionDefList;
        delete m_functionStrList;
        m_completion = 0;
        m_functionDefList = 0;
        m_functionStrList = 0;
}

void QuickOpenFunctionDialog::gotoFile( QString name )
{
        FunctionDefinitionModel *fmodel;
        //FunctionDefinitionList *funcList = new FunctionDefinitionList();
        funcList = new FunctionDefinitionList();
        FunctionDefinitionDom fdom;

        for( FunctionDefinitionList::ConstIterator it = m_functionDefList->begin() ; it!=m_functionDefList->end() ; ++it ){
                fdom = *it;
                fmodel = fdom.data();
                if( fmodel->name() == name ){
                        funcList->append( fdom );
                }
        }
        if( funcList->count() == 1 ){
                fdom = funcList->first();
                fmodel = fdom.data();
                QString fileNameStr = fmodel->fileName();
                int startline, startcol;
                fmodel->getStartPosition( &startline, &startcol );
                m_part->partController()->editDocument( KURL( fileNameStr), startline, startcol );
                selectClassViewItem( ItemDom(&(*fmodel)) );

        }else if( funcList->count() > 1 ){
                QString fileStr;

                QuickOpenFunctionChooseForm fdlg( this, name.ascii() );

                for( FunctionDefinitionList::Iterator it = funcList->begin() ; it!=funcList->end() ; ++it ){
                        fmodel = (*it).data();

                        fdlg.argBox->insertItem( m_part->languageSupport()->formatModelItem(fmodel) +
                                (fmodel->scope().isEmpty() ? "" : "   (in " + fmodel->scope().join("::") + ")"));
                        fileStr = KURL( fmodel->fileName() ).fileName();
                        KURL full_url( fmodel->fileName() );
                        KURL base_url( part()->project()->projectDirectory()+"/" );
                        fdlg.setRelativePath(fdlg.fileBox->count(),
                            KURL::relativeURL( base_url, full_url ));
                        fdlg.fileBox->insertItem(fileStr);
                }
                if( fdlg.exec() ){
                        int id = fdlg.argBox->currentItem();
                        if( id>-1 && id < (int) funcList->count() ){
                                FunctionDefinitionModel *model = (*funcList)[id].data();
                                int line, col;
                                model->getStartPosition( &line, &col );
                                selectClassViewItem( ItemDom(&(*model)) );
                                QString fileNameStr = model->fileName();
                                m_part->partController()->editDocument( KURL(fileNameStr), line );
                        }
                }
        }
        else{
                KMessageBox::error( this, i18n("Error: cannot find matching name function.") );
        }

        accept();
}

void QuickOpenFunctionDialog::slotExecuted(QListBoxItem* item)
{
        if( item ){
                gotoFile( item->text() );
        }
}

void QuickOpenFunctionDialog::executed(QListBoxItem*)
{
}

void QuickOpenFunctionDialog::slotTextChanged(const QString & text) {
    QString txt = text;
    //if(text.contains(':')/2 != 0) {
        QStringList parts = QStringList::split("::", text);
        if(text.endsWith("::") || parts.isEmpty()) {
            txt = "";
        }else{
            txt = parts.back();
            parts.pop_back();
        }

        if(text.contains(':')/2 != spaces) {
            if(text.contains(':')/2 < spaces) { ///reload all function-definitions
                m_functionDefList->clear();
                FileList fileList = m_part->codeModel()->fileList();
                FileDom fileDom;
                for( FileList::Iterator it = fileList.begin() ; it!=fileList.end() ; ++it ){
                    fileDom = *it;
                    *m_functionDefList += CodeModelUtils::allFunctionDefinitionsDetailed( fileDom ).functionList;
                }
            }

            if(!parts.isEmpty()) {
                FunctionDefinitionList accepted;
                FunctionDefinitionList::iterator it = m_functionDefList->begin();
                while(it != m_functionDefList->end()) {
                    QStringList scope = (*it)->scope();
                    QStringList::iterator mit = parts.begin();
                    QStringList::iterator sit = scope.begin();
                    bool fail = false;
                    while(mit != parts.end()) {
                        while(sit != scope.end() && *sit != *mit) ++sit;
                        if(sit == scope.end()) {
                            fail = true;
                            break;
                        }
                        ++mit;
                    }
                    if(!fail) accepted.append(*it);
                    ++it;
                }
                *m_functionDefList = accepted;
            }

            fillFunctions();

            spaces = text.contains(':')/2;
        }
    //}

    QuickOpenDialog::slotTextChanged(txt);
}


void QuickOpenFunctionDialog::slotReturnPressed()
{
        QListBoxItem *listboxItem = itemList->selectedItem();
        slotExecuted( listboxItem );
}

#include "quickopenfunctiondialog.moc"

