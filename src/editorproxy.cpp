// Ewww... need this to access KParts::Part::setWidget(), so that kdevelop
// doesn't need to be rearchitected for multiple views before the lazy view
// creation can go in
#define protected public
#include <kparts/part.h>
#undef protected

#include <qwidget.h>
#include <qtimer.h>

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

// FIXME remove - legacy
namespace KMdi {
   enum MdiMode {
      UndefinedMode  = 0,
      ToplevelMode   = 1,
      ChildframeMode = 2,
      TabPageMode    = 3,
      IDEAlMode      = 4
   };
}

EditorProxy::EditorProxy()
  : QObject()
{
    KConfig *config = KGlobal::config();
    config->setGroup("UI");
    //int mdimode = config->readEntry("MDIMode", KMdi::IDEAlMode);

    KAction *ac = new KAction( i18n("Show Context Menu"), TopLevel::getInstance()->main()->actionCollection(), "show_popup" );
    connect(ac, SIGNAL(triggered(bool)), SLOT(showPopup()));
    KShortcut cut ;/*= KStdAccel::shortcut(KStdAccel::PopupMenuContext);*/
    cut.append(Qt::CTRL + Qt::Key_Return);
    ac->setShortcut(cut);
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
        kWarning() << k_funcinfo << "Popup not found!" << endl;
        return;
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

    view->setContextMenu( popup );

    connect(popup, SIGNAL(aboutToShow()), this, SLOT(popupAboutToShow()));
}

void EditorProxy::popupAboutToShow()
{
#if 0 /// ### Qt4-porting
  QMenu *popup = qobject_cast<QMenu *>(sender());
  if (!popup)
    return;

  // ugly hack: remove all but the "original" items
  for (int index=popup->count()-1; index >= 0; --index)
  {
    int id = popup->idAt(index);
    if (m_popupIds.contains(id) == 0)
    {
      QMenuItem *item = popup->findItem(id);
          popup->removeItemAt(index);
//      kDebug(9000) << "removed id " << id << " at index " << index << endl;
    } else {
//        kDebug(9000) << "leaving id " << id << endl;
    }
  }
/*  // why twice !?!?
  // ugly hack: mark the "original" items
  m_popupIds.resize(popup->count());
  for (uint index=0; index < popup->count(); ++index)
    m_popupIds[index] = popup->idAt(index);
*/

  KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(DocumentController::getInstance()->activePart());
  if (!ro_part)
    return;
/*  // I disagree.. the EditorContext shouldn't emit the filecontext event
  // fill the menu in the file context
  FileContext context(ro_part->url().path(), false);
  Core::getInstance()->fillContextMenu(popup, &context);
*/
  // fill the menu in the editor context
  if (!ro_part->widget())
    return;

  SelectionInterface *selectIface = dynamic_cast<SelectionInterface*>(ro_part);
  ViewCursorInterface *cursorIface = dynamic_cast<ViewCursorInterface*>(ro_part->widget());
  EditInterface *editIface = dynamic_cast<EditInterface*>(ro_part);

  QString wordstr, linestr;
  bool hasMultilineSelection = false;
  if( selectIface && selectIface->hasSelection() )
  {
    hasMultilineSelection = ( selectIface->selection().contains('\n') != 0 );
    if ( !hasMultilineSelection )
    {
      wordstr = selectIface->selection();
    }
  }
  if( cursorIface && editIface )
  {
    uint line, col;
    line = col = 0;
    cursorIface->cursorPositionReal(&line, &col);
    linestr = editIface->textLine(line);
    if( wordstr.isEmpty() && !hasMultilineSelection ) {
      int startPos = qMax(qMin((int)col, (int)linestr.length()-1), 0);
      int endPos = startPos;
      while (startPos >= 0 && ( linestr[startPos].isLetterOrNumber() || linestr[startPos] == '_' ) )
          startPos--;
      while (endPos < (int)linestr.length() && ( linestr[endPos].isLetterOrNumber() || linestr[endPos] == '_' ) )
          endPos++;
      wordstr = (startPos==endPos)?
          QString() : linestr.mid(startPos+1, endPos-startPos-1);
    }
    kDebug(9000) << "Word:" << wordstr << ":" << endl;
    EditorContext context(ro_part->url(), line, col, linestr, wordstr);
    Core::getInstance()->fillContextMenu(popup, &context);
  } else {
    Core::getInstance()->fillContextMenu(popup, 0);
  }

  // Remove redundant separators (any that are first, last, or doubled)
  bool lastWasSeparator = true;
  for( uint i = 0; i < popup->count(); ) {
    int id = popup->idAt( i );
    if( lastWasSeparator && popup->findItem( id )->isSeparator() ) {
      popup->removeItem( id );
      // Since we removed an item, don't increment i
    } else {
      lastWasSeparator = false;
      i++;
    }
  }
  if( lastWasSeparator && popup->count() > 0 )
    popup->removeItem( popup->idAt( popup->count() - 1 ) );
#endif
}

void EditorProxy::showPopup( )
{
#if 0
    kDebug(9000) << k_funcinfo << endl;

    if ( KParts::Part * part = DocumentController::getInstance()->activePart() )
    {
        ViewCursorInterface *iface = dynamic_cast<ViewCursorInterface*>( part->widget() );
        if ( iface )
        {
            KTextEditor::View * view = static_cast<KTextEditor::View*>( part->widget() );
            Q3PopupMenu * popup = static_cast<Q3PopupMenu*>( view->factory()->container("ktexteditor_popup", view ) );

            popup->exec( view->mapToGlobal( iface->cursorCoordinates() ) );
        }
    }
#endif // ####TODO
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
