#ifndef __FILECREATE_TYPECHOOSER_H__
#define __FILECREATE_TYPECHOOSER_H__

class FileCreateFileType;
class FileCreatePart;

class FileCreateTypeChooser {

public:
  FileCreateTypeChooser(FileCreatePart * part) : m_part(part) { }
  virtual ~FileCreateTypeChooser() { }

  virtual void setPart(FileCreatePart * part) { m_part = part; }
  virtual FileCreatePart * part() const { return m_part; }
  virtual void refresh() = 0;

  // signals
  virtual void filetypeSelected(const FileCreateFileType * filetype) = 0;

protected:
  FileCreatePart * m_part;
};

#endif
