#ifndef _KDEVELOP_H_
#define _KDEVELOP_H_


#include <qlist.h>

class KDevelop;
class KDevComponent;


class KDevelopCore : QObject
{
    Q_OBJECT
    
public:
    KDevelopCore(KDevelop *gui);
    ~KDevelopCore();

    void setupKDevelop();
    
private:
    void initActions();
    void loadComponents();
    void initComponents();

private slots:
    void gotoSourceFile(const QString &filename, int lineno);
    void gotoDocumentationFile(const QString &filename);
    void gotoProjectApiDoc();
    void gotoProjectManual();

private:
    KDevelop *m_kdevelopgui;
    QList<KDevComponent> m_components;
};

#endif
