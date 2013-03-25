#include "parsesession.h"
#include "typebuilder.h"

using namespace KDevelop;

TypeBuilder::TypeBuilder()
: TypeBuilderBase()
{

}

bool TypeBuilder::VisitTypeLoc(clang::TypeLoc TL)
{
    /*TL.getTypePtr()->getAsCXXRecordDecl()->
    qWarning() << rangeFromLocations(TL.getBeginLoc(), TL.getEndLoc(), m_session->sourceManager()) << TL.getType().getAsString().c_str();
    if (TL.getType().isCanonical() && TL.getTypePtr()->isClassType())
    {
        qWarning() << "It's canonical";
        setLastType(StructureType::Ptr(new StructureType()).cast<AbstractType>());
    }*/
    return true;
}
