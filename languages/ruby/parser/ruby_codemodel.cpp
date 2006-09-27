// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#include "ruby_codemodel.h"

namespace ruby
  {


  // ---------------------------------------------------------------------------
#define CLASS CodeModel
#define BASECLASS KDevCodeModel

  CodeModel::CodeModel( QObject *parent )
      :  KDevCodeModel( parent )
  {}

  CodeModel::~CodeModel()
  {}

  void CodeModel::wipeout()
  {}

  void CodeModel::addCodeItem(CodeModelItem item)
  {
    beginAppendItem(item);
    endAppendItem();
  }

  void CodeModel::removeCodeItem(CodeModelItem item)
  {
    beginRemoveItem(item);
    endRemoveItem();
  }

#undef CLASS
#undef BASECLASS

  // ---------------------------------------------------------------------------
#define CLASS _CodeModelItem
#define BASECLASS KDevCodeItem

  _CodeModelItem::_CodeModelItem(CodeModel *model,  int kind)
      :  KDevCodeItem( QString::null,  0 )
      ,  _M_model(model)
      ,  _M_kind(kind)
      ,  _M_startLine( -1)
      ,  _M_startColumn( -1)
      ,  _M_endLine( -1)
      ,  _M_endColumn( -1)
  {}

  _CodeModelItem::~_CodeModelItem()
  {}

  _CodeModelItem *_CodeModelItem::itemAt(int index) const
    {
      return  static_cast<_CodeModelItem*>(KDevItemCollection::itemAt(index));
    }

  CodeModelItem _CodeModelItem::toItem() const
    {
      return  CodeModelItem(const_cast<_CodeModelItem*>(this));
    }

  int _CodeModelItem::kind() const
    {
      return  _M_kind;
    }

  void _CodeModelItem::setKind(int kind)
  {
    _M_kind =  kind;
  }

  KTextEditor::Cursor _CodeModelItem::startPosition() const
    {
      return  KTextEditor::Cursor(_M_startLine,  _M_startColumn);
    }

  void _CodeModelItem::setStartPosition(const KTextEditor::Cursor& cursor)
  {
    _M_startLine =  cursor.line();
    _M_startColumn =  cursor.column();
  }

  KTextEditor::Cursor _CodeModelItem::endPosition() const
    {
      return  KTextEditor::Cursor(_M_endLine,  _M_endColumn);
    }

  void _CodeModelItem::setEndPosition(const KTextEditor::Cursor& cursor)
  {
    _M_endLine =  cursor.line();
    _M_endColumn =  cursor.column();
  }

#undef CLASS
#undef BASECLASS


} // end of namespace ruby


