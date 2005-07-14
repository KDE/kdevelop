/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "kdevdesigner_part.h"

#include <kinstance.h>
#include <kaction.h>
#include <kactionclasses.h>
#include <kstdaction.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>

#include <qfile.h>
#include <qtextstream.h>
#include <qtoolbar.h>
#include <qmenubar.h>
#include <qptrlist.h>
#include <qstatusbar.h>

#include "mainwindow.h"
#include "designeraction.h"
#include "formwindow.h"

KDevDesignerPart::KDevDesignerPart( QWidget *parentWidget, const char *// widgetName
                                    ,
                                  QObject *parent, const char *name, const QStringList &args )
    : KInterfaceDesigner::Designer(parent, name)
{
    setInstance( KDevDesignerPartFactory::instance() );

    m_widget = new MainWindow( this, true );
    m_widget->reparent(parentWidget, QPoint(0,0));
    setupDesignerWindow();

    setWidget(m_widget);

    setupActions();
    if (args.contains("in shell"))
        setXMLFile("kdevdesigner_part_sh.rc");
    else
        setXMLFile("kdevdesigner_part.rc");

    setReadWrite(true);
    setModified(false);

    connect(m_widget, SIGNAL(formModified(bool )), this, SLOT(formModified(bool)));
}

void KDevDesignerPart::setupDesignerWindow()
{
    m_widget->menuBar()->hide();
    m_widget->layoutToolBar->hide();
    m_widget->projectToolBar->hide();
    m_widget->toolsToolBar->hide();
    m_widget->statusBar()->hide();
}

static QIconSet createPartIconSet( const QString &name )
{
    QIconSet ic( BarIcon( "" + name, KDevDesignerPartFactory::instance() ) );
    QString prefix = "designer_";
    int right = name.length() - prefix.length();
    ic.setPixmap( BarIcon( prefix + "d_" + name.right( right ), KDevDesignerPartFactory::instance() ),
		  QIconSet::Small, QIconSet::Disabled );
    return ic;
}

void KDevDesignerPart::setupActions( )
{
    KAction *action;
    action = KStdAction::openNew(this, SLOT(fileNew()), actionCollection());
    action = KStdAction::open(this, SLOT(fileOpen()), actionCollection());
    action = KStdAction::close(this, SLOT(fileClose()), actionCollection());
    stateSync(action, m_widget->actionFileClose);
    action = KStdAction::save(this, SLOT(save()), actionCollection());
    stateSync(action, m_widget->actionFileSave);
    action = KStdAction::saveAs(this, SLOT(fileSaveAs()), actionCollection());
    stateSync(action, m_widget->actionFileSaveAs);
    action = new KAction(i18n("Save Al&l"), 0, this, SLOT(fileSaveAs()), actionCollection(), "file_saveall");
    stateSync(action, m_widget->actionFileSaveAll);
    new KAction(i18n("Create &Template..."), 0, this, SLOT(fileCreateTemplate()), actionCollection(), "file_createtemplate");

    action = KStdAction::undo(this, SLOT(editUndo()), actionCollection());
    stateSync(action, m_widget->actionEditUndo);
    action = KStdAction::redo(this, SLOT(editRedo()), actionCollection());
    stateSync(action, m_widget->actionEditRedo);
    action = KStdAction::cut(this, SLOT(editCut()), actionCollection());
    stateSync(action, m_widget->actionEditCut);
    action = KStdAction::copy(this, SLOT(editCopy()), actionCollection());
    stateSync(action, m_widget->actionEditCopy);
    action = KStdAction::paste(this, SLOT(editPaste()), actionCollection());
    stateSync(action, m_widget->actionEditPaste);
    action = new KAction(i18n("&Delete"), Key_Delete, this, SLOT(editDelete()), actionCollection(), "edit_delete");
    stateSync(action, m_widget->actionEditDelete);
    action = KStdAction::selectAll(this, SLOT(editSelectAll()), actionCollection());
    stateSync(action, m_widget->actionEditSelectAll);
    action = new KAction(i18n("Chec&k Accelerators"), ALT + Key_R, this, SLOT(editAccels()), actionCollection(), "edit_accels");
    stateSync(action, m_widget->actionEditAccels);
    action = new KAction(i18n("S&lots..."), createPartIconSet("designer_editslots.png"), 0, this, SLOT(editFunctions()), actionCollection(), "edit_functions");
    stateSync(action, m_widget->actionEditFunctions);
    action = new KAction(i18n("Co&nnections..."), createPartIconSet("designer_connecttool.png"), 0, this, SLOT(editConnections()), actionCollection(), "edit_connections");
    stateSync(action, m_widget->actionEditConnections);
    action = new KAction(i18n("&Form Settings..."), 0, this, SLOT(editFormSettings()), actionCollection(), "edit_formsettings");
    stateSync(action, m_widget->actionEditFormSettings);

    action = new KAction(i18n("&Add File..."), 0, this, SLOT(projectAddFile()), actionCollection(), "project_addfile");
    stateSync(action, m_widget->actionProjectAddFile);
    action = new KAction(i18n("&Image Collection..."), 0, this, SLOT(projectImageCollection()), actionCollection(), "project_imagecollection");
    stateSync(action, m_widget->actionEditPixmapCollection);
    action = new KAction(i18n("&Database Connections..."), 0, this, SLOT(projectDatabaseCollections()), actionCollection(), "project_databasecollections");
    stateSync(action, m_widget->actionEditDatabaseConnections);
    action = new KAction(i18n("&Designer Project Settings..."), 0, this, SLOT(projectSettings()), actionCollection(), "project_settings");
    stateSync(action, m_widget->actionEditProjectSettings);

    KRadioAction *toggle;
    toggle = new KRadioAction(i18n("&Pointer"), createPartIconSet("designer_pointer.png"), Key_F2, actionCollection(), "tools_pointer");
    setupToolsAction(toggle, m_widget->actionPointerTool);
    pointerAction = toggle;
    toggle = new KRadioAction(i18n("&Connect Signal/Slots"), createPartIconSet("designer_connecttool.png"), Key_F3, actionCollection(), "tools_connect");
    setupToolsAction(toggle, m_widget->actionConnectTool);
    toggle = new KRadioAction(i18n("Tab &Order"), createPartIconSet("designer_ordertool.png"), Key_F4, actionCollection(), "tools_taborder");
    setupToolsAction(toggle, m_widget->actionOrderTool);
    toggle = new KRadioAction(i18n("Set &Buddy"), createPartIconSet("designer_setbuddy.png"), Key_F12, actionCollection(), "tools_setbuddy");
    setupToolsAction(toggle, m_widget->actionBuddyTool);
    new KAction(i18n("Configure Toolbox..."), 0, this, SLOT(toolsConfigureToolbox()), actionCollection(), "tools_toolbox");
    new KAction(i18n("Edit &Custom Widgets..."), 0, this, SLOT(toolsEditCustomWidgets()), actionCollection(), "tools_editcustomwidgets");

    action = new KAction(i18n("Adjust &Size"), createPartIconSet("designer_adjustsize.png"), CTRL + Key_J, this, SLOT(layoutAdjustSize()), actionCollection(), "layout_adjustsize");
    stateSync(action, m_widget->actionEditAdjustSize);
    action = new KAction(i18n("Lay Out &Horizontally"), createPartIconSet("designer_edithlayout.png"), CTRL + Key_H, this, SLOT(layoutHLayout()), actionCollection(), "layout_h");
    stateSync(action, m_widget->actionEditHLayout);
    action = new KAction(i18n("Lay Out &Vertically"), createPartIconSet("designer_editvlayout.png"), CTRL + Key_V, this, SLOT(layoutVLayout()), actionCollection(), "layout_v");
    stateSync(action, m_widget->actionEditVLayout);
    action = new KAction(i18n("Lay Out in &Grid"), createPartIconSet("designer_editgrid.png"), CTRL + Key_G, this, SLOT(layoutGridLayout()), actionCollection(), "layout_grid");
    stateSync(action, m_widget->actionEditGridLayout);
    action = new KAction(i18n("Lay Out Horizontally (in S&plitter)"), createPartIconSet("designer_editvlayoutsplit.png"), 0, this, SLOT(layoutSplitHLayout()), actionCollection(), "layout_splith");
    stateSync(action, m_widget->actionEditSplitHorizontal);
    action = new KAction(i18n("Lay Out Vertically (in Sp&litter)"), createPartIconSet("designer_edithlayoutsplit.png"), 0, this, SLOT(layoutSplitVLayout()), actionCollection(), "layout_splitv");
    stateSync(action, m_widget->actionEditSplitVertical);
    action = new KAction(i18n("&Break Layout"), createPartIconSet("designer_editbreaklayout.png"), CTRL + Key_B, this, SLOT(layoutBreak()), actionCollection(), "layout_break");
    stateSync(action, m_widget->actionEditBreakLayout);
    toggle = new KRadioAction(i18n("Add Spacer"), createPartIconSet("designer_spacer.png"), 0, actionCollection(), "layout_spacer");
    setupToolsAction(toggle, m_widget->actionInsertSpacer);

    action = new KAction(i18n("Preview &Form"), CTRL + Key_T, this, SLOT(windowPreview()), actionCollection(), "window_preview");
    stateSync(action, m_widget->actionPreview);
    action = new KAction(i18n("Ne&xt Form"), CTRL + Key_F6, this, SLOT(windowNext()), actionCollection(), "window_next");
    stateSync(action, m_widget->actionWindowNext);
    action = new KAction(i18n("Pre&vious Form"), CTRL + SHIFT + Key_F6, this, SLOT(windowPrev()), actionCollection(), "window_prev");
    stateSync(action, m_widget->actionWindowPrevious);

    action = KStdAction::preferences(this, SLOT(editPreferences()), actionCollection());
    action->setText(i18n("Configure &KDevDesigner..."));
    stateSync(action, m_widget->actionEditPreferences);
}

KDevDesignerPart::~KDevDesignerPart()
{
}

void KDevDesignerPart::setReadWrite(bool rw)
{
    ReadWritePart::setReadWrite(rw);
}

void KDevDesignerPart::setModified(bool modified)
{
/*    KAction *save = actionCollection()->action(KStdAction::stdName(KStdAction::Save));
    if (!save)
        return;
    if (modified)
        save->setEnabled(true);
    else
        save->setEnabled(false);*/

    ReadWritePart::setModified(modified);
}

bool KDevDesignerPart::openFile()
{
    m_widget->fileOpen("", "", m_file);
    return true;
}

bool KDevDesignerPart::saveFile()
{
    if (isReadWrite() == false)
        return false;
    m_widget->actionFileSaveAll->activate();
    return true;
}

void KDevDesignerPart::stateSync( KAction * kaction, QAction * qaction )
{
    if (!qaction)
        return;
    kaction->setEnabled(qaction->isEnabled());
    DesignerAction *ac = dynamic_cast<DesignerAction*>(qaction);
    if (!ac)
        return;
    connect(ac, SIGNAL(actionEnabled(bool )), kaction, SLOT(setEnabled(bool )));
}

void KDevDesignerPart::setupToolsAction( KRadioAction * toggle, QAction * action )
{
    if (!action)
        return;

    toggle->setGroup("tools");
    toggle->setExclusiveGroup("tools");
    connect(action, SIGNAL(toggled(bool )), this, SLOT(setToggleActionChecked(bool )));
    connect(toggle, SIGNAL(toggled(bool)), this, SLOT(setToggleActionOn(bool)));
    toggle->setChecked(action->isOn());
    m_actionMap[action] = toggle;
    m_actionMap2[toggle] = action;
}


// It's usually safe to leave the factory code alone.. with the
// notable exception of the KAboutData data
#include <kaboutdata.h>
#include <klocale.h>

KInstance*  KDevDesignerPartFactory::s_instance = 0L;
KAboutData* KDevDesignerPartFactory::s_about = 0L;

KDevDesignerPartFactory::KDevDesignerPartFactory()
    : KParts::Factory()
{
}

KDevDesignerPartFactory::~KDevDesignerPartFactory()
{
    delete s_instance;
    delete s_about;

    s_instance = 0L;
}

KParts::Part* KDevDesignerPartFactory::createPartObject( QWidget *parentWidget, const char *widgetName,
                                                        QObject *parent, const char *name,
                                                        const char *classname, const QStringList &args )
{
    // Create an instance of our Part
    KDevDesignerPart* obj = new KDevDesignerPart( parentWidget, widgetName, parent, name, args );

    // See if we are to be read-write or not
    if (QCString(classname) == "KParts::ReadOnlyPart")
        obj->setReadWrite(false);

    return obj;
}

KInstance* KDevDesignerPartFactory::instance()
{
    if( !s_instance )
    {
        s_about = new KAboutData("kdevdesignerpart", I18N_NOOP("KDevDesignerPart"), "0.1");
        s_about->addAuthor("Alexander Dymo", 0, "cloudtemple@mksat.net");
        s_about->addAuthor("Trolltech AS", 0, "info@trolltech.com");
        s_instance = new KInstance(s_about);
    }
    return s_instance;
}

extern "C"
{
    void* init_libkdevdesignerpart()
    {
        return new KDevDesignerPartFactory;
    }
}

//actions

void KDevDesignerPart::fileNew( )
{
    if (!m_widget->actionNewFile)
    {
        qWarning("wrong m_widget->actionNewFile");
        return;
    }
    m_widget->actionNewFile->activate();
}

void KDevDesignerPart::fileOpen()
{
    m_widget->fileOpen();
}

void KDevDesignerPart::fileSaveAs()
{
    m_widget->actionFileSaveAs->activate();
}

void KDevDesignerPart::fileClose( )
{
    m_widget->actionFileClose->activate();
}

void KDevDesignerPart::fileCreateTemplate( )
{
    m_widget->fileCreateTemplate();
}

void KDevDesignerPart::editUndo( )
{
    m_widget->actionEditUndo->activate();
}

void KDevDesignerPart::editRedo( )
{
    m_widget->actionEditRedo->activate();
}

void KDevDesignerPart::editCut( )
{
    m_widget->actionEditCut->activate();
}

void KDevDesignerPart::editPaste( )
{
    m_widget->actionEditPaste->activate();
}

void KDevDesignerPart::editCopy( )
{
    m_widget->actionEditCopy->activate();
}

void KDevDesignerPart::editDelete( )
{
    m_widget->actionEditDelete->activate();
}

void KDevDesignerPart::editSelectAll( )
{
    m_widget->actionEditSelectAll->activate();
}

void KDevDesignerPart::editAccels( )
{
    m_widget->actionEditAccels->activate();
}

void KDevDesignerPart::editFunctions( )
{
    m_widget->actionEditFunctions->activate();
}

void KDevDesignerPart::editConnections( )
{
    m_widget->actionEditConnections->activate();
}

void KDevDesignerPart::editFormSettings( )
{
    m_widget->actionEditFormSettings->activate();
}

void KDevDesignerPart::editPreferences( )
{
    m_widget->actionEditPreferences->activate();
}

void KDevDesignerPart::projectAddFile( )
{
    m_widget->actionProjectAddFile->activate();
}

void KDevDesignerPart::projectImageCollection( )
{
    m_widget->actionEditPixmapCollection->activate();
}

void KDevDesignerPart::projectDatabaseCollections( )
{
    m_widget->actionEditDatabaseConnections->activate();
}

void KDevDesignerPart::projectSettings( )
{
    m_widget->actionEditProjectSettings->activate();
}

void KDevDesignerPart::toolsConfigureToolbox( )
{
    m_widget->toolsConfigure();
}

void KDevDesignerPart::layoutAdjustSize( )
{
    m_widget->actionEditAdjustSize->activate();
}

void KDevDesignerPart::layoutHLayout( )
{
    m_widget->actionEditHLayout->activate();
}

void KDevDesignerPart::layoutVLayout( )
{
    m_widget->actionEditVLayout->activate();
}

void KDevDesignerPart::layoutGridLayout( )
{
    m_widget->actionEditGridLayout->activate();
}

void KDevDesignerPart::layoutSplitHLayout( )
{
    m_widget->actionEditSplitHorizontal->activate();
}

void KDevDesignerPart::layoutSplitVLayout( )
{
    m_widget->actionEditSplitVertical->activate();
}

void KDevDesignerPart::layoutBreak( )
{
    m_widget->actionEditBreakLayout->activate();
}

void KDevDesignerPart::windowPreview( )
{
    m_widget->actionPreview->activate();
}

void KDevDesignerPart::windowNext( )
{
    m_widget->actionWindowNext->activate();
}

void KDevDesignerPart::windowPrev( )
{
    m_widget->actionWindowPrevious->activate();
}

void KDevDesignerPart::statusMessage( const QString & msg )
{
    emit setStatusBarText(msg);
}

void KDevDesignerPart::setToggleActionChecked( bool b)
{
    if (!sender())
        return;
    const QAction *action = dynamic_cast<const QAction*>(sender());
    if (!action)
        return;
    if (b)
    {
        if (action == m_widget->actionPointerTool)
            pointerAction->setChecked(true);
        return;
    }
    KRadioAction *kaction = m_actionMap[action];
    if (!kaction)
        return;
//    kdDebug() << "untoggle action: " << kaction->text() << endl;
    kaction->blockSignals(true);
    kaction->setChecked(b);
    kaction->blockSignals(false);
}

void KDevDesignerPart::setToggleActionOn( bool b )
{
    if (!sender())
        return;
    const KRadioAction *action = dynamic_cast<const KRadioAction *>(sender());
    if (!action)
        return;
//    kdDebug() << (b?QString("toggle"):QString("untoggle")) << " action: " << action->text() << endl;
    QAction *qaction = m_actionMap2[action];
    if (!qaction)
        return;
    disconnect(qaction, SIGNAL(toggled(bool )), this, SLOT(setToggleActionChecked(bool )));
    qaction->setOn(b);
    connect(qaction, SIGNAL(toggled(bool )), this, SLOT(setToggleActionChecked(bool )));
}

void KDevDesignerPart::openProject( const QString & // projectFile
                                    )
{
    //TODO: implement
    return;
}

KInterfaceDesigner::DesignerType KDevDesignerPart::designerType( )
{
    return KInterfaceDesigner::QtDesigner;
}

void KDevDesignerPart::emitAddedFunction( const QString & form, KInterfaceDesigner::Function func )
{
    kdDebug() << "KDevDesignerPart::emitAddedFunction: form " << form << ", function: " << func.function << endl;
    emit addedFunction(designerType(), form, func);
}

void KDevDesignerPart::emitRemovedFunction( const QString & form, KInterfaceDesigner::Function func )
{
    kdDebug() << "KDevDesignerPart::emitRemovedFunction: form " << form << ", function: " << func.function << endl;
    emit removedFunction(designerType(), form, func);
}

void KDevDesignerPart::emitEditedFunction( const QString & form, KInterfaceDesigner::Function oldFunc, KInterfaceDesigner::Function func )
{
    kdDebug() << "KDevDesignerPart::emitEditedFunction: form " << form
        << ", old function: " << oldFunc.function
        << ", function: " << func.function << endl;

//    m_widget->formWindow()->clearSelection(false);
    emit editedFunction(designerType(), form, oldFunc, func);
}

void KDevDesignerPart::emitEditFunction(const QString &formName, const QString &functionName)
{
    kdDebug() << "KDevDesignerPart::emitEditedFunction: form " << formName
        << ", function: " << functionName << endl;
    emit editFunction(designerType(), formName, functionName);
}

void KDevDesignerPart::formModified(bool b)
{
    kdDebug() << "KDevDesignerPart::formModified " << b << endl;
    setModified(b);
}

void KDevDesignerPart::emitEditSource(const QString &formName)
{
    emit editSource(designerType(), formName);
}

void KDevDesignerPart::emitNewStatus(const QString &formName, int status)
{
    emit newStatus(formName, status);
}

void KDevDesignerPart::toolsEditCustomWidgets( )
{
    m_widget->toolsCustomWidget();
}

#include "kdevdesigner_part.moc"
