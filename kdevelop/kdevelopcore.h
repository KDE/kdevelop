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
    void addMethod(const QString &className);
    void addAttribute(const QString &className);
    void addToRepository(const QString &fileName);
    void removeFromRepository(const QString &fileName);
    void commitToRepository(const QString &fileName);
    void updateFromRepository(const QString &fileName);
    void executeMakeCommand(const QString &command);
    void executeAppCommand(const QString &command);
    void gotoSourceFile(const QString &fileName, int lineNo);
    void gotoDocumentationFile(const QString &fileName);
    void gotoProjectApiDoc();
    void gotoProjectManual();

private:
    KDevelop *m_kdevelopgui;
    QList<KDevComponent> m_components;
    KDevVersionControl *m_versioncontrol;
    KDevLanguageSupport *m_languagesupport;
    KDevComponent *m_makefrontend;
    KDevComponent *m_appfrontend;
    CProject *m_project;
};

#endif
