
#include "kidetexteditor.h"

using namespace KTextEditor;

class View::ViewPrivate
{
public:
  ViewPrivate()
  {
  }
  ~ViewPrivate()
  {
  }

  Document *m_doc;
  bool m_bContextPopup;
};

View::View( Document *doc, QWidget *parent, const char *name )
: QWidget( parent, name )
{
  d = new ViewPrivate;
  d->m_doc = doc;
  d->m_bContextPopup = true;
}

View::~View()
{
  delete d;
}

Document *View::document() const
{
  return d->m_doc;
}

void View::insertText( const QString &text, bool mark )
{
  int line, col;
  getCursorPosition( &line, &col );
  document()->insertAt( text, line, col, mark );
} 

void View::setInternalContextMenuEnabled( bool b )
{
  emit scrollValueChanged(20); 
  d->m_bContextPopup = b;
}

bool View::internalContextMenuEnabled() const
{
  return d->m_bContextPopup;
}

class Document::DocumentPrivate
{
public:
  DocumentPrivate()
  {
  }
  ~DocumentPrivate()
  {
  }

};

Document::Document( QObject *parent, const char *name )
 : KParts::ReadWritePart( parent, name )
{
  d = new DocumentPrivate;
}

Document::~Document()
{
  //one never knows...
  QListIterator<View> it( m_views );
  for (; it.current(); ++it )
    disconnect( it.current(), SIGNAL( destroyed() ),
		this, SLOT( slotViewDestroyed() ) );

  delete d;
}

QList<View> Document::views() const
{
  return m_views;
}

void Document::addView( View *view )
{
  if ( !view )
    return;

  if ( m_views.findRef( view ) != -1 )
    return;

  m_views.append( view );
  connect( view, SIGNAL( destroyed() ),
	   this, SLOT( slotViewDestroyed() ) );
}

void Document::removeView( View *view )
{
  if ( !view )
    return;

  disconnect( view, SIGNAL( destroyed() ),
	      this, SLOT( slotViewDestroyed() ) );

  m_views.removeRef( view );
}

void Document::slotViewDestroyed()
{
  const View *view = static_cast<const View *>( sender() );
  m_views.removeRef( view );
}

#include "kidetexteditor.moc"
