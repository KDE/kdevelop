template<typename U> U foo();

template<typename T>
struct bar
{
  template<typename U> U asdf();

  typedef decltype(foo<T>()) type1;
  typedef decltype(asdf<T>()) type2;
  typedef T type3;
};

// should have type int
bar<int>::type1 v1 = 0;
bar<int>::type2 v2 = 0;
