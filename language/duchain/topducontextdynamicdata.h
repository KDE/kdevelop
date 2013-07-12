class QFile;
/* This  is part of KDevelop
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

#ifndef KDEVPLATFORM_TOPDUCONTEXTDYNAMICDATA_H
#define KDEVPLATFORM_TOPDUCONTEXTDYNAMICDATA_H

#include <QtCore/QVector>
#include <QtCore/QMutex>
#include <QtCore/QByteArray>
#include <QtCore/QPair>
#include <interfaces/iastcontainer.h>

namespace KDevelop {

class TopDUContext;
class DUContext;
class Declaration;
class IndexedString;
class IndexedDUContext;
class DUChainBaseData;

typedef QPair<QByteArray, uint> ArrayWithPosition;

///This class contains dynamic data of a top-context, and also the repository that contains all the data within this top-context.
class TopDUContextDynamicData {
  public:
  TopDUContextDynamicData(TopDUContext* topContext);
  ~TopDUContextDynamicData();

  void clearContextsAndDeclarations();
  
  /**
   * Allocates an index for the given declaration in this top-context.
   * The returned index is never zero.
   * @param anonymous whether the declaration is temporary. If it is, it will be stored separately, not stored to disk,
   *                   and a duchain write-lock is not needed. Else, you need a write-lock when calling this.
  */
  uint allocateDeclarationIndex(Declaration* decl, bool temporary);
  
  Declaration* getDeclarationForIndex(uint index) const;
  
  bool isDeclarationForIndexLoaded(uint index) const;
  
  void clearDeclarationIndex(Declaration* decl);

  /**
   * Allocates an index for the given context in this top-context.
   * The returned index is never zero.
   * @param anonymous whether the context is temporary. If it is, it will be stored separately, not stored to disk,
   *                   and a duchain write-lock is not needed. Else, you need a write-lock when calling this.
  */
  uint allocateContextIndex(DUContext* ctx, bool temporary);
  
  DUContext* getContextForIndex(uint index) const;
  
  bool isContextForIndexLoaded(uint index) const;
  
  void clearContextIndex(DUContext* ctx);
  
  ///Stores this top-context to disk
  void store();
  
  ///Stores all remainings of this top-context that are on disk. The top-context will be fully dynamic after this.
  void deleteOnDisk();
  
  ///Whether this top-context is on disk(Either has been loaded, or has been stored)
  bool isOnDisk() const;
  
  ///Loads the top-context from disk, or returns zero on failure. The top-context will not be registered anywhere, and will have no ParsingEnvironmentFile assigned.
  ///Also loads all imported contexts. The Declarations/Contexts will be correctly initialized, and put into the symbol tables if needed.
  static TopDUContext* load(uint topContextIndex);

  ///Loads only the url out of the data stored on disk for the top-context.
  static IndexedString loadUrl(uint topContextIndex);

  static bool fileExists(uint topContextIndex);
  
  ///Loads only the list of importers out of the data stored on disk for the top-context.
  static QList<IndexedDUContext> loadImporters(uint topContextIndex);
  
  static QList<IndexedDUContext> loadImports(uint topContextIndex);

  bool isTemporaryContextIndex(uint index) const;
  bool isTemporaryDeclarationIndex(uint index) const ;
  
  bool m_deleting; ///Flag used during destruction

  struct ItemDataInfo {
    //parentContext 0 means the global context
    ItemDataInfo(uint _dataOffset = 0, uint _parentContext = 0) : dataOffset(_dataOffset), parentContext(_parentContext) {
    }
    uint dataOffset; //Offset of the data
    uint parentContext; //Parent context of the data
  };

  private:
    void unmap();
    //Converts away from an mmap opened file to a data array
    
    QString filePath() const;

    void loadData() const;

    
    static void verifyDataInfo(const ItemDataInfo& info, const QList<ArrayWithPosition>& data);

    static const char* pointerInData(const QList<ArrayWithPosition>& data, uint totalOffset);
    
    const char* pointerInData(uint offset) const;

    ItemDataInfo writeDataInfo(const ItemDataInfo& info, const DUChainBaseData* data, uint& totalDataOffset);

    TopDUContext* m_topContext;
    //May contain zero contexts if they were deleted
    mutable QVector<DUContext*> m_contexts;
    mutable DUContext** m_fastContexts;
    mutable int m_fastContextsSize;
    //May contain zero declarations if they were deleted
    mutable QVector<Declaration*> m_declarations;
    mutable Declaration** m_fastDeclarations;
    mutable int m_fastDeclarationsSize;
    
    mutable QVector<ItemDataInfo> m_contextDataOffsets;
    
    mutable QVector<ItemDataInfo> m_declarationDataOffsets;
    
    //Protects m_temporaryDeclarations, must be locked before accessing that vector
    static QMutex m_temporaryDataMutex;
    //For temporary declarations that will not be stored to disk, like template instantiations
    QVector<Declaration*> m_temporaryDeclarations;
    QVector<DUContext*> m_temporaryContexts;
    
    mutable QList<ArrayWithPosition> m_data;
    mutable QList<ArrayWithPosition> m_topContextData;
    bool m_onDisk;
    mutable bool m_dataLoaded;

    mutable QFile* m_mappedFile;
    mutable uchar* m_mappedData;
    mutable size_t m_mappedDataSize;
    mutable bool m_itemRetrievalForbidden;
};
}

Q_DECLARE_TYPEINFO(KDevelop::TopDUContextDynamicData::ItemDataInfo, Q_MOVABLE_TYPE);

#endif
