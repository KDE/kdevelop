// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#ifndef ruby_CODEMODEL_H
#define ruby_CODEMODEL_H

#include "kdevcodemodel.h"

#include "ruby_codemodel_fwd.h"

#include <QtCore/QTime>
#include <QtCore/QHash>
#include <QtCore/QList>

#include <ktexteditor/cursor.h>


#include "ruby_ast.h"

#include <QtCore/QString>
#include <QtCore/QStringList>

namespace ruby
  {

#define DECLARE_MODEL_NODE(k) \
enum { __node_kind = Kind_##k }; \
typedef KDevSharedPtr<k##ModelItem> Pointer;

#define ITEM(item) item##ModelItem
#define LIST(item) item##List

  template  <class _Target,  class _Source>
  _Target model_static_cast(_Source item)
  {
    typedef typename _Target::Type * _Target_pointer;

    _Target ptr =  static_cast<_Target_pointer>(item.data());
    return  ptr;
  }

  class CodeModel :  public KDevCodeModel
    {

    public:
      CodeModel( QObject *parent =  0 );
      virtual ~CodeModel();

      template  <class _Target>
      _Target create()
      {
        typedef typename _Target::Type _Target_type;

        _Target result =  _Target_type::create(this);
        return  result;
      }

      void addCodeItem(CodeModelItem item);
      void removeCodeItem(CodeModelItem item);

      void wipeout();

    private:
      CodeModel(const CodeModel &other);
      void operator=(const CodeModel &other);
    };

  class _CodeModelItem :  public KDevCodeItem
    {

    public:
      enum Kind
      {
        /* These are bit-flags resembling inheritance */
        KindMask =  (1 << 0) -  1,

        /* These are for classes that are not inherited from */
        FirstKind =  1 << 0
      };

    public:
      virtual ~_CodeModelItem();

      virtual _CodeModelItem *itemAt(int index) const;

      int kind() const;

      KTextEditor::Cursor startPosition() const;
      void setStartPosition(const KTextEditor::Cursor& cursor);

      KTextEditor::Cursor endPosition() const;
      void setEndPosition(const KTextEditor::Cursor& cursor);

      QTime timestamp() const
        {
          return  QTime();
        }

      inline CodeModel *model() const
        {
          return  _M_model;
        }

      CodeModelItem toItem() const;

    private:
      CodeModel *_M_model;
      int _M_kind;
      int _M_startLine;
      int _M_startColumn;
      int _M_endLine;
      int _M_endColumn;

    public:

    private:

    protected:
      _CodeModelItem(CodeModel *model,  int kind);
      void setKind(int kind);

    private:
      _CodeModelItem(const _CodeModelItem &other);
      void operator=(const _CodeModelItem &other);
    };

  template  <class _Target,  class _Source>
  _Target model_safe_cast(_Source item)
  {
    typedef typename _Target::Type * _Target_pointer;
    typedef typename _Source::Type * _Source_pointer;

    _Source_pointer source =  item.data();

    if  (source &&  source->kind() ==  _Target_pointer(0)->__node_kind)
      {
        _Target ptr(static_cast<_Target_pointer>(source));
        return  ptr;
      }

    return  _Target();
  }

  template  <typename _Target,  typename _Source>
  _Target model_dynamic_cast(_Source item)
  {
    typedef typename _Target::Type * _Target_pointer;
    typedef typename _Source::Type * _Source_pointer;

    _Source_pointer source =  item.data();

    if  (source &&  (source->kind() ==  _Target_pointer(0)->__node_kind
                     ||  (_Target_pointer(0)->__node_kind <=  int(_CodeModelItem::KindMask)
                          &&  ((source->kind() & _Target_pointer(0)->__node_kind)
                               ==  _Target_pointer(0)->__node_kind))))
      {
        _Target ptr(static_cast<_Target_pointer>(source));
        return  ptr;
      }

    return  _Target();
  }


} // end of namespace ruby

#endif


