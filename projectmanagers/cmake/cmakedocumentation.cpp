/* KDevelop CMake Support
 *
 * Copyright 2009 Aleix Pol <aleixpol@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "cmakedocumentation.h"
#include "cmakeutils.h"
#include <KDebug>
#include <QTimer>
#include <language/duchain/declaration.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/idocumentation.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/icore.h>
#include "cmakemanager.h"

CMakeDocumentation::CMakeDocumentation(const QString& cmakeCmd, CMakeManager* m)
    : mCMakeCmd(cmakeCmd), m_manager(m)
{
    QTimer::singleShot(0, this, SLOT(delayedInitialization()));
}

void CMakeDocumentation::delayedInitialization()
{
    collectIds("--help-command-list", Command);
    collectIds("--help-variable-list", Variable);
    collectIds("--help-module-list", Module);
}

void CMakeDocumentation::collectIds(const QString& param, Type type)
{
    QStringList ids=CMake::executeProcess(mCMakeCmd, QStringList(param)).split('\n');
    ids.takeFirst();
    foreach(const QString& name, ids)
    {
        m_typeForName[name.toLower()]=type;
    }
}

class CMakeDoc : public KDevelop::IDocumentation
{
    public:
        CMakeDoc(const QString& name, const QString& desc) : mName(name), mDesc(desc) {}
        
        virtual QWidget* documentationWidget(QWidget* ) { return 0; }
        virtual QString description() const { return mDesc; }
        virtual QString name() const { return mName; }
        virtual bool providesWidget() const { return false; }
    private:
        QString mName, mDesc;
};

KSharedPtr<KDevelop::IDocumentation> CMakeDocumentation::description(const QString& identifier, const KUrl& file)
{
    kDebug() << "seeking documentation for " << identifier;
    QString arg, id=identifier.toLower();
    Type t;
    if(m_typeForName.contains(id)) {
        switch(t=m_typeForName[id])
        {
            case Command:
                arg="--help-command";
                break;
            case Variable:
                arg="--help-variable";
                break;
            case Module:
                arg="--help-module";
                break;
            case Property:
            case Policy:
                break;
        }
        qDebug() << "type for" << id << m_typeForName[id];
    }
    
    QString desc;
    if(!arg.isEmpty())
        desc="<pre>"+CMake::executeProcess(mCMakeCmd, QStringList(arg) << identifier)+"</pre>";
    
    {
        KDevelop::IProject* p=KDevelop::ICore::self()->projectController()->findProjectForUrl(file);
        QPair<QString, QString> entry = m_manager->cacheValue(p, identifier);
        if(!entry.first.isEmpty())
            desc += i18n("<br /><em>Cache Value:</em> %1\n", entry.first);
        
        if(!entry.second.isEmpty())
            desc += i18n("<br /><em>Cache Documentation:</em> %1\n", entry.second);
        kDebug() << "cache info:" << entry << file;
    }
    
    if(desc.isEmpty())
        return KSharedPtr<KDevelop::IDocumentation>();
    else
        return KSharedPtr<KDevelop::IDocumentation>(new CMakeDoc(identifier, desc));
}

KSharedPtr<KDevelop::IDocumentation> CMakeDocumentation::documentationForDeclaration(KDevelop::Declaration* decl)
{
    return description(decl->identifier().toString(), decl->url().toUrl());
}
