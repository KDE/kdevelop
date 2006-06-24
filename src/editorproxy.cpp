// Ewww... need this to access KParts::Part::setWidget(), so that kdevelop
// doesn't need to be rearchitected for multiple views before the lazy view
// creation can go in
#define protected public
#include <kparts/part.h>
#undef protected

#include <QWidget>
#include <QTimer>

#include <kdeversion.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kapplication.h>

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <kxmlguiclient.h>
#include <kxmlguifactory.h>
#include <kmainwindow.h>
#include <kactioncollection.h>
#include <klocale.h>
#include <kstdaccel.h>
#include <kaction.h>
#include <kglobal.h>

#include "toplevel.h"
#include "documentcontroller.h"
#include "core.h"


#include "editorproxy.h"


using namespace KTextEditor;

EditorProxy *EditorProxy::s_instance = 0;

EditorProxy::EditorProxy()
  : QObject()
{
}


EditorProxy *EditorProxy::getInstance()
{
  if (!s_instance)
    s_instance = new EditorProxy;

  return s_instance;
}


void EditorProxy::setCursorPosition(KParts::Part *part, const KTextEditor::Cursor& cursor)
{
  if ( cursor.line() < 0 )
    return;

  if (!qobject_cast<KTextEditor::Document*>(part))
    return;

  KTextEditor::View *view = qobject_cast<View *>(part->widget());

  Cursor c = cursor;
  if (c.column() == 1)
    c.setColumn(0);

  if (view)
    view->setCursorPosition(c);
}

void EditorProxy::installPopup( KParts::Part * part )
{
    View *view = qobject_cast<View *>(part->widget());
    if (!view)
        return;

    QMenu *popup = view->contextMenu();

    if (!popup) {
        popup = view->defaultContextMenu();
        if (!popup) {
            kWarning() << k_funcinfo << "Popup not found!" << endl;
            return;
        }
        view->setContextMenu(popup);
    }

    KConfig *config = KGlobal::config();
    config->setGroup("UI");
    bool m_tabBarShown = ! config->readEntry("TabWidgetVisibility", 0);
    if (!m_tabBarShown)
    {
        KAction * action = TopLevel::getInstance()->main()->actionCollection()->action( "file_close" );
        if ( action && !action->isPlugged( popup ) )
        {
            popup->insertSeparator( popup->actions().count() ? popup->actions().first() : 0L );
            popup->insertAction( popup->actions().count() ? popup->actions().first() : 0L, action );
        }
        action = TopLevel::getInstance()->main()->actionCollection()->action( "file_closeother" );
        if ( action && !popup->actions().contains( action ) )
            popup->insertAction( popup->actions().count() > 1 ? popup->actions()[1] : 0L, action );
    }

    connect(view, SIGNAL(contextMenuAboutToShow(KTextEditor::View*, QMenu*)), this, SLOT(popupAboutToShow(KTextEditor::View*, QMenu*)));
}

void EditorProxy::popupAboutToShow(KTextEditor::View* view, QMenu* menu)
{
    Q_ASSERT(view && menu);
    return;
/*
    KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(DocumentController::getInstance()->activePart());
    if (!ro_part)
        return;

    // fill the menu in the editor context
    if (!ro_part->widget())
        return;

    QString wordstr, linestr;
    bool hasMultilineSelection = false;
    if( view->selection() )
    {
        Range selection = view->selectionRange();
        hasMultilineSelection = selection.start().line() == selection.end().line();
        if ( !hasMultilineSelection )
        {
            wordstr = view->selectionText();
        }
    }

    Cursor pos = view->cursorPosition();
    linestr = view->document()->line(pos.line());

    if( wordstr.isEmpty() && !hasMultilineSelection ) {
        int startPos = qMax(qMin(pos.column(), linestr.length() - 1), 0);
        int endPos = startPos;
        while (startPos >= 0 && ( linestr[startPos].isLetterOrNumber() || linestr[startPos] == '_' ) )
            startPos--;
        while (endPos < linestr.length() && ( linestr[endPos].isLetterOrNumber() || linestr[endPos] == '_' ) )
            endPos++;
        wordstr = (startPos==endPos)?
            QString() : linestr.mid(startPos+1, endPos-startPos-1);
    }
    kDebug(9000) << "Word:" << wordstr << ":" << endl;
    EditorContext context(ro_part->url(), pos, linestr, wordstr);
    Core::getInstance()->fillContextMenu(menu, &context);

    // Remove redundant separators (any that are first, last, or doubled)
    bool lastWasSeparator = true;
    foreach (QAction* action, menu->actions()) {
        if( lastWasSeparator && action->isSeparator() ) {
            menu->removeAction( action );
        } else {
            lastWasSeparator = action->isSeparator();
        }
    }

    if ( lastWasSeparator && menu->actions().count() )
        menu->removeAction( menu->actions().last() );
*/
}

QWidget * EditorProxy::widgetForPart( KParts::Part * part )
{
    if ( !part ) return 0;
    return part->widget();
}

QWidget * EditorProxy::topWidgetForPart( KParts::Part * part )
{
    if ( !part ) return 0;
    return part->widget();
}


#include "editorproxy.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
