#include <kdebug.h>

#include "cppsupport.h"
#include "cproject.h"
#include "ParsedClass.h"
#include "ParsedAttribute.h"
#include "ParsedMethod.h"
#include "ClassStore.h"
#include "classparser.h"
#include "caddclassmethoddlg.h"
#include "caddclassattributedlg.h"
#include "main.h"


CppSupport::CppSupport(QObject *parent, const char *name)
    : KDevLanguageSupport(parent, name)
{
    setInstance(CppSupportFactory::instance());

    m_store = 0;
    m_parser = 0;
}


CppSupport::~CppSupport()
{}


void CppSupport::projectOpened(CProject *prj)
{
    kdDebug(9007) << "CppSupport::projectOpened()" << endl;
}


void CppSupport::projectClosed()
{
    kdDebug(9007) << "CppSupport::projectClosed()" << endl;
}


void CppSupport::classStoreOpened(CClassStore *store)
{
    m_store = store;
    m_parser = new CClassParser(store);
}


void CppSupport::classStoreClosed()
{
    m_store = 0;
    delete m_parser;
    m_parser = 0;
}


void CppSupport::addedFileToProject(const QString &fileName)
{
    kdDebug(9007) << "CppSupport::addedFileToProject()" << endl;
    m_parser->parse(fileName);
}


void CppSupport::removedFileFromProject(const QString &fileName)
{
    kdDebug(9007) << "CppSupport::removedFileFromProject()" << endl;
    m_parser->removeWithReferences(fileName);
}


void CppSupport::savedFile(const QString &fileName)
{
    kdDebug(9007) << "CppSupport::savedFile()" << endl;

    if (CProject::getType(fileName) == CPP_HEADER)
        m_parser->parse(fileName);

    // We also need a way to tell other components that the class store
    // has changed...
}


bool CppSupport::hasFeature(Features feature)
{
    return (feature==AddMethod) || (feature==AddAttribute);
}


void CppSupport::newClassRequested()
{
}


void CppSupport::addMethodRequested(const QString &className)
{
    CAddClassMethodDlg dlg(0, "methodDlg");
    if (!dlg.exec())
        return;
    
    CParsedMethod *pm = dlg.asSystemObj();
    pm->setDeclaredInScope(className);

    int atLine = -1;
    CParsedClass *pc = m_store->getClassByName(className);
    
    if (pm->isSignal) {
        for (pc->signalIterator.toFirst(); pc->signalIterator.current(); ++pc->signalIterator) {
            CParsedMethod *meth = pc->signalIterator.current();
            if (meth->exportScope == pm->exportScope && 
                atLine < meth->declarationEndsOnLine)
                atLine = meth->declarationEndsOnLine;
        }
    } else if (pm->isSlot) {
        for (pc->slotIterator.toFirst(); pc->slotIterator.current(); ++pc->slotIterator) {
            CParsedMethod *meth = pc->slotIterator.current();
            if (meth->exportScope == pm->exportScope && 
                atLine < meth->declarationEndsOnLine)
                atLine = meth->declarationEndsOnLine;
        }
    } else {
        for (pc->methodIterator.toFirst(); pc->methodIterator.current(); ++pc->methodIterator) {
            CParsedMethod *meth = pc->methodIterator.current();
            if (meth->exportScope == pm->exportScope && 
                atLine < meth->declarationEndsOnLine)
                atLine = meth->declarationEndsOnLine;
        }
    }

    QString headerCode = asHeaderCode(pm);
    
    if (atLine == -1) {
        if (pm->isSignal) 
            headerCode.prepend(QString("signals:\n"));
        else if (pm->exportScope == PIE_PUBLIC)
            headerCode.prepend(QString("public:%1\n").arg(pm->isSlot? " slots" :  ""));
        else if (pm->exportScope == PIE_PROTECTED)
            headerCode.prepend(QString("protected:\n").arg(pm->isSlot? " slots" :  ""));
        else if (pm->exportScope == PIE_PRIVATE) 
            headerCode.prepend(QString("private:\n").arg(pm->isSlot? " slots" :  ""));
        else
            kdDebug(9003) << "CppSupport::selectedAddMethod: Unknown exportScope "
                          << (int)pm->exportScope << endl;

        atLine = pc->declarationEndsOnLine;
    } else 
        atLine++;

    gotoSourceFile(pc->declaredInFile, atLine);
    kdDebug(9007) << "####################" << "Adding at line " << atLine << " " 
                  << headerCode << endl
                  << "####################";

    QString cppCode = asCppCode(pm);
    
    gotoSourceFile(pc->definedInFile, atLine);
    kdDebug(9007) << "####################" << "Adding at line " << atLine
                  << " " << cppCode
                  << "####################" << endl;
    
    delete pm;
}


void CppSupport::addAttributeRequested(const QString &className)
{
    CAddClassAttributeDlg dlg(0, "attrDlg");
    if( !dlg.exec() )
      return;

    CParsedAttribute *pa = dlg.asSystemObj();
    pa->setDeclaredInScope(className);

    int atLine = -1;
    CParsedClass *pc = m_store->getClassByName(className);
    
    for (pc->attributeIterator.toFirst(); pc->attributeIterator.current(); ++pc->attributeIterator) {
        CParsedAttribute *attr = pc->attributeIterator.current();
        if (attr->exportScope == pa->exportScope && 
            atLine < attr->declarationEndsOnLine)
            atLine = attr->declarationEndsOnLine;
    }
    
    QString headerCode = asHeaderCode(pa);
    
    if (atLine == -1) {
        if (pa->exportScope == PIE_PUBLIC)
            headerCode.prepend("public: // Public attributes\n");
        else if (pa->exportScope == PIE_PROTECTED)
            headerCode.prepend("protected: // Protected attributes\n");
        else if (pa->exportScope == PIE_PRIVATE) 
            headerCode.prepend("private: // Private attributes\n");
        else
            kdDebug(9003) << "CppSupport::selectedAddAttribute: Unknown exportScope "
                          << (int)pa->exportScope << endl;

        atLine = pc->declarationEndsOnLine;
    } else 
        atLine++;

    gotoSourceFile(pc->declaredInFile, atLine);
    kdDebug(9007) << "####################" << "Adding at line " << atLine
                  << " " << headerCode
                  << "####################" << endl;

    delete pa;
}


QString CppSupport::asHeaderCode(CParsedMethod *pm)
{
    QString str = "  ";
    str += pm->comment;
    str += "\n  ";

    if (pm->isVirtual)
        str += "virtual ";
    
    if (pm->isStatic)
        str += "static ";
    
    str += pm->type;
    str += " ";
    str += pm->name;
    
    if (pm->isConst)
        str += " const";
    
    if (pm->isPure)
        str += " = 0";
    
    str += ";\n";

    return str;
}


QString CppSupport::asCppCode(CParsedMethod *pm)
{
    if (pm->isPure || pm->isSignal)
        return QString();

    QString str = pm->comment;
    str += "\n";

    // Take the path and replace all . with ::
    QString path = pm->path();
    path.replace( QRegExp( "\\." ), "::" );

    str += pm->type;
    str += " ";
    str += pm->path();
    
    if (pm->isConst)
        str += " const";
    
    str += "{\n}\n";

    return str;
}


QString CppSupport::asHeaderCode(CParsedAttribute *pa)
{
    QString str = "  ";
    str += pa->comment;
    str += "\n  ";

    if (pa->isConst)
        str += "const ";

    if (pa->isStatic)
        str += "static ";

    QString attrString;
    pa->asString(attrString);

    str += attrString;
    str += ";\n";

    return str;
}
