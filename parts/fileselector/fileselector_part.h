#ifndef __KDEVPART_FILESELECTOR_H__
#define __KDEVPART_FILESELECTOR_H__

#include <kdevplugin.h>

#include <kdialogbase.h>
#include <kfileitem.h>

#include <qguardedptr.h>

class KDevFileSelector;
class KDialogBase;

class FileSelectorPart : public KDevPlugin
{
    Q_OBJECT
public:
    FileSelectorPart(QObject *parent, const char *name, const QStringList &);
    virtual ~FileSelectorPart();

    KAction *newFileAction() const { return m_newFileAction; }

public slots:
//  void showTip();
//  void showOnStart();
//  void refresh();
    void newFile();
    void fileSelected(const KFileItem *file);

private slots:
    void slotProjectOpened();
    void slotConfigWidget( KDialogBase * );

private:
    QGuardedPtr<KDevFileSelector> m_filetree;
    KAction *m_newFileAction;
};


#endif
