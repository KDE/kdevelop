#include <QDate>
int main()
{
    QDate d(2010, 1, 20);
    Q_UNUSED(d.toString()); // prevent compiler optimizing away the unused value
    return 0;
}
