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


#include <qtooltip.h>
#include <qheader.h>
#include <qregexp.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kpopupmenu.h>
#include <klocale.h>
#include <kiconloader.h>

#include "kdevcore.h"
#include "kdevlanguagesupport.h"
#include "classstore.h"

#include "classviewfactory.h"
#include "classviewpart.h"
#include "classtooldlg.h"
#include "classtreebase.h"


void ClassTreeItem::init(const QString &text)
{
    if (m_item) {
        KConfig *config = ClassViewFactory::instance()->config();
        config->setGroup("General");
        bool showScoped = config->readBoolEntry("FullIdentifierScopes", false);
        setText(0, showScoped? scopedText() : m_item->asString());
    } else
        setText(0, text);
}


KPopupMenu *ClassTreeItem::createPopup()
{
    if (!m_item || m_item->itemType() == PIT_SCOPE)
        return 0;

    KDevLanguageSupport::Features features = classTree()->m_part->languageSupport()->features();

    KPopupMenu *popup = new KPopupMenu();
    if (features & KDevLanguageSupport::Declarations)
        popup->insertItem( i18n("Go to declaration..."), classTree(), SLOT(slotGotoDeclaration()) );
    if (m_item->itemType() == PIT_METHOD)
        popup->insertItem( i18n("Go to definition..."), classTree(), SLOT(slotGotoImplementation()) );

    QString title;
    switch(m_item->itemType()) {
    case PIT_CLASS:
        {
            title = i18n("Class");
            KDevLanguageSupport *ls = classTree()->m_part->languageSupport();
            bool hasAddMethod = features & KDevLanguageSupport::AddMethod;
            bool hasAddAttribute = features & KDevLanguageSupport::AddAttribute;
            if (hasAddMethod)
                popup->insertItem( i18n("Add method..."), classTree(), SLOT(slotAddMethod()));
            if (hasAddAttribute)
                popup->insertItem( i18n("Add attribute..."), classTree(), SLOT(slotAddAttribute()));
            popup->insertSeparator();
            popup->insertItem( i18n("Parent classes..."), classTree(), SLOT(slotClassBaseClasses()));
            popup->insertItem( i18n("Child classes..."), classTree(), SLOT(slotClassDerivedClasses()));
            popup->insertItem( i18n("Class tool..."), classTree(), SLOT(slotClassTool()));
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
    QString str;

    if (m_item)
        str = m_item->path();

    return str.replace(QRegExp("\\."), "::");
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


void ClassTreeScopeItem::setOpen(bool o)
{
    kdDebug(9003) << (o? "Open scope item" : "Close scope item") << endl;
    if (o && childCount() == 0) {

        ParsedScopeContainer *pScope = static_cast<ParsedScopeContainer*>(m_item);
        ClassTreeItem *lastItem = 0;

        // Ok, this is a hack...
        KDevLanguageSupport::Features features = classTree()->m_part->languageSupport()->features();
        
        // Add namespaces
        QList<ParsedScopeContainer> *scopeList = pScope->getSortedScopeList();
        for ( ParsedScopeContainer *pScope = scopeList->first();
              pScope != 0;
              pScope = scopeList->next() ) {
            lastItem = new ClassTreeScopeItem(this, lastItem, pScope);
        }
        delete scopeList;

        if (features & KDevLanguageSupport::Classes) {
            // Add classes
            QList<ParsedClass> *classList = pScope->getSortedClassList();
            for ( ParsedClass *pClass = classList->first();
                  pClass != 0;
                  pClass = classList->next() ) {
                lastItem = new ClassTreeClassItem(this, lastItem, pClass);
            }
            delete classList;
        }
            
        if (features & KDevLanguageSupport::Structs) {
            // Add structs
            QList<ParsedStruct> *structList = pScope->getSortedStructList();
            for ( ParsedStruct *pStruct = structList->first();
                  pStruct != 0;
                  pStruct = structList->next() ) {
                lastItem = new ClassTreeStructItem(this, lastItem, pStruct);
            }
            delete structList;
        }
            
        if (features & KDevLanguageSupport::Functions) {
            // Add functions
            QList<ParsedMethod> *methodList = pScope->getSortedMethodList();
            for ( ParsedMethod *pMethod = methodList->first();
                  pMethod != 0;
                  pMethod = methodList->next() ) {
                lastItem = new ClassTreeMethodItem(this, lastItem, pMethod);
            }
            delete methodList;
        }
            
        if (features & KDevLanguageSupport::Variables) {
            // Add attributes
            QList<ParsedAttribute> *attrList = pScope->getSortedAttributeList();
            for ( ParsedAttribute *pAttr = attrList->first();
                  pAttr != 0;
                  pAttr = attrList->next() ) {
                lastItem = new ClassTreeAttrItem(this, lastItem, pAttr);
            }
            delete attrList;
        }
            
    }
    
    ClassTreeItem::setOpen(o);
}


void ClassTreeClassItem::init()
{
    setExpandable(true);
    setPixmap(0, UserIcon("CVclass", KIcon::DefaultState, ClassViewFactory::instance()));
}


void ClassTreeClassItem::setOpen(bool o)
{
    kdDebug(9003) << (o? "Open class item" : "Close class item") << endl;
    if (o && childCount() == 0) {

        ParsedClass *pClass = static_cast<ParsedClass*>(m_item);
        ClassTreeItem *lastItem = 0;
        
        // Add nested classes
        QDictIterator<ParsedClass> &classIt = pClass->classIterator;
        for ( classIt.toFirst();
              classIt.current();
              ++classIt) {
            lastItem = new ClassTreeClassItem(this, lastItem, classIt.current());
        }

        // Add nested structs
        QList<ParsedStruct> *structList = pClass->getSortedStructList();
        for ( ParsedStruct *pStruct = structList->first();
              pStruct != 0;
              pStruct = structList->next() ) {
            lastItem = new ClassTreeStructItem(this, lastItem, pStruct);
        }
        delete structList;
        
        // Add methods
        QList<ParsedMethod> *methodList = pClass->getSortedMethodList();
        for ( ParsedMethod *pMethod = methodList->first();
              pMethod != 0;
              pMethod = methodList->next() ) {
            lastItem = new ClassTreeMethodItem(this, lastItem, pMethod);
        }
        delete methodList;

        // Add slots
        QList<ParsedMethod> *slotList = pClass->getSortedSlotList();
        for ( ParsedMethod *pSlot = slotList->first();
              pSlot != 0;
              pSlot = slotList->next() ) {
            lastItem = new ClassTreeMethodItem(this, lastItem, pSlot);
        }
        delete slotList;

        // Add signals
        QList<ParsedMethod> *signalList = pClass->getSortedSignalList();
        for ( ParsedMethod *pSignal = signalList->first();
              pSignal != 0;
              pSignal = signalList->next() ) {
            lastItem = new ClassTreeMethodItem(this, lastItem, pSignal);
        }
        delete signalList;

        // Add attributes
        QList<ParsedAttribute> *attrList = pClass->getSortedAttributeList();
        for ( ParsedAttribute *pAttr = attrList->first();
              pAttr != 0;
              pAttr = attrList->next() ) {
            lastItem = new ClassTreeAttrItem(this, lastItem, pAttr);
        }
        delete attrList;

    }
    
    ClassTreeItem::setOpen(o);
}


ClassTreeStructItem::ClassTreeStructItem(ClassTreeItem *parent, ClassTreeItem *lastSibling,
                                         ParsedStruct *parsedStruct)
    : ClassTreeItem(parent, lastSibling, parsedStruct)
{
    setExpandable(true);
    setPixmap(0, UserIcon("CVstruct", KIcon::DefaultState, ClassViewFactory::instance()));
}


void ClassTreeStructItem::setOpen(bool o)
{
    kdDebug(9003) << (o? "Open struct item" : "Close struct item") << endl;
    if (o && childCount() == 0) {

        ParsedStruct *pStruct = static_cast<ParsedStruct*>(m_item);
        ClassTreeItem *lastItem = 0;
        
        // Add structs
        QList<ParsedStruct> *structList = pStruct->getSortedStructList();
        for ( ParsedStruct *pStruct = structList->first();
              pStruct != 0;
              pStruct = structList->next() ) {
            lastItem = new ClassTreeStructItem(this, lastItem, pStruct);
        }
        delete structList;

    }
    
    ClassTreeItem::setOpen(o);
}


ClassTreeMethodItem::ClassTreeMethodItem(ClassTreeItem *parent, ClassTreeItem *lastSibling,
                                         ParsedMethod *parsedMethod)
    : ClassTreeItem(parent, lastSibling, parsedMethod)
{
    QString icon;
    
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

QString ClassTreeMethodItem::scopedText() const
{
    QString str;

    if (m_item)
        str = m_item->asString();

    return str;
}


ClassTreeAttrItem::ClassTreeAttrItem(ClassTreeItem *parent, ClassTreeItem *lastSibling,
                                     ParsedAttribute *parsedAttr)
    : ClassTreeItem(parent, lastSibling, parsedAttr)
{
    QString icon;
    
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

    QListViewItem *item = ctw->itemAt(p);
    QRect r = ctw->itemRect(item);

    if (item && r.isValid()) {
        ClassTreeItem *ctitem = static_cast<ClassTreeItem*>(item);
        QString str = ctitem->scopedText();
        if (!str.isEmpty())
            tip(r, str);
    }
}


ClassTreeBase::ClassTreeBase(ClassViewPart *part, QWidget *parent, const char *name)
    : KListView(parent, name)
{
    setFocusPolicy(ClickFocus);
    setRootIsDecorated(true);
    setSorting(-1);
    setFrameStyle(Panel | Sunken);
    setLineWidth(2); 
    header()->hide();
    addColumn("");

    (void) new ClassToolTip(this);
    
    connect( this, SIGNAL(mouseButtonPressed(int, QListViewItem*, const QPoint&, int)),
             this, SLOT(slotItemPressed(int, QListViewItem*)) );
    connect( this, SIGNAL(rightButtonPressed(QListViewItem*, const QPoint&, int)),
             this, SLOT(slotRightButtonPressed(QListViewItem*, const QPoint&)) );

    m_part = part;
}


ClassTreeBase::~ClassTreeBase()
{}


ClassTreeBase::TreeState ClassTreeBase::treeState() const
{
    TreeState state;

    ClassTreeBase *that = const_cast<ClassTreeBase*>(this);
    QListViewItemIterator it(that);
    for (; it.current(); ++it)
        if (it.current()->isOpen()) {
            QStringList path;
            QListViewItem *item = it.current();
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
        QListViewItemIterator it(this);
        for (; it.current(); ++it) {
            QStringList path;
            QListViewItem *item = it.current();
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


void ClassTreeBase::slotItemPressed(int button, QListViewItem *item)
{
    // We assume here that ALL (!) items in the list view
    // are ClassTreeItem's
    ClassTreeItem *ctitem = static_cast<ClassTreeItem*>(item);
    if (!ctitem)
        return;

    if (button == LeftButton) {
        QString toFile;
        int toLine = -1;
        static_cast<ClassTreeItem*>(item)->getImplementation(&toFile, &toLine);
        m_part->core()->gotoSourceFile(toFile, toLine);
	m_part->core()->lowerWidget(this);
    } else if (button == MidButton) {
        QString toFile;
        int toLine = -1;
        static_cast<ClassTreeItem*>(item)->getDeclaration(&toFile, &toLine);
        m_part->core()->gotoSourceFile(toFile, toLine);
	m_part->core()->lowerWidget(this);
    }
}


void ClassTreeBase::slotRightButtonPressed(QListViewItem *item, const QPoint &p)
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
    m_part->core()->gotoSourceFile(toFile, toLine);
}


void ClassTreeBase::slotGotoImplementation()
{
    QString toFile;
    int toLine = -1;
    
    contextItem->getImplementation(&toFile, &toLine);
    m_part->core()->gotoSourceFile(toFile, toLine);
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
