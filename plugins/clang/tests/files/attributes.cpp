#define Q_SIGNAL __attribute__((annotate("qt_signal")))
#define Q_SLOT __attribute__((annotate("qt_slot")))

class Base
{
public:
    virtual void slot();
};

class Derived : public Base
{
public:
    /// "isFinal" : false,
    /// "isSignal" : true,
    /// "isSlot" : false
    Q_SIGNAL void signal();

    /// "isFinal" : true,
    /// "isSignal" : false,
    /// "isSlot" : true
    Q_SLOT void slot() override final;
};
