#ifndef _KDEVELOP_H_
#define _KDEVELOP_H_


#include <qlist.h>

class KDevelop;
class KDevComponent;
class KDevVersionControl;
class KDevLanguageSupport;
class CProject;


class KDevelopCore : QObject
{
    Q_OBJECT
    
public:
    KDevelopCore(KDevelop *gui);
    ~KDevelopCore();

    void loadInitialComponents();

private:
    void initActions();
    void initComponent(KDevComponent *component);
    
    void loadVersionControl(const QString &vcsystem);
    void unloadVersionControl();
    void loadLanguageSupport(const QString &lang);
    void unloadLanguageSupport();
    void loadProject();
    void unloadProject();

private slots:
    // Handling of actions
    void slotFilePrint();
    void slotOptionsKDevelopSetup();

    // Handling of component requests
    void executeMakeCommand(const QString &command);
    void executeAppCommand(const QString &command);
    void gotoSourceFile(const QString &fileName, int lineNo);
    void gotoDocumentationFile(const QString &fileName);
    void gotoProjectApiDoc();
    void gotoProjectManual();

private:
    KDevelop *m_kdevelopgui;
    QList<KDevComponent> m_components;
    QList<KDevComponent> m_runningcomponents;
    
    KDevVersionControl *m_versioncontrol;
    KDevLanguageSupport *m_languagesupport;
    KDevComponent *m_makefrontend;
    KDevComponent *m_appfrontend;
    CProject *m_project;
    CClassStore *m_classstore;
};

#endif
