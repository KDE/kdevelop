// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#ifndef ruby_CODEMODEL_CHAMELEON_H
#define ruby_CODEMODEL_CHAMELEON_H

#include "ruby_codemodel.h"

namespace ruby
  {

  template  <class T>

  struct Nullable
    {
      bool isNull;
      T item;
      Nullable() :  isNull(true)
      {}

      Nullable(T _item) :  isNull(false),  item(_item)
      {}

    }

  ;

  /** Don't instantiate this class directly.
   * Instead, use the wrapper named ModelItemChameleon
   * which provides shared pointer abilities.
   */

  class _ModelItemChameleon :  public KDevShared
    {
      CodeModelItem _M_item;

    public:
      CodeModelItem item()
      {
        return  _M_item;
      }

      _ModelItemChameleon(CodeModelItem item) :  _M_item(item)
      {}

      ~_ModelItemChameleon()
      {}

    private:
      _ModelItemChameleon(const _ModelItemChameleon &other);
      void operator=(const _ModelItemChameleon &other);
    };

  class ModelItemChameleon :  public KDevSharedPtr<_ModelItemChameleon>
    {

    public:
      ModelItemChameleon(CodeModelItem item)
          :  KDevSharedPtr<_ModelItemChameleon>(new _ModelItemChameleon(item))
      {}

    }

  ;


} // end of namespace ruby

#endif


