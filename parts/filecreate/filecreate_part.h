/*
 *  Copyright (C) 2001 Julian Rockey <linux@jrockey.com>   
 */
 

#ifndef __KDEVPART_FILECREATE_H__
#define __KDEVPART_FILECREATE_H__


#include <qguardedptr.h>
#include <qptrlist.h>

#include <kdevplugin.h>
#include <kdevcreatefile.h>


class FileCreateWidget;
class FileCreateFileType;

class FileCreatePart : public KDevPlugin, public KDevCreateFile
{
  Q_OBJECT

public:
  FileCreatePart(QObject *parent, const char *name, const QStringList &);
  virtual ~FileCreatePart();

  QPtrList<FileCreateFileType> getFileTypes() const { return m_filetypes; }

  virtual KDevCreateFile::CreatedFile createNewFile(QString ext = QString::null,
                     QString dir = QString::null,
                     QString name = QString::null,
                     QString subtype = QString::null);
    
public slots:
  void slotProjectOpened();
  void slotProjectClosed();
  void slotFiletypeSelected(const FileCreateFileType *);
      
  
private:
    
  QGuardedPtr<FileCreateWidget> m_widget;
  QPtrList<FileCreateFileType> m_filetypes;

};


#endif
