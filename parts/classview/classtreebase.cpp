/***************************************************************************
 *   Copyright (C) 1999 by Jonas Nordin                                    *
 *   jonas.nordin@syncom.se                                                *
 *   Copyright (C) 2000-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "classtreebase.h"

#include <qtooltip.h>
#include <q3header.h>
#include <qregexp.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <kdebug.h>
#include <kconfig.h>
#include <kpopupmenu.h>
#include <klocale.h>
#include <kiconloader.h>

#include "kdevcore.h"
#include "kdevlanguagesupport.h"
#include "kdevmainwindow.h"
#include "kdevpartcontroller.h"
#include "classstore.h"

#include "classviewpart.h"
#include "classtooldlg.h"

KPopupMenu *ClassTreeItem::createPopup()
{
    if (!m_item || m_item->itemType() == PIT_SCOPE)
        return 0;

    KDevLanguageSupport::Features features = classTree()->m_part->languageSupport()->features();

    KPopupMenu *popup = new KPopupMenu();
    if (features & KDevLanguageSupport::Declarations)
        popup->insertItem( i18n("Go to Declaration"), classTree(), SLOT(slotGotoDeclaration()) );
    if (m_item->itemType() == PIT_METHOD)
        popup->insertItem( i18n("Go to Definition"), classTree(), SLOT(slotGotoImplementation()) );

    QString title;
    switch(m_item->itemType()) {
    case PIT_CLASS:
        {
            title = i18n("Class");
            bool hasAddMethod = features & KDevLanguageSupport::AddMethod;
            bool hasAddAttribute = features & KDevLanguageSupport::AddAttribute;
            if (hasAddMethod)
                popup->insertItem( i18n("Add Method..."), classTree(), SLOT(slotAddMethod()));
            if (hasAddAttribute)
                popup->insertItem( i18n("Add Attribute..."), classTree(), SLOT(slotAddAttribute()));
            popup->insertSeparator();
            popup->insertItem( i18n("Parent Classes..."), classTree(), SLOT(slotClassBaseClasses()));
            popup->insertItem( i18n("Child Classes..."), classTree(), SLOT(slotClassDerivedClasses()));
            popup->insertItem( i18n("Class Tool..."), classTree(), SLOT(slotClassTool()));
            }
        break;
    case PIT_STRUCT:
        title = i18n("Struct");
        break;
    case PIT_ATTRIBUTE:
        if (m_item->isGlobal())
            title = i18n("Variable");
        else
            title = i18n("Attribute");
        break;
    case PIT_METHOD:
        if (static_cast<ParsedMethod*>(m_item)->isSlot())
            title = i18n("Slot");
        else if (static_cast<ParsedMethod*>(m_item)->isSignal())
            title = i18n("Signal");
        else if (m_item->isGlobal())
            title = i18n("Function");
        else
            title = i18n("Method");
        break;
    default:
        ;
    }
    popup->insertSeparator();
    popup->insertTitle(title, -1, 0);
        
    return popup;
}


QString ClassTreeItem::scopedText() const
{
    if (m_item)
        return m_item->path();

    return QString::null;
}


void ClassTreeItem::getDeclaration(QString *toFile, int *toLine)
{
    if (m_item) {
        *toFile = m_item->declaredInFile();
        *toLine = m_item->declaredOnLine();
    }
}


void ClassTreeItem::getImplementation(QString *toFile, int *toLine)
{
    if (m_item) {
        *toFile = m_item->definedInFile();
        *toLine = m_item->definedOnLine();
    }
}


QString ClassTreeItem::text( int ) const
{
    if (m_item)
        return m_item->asString();
    return QString::null;
}


QString ClassTreeItem::tipText() const
{
    // Purposefully avoid virtual dispatch here
    return ClassTreeItem::text(0);
}


void ClassTreeOrganizerItem::init()
{
    setExpandable(true);
    setPixmap(0, SmallIcon("folder"));
}


void ClassTreeScopeItem::init()
{
    setExpandable(true);
    setPixmap(0, UserIcon("CVnamespace", KIcon::DefaultState, ClassViewFactory::instance()));
}


QString ClassTreeScopeItem::text( int col ) const
{
    if (!m_item)
        return QString::null;
    if (m_item->name().isEmpty())
        return i18n("Global");
    return ClassTreeItem::text( col );
}


void ClassTreeScopeItem::setOpen(bool o)
{
    if ( !m_item)
        return;

    kdDebug(9003) << (o? "Open scope item" : "Close scope item") << endl;
    if (o && childCount() == 0) {

        ParsedScopeContainer *pScope = static_cast<ParsedScopeContainer*>(m_item);
        ClassTreeItem *lastItem = 0;

        // Ok, this is a hack...
        KDevLanguageSupport::Features features = classTree()->m_part->languageSupport()->features();
        
        // Add namespaces
        Q3ValueList<ParsedScopeContainer*> scopeList = pScope->getSortedScopeList();
        Q3ValueList<ParsedScopeContainer*>::ConstIterator it;
        for (it = scopeList.begin(); it != scopeList.end(); ++it)
            lastItem = new ClassTreeScopeItem(this, lastItem, *it);

        if (features & KDevLanguageSupport::Classes) {
            // Add classes
            Q3ValueList<ParsedClass*> classList = pScope->getSortedClassList();
            Q3ValueList<ParsedClass*>::ConstIterator it;
            for (it = classList.begin(); it != classList.end(); ++it)
                lastItem = new ClassTreeClassItem(this, lastItem, *it);
        }
            
        if (features & KDevLanguageSupport::Structs) {
            // Add structs
            Q3ValueList<ParsedClass*> structList = pScope->getSortedStructList();
            Q3ValueList<ParsedClass*>::ConstIterator it;
            for (it = structList.begin(); it != structList.end(); ++it)
                lastItem = new ClassTreeClassItem(this, lastItem, *it, true);
        }

        if (features & KDevLanguageSupport::Functions) {
            // Add functions
            Q3ValueList<ParsedMethod*> methodList = pScope->getSortedMethodList();
            Q3ValueList<ParsedMethod*>::ConstIterator it;
            for (it = methodList.begin(); it != methodList.end(); ++it)
                lastItem = new ClassTreeMethodItem(this, lastItem, *it);
        }

        if (features & KDevLanguageSupport::Variables) {
            // Add attributes
            Q3ValueList<ParsedAttribute*> attrList = pScope->getSortedAttributeList();
            Q3ValueList<ParsedAttribute*>::ConstIterator it;
            for (it = attrList.begin(); it != attrList.end(); ++it)
                lastItem = new ClassTreeAttrItem(this, lastItem, *it);
        }

    }

    ClassTreeItem::setOpen(o);
}


void ClassTreeClassItem::init()
{
    setExpandable(true);
    setPixmap(0, UserIcon(m_isStruct ? "CVstruct" : "CVclass", KIcon::DefaultState, ClassViewFactory::instance()));
}


void ClassTreeClassItem::setOpen(bool o)
{
    if ( !m_item )
        return;
    kdDebug(9003) << (o? "Open class item" : "Close class item") << endl;
    if (o && childCount() == 0) {

        ParsedClass *pClass = static_cast<ParsedClass*>(m_item);
        ClassTreeItem *lastItem = 0;

        // Add nested classes
        Q3ValueList<ParsedClass*> classList = pClass->getSortedClassList();
        Q3ValueList<ParsedClass*>::ConstIterator classIt;
        for (classIt = classList.begin(); classIt != classList.end(); ++classIt)
            lastItem = new ClassTreeClassItem(this, lastItem, *classIt);

        // Add nested structs
        Q3ValueList<ParsedClass*> structList = pClass->getSortedStructList();
        Q3ValueList<ParsedClass*>::ConstIterator structIt;
        for (structIt = structList.begin(); structIt != structList.end(); ++structIt)
            lastItem = new ClassTreeClassItem(this, lastItem, *structIt, true);

        // Add methods
        Q3ValueList<ParsedMethod*> methodList = pClass->getSortedMethodList();
        Q3ValueList<ParsedMethod*>::ConstIterator methodIt;
        for (methodIt = methodList.begin(); methodIt != methodList.end(); ++methodIt)
            lastItem = new ClassTreeMethodItem(this, lastItem, *methodIt);

        // Add slots
        Q3ValueList<ParsedMethod*> slotList = pClass->getSortedSlotList();
        Q3ValueList<ParsedMethod*>::ConstIterator slotIt;
        for (slotIt = slotList.begin(); slotIt != slotList.end(); ++slotIt)
            lastItem = new ClassTreeMethodItem(this, lastItem, *slotIt);

        // Add signals
        Q3ValueList<ParsedMethod*> signalList = pClass->getSortedSignalList();
        Q3ValueList<ParsedMethod*>::ConstIterator signalIt;
        for (signalIt = signalList.begin(); signalIt != signalList.end(); ++signalIt)
            lastItem = new ClassTreeMethodItem(this, lastItem, *signalIt);

        // Add attributes
        Q3ValueList<ParsedAttribute*> attrList = pClass->getSortedAttributeList();
        Q3ValueList<ParsedAttribute*>::ConstIterator attrIt;
        for (attrIt = attrList.begin(); attrIt != attrList.end(); ++attrIt)
            lastItem = new ClassTreeAttrItem(this, lastItem, *attrIt);
        
    }
    
    ClassTreeItem::setOpen(o);
}

ClassTreeMethodItem::ClassTreeMethodItem(ClassTreeItem *parent, ClassTreeItem *lastSibling,
                                         ParsedMethod *parsedMethod)
    : ClassTreeItem(parent, lastSibling, parsedMethod)
{
    QString icon;

    if ( !parsedMethod )
      return;

    if (parsedMethod->isSignal())
        icon = "CVpublic_signal";
    else if (parsedMethod->isSlot()) {
        if (parsedMethod->isPublic())
            icon = "CVpublic_slot";
        else if (parsedMethod->isProtected())
            icon = "CVprotected_slot";
        else
            icon = "CVprivate_slot";
    }
    else if (parsedMethod->isPublic())
        icon = "CVpublic_meth";
    else if (parsedMethod->isProtected())
        icon = "CVprotected_meth";
    else if (parsedMethod->isPrivate())
        icon = "CVprivate_meth";
    else if (parsedMethod->isPackage())
        icon = "CVpackage_meth";
    else
        icon = "CVglobal_meth";

    setPixmap(0, UserIcon(icon, KIcon::DefaultState, ClassViewFactory::instance()));
}

QString ClassTreeMethodItem::text( int ) const
{
    QString str;

    if ( !m_item )
        return QString::null;
 
    ParsedMethod* method = static_cast<ParsedMethod*>(m_item);

    str = method->name();
    
    if( method->arguments.count() > 0 ) {
        str += "( ";
        for ( ParsedArgument *arg = method->arguments.first(); arg != NULL; arg = method->arguments.next() ) {
            if ( arg != method->arguments.getFirst() )
                str += ", ";

            str += arg->toString();
        }
        str += " )";
    } else {
        str += "()";
    }
    
    if( method->isConst() )
        str += " const";

    return str;
}


ClassTreeAttrItem::ClassTreeAttrItem(ClassTreeItem *parent, ClassTreeItem *lastSibling,
                                     ParsedAttribute *parsedAttr)
    : ClassTreeItem(parent, lastSibling, parsedAttr)
{
    QString icon;
    
    if ( !parsedAttr )
      return;

    if (parsedAttr->isPublic())
        icon = "CVpublic_var";
    else if (parsedAttr->isProtected())
        icon = "CVprotected_var";
    else if (parsedAttr->isPrivate())
        icon = "CVprivate_var";
    else if (parsedAttr->isPackage())
        icon = "CVpackage_var";
    else
        icon = "CVglobal_var";

    setPixmap(0, UserIcon(icon, KIcon::DefaultState, ClassViewFactory::instance()));
}


QString ClassTreeAttrItem::text( int ) const
{
    if ( !m_item )
        return QString::null;
    return m_item->name();
}

ClassTreeScriptItem::ClassTreeScriptItem(ClassTreeItem *parent, ClassTreeItem *lastSibling,
                                     ParsedScript *parsedScript)
    : ClassTreeItem(parent, lastSibling, parsedScript)
{
    QString icon;

    if ( !parsedScript )
      return;

    setExpandable(true);

    //need a icon for scripts
    icon = "CVpublic_var";
    setPixmap(0, UserIcon(icon, KIcon::DefaultState, ClassViewFactory::instance()));
}


QString ClassTreeScriptItem::text( int ) const
{
    if ( !m_item )
        return QString::null;
    return m_item->name();
}

void ClassTreeScriptItem::setOpen(bool o)
{
    if ( !m_item )
        return;
    kdDebug(9003) << (o? "Open script item" : "Close script item") << endl;
    if (o && childCount() == 0) {

        ParsedScript *pClass = static_cast<ParsedScript*>(m_item);
        ClassTreeItem *lastItem = 0;

        // Add methods
        Q3ValueList<ParsedMethod*> methodList = pClass->getSortedMethodList();
        Q3ValueList<ParsedMethod*>::ConstIterator methodIt;
        for (methodIt = methodList.begin(); methodIt != methodList.end(); ++methodIt)
            lastItem = new ClassTreeMethodItem(this, lastItem, *methodIt);

        // Add attributes
        Q3ValueList<ParsedAttribute*> attrList = pClass->getSortedAttributeList();
        Q3ValueList<ParsedAttribute*>::ConstIterator attrIt;
        for (attrIt = attrList.begin(); attrIt != attrList.end(); ++attrIt)
            lastItem = new ClassTreeAttrItem(this, lastItem, *attrIt);

    }

    ClassTreeItem::setOpen(o);
}


class ClassToolTip : public QToolTip
{
public:
    ClassToolTip( QWidget *parent )
        : QToolTip(parent)
        {}

protected:
    void maybeTip(const QPoint &p);
};


void ClassToolTip::maybeTip(const QPoint &p)
{
    ClassTreeBase *ctw = static_cast<ClassTreeBase*>(parentWidget());

    Q3ListViewItem *item = ctw->itemAt(p);
    QRect r = ctw->itemRect(item);

    if (item && r.isValid()) {
        ClassTreeItem *ctitem = static_cast<ClassTreeItem*>(item);
        QString str = ctitem->tipText();
        if (!str.isEmpty())
            tip(r, str);
    }
}


ClassTreeBase::ClassTreeBase(ClassViewPart *part, QWidget *parent, const char *name)
    : KListView(parent, name)
{
    setFocusPolicy(ClickFocus);
    setRootIsDecorated(true);
    setResizeMode(Q3ListView::LastColumn);
    setSorting(-1);
    header()->hide();
    addColumn(QString::null);

    (void) new ClassToolTip(this);
    
    connect( this, SIGNAL(executed(Q3ListViewItem*)),
             this, SLOT(slotItemExecuted(Q3ListViewItem*)) );
    connect( this, SIGNAL(mouseButtonPressed(int, Q3ListViewItem*, const QPoint&, int)),
             this, SLOT(slotItemPressed(int, Q3ListViewItem*)) );
    connect( this, SIGNAL(returnPressed( Q3ListViewItem*)), 
             SLOT( slotItemExecuted(Q3ListViewItem*)) );
    connect( this, SIGNAL(contextMenuRequested(Q3ListViewItem*, const QPoint&, int)),
             this, SLOT(slotContextMenuRequested(Q3ListViewItem*, const QPoint&)) );

    m_part = part;
}


ClassTreeBase::~ClassTreeBase()
{}


ClassTreeBase::TreeState ClassTreeBase::treeState() const
{
    TreeState state;

    ClassTreeBase *that = const_cast<ClassTreeBase*>(this);
    Q3ListViewItemIterator it(that);
    for (; it.current(); ++it)
        if (it.current()->isOpen()) {
            QStringList path;
            Q3ListViewItem *item = it.current();
            while (item) {
                path.prepend(item->text(0));
                item = item->parent();
            }
            state.append(path);
        }

    return state;
}


void ClassTreeBase::setTreeState(TreeState state)
{
    TreeStateIterator tsit;
    for (tsit = state.begin(); tsit != state.end(); ++tsit) {
        Q3ListViewItemIterator it(this);
        for (; it.current(); ++it) {
            QStringList path;
            Q3ListViewItem *item = it.current();
            while (item) {
                path.prepend(item->text(0));
                item = item->parent();
            }
            if (*tsit == path) {
                it.current()->setOpen(true);
                break;
            }
        }
    }
}



  
void ClassTreeBase::slotItemExecuted( Q3ListViewItem* item )
{
    if (!item)
        return;

    // toggle open state for parents
    if (item->childCount() > 0)
        setOpen(item, !isOpen(item));

    // We assume here that ALL (!) items in the list view
    // are ClassTreeItem's
    ClassTreeItem *ctitem = static_cast<ClassTreeItem*>(item);
    if (ctitem->isOrganizer())
        return;
        
    QString toFile;
    int toLine = -1;
    if (dynamic_cast<ClassTreeClassItem*>(item)) {
        ctitem->getDeclaration(&toFile, &toLine);
    }
    else {
	ctitem->getImplementation(&toFile, &toLine);
    }
    m_part->partController()->editDocument(toFile, toLine);
    m_part->mainWindow()->lowerView(this);
}


void ClassTreeBase::slotItemPressed(int button, Q3ListViewItem *item)
{
    if (!item)
        return;

    // We assume here that ALL (!) items in the list view
    // are ClassTreeItem's
    ClassTreeItem *ctitem = static_cast<ClassTreeItem*>(item);
    if (ctitem->isOrganizer())
        return;

    if (button == MidButton) {
        QString toFile;
        int toLine = -1;
        ctitem->getDeclaration(&toFile, &toLine);
        m_part->partController()->editDocument(toFile, toLine);
        m_part->mainWindow()->lowerView(this);
    }
}

void ClassTreeBase::slotContextMenuRequested(Q3ListViewItem *item, const QPoint &p)
{
    contextItem = static_cast<ClassTreeItem*>(item);
    
    KPopupMenu *popup = createPopup();
    popup->exec(p);
    delete popup;
}

void ClassTreeBase::slotGotoDeclaration()
{
    QString toFile;
    int toLine = -1;
    
    contextItem->getDeclaration(&toFile, &toLine);
    m_part->partController()->editDocument(toFile, toLine);
}


void ClassTreeBase::slotGotoImplementation()
{
    QString toFile;
    int toLine = -1;
    
    contextItem->getImplementation(&toFile, &toLine);
    m_part->partController()->editDocument(toFile, toLine);
}


void ClassTreeBase::slotAddMethod()
{
    if (m_part->languageSupport())
        m_part->languageSupport()->addMethod(contextItem->scopedText());
}


void ClassTreeBase::slotAddAttribute()
{
    if (m_part->languageSupport())
        m_part->languageSupport()->addAttribute(contextItem->scopedText());
}


void ClassTreeBase::slotClassBaseClasses()
{
    ClassToolDialog *dlg = new ClassToolDialog(m_part);
    dlg->setClassName(contextItem->scopedText());
    dlg->viewParents();
}


void ClassTreeBase::slotClassDerivedClasses()
{
    ClassToolDialog *dlg = new ClassToolDialog(m_part);
    dlg->setClassName(contextItem->scopedText());
    dlg->viewChildren();
}


void ClassTreeBase::slotClassTool()
{
    ClassToolDialog *dlg = new ClassToolDialog(m_part);
    dlg->setClassName(contextItem->scopedText());
    dlg->viewNone();
}

#include "classtreebase.moc"
