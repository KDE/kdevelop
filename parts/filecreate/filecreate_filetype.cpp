
#include "filecreate_filetype.h"

void FileCreateFileType::setSubtypesEnabled(bool enabled) {
  for(FileCreateFileType * subtype = m_subtypes.first();
      subtype;subtype=m_subtypes.next())
    subtype->setEnabled(enabled);
}
