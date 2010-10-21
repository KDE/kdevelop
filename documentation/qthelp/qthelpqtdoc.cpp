/*  This file is part of KDevelop
    Copyright 2009 Aleix Pol <aleixpol@kde.org>
    Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
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

#include "qthelpqtdoc.h"
#include <QTemporaryFile>

#include <KProcess>
#include <KAboutData>
#include <KStandardDirs>

#include <language/duchain/duchain.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>

#include <KIcon>
#include "qthelpdocumentation.h"

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
	return QDir::fromNativeSeparators(ret);
}



QtHelpQtDoc::QtHelpQtDoc(QObject *parent, const KComponentData &componentData, const QVariantList &args)
    : QtHelpProviderAbstract(parent, componentData, "qthelpcollection.qhc", args)
{
    Q_UNUSED(args);
    registerDocumentations();
}

void QtHelpQtDoc::registerDocumentations()
{
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
        foreach(const QString& file, d.entryList()) {
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

QIcon QtHelpQtDoc::icon() const
{
    return KIcon("qtlogo");
}

QString QtHelpQtDoc::name() const
{
    return i18n("QtHelp");
}
