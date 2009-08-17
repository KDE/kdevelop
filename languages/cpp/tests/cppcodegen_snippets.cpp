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
#include </ClassA.h>
ClassA::ClassA() : i(0), j(0.0)
{
    structVar.i = 0;
    ContainedStruct testStruct;
}

// ==ClassDerived.h==

class ClassBase
{
  public:
    int * publicMethod(int *);
    int publicOverriddenMethod(int, int &);
    int publicData;
  
  protected:
    long * protectedMethod(long *);
    long protectedOverriddenMethod(long, long &);
    long protectedData;
    
  private:
    short * privateMethod(short *);
    short provateOverriddenMethod(short, short &);
    short privateData;
};

class ClassDerived : public ClassBase
{
  public:
    int * publicMethod(int *);
    int publicOverriddenMethod(int, int &);
    int publicData;
    
  protected:
    long * protectedMethod(long *);
    long protectedOverriddenMethod(long, long &);
    long protectedData;
    
  private:
    short * privateMethod(short *);
    short provateOverriddenMethod(short, short &);
    short privateData;
};

//---AbstractClass.h---

class AbstractClass
{
  public:
    virtual ~AbstractClass();
    
    virtual void pureVirtual() = 0;
    virtual const int constPure(const int &) const = 0;
    
    virtual void regularVirtual();
    virtual const int constVirtual(const int &) const;
    
    int data;
};
