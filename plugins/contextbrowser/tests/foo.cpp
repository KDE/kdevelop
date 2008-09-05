#include "foo.h"
#include "bar.h"

void Foo::ctx1()
{
	Foo f;
	Bar b;
}

void ctx2()
{
	Foo f;
	Bar b;
}

BAR

