/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2008 Andreas Pakulat <apaku@gmx.de>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "qtdesignerwidget.h"

#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerFormWindowManagerInterface>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormEditorPluginInterface>
#include <QtCore/QPluginLoader>
#include <QtGui/QMdiSubWindow>

#include <kdebug.h>
#include <kaction.h>
#include <kstandardaction.h>
#include <kactioncollection.h>

#include <sublime/view.h>

QtDesignerWidget::QtDesignerWidget( QWidget* parent, QDesignerFormWindowInterface* form )
    : QMdiArea( parent ), KXMLGUIClient(), m_form( form )
{
    //     area->setScrollBarsEnabled( true ); //FIXME commented just to make it compile with the new qt-copy
    //     area->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
    //     area->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOn );

    setXMLFile( "kdevqtdesigner.rc" );

    QMdiSubWindow* window = addSubWindow(m_form, Qt::Window | Qt::WindowShadeButtonHint | Qt::WindowSystemMenuHint | Qt::WindowTitleHint);
    const QSize containerSize = m_form->mainContainer()->size();
    const QSize containerMinimumSize = m_form->mainContainer()->minimumSize();
    const QSize containerMaximumSize = m_form->mainContainer()->maximumSize();
    const QSize decorationSize = window->geometry().size() - window->contentsRect().size();
    window->resize(containerSize+decorationSize);
    window->setMinimumSize(containerMinimumSize+decorationSize);
    if( containerMaximumSize == QSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX) )
        window->setMaximumSize(containerMaximumSize);
    else
        window->setMaximumSize(containerMaximumSize+decorationSize);
    window->setWindowTitle( form->mainContainer()->windowTitle() );
    connect( m_form, SIGNAL(changed()), this, SLOT(formChanged()));

    setupActions();
}

void QtDesignerWidget::setupActions()
{

    QDesignerFormWindowManagerInterface* manager = m_form->core()->formWindowManager();
    KActionCollection* ac = actionCollection();

    ac->addAction( "file_save", KStandardAction::save( this, SLOT( saveActiveDocument() ), ac) );
    ac->addAction( "adjust_size", manager->actionAdjustSize() );
    ac->addAction( "break_layout", manager->actionBreakLayout() );
    ac->addAction( "designer_cut", manager->actionCut() );
    ac->addAction( "designer_copy", manager->actionCopy() );
    ac->addAction( "designer_paste", manager->actionPaste() );
    ac->addAction( "designer_delete", manager->actionDelete() );
    ac->addAction( "layout_grid", manager->actionGridLayout() );
    ac->addAction( "layout_horiz", manager->actionHorizontalLayout() );
    ac->addAction( "layout_vertical", manager->actionVerticalLayout() );
    ac->addAction( "layout_split_horiz", manager->actionSplitHorizontal() );
    ac->addAction( "layout_split_vert", manager->actionSplitVertical() );
    ac->addAction( "designer_undo", manager->actionUndo() );
    ac->addAction( "designer_redo", manager->actionRedo() );
    ac->addAction( "designer_select_all", manager->actionSelectAll() );
    foreach (QObject *plugin, QPluginLoader::staticInstances())
    {
        QDesignerFormEditorPluginInterface *fep;

        if ( (fep = qobject_cast<QDesignerFormEditorPluginInterface*>(plugin)) )
        {
            if ( !fep->isInitialized() )
                fep->initialize(m_form->core());

            fep->action()->setCheckable(true);
            if( fep->action()->text() == "Edit Signals/Slots" )
                actionCollection()->addAction("signaleditor", fep->action());
            if( fep->action()->text() == "Edit Buddies" )
                actionCollection()->addAction("buddyeditor", fep->action());
            if( fep->action()->text() == "Edit Tab Order" )
                actionCollection()->addAction("tabordereditor", fep->action());

            kDebug(9038) << "Added action:" << fep->action()->objectName() << "|" << fep->action()->text();
        }
    }


}

#include "qtdesignerwidget.moc"

