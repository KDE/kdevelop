#ifndef FLEXIBLETEXT_NOTIFIER
#define FLEXIBLETEXT_NOTIFIER

namespace SumTree {

  ///Override this so you are able to register your class to be notified of changes in a FlexibleText
  ///All of these functions are called by FlexibleText immediately BEFORE the replacement is done
  ///Only one of those notifications is called, even though a replacement may internally be implemented by erasing and inserting.
  ///These are allowed to throw DynamicTextError
  template<class StringType>
  class FlexibleTextNotifier {
    public:
      virtual void notifyFlexibleTextErase( int position, int length ) = 0;
      virtual void notifyFlexibleTextInsert( int position, const StringType& text ) = 0;
      virtual void notifyFlexibleTextReplace( int position, int length, const StringType& replacement ) = 0;
      virtual ~FlexibleTextNotifier(){};
  };
};

#endif
