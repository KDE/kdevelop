#ifndef _KDEVELOP_H_
#define _KDEVELOP_H_


#include <qlist.h>

class KDevelop;
class KDevComponent;
class KDevVersionControl;
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
    void loadVersionControl(const QString &name);
    void unloadVersionControl();

private slots:
    // Handling of actions
    void slotFilePrint();
    void slotOptionsKDevelopSetup();

    // Handling of component requests
    void addFileToRepository(const QString &filename);
    void removeFileFromRepository(const QString &filename);
    void commitFileToRepository(const QString &filename);
    void updateFileFromRepository(const QString &filename);
    void executeCommand(const QString &command);
    void gotoSourceFile(const QString &filename, int lineno);
    void gotoDocumentationFile(const QString &filename);
    void gotoProjectApiDoc();
    void gotoProjectManual();

private:
    KDevelop *m_kdevelopgui;
    QList<KDevComponent> m_components;
    KDevVersionControl *m_versioncontrol;
    CProject *m_project;
};

#endif
