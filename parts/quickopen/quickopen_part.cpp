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

#include "quickopen_part.h"
#include "quickopenclassdialog.h"
#include "quickopenfunctiondialog.h"
#include "quickopenfiledialog.h"

#include <kaction.h>
#include <kiconloader.h>
#include <klocale.h>
#include <klineedit.h>
#include <kdevgenericfactory.h>
#include <kdevplugininfo.h>

#include <kmainwindow.h>

#include <kparts/part.h>
#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/popupmenuinterface.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/selectioninterface.h>

#include <kdevmainwindow.h>
#include <kdevcore.h>
#include <kdevpartcontroller.h>
#include <kdevproject.h>
#include <codebrowserfrontend.h>


typedef KDevGenericFactory<QuickOpenPart> QuickOpenFactory;
static const KDevPluginInfo data("kdevquickopen");
K_EXPORT_COMPONENT_FACTORY( libkdevquickopen, QuickOpenFactory( data ) )

using namespace KTextEditor;

QuickOpenPart::QuickOpenPart(QObject *parent, const char *name, const QStringList& )
    : KDevPlugin(&data, parent, name ? name : "QuickOpenPart" )
{
    setInstance(QuickOpenFactory::instance());
    setXMLFile("kdevpart_quickopen.rc");

    m_actionQuickOpen = new KAction( i18n("Quick Open File..."), CTRL + ALT + Key_O,
				       this, SLOT(slotQuickFileOpen()),
				       actionCollection(), "quick_open" );
    m_actionQuickOpen->setToolTip(i18n("Quick open file in project"));
    m_actionQuickOpen->setWhatsThis(i18n("<b>Quick open</b><p>Provides a file name input form with completion listbox to quickly open file in a project."));

    m_actionQuickOpenClass = new KAction( i18n("Quick Open Class..."), CTRL + ALT + Key_C,
				          this, SLOT(slotQuickOpenClass()),
				          actionCollection(), "quick_open_class" );
    m_actionQuickOpenClass->setToolTip(i18n("Find class in project"));
    m_actionQuickOpenClass->setWhatsThis(i18n("<b>Find class</b><p>Provides a class name input form with completion listbox to quickly open a file where the class is defined."));

    m_actionFunctionOpen = new KAction( i18n("Quick Open Method..."), CTRL + ALT + Key_M, this, SLOT(slotQuickOpenFunction()), actionCollection(), "quick_open_function" );
    m_actionFunctionOpen->setToolTip(i18n("Quick open function in project"));

    m_switchToAction = new KAction(i18n("Switch To..."), KShortcut("CTRL+/"), this, SLOT(slotSwitchTo()), actionCollection(), "file_switchto");
    m_switchToAction->setToolTip(i18n("Switch to"));
    m_switchToAction->setWhatsThis(i18n("<b>Switch to</b><p>Prompts to enter the name of previously opened file to switch to."));

    connect( core(), SIGNAL(projectOpened()), this, SLOT(slotProjectOpened()) );
    connect( core(), SIGNAL(projectClosed()), this, SLOT(slotProjectClosed()) );
}


QuickOpenPart::~QuickOpenPart()
{
}

void QuickOpenPart::slotProjectOpened( )
{
}

void QuickOpenPart::slotProjectClosed( )
{
}

void QuickOpenPart::slotQuickFileOpen( )
{
    QuickOpenFileDialog dlg( this, mainWindow()->main() );
    dlg.exec();
}

void QuickOpenPart::slotQuickOpenClass( )
{
    QuickOpenClassDialog dlg( this, mainWindow()->main() );    
    dlg.nameEdit->setText(getWordInEditor());
    dlg.exec();
}

void QuickOpenPart::slotQuickOpenFunction()
{
    QuickOpenFunctionDialog dlg( this, mainWindow()->main() );
    dlg.nameEdit->setText(getWordInEditor());
    dlg.exec();
}

void QuickOpenPart::slotSwitchTo()
{
    QuickOpenFileDialog dlg( this, partController()->openURLs(), mainWindow()->main() );
    dlg.exec();
}


QString QuickOpenPart::getWordInEditor()
{
    KParts::ReadOnlyPart *
        ro_part = dynamic_cast<KParts::ReadOnlyPart*>(partController()->activePart());
    if (!ro_part)
        return "";
    SelectionInterface *selectIface = dynamic_cast<SelectionInterface*>(ro_part);
    ViewCursorInterface *cursorIface = dynamic_cast<ViewCursorInterface*>(ro_part->widget());
    EditInterface *editIface = dynamic_cast<EditInterface*>(ro_part);
    QString wordstr;
    bool hasMultilineSelection = false;
    if (selectIface && selectIface->hasSelection())
    {
        hasMultilineSelection = ( selectIface->selection().contains('\n') != 0 );
        if (!hasMultilineSelection)
            wordstr = selectIface->selection();
    }
    if (cursorIface && editIface)
    {
        uint line, col;
        line = col = 0;
        cursorIface->cursorPositionReal(&line, &col);
        QString linestr = editIface->textLine(line);
        if (wordstr.isEmpty() && !hasMultilineSelection)
        {
            int startPos = QMAX(QMIN((int)col, (int)linestr.length()-1), 0);
            int endPos = startPos;
            while (startPos >= 0 && 
                    ( linestr[startPos].isLetterOrNumber() || linestr[startPos] == '_' ) )
                startPos--;
            while (endPos < (int)linestr.length() && 
                    ( linestr[endPos].isLetterOrNumber() || linestr[endPos] == '_' ) )
                endPos++;
            wordstr = (startPos==endPos)? QString() : linestr.mid(startPos+1, endPos-startPos-1);
        }
    }
    return wordstr;
}

void QuickOpenPart::selectItem( ItemDom item )
{
    Extensions::KDevCodeBrowserFrontend* f = extension< Extensions::KDevCodeBrowserFrontend > ( "KDevelop/CodeBrowserFrontend" );
    
    if(f != 0) {
        ItemDom itemDom( &(*item) );
        f->jumpedToItem( itemDom );
    } else {
        kdDebug() << "could not find the proper extension\n";
    }
}


#include "quickopen_part.moc"
