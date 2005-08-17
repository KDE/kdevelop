#ifndef __KDEVPART_FILESELECTOR_H__
#define __KDEVPART_FILESELECTOR_H__

#include <kdevplugin.h>

#include <kdialogbase.h>
#include <kfileitem.h>

#include <qpointer.h>

class KDevFileSelector;
class KDialogBase;

class FileSelectorPart : public KDevPlugin
{
    Q_OBJECT
public:
    FileSelectorPart(QObject *parent, const char *name, const QStringList &);
    virtual ~FileSelectorPart();

public slots:
//  void showTip();
//  void showOnStart();
//  void refresh();
    void fileSelected(const KFileItem *file);

private slots:
    void slotProjectOpened();
    void slotConfigWidget( KDialogBase * );

private:
    QPointer<KDevFileSelector> m_filetree;
};


#endif
