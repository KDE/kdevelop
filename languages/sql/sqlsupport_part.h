#ifndef __KDEVPART_SQLSUPPORT_H__
#define __KDEVPART_SQLSUPPORT_H__

#include <qguardedptr.h>
#include <qstringlist.h>

#include <kprocess.h>
#include <kdialogbase.h>

#include "kdevplugin.h"
#include "kdevlanguagesupport.h"

class SqlListAction;
class SqlOutputWidget;
class CodeModel;

class SQLSupportPart : public KDevLanguageSupport
{
  Q_OBJECT

public:

    SQLSupportPart(QObject *parent, const char *name, const QStringList &);
    virtual ~SQLSupportPart();

    static QString cryptStr(const QString& aStr);
    const QStringList& connections() const { return conNames; }


protected:
    virtual Features features();
    virtual KMimeType::List mimeTypes();

private slots:
    void slotRun();
    void projectConfigWidget(KDialogBase *dlg);
    void projectOpened();
    void projectClosed();
    void savedFile(const KURL &fileName);
    void addedFilesToProject(const QStringList &fileList);
    void removedFilesFromProject(const QStringList &fileList);
    void parse();
    void loadConfig();
    void activeConnectionChanged();

private:
    void clearConfig();
    void updateCatalog();
    SqlListAction* dbAction;
    SqlOutputWidget* m_widget;
    QStringList conNames;
    CodeModel *m_store;
};

#endif
