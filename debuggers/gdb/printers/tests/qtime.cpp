#include <QTime>
int main()
{
    QTime t(15, 30, 10, 123);
    Q_UNUSED(t.toString()); // prevent compiler optimizing away the unused object
    return 0;
}
