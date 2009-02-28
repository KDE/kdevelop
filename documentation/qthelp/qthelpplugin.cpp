/*  This file is part of KDevelop
    Copyright 2009 Aleix Pol <aleixpol@kde.org>
    Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>

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
#include <QWebFrame>

K_PLUGIN_FACTORY(QtHelpFactory, registerPlugin<QtHelpPlugin>(); )
K_EXPORT_PLUGIN(QtHelpFactory(KAboutData("kdevqthelp","kdevqthelp", ki18n("QtHelp"), "0.1", ki18n("Check Qt Help documentation"), KAboutData::License_GPL)))

class QtHelpDocumentation : public KDevelop::IDocumentation
{
	public:
		QtHelpDocumentation(const QString& name, const QMap<QString, QUrl>& info, QHelpEngineCore* e)
			: m_info(info), m_engine(e), m_name(name), m_view(0) { Q_ASSERT(!m_info.isEmpty()); }
		
		virtual QString name() const { return m_name; }
		virtual QString description() const {
            HelpNetworkAccessManager access(m_engine, 0);
            QUrl url(m_info.values().first());
            QByteArray data = m_engine->fileData(url);

            //Extract a short description from the html data
            QString dataString = QString::fromLatin1(data); ///@todo encoding
            QString fragment = url.fragment();
            
            QString p = "((\\\")|(\\\'))";
            QString exp = "< a name = " + p + fragment + p + " > < / a >";
            QString optionalSpace = "( )*";
            exp.replace(" ", optionalSpace);
            QRegExp findFragment(exp);
            int pos = findFragment.indexIn(dataString);
            if(fragment.isEmpty()) {
                pos = 0;
            }else{
                //Check if there is a title opening-tag right before the fragment, and if yes add it, so we have a nicely formatted caption
                QString titleRegExp("< h\\d class = \".*\" >");
                titleRegExp.replace(" ", optionalSpace);
                QRegExp findTitle(titleRegExp);
                int titleStart = findTitle.lastIndexIn(dataString, pos);
                int titleEnd = titleStart + findTitle.matchedLength();
                if(titleStart != -1) {
                    QString between = dataString.mid(titleEnd, pos-titleEnd).trimmed();
//                     if(between.isEmpty())
                        pos = titleStart;
                }
            }
            
            if(pos != -1) {
                
                QString exp = "< a name = " + p + "((\\S)*)" + p + " > < / a >";
                exp.replace(" ", optionalSpace);
                QRegExp nextFragmentExpression(exp);
                int endPos = nextFragmentExpression.indexIn(dataString, pos+(fragment.size() ? findFragment.matchedLength() : 0));
                if(endPos == -1)
                    endPos = dataString.size();

                {
                    //Find the end of the last paragraph or newline, so we don't add prefixes of the following fragment
                    QString newLineRegExp("< br / > | < / p >");
                    newLineRegExp.replace(" ", optionalSpace);
                    QRegExp lastNewLine(newLineRegExp);
                    int newEnd = dataString.lastIndexOf(lastNewLine, endPos);
                    if(newEnd != -1 && newEnd > pos)
                        endPos = newEnd + lastNewLine.matchedLength();
                }
                
                {
                    //Find the title, and start from there
                    QString titleRegExp("< h\\d class = \"title\" >");
                    titleRegExp.replace(" ", optionalSpace);
                    QRegExp findTitle(titleRegExp);
                    int idx = findTitle.indexIn(dataString);
                    if(idx > pos && idx < endPos)
                        pos = idx;
                }
             
                
                QString thisFragment = dataString.mid(pos, endPos - pos);
                
                {
                    //Completely remove the first large header found, since we don't need a header
                    QString headerRegExp("< h\\d.*>.*< / h\\d >");
                    headerRegExp.replace(" ", optionalSpace);
                    QRegExp findHeader(headerRegExp);
                    findHeader.setMinimal(true);
                    int idx = findHeader.indexIn(thisFragment);
                    if(idx != -1) {
                        thisFragment.remove(idx, findHeader.matchedLength());
                    }
                }
                
                {
                    //Replace all gigantic header-font sizes with <big>

                    {
                        QString sizeRegExp("< h\\d ");
                        sizeRegExp.replace(" ", optionalSpace);
                        QRegExp findSize(sizeRegExp);
                        thisFragment.replace(findSize, "<big ");
                    }
                    {
                        QString sizeCloseRegExp("< / h\\d >");
                        sizeCloseRegExp.replace(" ", optionalSpace);
                        QRegExp closeSize(sizeCloseRegExp);
                        thisFragment.replace(closeSize, "</big><br />");
                    }
                }
                
                {
                    //Replace paragraphs by newlines
                    
                    QString begin("< p >");
                    begin.replace(" ", optionalSpace);
                    
                    QRegExp findBegin(begin);
                    thisFragment.replace(findBegin, "");

                    QString end("< /p >");
                    end.replace(" ", optionalSpace);
                    
                    QRegExp findEnd(end);
                    thisFragment.replace(findEnd, "<br />");
                }
                
                {
                    //Remove links, because they won't work
                    QString link("< a href = " + p + ".*" + p);
                    link.replace(" ", optionalSpace);
                    QRegExp exp(link, Qt::CaseSensitive);
                    exp.setMinimal(true);
                    thisFragment.replace(exp, "<a ");
                }
                
                return thisFragment;
            }
            
            return QStringList(m_info.keys()).join(", ");
        }
        
        virtual QWidget* documentationWidget(QWidget* parent)
        {
            if(!m_view) {
                m_view=new QWebView(parent);
                m_view->page()->setNetworkAccessManager(new HelpNetworkAccessManager(m_engine, 0));
            }
            
            QUrl url=m_info[m_info.keys().first()];
            m_view->load(url);
            
            return m_view;
        }

	private:
		QMap<QString, QUrl> m_info;
		QHelpEngineCore* m_engine;
		QString m_name;
        static QPointer<QWebView> m_view;
};

QString qtDocsLocation(const QString& qmake)
{
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
	QStringList qmakes;
    KStandardDirs::findAllExe(qmakes, "qmake");
	QString fileName;
    foreach(const QString& qmake, qmakes) {
        fileName=qtDocsLocation(qmake)+"/qch/qt.qch";
        if(QFile::exists(fileName)) {
            kDebug() << "checking doc: " << fileName;
            break;
        }
    }
    
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
	
	if(dec) {
        KDevelop::QualifiedIdentifier qid = dec->qualifiedIdentifier();
        QStringList idLIst;
        for(int a = 0; a < qid.count(); ++a)
            idLIst << qid.at(a).identifier().str(); //Copy over the identifier components, without the template-parameters
        
		QString id = idLIst.join("::");
		if(!id.isEmpty()) {
			QMap<QString, QUrl> links=m_engine.linksForIdentifier(id);
            
            kDebug() << "doc_found" << id << links;
			if(!links.isEmpty())
				return KSharedPtr<KDevelop::IDocumentation>(new QtHelpDocumentation(id, links, &m_engine));
		}
	}
	
	return KSharedPtr<KDevelop::IDocumentation>();
}

