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

  /**
   * Call this method to create a new file, within or without the project. Supply as
   * much information as you know. Leave what you don't know as QString::null.
   * The user will be prompted as necessary for the missing information, and the
   * file created, and added to the project as necessary.
   */ 
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
