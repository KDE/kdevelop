/*
 * This is just a test.
 * You may do with it as you please.
 */

#ifndef CLASSNAME_H
#define CLASSNAME_H

class ClassName : public QObject
{
public:
    void doSomething(double howMuch, bool doSomethingElse);
    int getSomeOtherNumber();

    QString name;
    int number;
    SomeCustomType data;
};

#endif // CLASSNAME_H
