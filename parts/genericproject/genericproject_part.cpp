/***************************************************************************
 *   Copyright (C) 2003 Roberto Raggi                                      *
 *   roberto@kdevelop.org                                                  *
 *   Copyright (C) 2003 Alexander Dymo                                     *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "genericproject_widget.h"
#include "genericproject_part.h"

#include "kdevcore.h"
#include "kdevmainwindow.h"

#include <kiconloader.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <ktrader.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kparts/componentfactory.h>

#include <qwhatsthis.h>
#include <qdir.h>

#include "kdevbuildsystem.h"

#include "genericlistviewitem.h"
#include "variantserializer.h"

#if QT_VERSION < 0x030100
inline QString QDomDocument_toString(QDomDocument & cQDomDocument, int indent )
{
    QString str;
    QTextStream s( str, IO_WriteOnly );
    cQDomDocument.save( s, indent );

    return str;
};
#endif // Qt<3.1.0

typedef KGenericFactory<GenericProjectPart> GenericProjectFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevgenericproject, GenericProjectFactory( "kdevgenericproject" ) );

GenericProjectPart::GenericProjectPart(QObject *parent, const char *name, const QStringList& )
    : KDevProject("KDevPart", "kdevpart", parent, name ? name : "GenericProjectPart" )
{
    setInstance( GenericProjectFactory::instance() );
    setXMLFile( "kdevgenericproject.rc" );

    m_widget = new GenericProjectWidget( this );

    QWhatsThis::add(m_widget, i18n("Generic Project Manager"));

    mainWindow()->embedSelectViewRight( m_widget, i18n("Project Manager"), i18n("Project Manager") );

    m_mainBuild = new BuildGroupItem("/", 0);
    m_mainGroup = m_widget->addGroup(m_mainBuild);
}

GenericProjectPart::~GenericProjectPart()
{
    delete (GenericProjectWidget*) m_widget;
}

/*void GenericProjectPart::setMainGroup( BuildGroupItem * mainGroup )
{
    m_mainGroup = mainGroup;
    emit mainGroupChanged( m_mainGroup );
}*/

void GenericProjectPart::openProject( const QString & dirName, const QString & projectName )
{
    m_buildDir = dirName;
    m_projectDir = dirName;
    m_projectName = projectName;

    QDomDocument &dom = *projectDom();

    QString bsName = DomUtil::readEntry(dom, "/kdevgenericproject/backend");
    kdDebug() << "------------------------------------------------------" << endl;
    kdDebug() << "Looking for build system support for " << bsName << endl;

    KTrader::OfferList buildBackendsOffers =
        KTrader::self()->query(QString::fromLatin1("KDevelop/BuildSystems"),
                            QString::fromLatin1("[X-KDevelop-BuildSystem] == '%1'").arg(bsName));
    if (buildBackendsOffers.isEmpty()) {
        KMessageBox::sorry(0, i18n("No build system plugin for %1 found.").arg(bsName));
        return;
    }

    QValueList<KService::Ptr>::ConstIterator it;
    for (it = buildBackendsOffers.begin(); it != buildBackendsOffers.end(); ++it) {
        kdDebug() << (*it)->property("Name").toString() << endl;
        kdDebug() << (*it)->property("X-KDevelop-BuildSystem").toString() << endl;
    }

    kdDebug() << "Offers count: " << buildBackendsOffers.count() << endl;;

    KService::Ptr buildSystemService;
    KTrader::OfferList::ConstIterator serviceIt = buildBackendsOffers.begin();
    for ( ; serviceIt != buildBackendsOffers.end(); ++serviceIt )
       buildSystemService = *serviceIt;

/*    QStringList args;
    QVariant prop = buildSystemService->property( "X-KDevelop-Args" );
    if ( prop.isValid() )
        args = QStringList::split( " ", prop.toString() );  , this->name(), args */
/*    KDevBuildSystem *buildSystem = KParts::ComponentFactory
        ::createInstanceFromService<KDevBuildSystem>( buildSystemService, this,
        buildSystemService->name().latin1());*/
    int error;
    m_buildSystem = KParts::ComponentFactory
        ::createInstanceFromService<KDevBuildSystem>( buildSystemService, 0,
        buildSystemService->name().latin1(), QStringList(), &error);
    kdDebug() << "error: " << error << endl;

    if ( !m_buildSystem ) {
        KMessageBox::sorry(0, i18n("Could not create build system plugin for %1.").arg(bsName));
        return;
    }
    m_buildSystem->initProject(this);

/*    // Set the default directory radio to "executable"
    if (DomUtil::readEntry(dom, "/kdevpascalproject/run/directoryradio") == "" ) {
        DomUtil::writeEntry(dom, "/kdevpascalproject/run/directoryradio", "executable");
    }
*/
    loadProjectConfig(DomUtil::readEntry(dom, "/kdevgenericproject/project"));

}

void GenericProjectPart::closeProject( )
{
}

QString GenericProjectPart::projectDirectory( ) const
{
    return m_projectDir;
}

QString GenericProjectPart::projectName( ) const
{
    return m_projectName;
}

DomUtil::PairList GenericProjectPart::runEnvironmentVars( ) const
{
    return DomUtil::PairList();
}

QString GenericProjectPart::mainProgram( bool relative ) const
{
    return "";
}

QString GenericProjectPart::runDirectory( ) const
{
    return m_runDir;
}

QString GenericProjectPart::runArguments( ) const
{
    return "";
}

QString GenericProjectPart::activeDirectory( ) const
{
    return m_activeDir;
}

QString GenericProjectPart::buildDirectory( ) const
{
    return m_buildDir;
}

QStringList GenericProjectPart::allFiles( ) const
{
    return QStringList();
}

void GenericProjectPart::addFiles( const QStringList & fileList )
{
    BuildTargetItem *tit = m_widget->activeTarget();
    if (!tit)
        return;
    for (QStringList::const_iterator it = fileList.begin(); it != fileList.end(); ++it)
    {
        addFilePrivate(*it, tit);
    }
}

void GenericProjectPart::addFile( const QString & fileName )
{
    kdDebug() << "GenericProjectPart::addFile " << fileName << endl;
    BuildTargetItem *tit = m_widget->activeTarget();
    if (!tit)
        return;
    kdDebug() << "Active target is " << tit->name() << endl;
    addFilePrivate(fileName, tit);
}

void GenericProjectPart::addFilePrivate( const QString & fileName, BuildTargetItem *tit )
{
    kdDebug() << "GenericProjectPart::addFilePrivate " << fileName << endl;
    KURL url;
    url.setPath(projectDirectory() + "/" + fileName);
    kdDebug() << "GenericProjectPart::addFilePrivate " << url.prettyURL() << endl;
    BuildFileItem *fit = new BuildFileItem(url, tit);
    m_widget->addFile(fit);
}

void GenericProjectPart::removeFiles( const QStringList & fileList )
{
}

void GenericProjectPart::removeFile( const QString & fileName )
{
}

void GenericProjectPart::loadProjectConfig( QString projectFile )
{
    QDomDocument dom;
    DomUtil::openDOMFile(dom, QDir::cleanDirPath(projectDirectory() + "/" + projectFile));

#if QT_VERSION >= 0x030100
    kdDebug() << dom.toString(4) << endl;
#else
    kdDebug() << QDomDocument_toString(dom, 4) << endl;
#endif

    QDomElement docElem = dom.documentElement();

    QDomNode n = docElem.firstChild();
    while( !n.isNull() ) {
        QDomElement e = n.toElement();
        if( (!e.isNull()) && (e.tagName() == "group") )
        {
            kdDebug() << "GenericProjectPart: parsing group " << e.attribute("name") << endl;
            parseGroup(e, m_mainGroup);
        }
        n = n.nextSibling();
    }
}

void GenericProjectPart::parseGroup( const QDomElement & el, const GenericGroupListViewItem *parentGroup )
{
    GenericGroupListViewItem *group = createGroupItem(el, parentGroup);

    QDomNode n = el.firstChild();
    while( !n.isNull() ) {
        QDomElement e = n.toElement();
        if (e.isNull())
            return;
        if (e.tagName() == "group")
        {
            kdDebug() << "GenericProjectPart: parsing group " << e.attribute("name") << endl;
            parseGroup(e, group);
        }
        else if (e.tagName() == "target")
        {
            kdDebug() << "GenericProjectPart: parsing target " << e.attribute("name") << endl;
            parseTarget(e, group->groupItem());
        }

        n = n.nextSibling();
    }
}

void GenericProjectPart::parseTarget( const QDomElement & el, BuildGroupItem *group )
{
    BuildTargetItem *ti = createTargetItem(el, group);

    QDomNode n = el.firstChild();
    while( !n.isNull() ) {
        QDomElement e = n.toElement();
        if (e.isNull())
            return;
        if (e.tagName() == "file")
        {
            kdDebug() << "GenericProjectPart: parsing file " << e.attribute("name") << endl;
            parseFile(e, ti);
        }
        n = n.nextSibling();
    }
}

void GenericProjectPart::parseFile( const QDomElement & el, BuildTargetItem *target )
{
    createFileItem(el, target);
}

GenericGroupListViewItem * GenericProjectPart::createGroupItem( const QDomElement & el, const GenericGroupListViewItem *parent )
{
    BuildGroupItem *it = new BuildGroupItem(el.attribute("name"), parent->groupItem());
    kdDebug() << "adding group to widget " << el.attribute("name") << endl;
    GenericGroupListViewItem *git = m_widget->addGroup(it);

    QDomNode n = el.firstChild();
    while( !n.isNull() ) {
        QDomElement e = n.toElement();
        if (e.isNull())
            return 0;
        if (e.tagName() == "attribute")
        {
            it->setAttribute(e.attribute("name") , VariantSerializer::loadValue(e));
            kdDebug() << "GenericProjectPart: parsing attribute " << e.tagName() << endl;
        }
        n = n.nextSibling();
    }

    return git;
}

BuildTargetItem * GenericProjectPart::createTargetItem( const QDomElement & el, BuildGroupItem *group )
{
    BuildTargetItem *it = new BuildTargetItem(el.attribute("name"), group);
    return it;
}

BuildFileItem * GenericProjectPart::createFileItem( const QDomElement & el, BuildTargetItem *target )
{
    KURL url;
    kdDebug() << "GenericProjectPart::createFileItem url" << endl;
    url.setPath(QDir::cleanDirPath(projectDirectory() + "/" + target->parentGroup()->path() + "/" + el.attribute("name")));
    kdDebug() << "GenericProjectPart::createFileItem new" << endl;
    BuildFileItem *it = new BuildFileItem(url, target);
    return it;
}

KDevBuildSystem *GenericProjectPart::buildSystem( ) const
{
    return m_buildSystem;
}

#include "genericproject_part.moc"
