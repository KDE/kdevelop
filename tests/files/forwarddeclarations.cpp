class Base
{
    /// "type" : { "toString" : "Base::Private" }
    class Private;
    /// "toString" : "Base::Private* d"
    Private *d;

    /// "type" : { "toString" : "Base::Private" }
    struct Private;
    /// "toString" : "Base::Private* d"
    Private *d;
};

class Base::Private {};

struct Base::Private {};
