#include <QtTest/QtTest>
#include <serialization/itemrepositoryregistry.h>
#include <shell/sessioncontroller.h>

#include <tests/testcore.h>
#include <tests/autotestshell.h>

class TestItemRepositoryRegistryDeferredDeletion : public QObject {
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

    void testDeferredDeletion() {
      // Create and shutdown a TestCore, giving the session a custom name
      // so it will stay (won't be temporary) and request the session deletion
      // from the same core instance.

      static const char* sessionName = "test-itemrepositoryregistry-deferreddeletion";
      QString repositoryPath;

      {
        initCore(sessionName);

        // The session with a custom name shall not be temporary
        QVERIFY(!KDevelop::Core::self()->activeSession()->isTemporary());

        // The repository shall exist
        repositoryPath = KDevelop::globalItemRepositoryRegistry().path();
        QVERIFY(QFile::exists(repositoryPath));

        // The repository shall survive session deletion request
        KDevelop::Core::self()->sessionController()->deleteSession(KDevelop::Core::self()->sessionController()->activeSessionLock());
        QVERIFY(QFile::exists(repositoryPath));

        // The repository shall die together with the core shutdown
        destroyCore();
        QVERIFY(!QFile::exists(repositoryPath));
      }
    }

    void cleanupTestCase() {
    }
};

#include "test_itemrepositoryregistry_deferred.moc"

QTEST_MAIN(TestItemRepositoryRegistryDeferredDeletion)
