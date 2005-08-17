/*
 *  Copyright (C) 2003 Roberto Raggi (roberto@kdevelop.org)
 *  Copyright (C) 2005 Harald Fernengel (harry@kdevelop.org)
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
#include "quickopen_model.h"
#include "quickopen_filtermodel.h"

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

#include <QtGui/QtGui> // todo: remove me

static const KDevPluginInfo data("kdevquickopen");

typedef KDevGenericFactory<QuickOpenPart> QuickOpenFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevquickopen, QuickOpenFactory( data ) )

QuickOpenPart::QuickOpenPart(QObject *parent, const char *name, const QStringList& )
    : KDevPlugin(&data, parent)
{
    setObjectName(QString::fromUtf8(name));
    setInstance(QuickOpenFactory::instance());
    setXMLFile("kdevpart_quickopen.rc");

    m_actionQuickOpen = new KAction( i18n("Quick Open..."), Qt::CTRL + Qt::ALT + Qt::Key_O,
				       this, SLOT(slotQuickOpen()),
				       actionCollection(), "quick_open" );
    m_actionQuickOpen->setToolTip(i18n("Quick open locations in project"));
    m_actionQuickOpen->setWhatsThis(i18n("<b>Quick open</b><p>Provides an input form with completion to quickly open files or symbols in a project."));

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

void QuickOpenPart::slotQuickOpen( )
{
    QStringList list1;
    list1 << "model1, 0" << "model1, 1" << "model1, 2" << "model1, 3" << "model1, 4";
    QStringList list2;
    list2 << "model2, 0" << "model2, 1" << "model2, 2" << "model2, 3" << "model2, 4";

    QStringListModel model1(list1);
    QStringListModel model2(list2);

    QDialog dlg;
    QVBoxLayout layout(&dlg);
    QLineEdit edit(&dlg);
    QListView view(&dlg);
    QuickOpenModel model;
    QuickOpenFilterModel filter(&model);
    connect(&edit, SIGNAL(textChanged(QString)), &filter, SLOT(setFilter(QString)));
    model.addChildModel(&model1, "title1");
    model.addChildModel(&model2, "title2");

    if (project())
        model.addChildModel(new QStringListModel(project()->allFiles(), &model), "Project Files");

    view.setModel(&filter);
    layout.addWidget(&edit);
    layout.addWidget(&view);
    dlg.exec();
    //QuickOpenDialog dlg( this, mainWindow()->main() );
    //dlg.exec();
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
