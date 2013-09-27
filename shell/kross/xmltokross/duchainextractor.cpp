/***************************************************************************
 *   Copyright 2008 Aleix Pol <aleixpol@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "duchainextractor.h"
#include <interfaces/ilanguage.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/iplugin.h>
#include <interfaces/iplugincontroller.h>
#include <language/interfaces/ilanguagesupport.h>
#include <language/duchain/duchain.h>
#include <language/duchain/dumpchain.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>
#include <language/backgroundparser/backgroundparser.h>
#include <shell/projectcontroller.h>
#include <shell/core.h>
#include "krosswrapper.h"
#include "krossimpl.h"
#include "dummybsm.h"
#include "duchainreader.h"
#include "project.h"
#include <QApplication>
#include <QFile>

using namespace KDevelop;

class KrossInterfaceCreator : public DUChainReader
{
    public:
        KrossInterfaceCreator(TopDUContext* top) : DUChainReader(top) {}
        virtual void writeDocument() { qDebug() << "start doc"; }
        virtual void writeClass(const QString& classname, const QString& , const QList<QStringList>& )
            { qDebug() << "start class" << classname; }
        virtual void writeNamespace(const QString& name) { qDebug() << "start namespace" << name; }
        virtual void writeVariable(const QString& name, const QString& type, bool isConst)
            { qDebug() << "start var" << name << type << isConst; }
        virtual void writeEndClass() { qDebug() << "end class"; }
        virtual void writeEndDocument() { qDebug() << "end doc"; }
        virtual void writeEndFunction(const method& m)
        {
            QStringList args;
            foreach(const method::argument& arg, m.args)
            { args+= arg.type+' '+arg.name+'='+arg.def; }
            qDebug() << "end func" << m.returnType << m.funcname << args.join(", "); 
        }
        virtual void writeEndEnum(const QStringList& flags) { qDebug() << "start enum" << flags; }
};


DUChainExtractor::DUChainExtractor(QObject* parent) : QObject(parent), m_done(false)
{
    connect(Core::self()->languageController()->backgroundParser(), SIGNAL(parseJobFinished(KDevelop::ParseJob*)),
            this, SLOT(parsingFinished(KDevelop::ParseJob*)));
}

// TODO: use Path API?
void DUChainExtractor::start(const KUrl& _input, const KUrl& builddir,
                             const KUrl::List& includes, const QString& filename, 
                             const QString& directory, const QStringList& toinclude, const QString& output)
{
    input=_input;
    m_filename=filename;
    m_directory=directory;
    m_toinclude=toinclude;
    m_output=output;
    
    m_manager = new DummyBSM(0, QVariantList(), Path::List() << Path(_input));
    m_manager->setBuildDir(Path(builddir));
    m_manager->setIncludeDirectories(toPathList(includes));
    DumbProject* project = new DumbProject();
    project->setManagerPlugin(m_manager);
    Core::self()->projectControllerInternal()->addProject(project);
    Core::self()->languageController()->backgroundParser()->addDocument(IndexedString(input));
}

void DUChainExtractor::parsingFinished(KDevelop::ParseJob* job)
{
    KDevelop::ILanguageSupport* cppLangSup=Core::self()->languageController()->language("C++")->languageSupport();
    
    DUChainReadLocker lock(DUChain::lock());
    TopDUContext* top=cppLangSup->standardContext(input);
//     TopDUContext* top=DUChain::self()->chainForDocument(input);
    qDebug() << "takatakataka" << input << top << job;
    
    KrossWrapper r(top);
    r.setFileName(m_filename);
    r.setIncludes(m_toinclude);
    r.start();
    
    qDebug() << qPrintable(QString(33, '-'));
    
    if(m_output.isEmpty())
        QTextStream(stdout) << r.output;
    else
    {
        QFile f(m_output);
        if(!f.open(QIODevice::WriteOnly | QIODevice::Text))
            return;
        QTextStream(&f) << r.output;
    }
    
    if(!m_filename.isEmpty()) {
        QFile headerFile(m_directory+'/'+m_filename+".h");
        if(!headerFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qDebug() << "error. can't write the header: " << m_filename+".h";
            qApp->quit();
        }
        
        QTextStream out(&headerFile);
        out << r.handlersHeader;
        headerFile.close();
    }
    
    if(m_writeImpl)
    {
        KrossImpl r(top);
        r.setIncludes(m_toinclude);
        r.start();
        
        QFile implFile(m_directory+'/'+m_filename+"impl.h");
        if(!implFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qDebug() << "error. can't write the impl: " << m_writeImpl << ".h";
            qApp->quit();
        }
        
        QTextStream out(&implFile);
        out << r.output;
        implFile.close();
    }
    
    m_done=true;
    qApp->quit();
}

void DUChainExtractor::progressUpdated(int minimum, int maximum, int value)
{
    qDebug() << "progress" << value << "/" << maximum << minimum;
}

#include "duchainextractor.moc"
