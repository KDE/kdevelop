/*
 * This is just a test.
 * You may do with it as you please.
 */

#ifndef KDEVPLATFORM_CLASSNAME_H
#define KDEVPLATFORM_CLASSNAME_H

class ClassName : public QObject
{
public:
    void doSomething(double howMuch, bool doSomethingElse);
    int getSomeOtherNumber();

    QString name;
    int number;
    SomeCustomType data;
};

#endif // KDEVPLATFORM_CLASSNAME_H
