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
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

#include <kdevmainwindow.h>
#include <kdevcore.h>
#include <kdevpartcontroller.h>
#include <kdevproject.h>

typedef KDevGenericFactory<QuickOpenPart> QuickOpenFactory;
static const KDevPluginInfo data("kdevquickopen");
K_EXPORT_COMPONENT_FACTORY( libkdevquickopen, QuickOpenFactory( data ) )

using namespace KTextEditor;

QuickOpenPart::QuickOpenPart(QObject *parent, const char *name, const QStringList& )
    : KDevPlugin(&data, parent)
{
    setObjectName(QString::fromUtf8(name));
    setInstance(QuickOpenFactory::instance());
    setXMLFile("kdevpart_quickopen.rc");

    m_actionQuickOpen = new KAction( i18n("Quick Open File..."), Qt::CTRL + Qt::ALT + Qt::Key_O,
				       this, SLOT(slotQuickFileOpen()),
				       actionCollection(), "quick_open" );
    m_actionQuickOpen->setToolTip(i18n("Quick open file in project"));
    m_actionQuickOpen->setWhatsThis(i18n("<b>Quick open</b><p>Provides a file name input form with completion listbox to quickly open file in a project."));

    m_actionQuickOpenClass = new KAction( i18n("Quick Open Class..."), Qt::CTRL + Qt::ALT + Qt::Key_C,
				          this, SLOT(slotQuickOpenClass()),
				          actionCollection(), "quick_open_class" );
    m_actionQuickOpenClass->setToolTip(i18n("Find class in project"));
    m_actionQuickOpenClass->setWhatsThis(i18n("<b>Find class</b><p>Provides a class name input form with completion listbox to quickly open a file where the class is defined."));

    m_actionFunctionOpen = new KAction( i18n("Quick Open Method..."), Qt::CTRL + Qt::ALT + Qt::Key_M, this, SLOT(slotQuickOpenFunction()), actionCollection(), "quick_open_function" );
    m_actionFunctionOpen->setToolTip(i18n("Quick open function in project"));

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

QString QuickOpenPart::getWordInEditor()
{
    KParts::ReadOnlyPart *
        ro_part = dynamic_cast<KParts::ReadOnlyPart*>(partController()->activePart());
    if (!ro_part)
        return "";

    KTextEditor::Document *document = qobject_cast<KTextEditor::Document*>(ro_part);
    KTextEditor::View *view = qobject_cast<KTextEditor::View*>(ro_part->widget());

    QString wordstr;
    bool hasMultilineSelection = false;
    if (view && view->selection())
    {
        hasMultilineSelection = ( view->selectionText().contains('\n') != 0 );
        if (!hasMultilineSelection)
            wordstr = view->selectionText();
    }
    if (view && document)
    {
        KTextEditor::Cursor c = view->cursorPosition();
        int line = c.line(), col = c.column();

        QString linestr = document->line(line);
        if (wordstr.isEmpty() && !hasMultilineSelection)
        {
            int startPos = qMax(qMin((int)col, (int)linestr.length()-1), 0);
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

#include "quickopen_part.moc"
