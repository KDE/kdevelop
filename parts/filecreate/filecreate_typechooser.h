#ifndef __FILECREATE_TYPECHOOSER_H__
#define __FILECREATE_TYPECHOOSER_H__

#include "filecreate_typechoosersig.h"

#include <kdebug.h>

class FileCreateFileType;
class FileCreatePart;

class FileCreateTypeChooser  {
  
public:

  FileCreateTypeChooser(FileCreatePart * part) : m_part(part), m_current(NULL) {
    m_signaller = new Signaller;
  }
  virtual ~FileCreateTypeChooser() { delete m_signaller; }

  virtual void setPart(FileCreatePart * part) { m_part = part; }
  virtual FileCreatePart * part() const { return m_part; }
  virtual void refresh() = 0;
  virtual const FileCreateFileType * current() const { return m_current; }

  // signals
  virtual void filetypeSelected(const FileCreateFileType * filetype) {
    //m_current = filetype;
    kdDebug(9034) << "type selected: about to signal" << endl;
    if (filetype) m_signaller->signal(filetype);
    kdDebug(9034) << "type selected: done signal" << endl;
  }

  const Signaller * signaller() const { return m_signaller; }  
  
protected:
  FileCreatePart * m_part;
  Signaller * m_signaller;
  const FileCreateFileType * m_current;

};

#endif
