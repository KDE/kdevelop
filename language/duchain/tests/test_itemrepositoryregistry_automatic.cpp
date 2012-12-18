#include <QtTest/QtTest>
#include <language/duchain/repositories/itemrepositoryregistry.h>
#include <shell/sessioncontroller.h>

#include <tests/testcore.h>
#include <tests/autotestshell.h>

class TestItemRepositoryRegistryAutomaticDeletion : public QObject {
  Q_OBJECT
    void initCore(const QString& sessionName = QString()) {
      KDevelop::TestCore* core = new KDevelop::TestCore();
      core->initialize(KDevelop::Core::NoUi, sessionName);
    }

    void destroyCore() {
      KDevelop::TestCore::shutdown();
    }

  private slots:
    void initTestCase() {
      KDevelop::AutoTestShell::init();
    }

    void testTemporarySessionDeletion() {
      // Create and shutdown a TestCore. The session created by it is temporary
      // and thus shall be deleted upon core shutdown together with its
      // item-repository directory.

      {
        initCore();

        // The session created by TestCore shall be temporary
        QVERIFY(KDevelop::Core::self()->activeSession()->isTemporary());

        // The repository shall exist
        QString repositoryPath = KDevelop::globalItemRepositoryRegistry().path();
        QVERIFY(QFile::exists(repositoryPath));

        // The repository shall die with the core shutdown
        destroyCore();
        QVERIFY(!QFile::exists(repositoryPath));
      }
    }

    void cleanupTestCase() {
    }
};

#include "test_itemrepositorymanagement_automatic.moc"

QTEST_MAIN(TestItemRepositoryRegistryAutomaticDeletion)
