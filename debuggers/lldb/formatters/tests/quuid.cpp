#include <QUuid>
int main() {
    QUuid id("{9ec3b70b-d105-42bf-b3b4-656e44d2e223}");
    Q_UNUSED(id.toString()); // prevent compiler optimizing away unused object
    return 0;
}
