#ifndef _CPPSUPPORT_H_
#define _CPPSUPPORT_H_


#include "kdevlanguagesupport.h"

class CParsedMethod;
class CParsedAttribute;
class CClassStore;


class CppSupport : public KDevLanguageSupport
{
    Q_OBJECT

public:
    CppSupport( QObject *parent=0, const char *name=0 );
    ~CppSupport();

protected:
    virtual void projectOpened(CProject *prj);
    virtual void projectClosed();
    virtual void classStoreOpened(CClassStore *store);
    virtual void classStoreClosed();
    virtual void addedFileToProject(const QString &name);
    virtual void removedFileFromProject(const QString &name);
    virtual void savedFile(const QString &name);

    virtual void addMethodRequested(const QString &className);
    virtual void addAttributeRequested(const QString &className);

private:
    QString asHeaderCode(CParsedMethod *pm);
    QString asCppCode(CParsedMethod *pm);
    QString asHeaderCode(CParsedAttribute *pa);

    CClassStore *m_store;
};

#endif
