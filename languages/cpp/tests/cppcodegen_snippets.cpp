// ==ClassA.h==

class ClassA
{
  public:
	ClassA();
  
  private:
	int i;
	float f, j;
	
	struct ContainedStruct
	{
	  int i;
	  ClassA * p;
	} structVar;
};

// ==ClassA.cpp==

ClassA::ClassA() : i(0), j(0.0)
{
    structVar.i = 0; 
}

// ==ClassBase==

class ClassBase
