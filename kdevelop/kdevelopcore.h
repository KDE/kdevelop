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

private slots:
    void gotoSourceFile(const QString &filename, int lineno);
    void gotoDocumentationFile(const QString &filename);
    void gotoProjectApiDoc();
    void gotoProjectManual();

private:
    void loadComponents(KDevelop *gui);
    void initComponents(KDevelop *gui);

    QList<KDevComponent> m_components;
};

#endif
