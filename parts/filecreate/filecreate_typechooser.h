#ifndef __FILECREATE_TYPECHOOSER_H__
#define __FILECREATE_TYPECHOOSER_H__

#include "filecreate_typechoosersig.h"

class FileCreateFileType;
class FileCreatePart;

class FileCreateTypeChooser  {
  
public:

  FileCreateTypeChooser(FileCreatePart * part) : m_part(part) {
    m_signaller = new Signaller;
  }
  virtual ~FileCreateTypeChooser() { delete m_signaller; }

  virtual void setPart(FileCreatePart * part) { m_part = part; }
  virtual FileCreatePart * part() const { return m_part; }
  virtual void refresh() = 0;

  // signals
  virtual void filetypeSelected(const FileCreateFileType * filetype) {
    if (filetype) m_signaller->signal(filetype);
  }

  const Signaller * signaller() const { return m_signaller; }  
  
protected:
  FileCreatePart * m_part;
  Signaller * m_signaller;

};

#endif
