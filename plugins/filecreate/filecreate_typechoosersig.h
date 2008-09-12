#ifndef __FILECREATE_TYPECHOOSERSIG_H__
#define __FILECREATE_TYPECHOOSERSIG_H__

#include <qobject.h>

namespace FileCreate {

class FileType;

class Signaller : public QObject {
  Q_OBJECT

public:
  Signaller() : QObject() { } 
  virtual ~Signaller() { } 

  virtual void signal(const FileType * filetype ) {
    emit filetypeSelected(filetype);
  }
  
    
signals:
  void filetypeSelected(const FileType * filetype);
};

}

#endif
