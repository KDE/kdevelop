#ifndef _CLASSVIEW_H_
#define _CLASSVIEW_H_

#include <kaction.h>
#include "ParsedClass.h"
#include "ctreehandler.h"
#include "kdevcomponent.h"
#include "classactions.h"


class ClassWidget;
class CClassStore;


class ClassView : public KDevComponent
{
    Q_OBJECT

public:
    ClassView( QWidget *parent=0, const char *name=0 );
    ~ClassView();

protected:
    virtual void projectOpened(CProject *prj);
    virtual void projectClosed();
    virtual void classStoreOpened(CClassStore *store);
    virtual void classStoreClosed();

    virtual void addedFileToProject(const QString &name);
    virtual void removedFileFromProject(const QString &name);
    virtual void savedFile(const QString &name);

private slots:
    void selectedClass();
    void selectedMethod();
    void switchedDeclImpl();
    void selectedGotoDeclaration();
    void selectedGotoImplementation();
    void selectedGotoClassDeclaration();
    void selectedAddMethod();
    void selectedAddAttribute();
    
private:
    ClassWidget *classWidget()
        { return (ClassWidget*) KDevComponent::widget(); }
    friend class ClassWidget;

    CParsedClass *getClass(const QString &className);
    void gotoDeclaration(const QString &classname,
                         const QString &declName,
                         THType type);
    void gotoImplementation(const QString &classname,
                            const QString &declName,
                            THType type);

    ClassListAction *classes_action;
    MethodListAction *methods_action;
    KDevDelayedPopupAction *popup_action;
    CClassStore *m_store;
    bool m_cv_decl_or_impl;
};

#endif
