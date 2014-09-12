/***************************************************************************
 *   Copyright 2009 Evgeniy Ivanov <powerfox@kde.ru>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

/* TODO:
- crossplatform sockets? QTcpSocket can't be used, see comments in the code
- check fingerprint using libssh2_hostkey_hash.
- support keyboard-interactive auth.
- kde wallet support
- FIXME: integrate to kdessh from kdeutils? Requires moving kdessh to kdebase, also
it has similar functionality, but I failed to make it work correctly.
*/

#include "main.h"

#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KMessageBox>

#include <QtNetwork/QTcpSocket>

#include <cstring>
#include <string>
#include <memory>
#include <cerrno>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <libssh2.h>



void show_error(const QString &error)
{
        KMessageBox::error(0, error);
}

void shutdown_ssh(LIBSSH2_SESSION *session, const QString &error = "")
{
    if (!error.isEmpty())
        show_error(error);

    libssh2_session_disconnect(session, "See you later, alligator!");
    libssh2_session_free(session);
}

int main (int argc, char *argv[])
{
    KAboutData aboutData(
                         "kssh_client",
                         0,
                         ki18n("Simple KDE ssh client provider"),
                         "0.1",
                         ki18n("Displays a KDE Password Dialog and establishes an ssh connection using libssh. After connecting it executes 'svnserve -t' to provide tunneling."),
                         KAboutData::License_GPL,
                         ki18n("(c) 2009"),
                         ki18n("Use this client instead of standard ssh when using subversion to provide a KDE Password Dialog"
                               " instead of a command-line ssh password prompt."),
                         "http://kde.org",
                         "submit@bugs.kde.org");
    KCmdLineArgs::init( argc, argv, &aboutData );
    
    //according to docs we should be invoked as following:
    //<command> <hostname> svnserve -t
    KCmdLineOptions k_options;
    k_options.add("+hostname", ki18n("Hostname to connect"));
    k_options.add("+subversioncommand", ki18n("Subversion command to call")); //FIXME anywhere always svnserve -t
    k_options.add("t", ki18n("svnserve option"));
    KCmdLineArgs::addCmdLineOptions( k_options );

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    
    KApplication app;

    std::string username;
    std::string host;
    if (args->arg(0).contains('@'))
    {
        username = args->arg(0).split('@')[0].toStdString();
        host = args->arg(0).split('@')[1].toStdString();
    }
    else
        host = args->arg(0).toStdString();
    
    const unsigned int port = 22;
        
    //QTcpSocket makes libssh2_session_startup failed with *EAGAIN
    //QTcpSocket::readReady doesn't help, since socket is used multiple times inside libssh2_session_startup
    int sock;
    struct sockaddr_in addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
    {
        show_error(i18n("Cannot create socket, operation aborted."));
        return 1;
    }
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    struct hostent *he;
    if (!(he = gethostbyname(host.c_str()) ) )
    {
        show_error(i18n("Get host by name failed, operation aborted."));
        return 1;
    }
    addr.sin_addr = *((struct in_addr *)he->h_addr);

    if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("connect");
        show_error(i18n("Cannot connect to host, operation aborted."));
        return 1;
    }

    
    LIBSSH2_SESSION *session;
    if (!(session = libssh2_session_init()) )
    {
        shutdown_ssh(session, i18n("Cannot start an ssh session, operation aborted."));
        return 1;
    }

    if (libssh2_session_startup(session, sock)) {
        shutdown_ssh(session, i18n("SSH connection failed, operation aborted."));
        return 1;
    }
    
    char* auth_methods = 0;
    if (!(auth_methods = libssh2_userauth_list(session, username.c_str(), username.size()+1)))
    {
        shutdown_ssh(session, i18n("Server does not support any authentication methods for the user, operation aborted."));
        return 1;
    }
    kDebug() << "Supported auth methods:" << auth_methods;
    
    unsigned int auth_mask = 0;
    QStringList supported_auth_methods = QString(auth_methods).split(',');
    if (supported_auth_methods.contains("password") )
        auth_mask |= 1;
    if (supported_auth_methods.contains("publickey") )
        auth_mask |= 2;    
    
    AuthForm *form = new AuthForm(QString(username.c_str()), AuthForm::AuthType(auth_mask));
    
    KDialog *dialog = new KDialog();
    dialog->setCaption(i18n("ssh auth"));
    dialog->setButtons(KDialog::Ok);
    dialog->setMainWidget(form);
    dialog->exec();

    username = form->username->text().toStdString();
    const std::string passphrase = form->password_or_passphrase->text().toStdString();    
    const std::string pub_key_filename = form->pub_key->text().toStdString();
    const std::string private_key_filename = form->private_key->text().toStdString();
    
    const uint auth_type = form->select_auth_type->itemData(form->select_auth_type->currentIndex()).toInt();
    int auth;

    switch(auth_type)
    {
        case AuthForm::PASSWORD:
            auth = libssh2_userauth_password(session, username.c_str(), passphrase.c_str());
            break;
        case AuthForm::PUBKEY:
            auth = libssh2_userauth_publickey_fromfile(session, username.c_str(), pub_key_filename.c_str(),
                                                       private_key_filename.c_str(), passphrase.c_str());
            break;
        default:
            //shouldn't happen
            shutdown_ssh(session, i18n("Unsupported authentication type, could possibly be a bug. Operation aborted."));
            return 1;
    }

    if (auth)
    {
        shutdown_ssh(session, i18n("Authentication failed, operation aborted."));
        return 1; 
    }
    
    LIBSSH2_CHANNEL *channel;
    
    if (!(channel = libssh2_channel_open_session(session))) {
        shutdown_ssh(session, i18n("Cannot open ssh channel, operation aborted."));
        return 1; 
    }

    if(libssh2_channel_exec(channel, "svnserve -t") )
    {
        shutdown_ssh(session, i18n("Failed to launch 'svnserve -t', operation aborted."));
        return 1;
    }
    
    char remotebuf[BUFSIZ];
    char localbuf[BUFSIZ];
    int readremote, readlocal;
    
    //stdin shoudn't block, since svn can wait for some data from remote server
    int flags = fcntl(0, F_GETFL, 0);
    if (fcntl(0, F_SETFL, flags | O_NONBLOCK))
    {
        shutdown_ssh(session, i18n("Cannot set non-blocking mode for stdin, operation aborted."));
        return 1;
    }


    while ((readremote = libssh2_channel_read(channel, remotebuf, BUFSIZ)) > 0 ||
        readremote == LIBSSH2_ERROR_EAGAIN) 
    {
        //Don't block after first read, since server can wait svn sent some info.
        libssh2_channel_set_blocking(channel, 0);
        if (readremote > 0)
        {
            int written = 0;
            while ( (written += write(1, remotebuf, readremote)) < readremote) ;
        }
        readlocal = read(0,localbuf,BUFSIZ); //FIXME: Check EAGAIN and other errors?
        if (readlocal > 0)
        {
            libssh2_channel_write(channel, localbuf, readlocal);
        }
    }
    libssh2_channel_close(channel);
    //libssh2_channel_wait_closed(channel); //doubt we need it
    libssh2_channel_free(channel);
    libssh2_session_disconnect(session, "Goodbye");
    libssh2_session_free(session);

    return 0;
}
