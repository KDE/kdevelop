#ifndef __FILECREATE_TYPECHOOSERSIG_H__
#define __FILECREATE_TYPECHOOSERSIG_H__

class FileCreateFileType;

#include <qobject.h>

class Signaller : public QObject {
  Q_OBJECT

public:
  Signaller() : QObject() { } 
  virtual ~Signaller() { } 

  void signal(const FileCreateFileType * filetype ) {
    emit filetypeSelected(filetype);
  }
  
    
signals:
  void filetypeSelected(const FileCreateFileType * filetype);
};

#endif
