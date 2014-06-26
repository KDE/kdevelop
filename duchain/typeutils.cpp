#include "typeutils.h"

#include <language/duchain/types/indexedtype.h>

using namespace KDevelop;

IndexedType TypeUtils::removeConstModifier(const IndexedType& indexedType)
{
    AbstractType::Ptr type = indexedType.abstractType();
    removeConstModifier(type);
    return type->indexed();
}

void TypeUtils::removeConstModifier(AbstractType::Ptr& type)
{
  if(type && type->modifiers() & AbstractType::ConstModifier)
  {
    type->setModifiers(type->modifiers() & (~AbstractType::ConstModifier));
  }
}
