#include <qlcdnumber.h>
#include <qpopupmenu.h>
#include <qwhatsthis.h>
#include <kdebug.h>
#include <klocale.h>
#include <kaction.h>

#include "classview.h"
#include "classactions.h"
#include "main.h"
#include "cproject.h"
#include "ctreehandler.h"
#include "ClassStore.h"
#include "kdevlanguagesupport.h"


ClassView::ClassView(QObject *parent, const char *name)
    : KDevComponent(parent, name)
{
    setInstance(ClassFactory::instance());
    setXMLFile("kdevclassview.rc");
    
    m_cv_decl_or_impl = false;
    m_langsupport = 0;
    m_store = 0;
}


ClassView::~ClassView()
{}


void ClassView::setupGUI()
{
    QLCDNumber *w = new QLCDNumber();
    w->display(42);
    //    w->setIcon()
    w->setCaption(i18n("Class view"));
    QWhatsThis::add(w, i18n("Class View\n\n"
                            "The class viewer shows all classes, methods and variables "
                            "of the current project files and allows switching to declarations "
                            "and implementations. The right button popup menu allows more specialized "
                            "functionality."));

    embedWidget(w, SelectView, i18n("CV"), i18n("class tree view"));
    
    classes_action = new ClassListAction(i18n("Classes"), 0, this, SLOT(selectedClass()),
                                         actionCollection(), "class_combo");
    methods_action = new MethodListAction(i18n("Methods"), 0, this, SLOT(selectedMethod()),
                                          actionCollection(), "method_combo");
    popup_action  = new DelayedPopupAction(i18n("Declaration/Implementation"), "classwiz", 0, this, SLOT(switchedDeclImpl()),
                                           actionCollection(), "class_wizard");
    setupPopup();
}


void ClassView::setupPopup()
{
    QPopupMenu *popup = popup_action->popupMenu();

    popup->clear();
    popup->insertItem(i18n("Goto declaration"), this, SLOT(selectedGotoDeclaration()));
    popup->insertItem(i18n("Goto implementation"), this, SLOT(selectedGotoImplementation()));
    popup->insertItem(i18n("Goto class declaration"), this, SLOT(selectedGotoClassDeclaration()));

    if (m_langsupport) {
        bool hasAddMethod = m_langsupport->hasFeature(KDevLanguageSupport::AddMethod);
        bool hasAddAttribute = m_langsupport->hasFeature(KDevLanguageSupport::AddAttribute);
        if (hasAddMethod || hasAddAttribute) 
            popup->insertSeparator();
        if (hasAddMethod)
            popup->insertItem(i18n("Add method..."), this, SLOT(selectedAddMethod()));
        if (hasAddAttribute)
            popup->insertItem(i18n("Add attribute..."), this, SLOT(selectedAddAttribute()));
    }
}


void ClassView::projectOpened(CProject *prj)
{
    kdDebug(9003) << "ClassView::projectOpened()" << endl;
    classes_action->setEnabled(true);
    methods_action->setEnabled(true);
    popup_action->setEnabled(true);
}

void ClassView::projectClosed()
{
    kdDebug(9003) << "ClassView::projectClosed()" << endl;
    classes_action->setEnabled(false);
    methods_action->setEnabled(false);
    popup_action->setEnabled(false);
}


void ClassView::languageSupportOpened(KDevLanguageSupport *ls)
{
    m_langsupport = ls;
    setupPopup();
}

void ClassView::languageSupportClosed()
{
    m_langsupport = 0;
    setupPopup();
}


void ClassView::classStoreOpened(CClassStore *store)
{
    kdDebug(9003) << "ClassView::classStoreOpened()" << endl;
    classes_action->setClassStore(store);
    methods_action->setClassStore(store);
    classes_action->refresh();
    classes_action->refresh();
    m_store = store;
}


void ClassView::classStoreClosed()
{
    kdDebug(9003) << "ClassView::classStoreClosed()" << endl;
    classes_action->setClassStore(0);
    methods_action->setClassStore(0);
    classes_action->refresh();
    classes_action->refresh();
    m_store = 0;
}


void ClassView::addedFileToProject(const QString &name)
{
    kdDebug(9003) << "ClassView::addedFileToProject()" << endl;
    // This could be much finer-grained...
    //    classWidget()->refresh();
}


void ClassView::removedFileFromProject(const QString &name)
{
    kdDebug(9003) << "ClassView::removedFileFromProject()" << endl;
    // This could be much finer-grained...
    //    classWidget()->refresh();
}


void ClassView::savedFile(const QString &name)
{
    kdDebug(9003) << "ClassView::savedFile()" << endl;
    if (CProject::getType(name) == CPP_HEADER)
        ;
        //        classWidget()->refresh();
}


/**
 * The user selected a class in the toolbar class combo.
 */
void ClassView::selectedClass()
{
    QString className = classes_action->currentText();
    if (className.isEmpty())
        return;
    
    kdDebug(9003) << "ClassView: Class selected: " << className << endl;
    methods_action->refresh(className);
}


/**
 * The user selected a method in the toolbar method combo.
 */
void ClassView::selectedMethod()
{
    QString className = classes_action->currentText();
    QString methodName = methods_action->currentText();
    if (className.isEmpty() || methodName.isEmpty())
        return;

    kdDebug(9003) << "ClassView: Method selected: "
                  << className << "::" << methodName << endl;
    m_cv_decl_or_impl = true;
    gotoImplementation(className, methodName, THPUBLIC_METHOD);
}


/**
 * The user clicked on the class wizard button.
 */
void ClassView::switchedDeclImpl()
{
    QString className = classes_action->currentText();
    QString methodName = methods_action->currentText();

    kdDebug(9003) << "ClassView::switchedDeclImpl" << endl;
    if (m_cv_decl_or_impl) {
        m_cv_decl_or_impl = false;
        gotoDeclaration(className, methodName, methodName.isEmpty()? THCLASS : THPUBLIC_METHOD);
    } else {
        m_cv_decl_or_impl = true;
        if (methodName.isEmpty())
            gotoDeclaration(className, "", THCLASS);
        else
            gotoImplementation(className, methodName, THPUBLIC_METHOD);
    }
}


/**
 * The user selected "Goto declaration" from the delayed class wizard popup.
 */
void ClassView::selectedGotoDeclaration()
{
    QString className = classes_action->currentText();
    QString methodName = methods_action->currentText();
    
    gotoDeclaration(className, methodName, methodName.isEmpty()? THCLASS : THPUBLIC_METHOD);
}


/**
 * The user selected "Goto class declaration" from the delayed class wizard popup.
 */
void ClassView::selectedGotoClassDeclaration()
{
    QString className = classes_action->currentText();
    
    gotoDeclaration(className, "", THCLASS);
}


/**
 * The user selected "Goto implementation" from the delayed class wizard popup.
 */
void ClassView::selectedGotoImplementation()
{
    QString className = classes_action->currentText();
    QString methodName = methods_action->currentText();

    if (methodName.isEmpty())
        gotoDeclaration(className, "", THCLASS);
    else
        gotoImplementation(className, methodName, THPUBLIC_METHOD);
}


/**
 * The user selected "Add method..." from the delayed class wizard popup.
 */
void ClassView::selectedAddMethod()
{
    if (m_langsupport)
        m_langsupport->addMethodRequested(classes_action->currentText());
}


/**
 * The user selected "Add attribute..." from the delayed class wizard popup.
 */
void ClassView::selectedAddAttribute()
{
    if (m_langsupport)
        m_langsupport->addAttributeRequested(classes_action->currentText());
}


CParsedClass *ClassView::getClass(const QString &className)
{
    if (className.isEmpty())
        return 0;

    kdDebug(9003) << "ClassView::getClass " << className << endl;
    CParsedClass *pc = m_store->getClassByName(className);
    if (pc && pc->isSubClass)
        classes_action->setCurrentItem(className);
    
    return pc;
}


void ClassView::gotoDeclaration(const QString &className,
                                const QString &declName,
                                THType type)
{
    kdDebug(9003) << "ClassView::gotoDeclaration " << className << "::" << declName << endl;
    
    QString toFile;
    int toLine = -1;
    
    CParsedClass *pc = getClass(className);
    CParsedStruct *ps = 0;
    CParsedAttribute *pa = 0;
    
    switch(type) {
    case THCLASS:
        toFile = pc->declaredInFile;
        toLine = pc->declaredOnLine;
        break;
    case THSTRUCT:
        if (pc)
            pc->getStructByName(declName);
        else
            ps = m_store->globalContainer.getStructByName(declName);
        toFile = ps->declaredInFile;
        toLine = ps->declaredOnLine;
        break;
    case THPUBLIC_ATTR:
    case THPROTECTED_ATTR:
    case THPRIVATE_ATTR:
        if (pc)
            pa = pc->getAttributeByName(declName);
        else {
            ps = m_store->globalContainer.getStructByName(className);
            if (ps)
                pa = ps->getAttributeByName(declName);
        }
        break;
    case THPUBLIC_METHOD:
    case THPROTECTED_METHOD:
    case THPRIVATE_METHOD:
        pa = pc->getMethodByNameAndArg(declName);
        // If at first we don't succeed...
        if (!pa)
            pa = pc->getSlotByNameAndArg(declName);      
        break;
    case THPUBLIC_SLOT:
    case THPROTECTED_SLOT:
    case THPRIVATE_SLOT:
        pa = pc->getSlotByNameAndArg(declName);
      break;
    case THSIGNAL:
        pa = pc->getSignalByNameAndArg(declName);
      break;
    case THGLOBAL_FUNCTION:
        pa = m_store->globalContainer.getMethodByNameAndArg(declName);
      break;
    case THGLOBAL_VARIABLE:
        pa = m_store->globalContainer.getAttributeByName(declName);
        break;
    default:
        kdDebug(9003) << "Unknown type " << (int)type << " in CVGotoDeclaration." << endl;
        break;
    }
    
    // Fetch the line and file from the attribute if the value is set.
    if (pa) {
        toFile = pa->declaredInFile;
        toLine = pa->declaredOnLine;
    }
    
    if (toLine != -1) {
        kdDebug(9003) << "Classview switching to file " << toFile << "@ line " << toLine << endl;
        emit gotoSourceFile(toFile, toLine);
    }
}


void ClassView::gotoImplementation(const QString &className,
                                   const QString &declName,
                                   THType type)
{
    kdDebug(9003) << "ClassView::gotoImplementation " << className << "::" << declName << endl;
    CParsedClass *pc = getClass(className);
    CParsedMethod *pm = 0;
    
    switch(type) {
    case THPUBLIC_SLOT:
    case THPROTECTED_SLOT:
    case THPRIVATE_SLOT:
        if (pc)
            pm = pc->getSlotByNameAndArg(declName);
        break;
    case THPUBLIC_METHOD:
    case THPROTECTED_METHOD:
    case THPRIVATE_METHOD:
        if (pc) {
            pm = pc->getMethodByNameAndArg(declName);
            // If at first we don't succeed...
            if (!pm)
                pm = pc->getSlotByNameAndArg(declName); 
        }
        break;
    case THGLOBAL_FUNCTION:
        pm = m_store->globalContainer.getMethodByNameAndArg(declName);
        break;
    default:
        kdDebug(9003) << "Unknown type " << (int)type << "in CVGotoDefinition." << endl;
    }
    
    if (pm)
        emit gotoSourceFile(pm->definedInFile, pm->definedOnLine);
}
