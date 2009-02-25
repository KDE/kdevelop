/*  This file is part of KDevelop
    Copyright 2009 Aleix Pol <aleixpol@kde.org>

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
#include <QWebView>
#include <QDebug>
#include <QHelpIndexModel>
#include <QTemporaryFile>

#include <KProcess>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <KAboutData>
#include <KStandardDirs>

#include <interfaces/idocumentation.h>
#include <language/duchain/duchain.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>

#include "qthelpnetwork.h"

K_PLUGIN_FACTORY(QtHelpFactory, registerPlugin<QtHelpPlugin>(); )
K_EXPORT_PLUGIN(QtHelpFactory(KAboutData("kdevqthelp","kdevqthelp", ki18n("QtHelp"), "0.1", ki18n("Check Qt Help documentation"), KAboutData::License_GPL)))

class QtHelpDocumentation : public KDevelop::IDocumentation
{
	public:
		QtHelpDocumentation(const QString& name, const QMap<QString, QUrl>& info, QHelpEngineCore* e)
			: m_info(info), m_engine(e), m_name(name) { Q_ASSERT(!m_info.isEmpty()); }
		
		virtual QString name() const { return m_name; }
		virtual QString description() const { return QStringList(m_info.keys()).join(", "); }
		virtual QWidget* documentationWidget(QWidget* parent)
		{
// 			QTemporaryFile file;
// 			if(!file.open())
// 				kDebug() << "error";
// 			else
			{
				QWebView* b=new QWebView(parent);
				b->page()->setNetworkAccessManager(new HelpNetworkAccessManager(m_engine, 0));
				QUrl url=m_info[m_info.keys().first()];
// 				QByteArray data = m_engine.fileData(url);
// 				b->setContent(data, QString(), url);
// 				b->loadResource(QTextDocument::HtmlResource, data);
				b->load(url);
				
				return b;
			}
			return 0;
		}
		
	private:
		QMap<QString, QUrl> m_info;
		QHelpEngineCore* m_engine;
		QString m_name;
};

QString qtDocsLocation()
{
	QString qmake=KStandardDirs::findExe("qmake");
	QString ret;
	
	KProcess p;
	p.setOutputChannelMode(KProcess::MergedChannels);
	p.setProgram(qmake, QStringList("-query") << "QT_INSTALL_DOCS");
	p.start();
	
	if(p.waitForFinished(5000))
	{
		QByteArray b = p.readAllStandardOutput();
		ret.prepend(b.trimmed());
	}
	else
		kDebug() << "failed to execute qmake to retrieve the docs";
	
	kDebug() << "qtdoc=" << ret;
	Q_ASSERT(qmake.isEmpty() || !ret.isEmpty());
	return ret;
}

QtHelpPlugin::QtHelpPlugin(QObject* parent, const QVariantList& args)
	: KDevelop::IPlugin(QtHelpFactory::componentData(), parent)
	, m_engine(KStandardDirs::locateLocal("data", "qthelpcollection", QtHelpFactory::componentData()))
{
	
	QString fileName=qtDocsLocation()+"/qch/qt.qch";
	bool b=m_engine.setupData();
	kDebug() << "setup" << b << m_engine.error();
	
	QString fileNamespace = QHelpEngineCore::namespaceName(fileName);
	
	if (!fileNamespace.isEmpty() && !m_engine.registeredDocumentations().contains(fileNamespace)) {
		kDebug() << "loading doc" << fileName << fileNamespace;
		if(m_engine.registerDocumentation(fileName))
			kDebug() << "documentation added successfully" << fileName;
		else
			kDebug() << "error >> " << fileName << m_engine.error();
	}
	
	b=m_engine.setupData();
	kDebug() << "registered" << b << m_engine.error() << m_engine.registeredDocumentations();
}


KSharedPtr< KDevelop::IDocumentation > QtHelpPlugin::documentationForDeclaration(KDevelop::Declaration* dec)
{
	KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
	
	if(dec && dec->internalContext()) {
		QString id = dec->internalContext()->scopeIdentifier(true).toString();
		if(!id.isEmpty()) {
			QMap<QString, QUrl> links=m_engine.linksForIdentifier(id);
			if(!links.isEmpty())
				return KSharedPtr<KDevelop::IDocumentation>(new QtHelpDocumentation(id, links, &m_engine));
		}
	}
	
	return KSharedPtr<KDevelop::IDocumentation>();
}

