#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kaction.h>
#include "mdimainfrmcomponent.h"
#include "mdiframe.h"
#include "main.h"


MdiMainFrmComponent::MdiMainFrmComponent(QObject *parent, const char *name)
    : KDevComponent(parent, name)
{
    setInstance(MdiMainFrmFactory::instance());
    setXMLFile("kdevmdimainfrmcomponent.rc");

    m_widget = 0;
}


MdiMainFrmComponent::~MdiMainFrmComponent()
{}


void MdiMainFrmComponent::setupGUI()
{
    kdDebug(9005) << "Building QextMDI-MainFrame" << endl;

    m_widget = new MdiFrame(0L);
    emit embedWidget(m_widget, DocumentView, i18n("QextMdi-MainFrm"), i18n("source file views area"));

    KAction *action;
    action = new KAction( i18n("&Close"), 0, m_widget, SLOT(closeActiveView()),
                          actionCollection(), "close_view");
    action->setStatusText( i18n("Closes the focused document view") );
    action = new KAction( i18n("Close &All"), 0, m_widget, SLOT(closeAllViews()),
                          actionCollection(), "close_all_views");
    action->setStatusText( i18n("Closes all document views") );
    action = new KAction( i18n("&Iconify All"), 0, m_widget, SLOT(iconifyAllViews()),
                          actionCollection(), "iconify_all_views");
    action->setStatusText( i18n("Iconifies all document views") );

    action = new KAction( i18n("Ca&scade windows"), 0, m_widget, SLOT(cascadeWindows()),
                          actionCollection(), "cascade_windows");
    action->setStatusText( i18n("Repositiones all document views in a cascading order") );
    action = new KAction( i18n("Cascade &maximized"), 0, m_widget, SLOT(cascadeMaximized()),
                          actionCollection(), "cascade_maximized");
    action->setStatusText( i18n("Repositiones all document views maximized and in a cascading order") );
    action = new KAction( i18n("Expand &vertical"), 0, m_widget, SLOT(expandVertical()),
                          actionCollection(), "expand_vertical");
    action->setStatusText( i18n("Maximizes all document views in vertical direction, only") );
    action = new KAction( i18n("Expand &horizontal"), 0, m_widget, SLOT(expandHorizontal()),
                          actionCollection(), "expand_horizontal");
    action->setStatusText( i18n("Maximizes all document views in horizontal direction, only") );
    action = new KAction( i18n("A&nodine's tile"), 0, m_widget, SLOT(tileAnodine()),
                          actionCollection(), "anodines_tile");
    action->setStatusText( i18n("") );
    action = new KAction( i18n("&Pragma's tile"), 0, m_widget, SLOT(tilePragma()),
                          actionCollection(), "pragmas_tile");
    action->setStatusText( i18n("") );
    action = new KAction( i18n("Tile v&ertically"), 0, m_widget, SLOT(tileVertically()),
                          actionCollection(), "tile_vertically");
    action->setStatusText( i18n("Tiles all document views vertically") );

    action = new KAction( i18n("&Toplevel mode"), 0, m_widget, SLOT(switchToToplevelMode()),
                          actionCollection(), "toplevel_mode");
    action->setStatusText( i18n("Undocks all document views to toplevel windows and drop the views area of KDevelop") );
    action = new KAction( i18n("C&hildframe mode"), 0, m_widget, SLOT(switchToChildframeMode()),
                          actionCollection(), "childframe_mode");
    action->setStatusText( i18n("Docks all document views to the views area of KDevelop") );
}


void MdiMainFrmComponent::projectOpened(CProject *prj)
{
}


void MdiMainFrmComponent::projectClosed()
{
}
