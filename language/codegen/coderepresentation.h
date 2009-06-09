/*
   Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef CODEREPRESENTATION_H
#define CODEREPRESENTATION_H

#include "../languageexport.h"
#include <language/duchain/indexedstring.h>

class QString;

namespace KTextEditor {
    class Range;
}

namespace KDevelop {

    class IndexedString;
    
///Allows getting code-lines conveniently, either through an open editor, or from a disk-loaded file.
class KDEVPLATFORMLANGUAGE_EXPORT CodeRepresentation {
  public:
    virtual ~CodeRepresentation() {
    }
    virtual QString line(int line) const = 0;
    virtual int lines() const = 0;
    virtual QString text() const = 0;
    ///Overwrites the text in the file with the new given one
    ///@return true on success
    virtual bool setText(QString) = 0;
    
    ///Can be used for example from tests to disallow on-disk changes. When such a change is done, an assertion is triggered.
    ///You should enable this within tests, unless you really want to work on the disk.
    static void setDiskChangesForbidden(bool changesForbidden);
};

class KDEVPLATFORMLANGUAGE_EXPORT DynamicCodeRepresentation : public CodeRepresentation {
  public:
      ///Used to group edit-history together. Call this before a bunch of replace(), and endEdit in the end.
      virtual void startEdit() = 0;
      virtual bool replace(const KTextEditor::Range& range, QString oldText, QString newText, bool ignoreOldText = false) = 0;
      virtual QString rangeText(KTextEditor::Range range) const = 0;
      ///Must be called exactly once per startEdit()
      virtual void endEdit() = 0;
};

///Creates a code-representation for the given url, that allows conveniently accessing its data. Returns zero on failure.
KDEVPLATFORMLANGUAGE_EXPORT CodeRepresentation* createCodeRepresentation(IndexedString url);

///An artificial source-code representation that can be used for testing.
///It inserts itself automatically into the code-representation framework,
///logically representing the contents of a file. Thus, it can be used for testing
///refactoring.
class KDEVPLATFORMLANGUAGE_EXPORT InsertArtificialCodeRepresentation {
    public:
        InsertArtificialCodeRepresentation(IndexedString file, QString text);
        ~InsertArtificialCodeRepresentation();
        void setText(QString text);
        QString text();
    private:
        IndexedString m_file;
};

}


#endif
