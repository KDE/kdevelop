#ifndef __KDEV_PARTCONTROLLER_H__
#define __KDEV_PARTCONTROLLER_H__


#include <kurl.h>
#include <kparts/partmanager.h>


class KDevPartController : public KParts::PartManager
{
  Q_OBJECT

public:

  KDevPartController(QWidget *parent);

  virtual void editDocument(const KURL &url, int lineNum=-1) = 0;
  virtual void showDocument(const KURL &url, int lineNum=-1) = 0;

  virtual void saveAllFiles() = 0; 
  virtual void revertAllFiles() = 0;

  /**
   * Goes to a given location in a source file and marks the line.
   * This is used by the debugger to mark the location where the
   * the debugger has stopped.
   */
  virtual void gotoExecutionPoint(const KURL &url, int lineNum=0) = 0;

  /**
   * Clear the execution point. Usefull if debugging has ended.
   */
  virtual void clearExecutionPoint() = 0;


signals:

  /**
   * Emitted when a document has been saved.
   */
  void savedFile(const QString &fileName);

  /**
   * Emitted when a document has been loaded.
   */
  void loadedFile(const QString &fileName);

};



#endif
