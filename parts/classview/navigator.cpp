/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   adymo@mksat.net                                                       *
 *   Portions Copyright (C) 2002-2003 by Roberto Raggi                     *
 *   roberto@kdevelop.org                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "navigator.h"

#include <qtimer.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <ktexteditor/viewcursorinterface.h>

#include <kcomboview.h>
#include <klistviewaction.h>
#include <kdevpartcontroller.h>
#include <kdevlanguagesupport.h>
#include <codemodel_utils.h>

#include "classviewpart.h"


struct NavOp
{
   NavOp(Navigator *navigator, const QString &fullName)
       :m_navigator(navigator), m_fullName(fullName) {}

   bool operator() ( const FunctionDefinitionDom& def ) const
   {
       return (m_navigator->fullFunctionDefinitionName(def) == m_fullName);
   }
   bool operator() ( const FunctionDom& def ) const
   {
       return (m_navigator->fullFunctionDeclarationName(def) == m_fullName);
   }

private:
    Navigator *m_navigator;
    QString m_fullName;
};



class FunctionNavItem: public QListViewItem {
public:
    enum Type { Declaration, Definition };
    
    FunctionNavItem(ClassViewPart *part, QListView *parent, QString name, Type type)
        :QListViewItem(parent, name), m_part(part), m_type(type) {}
    FunctionNavItem(ClassViewPart *part, QListViewItem *parent, QString name, Type type)
        :QListViewItem(parent, name), m_part(part), m_type(type) {}
    ~FunctionNavItem() {}
    
    virtual void setup()
    {
        QListViewItem::setup();
        setPixmap( 0, UserIcon("CVpublic_meth", KIcon::DefaultState, m_part->instance()) );
    }
    Type type() { return m_type; }
    
private:
    ClassViewPart *m_part;
    Type m_type;
};


Navigator::Navigator(ClassViewPart *parent, const char *name)
 : QObject(parent, name), m_part(parent)
{
    m_state = GoToDefinitions;
    m_navNoDefinition = true;

    m_syncTimer = new QTimer(this);
    connect(m_syncTimer, SIGNAL(timeout()), this, SLOT(syncFunctionNav()));
}

Navigator::~Navigator()
{
}

void Navigator::selectFunctionNav(QListViewItem *item)
{
    FunctionNavItem *nav = dynamic_cast<FunctionNavItem*>(item);
    if (!nav)
        return;

    FileDom file = m_part->codeModel()->fileByName(m_part->m_activeFileName);
    if (!file)
        return;
        
    switch (nav->type())
    {
        case FunctionNavItem::Definition: //jump to definition
        {
            FileList files;
            files.append(file);
            FunctionDefinitionList deflist;
            CodeModelUtils::findFunctionDefinitions(NavOp(this, nav->text(0)), files, deflist);
            if (deflist.count() < 1)
                return;
            
            FunctionDefinitionDom fun = deflist.first();
            if (!fun)
                return;
            int startLine = 0, startColumn = 0;
            fun->getStartPosition(&startLine, &startColumn);
            m_part->partController()->editDocument(KURL(fun->fileName()), startLine);
            break;
        }
        case FunctionNavItem::Declaration: //jump to declaration
        {
            FileList files;
            files.append(file);
            FunctionList declist;
            CodeModelUtils::findFunctionDeclarations(NavOp(this, nav->text(0)), files, declist);
            if (declist.count() < 1)
                return;
            
            FunctionDom fun = declist.first();
            if (!fun)
                return;
            int startLine = 0, startColumn = 0;
            fun->getStartPosition(&startLine, &startColumn);
            m_part->partController()->editDocument(KURL(fun->fileName()), startLine);
            break;
        }
    }
}

void Navigator::functionNavUnFocused()
{
    /*if (m_navNoDefinition)
        m_part->m_functionsnav->view()->setCurrentText(NAV_NODEFINITION);
    else*/
    if (m_part->m_functionsnav->view()->currentItem())
        m_part->m_functionsnav->view()->setCurrentText(m_part->m_functionsnav->view()->currentItem()->text(0));
    else
        m_part->m_functionsnav->view()->setCurrentText(NAV_NODEFINITION);
}

void Navigator::functionNavFocused()
{
    m_navNoDefinition = (m_part->m_functionsnav->view()->currentText() == NAV_NODEFINITION);
    m_part->m_functionsnav->view()->setCurrentText("");
}

void Navigator::slotCursorPositionChanged()
{
    //FIXME: we assume that background parser delay is set globally in kdeveloprc file
    //for all available language supports
    //this is reasonable suggestion because problem reporter should be the same for all languages
    KConfig* config = kapp->config();
    config->setGroup( "General Options" );
    int m_delay = config->readNumEntry( "BgParserDelay", 250 );
    
    m_syncTimer->changeInterval(500 >= m_delay+100 ? 500 : m_delay+100 );
}

void Navigator::stopTimer( )
{
    m_syncTimer->stop();
}

void Navigator::syncFunctionNavDelayed(int delay)
{
    m_syncTimer->changeInterval(delay);
}

void Navigator::syncFunctionNav()
{
    m_syncTimer->stop();
    
    if (FunctionDefinitionDom fun = currentFunctionDefinition())
    {
        if (functionNavDefs[fullFunctionDefinitionName(fun)])
        {
            m_part->m_functionsnav->view()->blockSignals(true);
            m_part->m_functionsnav->view()->setCurrentActiveItem(functionNavDefs[fullFunctionDefinitionName(fun)]);
            m_part->m_functionsnav->view()->blockSignals(false);
        }
    }
    else if (FunctionDom fun = currentFunctionDeclaration())
    {
        if (functionNavDecls[fullFunctionDeclarationName(fun)])
        {
            m_part->m_functionsnav->view()->blockSignals(true);
            m_part->m_functionsnav->view()->setCurrentActiveItem(functionNavDecls[fullFunctionDeclarationName(fun)]);
            m_part->m_functionsnav->view()->blockSignals(false);
        }
    }
    else
        m_part->m_functionsnav->view()->setCurrentText(NAV_NODEFINITION);
}

void Navigator::refreshNavBars(const QString &activeFileName, bool clear)
{
    kdDebug() << "Navigator::refreshNavBars" << endl;
    if (clear)
    {
        m_part->m_functionsnav->view()->clear();
        functionNavDefs.clear();
        functionNavDecls.clear();
    }
    
    FileDom file = m_part->codeModel()->fileByName(activeFileName);
    if (!file)
        return;

    QStringList toLeave;
    
    FunctionList list1 = CodeModelUtils::allFunctions(file);
    for (FunctionList::const_iterator it = list1.begin(); it != list1.end(); ++it)
    {
        QString fullName = fullFunctionDeclarationName(*it);
        
        if (clear || !functionNavDecls[fullName])
        {
            FunctionNavItem *item = new FunctionNavItem(m_part,
                m_part->m_functionsnav->view()->listView(), fullName,
                FunctionNavItem::Declaration);
            functionNavDecls[fullName] = item;
            m_part->m_functionsnav->view()->addItem(item);
        }
        toLeave << fullName;
    }
    kdDebug() << "leave list: " << toLeave << endl;
    
    //remove items not in toLeave list
    for (QMap<QString, QListViewItem*>::iterator it = functionNavDecls.begin();
        it != functionNavDecls.end(); ++it)
    {
        if (!toLeave.contains(it.key()))
        {
            if (it.data())
                m_part->m_functionsnav->view()->removeItem(it.data());
            functionNavDecls.remove(it);
        }
    }
        
    toLeave.clear();
    FunctionDefinitionList list = CodeModelUtils::allFunctionDefinitionsDetailed(file).functionList;
    for (FunctionDefinitionList::const_iterator it = list.begin(); it != list.end(); ++it)
    {
        QString fullName = fullFunctionDefinitionName(*it);
        
        if (clear || !functionNavDefs[fullName])
        {
            FunctionNavItem *item = new FunctionNavItem(m_part,
                m_part->m_functionsnav->view()->listView(), fullName, FunctionNavItem::Definition);
            functionNavDefs[fullName] = item;
            m_part->m_functionsnav->view()->addItem(item);
        }
            
        //remove unnecessary items with function declarations for which a definition item was created
        if (functionNavDecls[fullName])
        {
            m_part->m_functionsnav->view()->removeItem(functionNavDecls[fullName]);
            functionNavDecls.remove(fullName);
        }
        
        toLeave << fullName;
    }
        
    kdDebug() << "leave list: " << toLeave << endl;
    //remove items not in toLeave list
    for (QMap<QString, QListViewItem*>::iterator it = functionNavDefs.begin();
        it != functionNavDefs.end(); ++it)
    {
        if (!toLeave.contains(it.key()))
        {
            if (it.data())
                m_part->m_functionsnav->view()->removeItem(it.data());
            functionNavDefs.remove(it);
        }
    }
}

void Navigator::refresh()
{
    refreshNavBars(m_part->m_activeFileName, true);
}

void Navigator::addFile(const QString & file)
{
    kdDebug() << "Navigator::addFile" << endl;
    if (file == m_part->m_activeFileName)
    {
        kdDebug() << "Navigator::addFile, processing active file" << endl;
        refreshNavBars(m_part->m_activeFileName, false);
    }
}



FunctionDefinitionDom Navigator::currentFunctionDefinition()
{
    if (!m_part->m_activeViewCursor)
        return FunctionDefinitionDom();

    unsigned int line, column;
    m_part->m_activeViewCursor->cursorPositionReal(&line, &column);
    return functionDefinitionAt(line, column);
}

FunctionDefinitionDom Navigator::functionDefinitionAt(int line, int column)
{
    if (!m_part->codeModel()->hasFile(m_part->m_activeFileName))
        return FunctionDefinitionDom();

    FileDom file = m_part->codeModel()->fileByName(m_part->m_activeFileName);
    return functionDefinitionAt(model_cast<NamespaceDom>(file), line, column);
}

FunctionDefinitionDom Navigator::functionDefinitionAt(NamespaceDom ns, int line, int column)
{
    NamespaceList namespaceList = ns->namespaceList();
    for (NamespaceList::iterator it=namespaceList.begin(); it!=namespaceList.end(); ++it)
    {
        if (FunctionDefinitionDom def = functionDefinitionAt(*it, line, column))
            return def;
    }

    ClassList classList = ns->classList();
    for (ClassList::iterator it=classList.begin(); it!=classList.end(); ++it)
    {
        if (FunctionDefinitionDom def = functionDefinitionAt(*it, line, column))
            return def;
    }

    FunctionDefinitionList functionDefinitionList = ns->functionDefinitionList();
    for (FunctionDefinitionList::iterator it=functionDefinitionList.begin();
        it!=functionDefinitionList.end(); ++it )
    {
        if (FunctionDefinitionDom def = functionDefinitionAt(*it, line, column))
            return def;
    }

    return FunctionDefinitionDom();
}

FunctionDefinitionDom Navigator::functionDefinitionAt(ClassDom klass, int line, int column)
{
    ClassList classList = klass->classList();
    for (ClassList::iterator it=classList.begin(); it!=classList.end(); ++it)
    {
        if (FunctionDefinitionDom def = functionDefinitionAt(*it, line, column))
            return def;
    }

    FunctionDefinitionList functionDefinitionList = klass->functionDefinitionList();
    for (FunctionDefinitionList::Iterator it=functionDefinitionList.begin();
        it!=functionDefinitionList.end(); ++it)
    {
        if (FunctionDefinitionDom def = functionDefinitionAt(*it, line, column))
            return def;
    }

    return FunctionDefinitionDom();
}

FunctionDefinitionDom Navigator::functionDefinitionAt(FunctionDefinitionDom fun, int line, int column)
{
    int startLine, startColumn;
    int endLine, endColumn;

    fun->getStartPosition(&startLine, &startColumn);
    fun->getEndPosition(&endLine, &endColumn);

    if (!(line >= startLine && line <= endLine))
        return FunctionDefinitionDom();

    if (line == startLine && column < startColumn)
        return FunctionDefinitionDom();

    if (line == endLine && column > endColumn)
        return FunctionDefinitionDom();

    return fun;
}


FunctionDom Navigator::currentFunctionDeclaration()
{
    if (!m_part->m_activeViewCursor)
        return FunctionDom();

    unsigned int line, column;
    m_part->m_activeViewCursor->cursorPositionReal(&line, &column);
    return functionDeclarationAt(line, column);
}

FunctionDom Navigator::functionDeclarationAt(int line, int column)
{
    if (!m_part->codeModel()->hasFile(m_part->m_activeFileName))
        return FunctionDom();

    FileDom file = m_part->codeModel()->fileByName(m_part->m_activeFileName);
    return functionDeclarationAt(model_cast<NamespaceDom>(file), line, column);
}

FunctionDom Navigator::functionDeclarationAt(NamespaceDom ns, int line, int column)
{
    NamespaceList namespaceList = ns->namespaceList();
    for (NamespaceList::iterator it=namespaceList.begin(); it!=namespaceList.end(); ++it)
    {
        if (FunctionDom def = functionDeclarationAt(*it, line, column))
            return def;
    }

    ClassList classList = ns->classList();
    for (ClassList::iterator it=classList.begin(); it!=classList.end(); ++it)
    {
        if (FunctionDom def = functionDeclarationAt(*it, line, column))
            return def;
    }

    FunctionList functionList = ns->functionList();
    for (FunctionList::iterator it=functionList.begin();
        it!=functionList.end(); ++it )
    {
        if (FunctionDom def = functionDeclarationAt(*it, line, column))
            return def;
    }

    return FunctionDom();
}

FunctionDom Navigator::functionDeclarationAt(ClassDom klass, int line, int column)
{
    ClassList classList = klass->classList();
    for (ClassList::iterator it=classList.begin(); it!=classList.end(); ++it)
    {
        if (FunctionDom def = functionDeclarationAt(*it, line, column))
            return def;
    }

    FunctionList functionList = klass->functionList();
    for (FunctionList::Iterator it=functionList.begin();
        it!=functionList.end(); ++it)
    {
        if (FunctionDom def = functionDeclarationAt(*it, line, column))
            return def;
    }

    return FunctionDom();
}

FunctionDom Navigator::functionDeclarationAt(FunctionDom fun, int line, int column)
{
    int startLine, startColumn;
    int endLine, endColumn;

    fun->getStartPosition(&startLine, &startColumn);
    fun->getEndPosition(&endLine, &endColumn);

    if (!(line >= startLine && line <= endLine))
        return FunctionDom();

    if (line == startLine && column < startColumn)
        return FunctionDom();

    if (line == endLine && column > endColumn)
        return FunctionDom();

    return fun;
}


QString Navigator::fullFunctionDefinitionName(FunctionDefinitionDom fun)
{        
    QStringList scope = fun->scope();
    QString funName = scope.join(".");
    if (!funName.isEmpty())
        funName += ".";
    funName += m_part->languageSupport()->formatModelItem(fun, true);   
    funName = m_part->languageSupport()->formatClassName(funName);
    
    return funName;
}

QString Navigator::fullFunctionDeclarationName(FunctionDom fun)
{
    QStringList scope = fun->scope();
    QString funName = scope.join(".");
    if (!funName.isEmpty())
        funName += ".";
    funName += m_part->languageSupport()->formatModelItem(fun, true);   
    funName = m_part->languageSupport()->formatClassName(funName);
    
    return funName;
}

#include "navigator.moc"
