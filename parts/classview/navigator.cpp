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

//using namespace Widgets;

//#define BOLDFONTSMALLER


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



class FunctionNavItem: public FancyQListViewItem {
private:
    typedef FancyQListViewItem Base;
public:
    enum Type { Declaration, Definition };

    FunctionNavItem(TextPaintStyleStore& styles, ClassViewPart *part, QListView *parent, QString name, Type type)
        :Base(styles, parent, name, ""), m_part(part), m_type(type) {}
    FunctionNavItem(TextPaintStyleStore& styles, ClassViewPart *part, QListViewItem *parent, QString name, Type type)
        :Base(styles, parent, name, ""), m_part(part), m_type(type) {}
    ~FunctionNavItem() {}

    virtual void setup()
    {
        Base::setup();
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
    //this is reasonable assumption because problem reporter should be the same for all languages
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

    if (FunctionDom fun = currentFunction())
    {
        if( !fun->isFunctionDefinition() ) {
        if (m_functionNavDecls[fullFunctionDeclarationName(fun)])
        {
            m_part->m_functionsnav->view()->blockSignals(true);
            m_part->m_functionsnav->view()->setCurrentActiveItem(m_functionNavDecls[fullFunctionDeclarationName(fun)]);
            m_part->m_functionsnav->view()->blockSignals(false);
        }
        } else {
        if (m_functionNavDefs[fullFunctionDeclarationName(fun)])
        {
            m_part->m_functionsnav->view()->blockSignals(true);
            m_part->m_functionsnav->view()->setCurrentActiveItem(m_functionNavDefs[fullFunctionDeclarationName(fun)]);
            m_part->m_functionsnav->view()->blockSignals(false);
        }
        }
    }
    else
        m_part->m_functionsnav->view()->setCurrentText(NAV_NODEFINITION);
}

void Navigator::refreshNavBars(const QString &activeFileName, bool clear)
{
    kdDebug(9003) << k_funcinfo << endl;
    if (clear)
    {
        m_part->m_functionsnav->view()->clear();
        m_functionNavDefs.clear();
        m_functionNavDecls.clear();
    }

    FileDom file = m_part->codeModel()->fileByName(activeFileName);
    if (!file)
        return;

    QStringList toLeave;

    FunctionList list1 = CodeModelUtils::allFunctionsExhaustive(file);
    FunctionList::const_iterator flEnd = list1.end();
    for (FunctionList::const_iterator it = list1.begin(); it != flEnd; ++it)
    {
        QString fullName = fullFunctionDeclarationName(*it);

        if (clear || !m_functionNavDecls[fullName] && m_part->m_functionsnav->view()->listView())
        {
            FunctionNavItem *item = new FunctionNavItem(m_styles, m_part,
                m_part->m_functionsnav->view()->listView(), fullName,
                FunctionNavItem::Declaration);
            m_functionNavDecls[fullName] = item;
            item->setItem( 0, fullFunctionItem( *it ) );
            
            m_part->m_functionsnav->view()->addItem(item);
        }
        toLeave << fullName;
    }
    kdDebug(9003) << k_funcinfo << "leave list: " << toLeave << endl;

    //remove items not in toLeave list
    QMap<QString, QListViewItem*>::iterator it = m_functionNavDecls.begin();
    while ( it != m_functionNavDecls.end() )
    {
        QMap<QString, QListViewItem*>::iterator it2 = it;
        ++it;
        if ( !toLeave.contains( it2.key() ) )
        {
            if (it2.data())
            {
                m_part->m_functionsnav->view()->removeItem(it2.data());
            }
            m_functionNavDecls.remove(it2);
        }
    }

    toLeave.clear();
    FunctionDefinitionList list = CodeModelUtils::allFunctionDefinitionsExhaustive(file);
    for (FunctionDefinitionList::const_iterator it = list.begin(); it != list.end(); ++it)
    {
        QString fullName = fullFunctionDefinitionName(*it);

        if (clear || !m_functionNavDefs[fullName])
        {
            FunctionNavItem *item = new FunctionNavItem(m_styles, m_part,
                m_part->m_functionsnav->view()->listView(), fullName, FunctionNavItem::Definition);
            m_functionNavDefs[fullName] = item;
            item->setItem( 0, fullFunctionItem( *it ) );
            m_part->m_functionsnav->view()->addItem(item);
        }

        //remove unnecessary items with function declarations for which a definition item was created
        if (m_functionNavDecls[fullName])
        {
            m_part->m_functionsnav->view()->removeItem(m_functionNavDecls[fullName]);
            m_functionNavDecls.remove(fullName);
        }

        toLeave << fullName;
    }

    kdDebug(9003) << k_funcinfo << "leave list: " << toLeave << endl;
    //remove items not in toLeave list
    QMap<QString, QListViewItem*>::iterator itt = m_functionNavDefs.begin();
    while ( itt != m_functionNavDefs.end() )
    {
        QMap<QString, QListViewItem*>::iterator it2 = itt;
        ++itt;
        if ( !toLeave.contains( it2.key() ) )
        {
            if (it2.data())
            {
                m_part->m_functionsnav->view()->removeItem(it2.data());
            }
            m_functionNavDefs.remove(it2);
        }
    }
}

void Navigator::refresh()
{
    refreshNavBars(m_part->m_activeFileName, true);
}

void Navigator::addFile(const QString & file)
{
    kdDebug(9003) << k_funcinfo << "file: " << file << endl;
    if (file == m_part->m_activeFileName)
    {
        kdDebug(9003) << k_funcinfo << "processing active file" << endl;
        refreshNavBars(m_part->m_activeFileName, false);
    }
}




FunctionDom Navigator::currentFunction()
{
    if (!m_part->m_activeViewCursor)
        return FunctionDom();

    unsigned int line, column;
    m_part->m_activeViewCursor->cursorPositionReal(&line, &column);
    CodeModelUtils::CodeModelHelper hlp( m_part->codeModel(), m_part->codeModel()->fileByName( m_part->m_activeFileName) );
    return hlp.functionAt( line, column );
}


///Some time this might be moved into the language-support-part, so each language
///can highlight as it likes
TextPaintItem highlightFunctionName(QString function, int type, TextPaintStyleStore& styles) {
    TextPaintItem ret;
    
    if( !styles.hasStyle( type ) ) {
        QFont font = styles.getStyle( 0 ).font;
        
        switch(type) {
            case 1:
            default:
                font.setWeight( QFont::DemiBold );
                ///since bold makes the font a little bigger, make it smaller again
#ifdef BOLDFONTSMALLER
                font.setPointSize( (font.pointSize() * 9) / 10 );
#endif
        }
        
        styles.addStyle( type, font );
    }
    
    QString args;
    QString fScope;
    int cutpos;
    
    if((cutpos = function.find('(')) != -1) {
        args = function.right( function.length() - cutpos );
        function = function.left( cutpos );
    } else {
        ret.addItem( function );
        return ret;
    }
    if((cutpos = function.findRev(':')) != -1 || (cutpos = function.findRev('.')) != -1) {
        fScope = function.left( cutpos + 1 );
        function = function.right( function.length() - cutpos - 1);
    }
    if( !fScope.isEmpty() ) ret.addItem(fScope);
    ret.addItem( function, type );
    if( !args.isEmpty() ) ret.addItem(args);
    return ret;
}


template <class DomType>
TextPaintItem Navigator::fullFunctionItem(DomType fun)
{
    
    QStringList scope = fun->scope();
    QString function = scope.join(".");
    if (!function.isEmpty())
        function += ".";
    function += m_part->languageSupport()->formatModelItem(fun, true);
    function = m_part->languageSupport()->formatClassName(function);
    
    return highlightFunctionName(function, 1, m_styles);
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
