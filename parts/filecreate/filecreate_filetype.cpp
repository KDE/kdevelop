/***************************************************************************
 *   Copyright (C) 2003 by Julian Rockey                                   *
 *   linux@jrockey.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "filecreate_filetype.h"

void FileCreateFileType::setSubtypesEnabled(bool enabled) {
  for(FileCreateFileType * subtype = m_subtypes.first();
      subtype;subtype=m_subtypes.next())
    subtype->setEnabled(enabled);
}
