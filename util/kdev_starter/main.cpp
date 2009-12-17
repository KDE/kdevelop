#include <string.h>
#include <iostream>
#include <QtCore/QProcess>
#include <QStringList>
#include <stdlib.h>

/**
 * This is a very simple helper application that allows starting a new kdevplatform-based application instance with a specific
 * session-id that is given on the command-line. This app is used internally for starting up new instances with specific sessions.
 */

int main(int argc, char** argv)
{
  if(argc < 3) {
    std::cerr << "need at least two arguments: app-name and session-id" << std::endl;
    return 1;
  }
  
  setenv("KDEV_SESSION", argv[2], 1);
  
  QStringList args;
  for(int a = 3; a < argc; ++a)
    args << QString(argv[a]);
  
  QProcess process;
  process.setProcessChannelMode(QProcess::ForwardedChannels);
  process.start(QString(argv[1]), args);
  process.waitForFinished(-1);
  return 0;
}
