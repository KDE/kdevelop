#include <QGuiApplication>
#include <QtQuick/QQuickView>
#include <QtQml/QQmlEngine>

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv, 5);

    QQuickView viewer;
    viewer.setSource(QUrl("src/qml/main.qml"));
    viewer.show();

    QObject::connect(viewer.engine(), SIGNAL(quit()), qApp, SLOT(quit()));
    return app.exec();
}
