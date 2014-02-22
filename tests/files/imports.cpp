
class Base
{
    /// "definition" : { "internalContext" : { "importedParents" : { "0" : { "owner" : { "identifier" : "Base" } } } } }
    int member();

    /// "internalContext" : { "importedParents" : { "0" : { "owner" : { "identifier" : "Base" } } } }
    int member2()
    {
        return field;
    }

    int field;
};

int Base::member()
{
  return field;
}

/// "internalContext" : { "importedParents" : { "0" : { "owner" : { "identifier" : "Base" } } } }
class Derived : public Base
{
    /// "definition" : { "internalContext" : { "importedParents" : { "0" : { "owner" : { "identifier" : "Derived" } } } } }
    int derivedMember();
};

int Derived::derivedMember()
{
    return 0;
}
