/***************************************************************************
 *   Copyright (C) 2003 by Roberto Raggi                                   *
 *   roberto@kdevelop.org                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "classbrowser_widget.h"
#include "classbrowser_part.h"
#include "catalog.h"

#include <qwhatsthis.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kaction.h>
#include <kdebug.h>

#include <kdevcore.h>
#include <kdevpartcontroller.h>
#include <kdevmainwindow.h>
#include <kdevcoderepository.h>
#include <kdevlanguagesupport.h>

#include <kcomboview.h>
#include <klistviewaction.h>


class TagListViewItem: public QListViewItem{
public:
    TagListViewItem(QListView *parent, Tag tag, QString name):
        QListViewItem(parent, name), m_tag(tag)
    {
    }
    Tag tag() const { return m_tag; }
private:
    Tag m_tag;
};


typedef KGenericFactory<ClassBrowserPart> ClassBrowserFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevclassbrowser, ClassBrowserFactory( "kdevclassbrowser" ) )

ClassBrowserPart::ClassBrowserPart( QObject* parent, const char* name, const QStringList& )
    : KDevPlugin("ClassBrowser", "classbrowser", parent, name ? name : "ClassBrowserPart" )
{
    setInstance( ClassBrowserFactory::instance() );
    setXMLFile( "kdevpart_classbrowser.rc" );

    m_widget = new ClassBrowserWidget( this );
    QWhatsThis::add( m_widget, i18n("Class browser") );

    setupActions();

    mainWindow()->embedSelectView( m_widget, i18n("Class Browser"), i18n("Class Browser") );

    connect( codeRepository(), SIGNAL(catalogRegistered(Catalog*)),
	     this, SLOT(slotCatalogAdded(Catalog*)) );
    connect( codeRepository(), SIGNAL(catalogUnregistered(Catalog*)),
	     this, SLOT(slotCatalogRemoved(Catalog*)) );
    connect( codeRepository(), SIGNAL(catalogChanged(Catalog*)),
	     this, SLOT(slotCatalogChanged(Catalog*)) );

    connect( core(), SIGNAL(projectOpened()), this, SLOT(slotProjectOpened()) );
    connect( core(), SIGNAL(projectClosed()), this, SLOT(slotProjectClosed()) );
    connect( core(), SIGNAL(languageChanged()), this, SLOT(slotProjectOpened()) );
}

ClassBrowserPart::~ClassBrowserPart()
{
    delete (ClassBrowserWidget*) m_widget;
}

void ClassBrowserPart::slotProjectOpened()
{
    QValueList<Catalog*> l = codeRepository()->registeredCatalogs();
    QValueList<Catalog*>::Iterator it = l.begin();
    while( it != l.end() ){
        Catalog* catalog = *it;
        ++it;

        m_widget->addCatalog( catalog );
    }

    connect( languageSupport(), SIGNAL(updatedSourceInfo()), this, SLOT(refresh()) );
}

void ClassBrowserPart::slotProjectClosed()
{
    //m_widget->clear();
}

void ClassBrowserPart::setupActions( )
{
    m_actionNamespaces = new KListViewAction( new KComboView(true), i18n("Namespaces"), 0, 0, 0, actionCollection(), "namespaces_combo" );
    connect( m_actionNamespaces->view(), SIGNAL(activated(QListViewItem*)), this, SLOT(selectNamespace(const QListViewItem*)) );

    m_actionClasses = new KListViewAction( new KComboView(true), i18n("Classes"), 0, 0, 0, actionCollection(), "classes_combo" );
    connect( m_actionClasses->view(), SIGNAL(activated(QListViewItem*)), this, SLOT(selectClass(const QListViewItem*)) );

    m_actionMethods = new KListViewAction( new KComboView(true), i18n("Methods"), 0, 0, 0, actionCollection(), "methods_combo" );
    connect( m_actionMethods->view(), SIGNAL(activated(QListViewItem*)), this, SLOT(selectMethod(QListViewItem*)) );
}

void ClassBrowserPart::slotCatalogAdded( Catalog * catalog )
{
    Q_UNUSED( catalog );
    refresh();
}

void ClassBrowserPart::slotCatalogRemoved( Catalog * catalog )
{
    Q_UNUSED( catalog );
    refresh();
}

void ClassBrowserPart::slotCatalogChanged( Catalog * catalog )
{
    Q_UNUSED( catalog );

/*    QString m_selectedNamespaces = m_actionNamespaces->currentText();
    QString m_selectedClasses = m_actionClasses->currentText();
    QString m_selectedMethods = m_actionMethods->currentText();
*/
    refresh();

    //m_actionNamespaces->setCurrentText( m_selectedNamespaces );
    //m_actionClasses->setCurrentText( m_selectedClasses );
    //m_actionMethods->setCurrentText( m_selectedMethods );
    //m_actionFields->setCurrentText( m_selectedFields );
}

void ClassBrowserPart::refresh( )
{
    kdDebug() << "---------------------------------- REFRESH!!!!!!!!" << endl;
    QValueList<Tag> namespaceList;

    QValueList<Catalog::QueryArgument> args;
    args << Catalog::QueryArgument( "kind", Tag::Kind_Namespace );

    m_actionNamespaces->view()->clear();
    QValueList<Catalog*> l = codeRepository()->registeredCatalogs();
    QValueList<Catalog*>::Iterator it = l.begin();
    while( it != l.end() ){
        Catalog* catalog = *it;
        ++it;

        namespaceList += catalog->query( args );
    }

    namespaceList = ClassBrowserUtils::simplifyNamespaces( namespaceList );

    new QListViewItem(m_actionNamespaces->view()->listView(), i18n("(Global Scope)"));
    QValueList<Tag>::Iterator dit = namespaceList.begin();
    while( dit != namespaceList.end() ){
        new TagListViewItem(m_actionNamespaces->view()->listView(), *dit, (*dit).path());
        ++dit;
    }

    refreshClasses();
    refreshMethods();
    adjust();
}

void ClassBrowserPart::selectNamespace( const QListViewItem * it )
{
    Q_UNUSED( it );

    refreshClasses();
    refreshMethods();
    adjust();
}

void ClassBrowserPart::selectClass( const QListViewItem * it )
{
    Q_UNUSED( it );

    refreshMethods();
    adjust();
}

void ClassBrowserPart::selectMethod( QListViewItem * it )
{
    TagListViewItem * tagItem = dynamic_cast<TagListViewItem*>(it);
    if (!tagItem)
        return;

    int line, col;
    tagItem->tag().getStartPosition( &line, &col );
    kdDebug() << "found tag at " << tagItem->tag().fileName() << " " << line << ", " << col << endl;
    partController()->editDocument( KURL(tagItem->tag().fileName()), line );
    adjust();
}

void ClassBrowserPart::refreshClasses( )
{
    QValueList<Tag> classList;

    QStringList scope;
    if (m_actionNamespaces->view()->currentItem())
    {
        TagListViewItem *tagItem = dynamic_cast<TagListViewItem*>(m_actionNamespaces->view()->currentItem());
        if (tagItem)
            scope = tagItem->tag().scope();
    }

    QValueList<Catalog::QueryArgument> args;
    args << Catalog::QueryArgument( "kind", Tag::Kind_Class )
        << Catalog::QueryArgument( "scope", scope );

    m_actionClasses->view()->clear();
    QValueList<Catalog*> l = codeRepository()->registeredCatalogs();
    QValueList<Catalog*>::Iterator it = l.begin();
    while( it != l.end() ){
        Catalog* catalog = *it;
        ++it;

        classList += catalog->query( args );
    }

    new QListViewItem(m_actionClasses->view()->listView(), i18n("(Globals)"));
    QValueList<Tag>::Iterator dit = classList.begin();
    while( dit != classList.end() ){
        new TagListViewItem(m_actionClasses->view()->listView(), *dit, (*dit).name());
        ++dit;
    }

    adjust();
}

void ClassBrowserPart::refreshMethods( )
{
    QValueList<Tag> methodList;

    QStringList scope;
    if (m_actionNamespaces->view()->currentItem())
    {
        TagListViewItem *tagItem = dynamic_cast<TagListViewItem*>(m_actionNamespaces->view()->currentItem());
        if (tagItem)
            scope = tagItem->tag().scope();
    }

    if (m_actionClasses->view()->currentItem())
    {
        TagListViewItem *tagItem = dynamic_cast<TagListViewItem*>(m_actionClasses->view()->currentItem());
        if (tagItem)
            scope << tagItem->tag().name();
    }

    QValueList<Catalog::QueryArgument> args;
    args << Catalog::QueryArgument( "kind", Tag::Kind_FunctionDeclaration )
        << Catalog::QueryArgument( "scope", scope );

    if( !m_selectedFileName.isEmpty() )
        args << Catalog::QueryArgument( "fileName", m_selectedFileName );

    kdDebug() << "inside refreshMethods" << endl;
    m_actionMethods->view()->clear();
    QValueList<Catalog*> l = codeRepository()->registeredCatalogs();
    QValueList<Catalog*>::Iterator it = l.begin();
    while( it != l.end() ){
        Catalog* catalog = *it;
        ++it;

        methodList += catalog->query( args );
    }

    new QListViewItem(m_actionMethods->view()->listView(), "");
    QValueList<Tag>::Iterator dit = methodList.begin();
    while( dit != methodList.end() ){
        new TagListViewItem(m_actionMethods->view()->listView(), *dit, languageSupport()->formatTag(*dit));
        ++dit;
    }

    adjust();
}

void ClassBrowserPart::adjust( )
{
/*    m_actionNamespaces->setComboWidth( 200 );
    m_actionClasses->setComboWidth( 150 );
    m_actionMethods->setComboWidth( 300 );*/
    m_actionNamespaces->view()->setMinimumWidth(200);
    m_actionClasses->view()->setMinimumWidth(150);
    m_actionMethods->view()->setMinimumWidth(150);
}

#include "classbrowser_part.moc"
