/***************************************************************************
 *   Copyright (C) 1999 by Jonas Nordin                                    *
 *   jonas.nordin@syncom.se                                                *
 *   Copyright (C) 2000-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "classactions.h"

#include <qpopupmenu.h>
#include <qcombobox.h>
#include <qstringlist.h>
#include <qtl.h>
#include <kdebug.h>
#include <klocale.h>
#include <ktoolbar.h>

#include "kdevplugin.h"
#include "kdevlanguagesupport.h"
#include "classstore.h"

ClassListAction::ClassListAction(KDevPlugin *part, const QString &text, 
				 const KShortcut& cut,
                                 const QObject *receiver, const char *slot,
                                 KActionCollection *parent, const char *name)
    : KWidgetAction( m_combo = new QComboBox(), text, cut, 0, 0, parent, name), m_part(part)
{
#if (QT_VERSION >= 0x030100)
    m_combo->setEditable( true );
    m_combo->setAutoCompletion( true );
#endif
    
    m_combo->setMinimumWidth( 200 );
    m_combo->setMaximumWidth( 400 );
    
    //connect( m_combo, SIGNAL(returnPressed()), receiver, slot );
    connect( m_combo, SIGNAL(activated(const QString&)), receiver, slot );
    
    setShortcutConfigurable( false );
    setAutoSized( true );
}
 

void ClassListAction::setCurrentItem(const QString & /*item*/)
{
//    m_combo->setCurrentItem( item );
}


void ClassListAction::setCurrentClassName(const QString &name)
{
    setCurrentItem( m_part->languageSupport()->formatClassName(name) );
}


QString ClassListAction::currentClassName()
{
    QString text = m_combo->currentText();
    if (text == i18n("(Globals)"))
        return QString::null;
        
    return m_part->languageSupport()->unformatClassName( text );
}


void ClassListAction::refresh()
{
    ClassStore *store = m_part->classStore();
    KDevLanguageSupport *ls = m_part->languageSupport();
    
    QStringList rawList = store->getSortedClassNameList();

    QStringList list;
    list << i18n("(Globals)");

    QStringList::ConstIterator it;
    for (it = rawList.begin(); it != rawList.end(); ++it)
        list << ls->formatClassName(*it);

    m_combo->clear();
    m_combo->insertStringList( list );
}


MethodListAction::MethodListAction(KDevPlugin *part, const QString &text, const KShortcut& accel,
                                   const QObject *receiver, const char *slot,
                                   KActionCollection *parent, const char *name)
    : KWidgetAction( m_combo = new QComboBox(), text, accel, 0, 0, parent, name), m_part(part)
{    
#if (QT_VERSION >= 0x030100)
    m_combo->setEditable( true );
    m_combo->setAutoCompletion( true );
#endif
    
    m_combo->setMinimumWidth( 200 );
    m_combo->setMaximumWidth( 400 );
    
    //connect( m_combo, SIGNAL(returnPressed()), receiver, slot );
    connect( m_combo, SIGNAL(activated(const QString&)), receiver, slot );
    
    setShortcutConfigurable( false );
    setAutoSized( true );
}


static QString method2string( ParsedMethod* pm )
{
    return pm->asString();
}

void MethodListAction::refresh(const QString &className)
{
    kdDebug(9003) << "MethodListAction::refresh " << className << endl;
    ParsedClass *pc;
    QStringList list;

    ClassStore *store = m_part->classStore();

    if (className.isEmpty()) {
        // Global functions
        ParsedScopeContainer *psc = store->globalScope();
        for (psc->methodIterator.toFirst(); psc->methodIterator.current(); ++psc->methodIterator) {
            QString str = method2string( psc->methodIterator.current() );
            list << str;
        }
    } else if ((pc = store->getClassByName(className)) != 0) {
        // Methods of the given class
        for (pc->methodIterator.toFirst(); pc->methodIterator.current(); ++pc->methodIterator) {
            QString str = method2string( pc->methodIterator.current() );
            list << str;
        }
        for (pc->slotIterator.toFirst(); pc->slotIterator.current(); ++pc->slotIterator) {
            QString str = method2string( pc->slotIterator.current() );
            list << str;
        }
    }

    qHeapSort(list);
    
    m_combo->clear();
    m_combo->insertStringList( list );
}


QString MethodListAction::currentMethodName()
{
    return m_combo->currentText();
}


DelayedPopupAction::DelayedPopupAction(const QString& text, const QString& pix, const KShortcut& accel,
                                       QObject *receiver, const char *slot,
                                       KActionCollection* parent, const char* name )
    : KAction(text, pix, accel, receiver, slot, parent, name)
{
    m_popup = 0;
}


DelayedPopupAction::~DelayedPopupAction()
{
    if ( m_popup )
        delete m_popup;
}


int DelayedPopupAction::plug(QWidget *widget, int index)
{
    if (widget->inherits("KToolBar")) {
        KToolBar *bar = (KToolBar *)widget;
        connect( bar, SIGNAL(destroyed()), this, SLOT(slotDestroyed()) );
        
        int id = KAction::getToolButtonID();
        bar->insertButton(icon(), id, SIGNAL( clicked() ), this,
                          SLOT(slotActivated()), isEnabled(), plainText(),
                          index);
        addContainer(bar, id);
        bar->setDelayedPopup(id, popupMenu(), true);
        
        return containerCount()-1;
    }
    
    return KAction::plug(widget, index);
}


void DelayedPopupAction::unplug(QWidget *widget)
{
    if (widget->inherits( "KToolBar")) {
        KToolBar *bar = (KToolBar *)widget;
        
        int idx = findContainer(bar);
        if (idx == -1)
            return;
        
        bar->removeItem(menuId(idx));
        removeContainer(idx);
        
        return;
    }
    
    KAction::unplug(widget);
}


QPopupMenu *DelayedPopupAction::popupMenu()
{
    if (!m_popup)
        m_popup = new QPopupMenu();
    
    return m_popup;
}

#if !(KDE_VERSION > 305)
#include <kapplication.h>
//--------------------begin-of-copy-of-kde-3.1-----------
KWidgetAction::KWidgetAction( QWidget* widget,
    const QString& text, const KShortcut& cut,
    const QObject* receiver, const char* slot,
    KActionCollection* parent, const char* name )
  : KAction( text, cut, receiver, slot, parent, name )
  , m_widget( widget )
  , m_autoSized( false )
{
}

KWidgetAction::~KWidgetAction()
{
}

void KWidgetAction::setAutoSized( bool autoSized )
{
  if( m_autoSized == autoSized )
    return;

  m_autoSized = autoSized;

  if( !m_widget || !isPlugged() )
    return;

  KToolBar* toolBar = (KToolBar*)m_widget->parent();
  int i = findContainer( toolBar );
  if ( i == -1 )
    return;
  int id = itemId( i );

  toolBar->setItemAutoSized( id, m_autoSized );
}

int KWidgetAction::plug( QWidget* w, int index )
{
  if (kapp && !kapp->authorizeKAction(name()))
      return -1;

  if ( !w->inherits( "KToolBar" ) ) {
    kdError() << "KWidgetAction::plug: KWidgetAction must be plugged into KToolBar." << endl;
    return -1;
  }
  if ( !m_widget ) {
    kdError() << "KWidgetAction::plug: Widget was deleted or null!" << endl;
    return -1;
  }

  KToolBar* toolBar = static_cast<KToolBar*>( w );

  int id = KAction::getToolButtonID();

  m_widget->reparent( toolBar, QPoint() );
  toolBar->insertWidget( id, 0, m_widget, index );
  toolBar->setItemAutoSized( id, m_autoSized );

  addContainer( toolBar, id );

  connect( toolBar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

  return containerCount() - 1;
}

void KWidgetAction::unplug( QWidget *w )
{
  // ### shouldn't this method check if w == m_widget->parent() ? (Simon)
  if( !m_widget )
    return;

  m_widget->reparent( 0L, QPoint(), false /*showIt*/ );

  KAction::unplug( w );
}


void KWidgetAction::virtual_hook( int id, void* data )
{ KAction::virtual_hook( id, data ); }

//--------------------end-of-copy-of-kde-3.1-----------
#endif // !(KDE_VERSION > 305)

#include "classactions.moc"
