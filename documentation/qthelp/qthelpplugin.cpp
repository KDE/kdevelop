/*  This file is part of KDevelop
    Copyright 2009 Aleix Pol <aleixpol@kde.org>
    Copyright 2010 Benjamin Port <port.benjamin@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "qthelpplugin.h"

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <KAboutData>

#include "qthelpsettings.h"
#include "qthelpprovider.h"

K_PLUGIN_FACTORY(QtHelpFactory, registerPlugin<QtHelpPlugin>(); )
K_EXPORT_PLUGIN(QtHelpFactory(KAboutData("kdevqthelp","kdevqthelp", ki18n("QtHelp"), "0.1", ki18n("Check Qt Help documentation"), KAboutData::License_GPL)))

QtHelpPlugin::QtHelpPlugin(QObject* parent, const QVariantList& args)
    : KDevelop::IPlugin(QtHelpFactory::componentData(), parent)
    , documentationProviders()
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IDocumentationProviderProvider )
    readConfig();
    Q_UNUSED(args);
<<<<<<< HEAD
    QStringList qmakes;
    QStringList tmp;
    KStandardDirs::findAllExe(tmp, "qmake");
    qmakes += tmp;
    KStandardDirs::findAllExe(tmp, "qmake-qt4");
    qmakes += tmp;
    QString dirName;
    foreach(const QString& qmake, qmakes) {
        /// check both in doc/ and doc/qch/
        dirName=qtDocsLocation(qmake)+"/qch/";
        QString fileName=dirName+"qt.qch";
        if(QFile::exists(fileName)) {
            kDebug() << "checking doc: " << fileName;
            break;
        } else
            dirName.clear();
            
        dirName=qtDocsLocation(qmake);
        fileName=dirName+"/qt.qch";
        if(QFile::exists(fileName)) {
            kDebug() << "checking doc: " << fileName;
            break;
        } else
            dirName.clear();
    }
    
    if(!dirName.isEmpty()) {
        QDir d(dirName);
        foreach(const QString& file, d.entryList(QDir::NoDotAndDotDot)) {
            QString fileName=dirName+'/'+file;
            QString fileNamespace = QHelpEngineCore::namespaceName(fileName);
            
            if (!fileNamespace.isEmpty() && !m_engine.registeredDocumentations().contains(fileNamespace)) {
                kDebug() << "loading doc" << fileName << fileNamespace;
                if(m_engine.registerDocumentation(fileName))
                    kDebug() << "documentation added successfully" << fileName;
                else
                    kDebug() << "error >> " << fileName << m_engine.error();
            }
        }
        kDebug() << "registered" << m_engine.error() << m_engine.registeredDocumentations();
    }
    else
        kDebug() << "no QtHelp found at all";
}

KSharedPtr< KDevelop::IDocumentation > QtHelpPlugin::documentationForDeclaration(KDevelop::Declaration* dec) const
{
	
	if(dec) {
        QStringList idList;
        {
        KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
        KDevelop::QualifiedIdentifier qid = dec->qualifiedIdentifier();
        for(int a = 0; a < qid.count(); ++a)
            idList << qid.at(a).identifier().str(); //Copy over the identifier components, without the template-parameters
        }
        
		QString id = idList.join("::");
		if(!id.isEmpty()) {
			QMap<QString, QUrl> links=m_engine.linksForIdentifier(id);
            
            kDebug() << "doc_found" << id << links;
			if(!links.isEmpty())
				return KSharedPtr<KDevelop::IDocumentation>(new QtHelpDocumentation(id, m_engine.linksForIdentifier(id)));
		}
	}
	
	return KSharedPtr<KDevelop::IDocumentation>();
}
=======
>>>>>>> Adapt QtHelpPlugin to IDocumentProviderProvider

}

void QtHelpPlugin::readConfig()
{
    QtHelpSettings::self()->readConfig();
    QStringList qtHelpPathList = QtHelpSettings::qchList();
    foreach(QString fileName,qtHelpPathList){
        QVariantList a;
        documentationProviders.append(new QtHelpProvider(this, QtHelpFactory::componentData(), fileName, a));
    }
}

QList<KDevelop::IDocumentationProvider*> QtHelpPlugin::providers()
{
    return documentationProviders;
}
