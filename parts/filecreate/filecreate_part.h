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
class FileCreateTypeChooser;

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

  void selectWidget(int widgetNumber);
  FileCreateTypeChooser * widget() const {
    return (m_selectedWidget>=0 && m_selectedWidget<m_numWidgets) ?
                              m_availableWidgets[m_selectedWidget] : NULL;
  }


  /**
   * Finds the file type object for a given extension and optionally subtype.
   * You can omit the subtype and specify the extension as ext-subtype if you wish.
   */
  FileCreateFileType * getType(const QString & ext, const QString subtype = QString::null);
    
public slots:
  void slotProjectOpened();
  void slotProjectClosed();
  void slotFiletypeSelected(const FileCreateFileType *);
      
  
private:
    
  //FileCreateTypeChooser * m_widget;
  int m_selectedWidget;
  QPtrList<FileCreateFileType> m_filetypes;

  FileCreateTypeChooser * m_availableWidgets[2];
  int m_numWidgets;

  bool setWidget(FileCreateTypeChooser * widget);
  void refresh();
  int readTypes(const QDomDocument & dom, bool enable);
  
};


#endif
