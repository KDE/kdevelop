/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "qtdesignerintegration.h"

#include <qpair.h>
#include <qregexp.h>
#include <qfileinfo.h>

#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kurl.h>

#include <rurl.h>
#include <domutil.h>
#include <kdevpartcontroller.h>
#include <kdevcreatefile.h>
#include <kdevlanguagesupport.h>
#include <kdevproject.h>

#include "codemodel_utils.h"
#include "implementationwidget.h"

QtDesignerIntegration::QtDesignerIntegration(KDevLanguageSupport *part, ImplementationWidget *impl, bool classHasDefinitions, const char* name)
    :KDevDesignerIntegration(part, name), m_part(part), m_impl(impl),
    m_classHasDefinitions(classHasDefinitions)
{
}

QtDesignerIntegration::~QtDesignerIntegration()
{
    delete m_impl;
}

void QtDesignerIntegration::addFunction(const QString& formName, KInterfaceDesigner::Function function)
{
    kdDebug() << "QtDesignerIntegration::addFunction: form: " << formName << ", function: " << function.function << endl;

    if (!m_implementations.contains(formName))
        if (!selectImplementation(formName))
            return;

    ClassDom klass = m_implementations[formName];
    if (!klass)
    {
        KMessageBox::error(0, i18n("Cannot find implementation class for form: %1").arg(formName));
        return;
    }

    addFunctionToClass(function, klass);
}

void QtDesignerIntegration::editFunction(const QString& formName, KInterfaceDesigner::Function oldFunction, KInterfaceDesigner::Function function)
{
    kdDebug() << "QtDesignerIntegration::editFunction: form: " << formName
        << ", old function: " << oldFunction.function
        << ", function: " << function.function << endl;
}

void QtDesignerIntegration::removeFunction(const QString& formName, KInterfaceDesigner::Function function)
{
    kdDebug() << "QtDesignerIntegration::removeFunction: form: " << formName << ", function: " << function.function << endl;
}

bool QtDesignerIntegration::selectImplementation(const QString &formName)
{
    QFileInfo fi(formName);
    if (!fi.exists())
        return false;

    if (m_impl->exec(formName))
    {
        m_implementations[formName] = m_impl->selectedClass();
        return true;
    }
    return false;
}

void QtDesignerIntegration::loadSettings(QDomDocument dom, QString path)
{
    QDomElement el = DomUtil::elementByPath(dom, path + "/qtdesigner");
    if (el.isNull())
        return;
    QDomNodeList impls = el.elementsByTagName("implementation");
    for (uint i = 0; i < impls.count(); ++i)
    {
        QDomElement el = impls.item(i).toElement();
        if (el.isNull())
            continue;
        QString implementationPath = Relative::File(m_part->project()->projectDirectory(),
            el.attribute("implementationpath"), true).urlPath();
        FileDom file = m_part->codeModel()->fileByName(implementationPath);
        if (!file)
            continue;
        ClassList cllist = file->classByName(el.attribute("class"));
        QString uiPath = Relative::File(m_part->project()->projectDirectory(),
            el.attribute("path"), true).urlPath();
        if (cllist.count() > 0)
            m_implementations[uiPath] = cllist.first();
    }
}

void QtDesignerIntegration::saveSettings(QDomDocument dom, QString path)
{
    kdDebug() << "QtDesignerIntegration::saveSettings" << endl;
    QDomElement el = DomUtil::createElementByPath(dom, path + "/qtdesigner");
    for (QMap<QString, ClassDom>::const_iterator it = m_implementations.begin();
        it != m_implementations.end(); ++it)
    {
        QDomElement il = dom.createElement("implementation");
        el.appendChild(il);
        il.setAttribute("path", 
            Relative::File(m_part->project()->projectDirectory(), it.key()).rurl());
        il.setAttribute("implementationpath",
            Relative::File(m_part->project()->projectDirectory(), it.data()->fileName()).rurl());
        il.setAttribute("class", it.data()->name());
    }
}

void QtDesignerIntegration::openFunction(const QString &formName, const QString &functionName)
{
    kdDebug() << "QtDesignerIntegration::openFunction, formName = " << formName
        << ", functionName = " << functionName << endl;
    QString fn = functionName;
    if (fn.find("(") > 0)
        fn.remove(fn.find("("), fn.length());

    if (!m_implementations[formName])
        return;

    int line = -1, col = -1;

    QString impl = m_implementations[formName]->fileName();
    processImplementationName(impl);

    if (m_part->codeModel()->hasFile(impl))
    {
        if (m_classHasDefinitions)
        {
            FunctionDefinitionList list =
                m_part->codeModel()->fileByName(impl)->functionDefinitionList();
            for (FunctionDefinitionList::const_iterator it = list.begin(); it != list.end(); ++it)
            {
                if ((*it)->name() == fn)
                    (*it)->getStartPosition(&line, &col);
            }
        }
        else
        {
            FunctionList list =
                m_part->codeModel()->fileByName(impl)->functionList();
            for (FunctionList::const_iterator it = list.begin(); it != list.end(); ++it)
            {
                if ((*it)->name() == fn)
                    (*it)->getStartPosition(&line, &col);
            }
        }
    }

    m_part->partController()->editDocument(KURL(impl), line, col);
}

void QtDesignerIntegration::processImplementationName(QString &// name
                                                      )
{
}

void QtDesignerIntegration::openSource(const QString &formName)
{
    if (!m_implementations.contains(formName))
        if (!selectImplementation(formName))
            return;
    
    m_part->partController()->editDocument(KURL(m_implementations[formName]->fileName()), -1, -1);
}

#include "qtdesignerintegration.moc"
