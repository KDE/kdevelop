

#ifndef __KDEVPART_FILESELECTOR_H__
#define __KDEVPART_FILESELECTOR_H__

#include <qguardedptr.h>
#include <kdialogbase.h>
#include <kdevplugin.h>
#include <kfileitem.h>


class FileSelectorWidget;

class FileSelectorPart : public KDevPlugin
{
  Q_OBJECT

public:

  FileSelectorPart(QObject *parent, const char *name, const QStringList &);
  ~FileSelectorPart();

public slots:

//  void showTip();
//  void showOnStart();
//  void refresh();
	void fileSelected(const KFileItem *file);

 private slots:
   void slotProjectOpened();
private:
    QGuardedPtr<FileSelectorWidget> m_filetree;


};


#endif
