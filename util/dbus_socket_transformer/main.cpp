/***************************************************************************
 *   Copyright 2011 David Nolden <david.nolden.kdevelop@art-master.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include <stdlib.h>
#include <QString>
#include <QLocalSocket>
#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <fcntl.h>
#include <cerrno>
#include <assert.h>

/**
 * The goal of this utility is transforming the abstract unix-socket which is used by dbus
 * into a TCP socket which can be forwarded to a target machine by ssh tunneling, and then on
 * the target machine back into an abstract unix socket.
 * 
 * This tool basically works similar to the "socat" utility, except that it works properly
 * for this special case. It is merely responsible for the transformation between abstract unix
 * sockets and tcp sockets.
 * */

const bool debug = false;

/**
 * Shuffles all data between the two file-descriptors until one of them fails or reaches EOF.
 * */
void shuffleBetweenStreams(int side1, int side2)
{
    char buffer[500];
    char buffer2[500];
    
    // Set non-blocking mode
    int opts = fcntl(side1,F_GETFL);
    opts |= O_NONBLOCK;
    fcntl(side1, F_SETFL, opts);

    opts = fcntl(side2,F_GETFL);
    opts |= O_NONBLOCK;    
    fcntl(side2, F_SETFL, opts);
    
    while(true)
    {
        int r1 = read(side1, buffer, 500);
        int r2 = read(side2, buffer2, 500);

        if(r1 < -1 || r1 == 0)
        {
            if(debug)
                std::cerr << "stream 1 failed: " << r1 << std::endl;
            return;
        }
        if(r2 < -1 || r2 == 0)
        {
            if(debug)
                std::cerr << "stream 2 failed: " << r2 << std::endl;
            return;
        }
        
        if(r1 > 0)
        {
            if(debug)
                std::cerr << "transferring " << r1 << " from 1 to 2" << std::endl;

            opts = fcntl(side2,F_GETFL);
            opts ^= O_NONBLOCK;    
            fcntl(side2, F_SETFL, opts);
            
            int w2 = send(side2, buffer, r1, MSG_NOSIGNAL);
            
            if(w2 < 0)
            {
                if(debug)
                    std::cerr << "writing to side 2 failed, ending: " << w2 << std::endl;
                return;
            }
            assert(w2 == r1);
            
            opts = fcntl(side2,F_GETFL);
            opts |= O_NONBLOCK;    
            fcntl(side2, F_SETFL, opts);
        }

        if(r2 > 0)
        {
            if(debug)
                std::cerr << "transferring " << r2 << " from 2 to 1" << std::endl;
            opts = fcntl(side1,F_GETFL);
            opts ^= O_NONBLOCK;    
            fcntl(side1, F_SETFL, opts);
            
            int w1 = send(side1, buffer2, r2, MSG_NOSIGNAL);
            
            if(w1 < 0)
            {
                if(debug)
                    std::cerr << "writing to side 1 failed, ending: " << w1 << std::endl;
                return;
            }
            assert(w1 == r2);
            
            opts = fcntl(side1,F_GETFL);
            opts |= O_NONBLOCK;    
            fcntl(side1, F_SETFL, opts);
        }
        usleep(1000);
    }
}

int main(int argc, char** argv)
{
    int serverfd;
    
    if(argc < 2)
    {
        std::cerr << "need arguments:" << std::endl;
        std::cerr << "[port]                    -   Open a server on this TCP port and forward connections to the local DBUS session"
                     "                              (the DBUS_SESSION_BUS_ADDRESS environment variable must be set)";
        std::cerr << "[port]  [fake dbus path]  -   Open a server on the fake dbus path and forward connections to the given local TCP port";
        std::cerr << ""
                     "The last argument may be the --bind-only option, in which case the application only tries to"
                     "open the server, but does not wait for clients to connect. This is useful to test whether the"
                     "server port/path is available.";
        return 10;
    }
    
    bool waitForClients = true;
    
    if(QString(argv[argc-1]) == "--bind-only")
    {
        waitForClients = false;
        argc -= 1;
    }

    QString dbusAddress(getenv("DBUS_SESSION_BUS_ADDRESS"));

    QString path;
    
    if(argc == 2)
    {
        if(waitForClients)
            std::cout << "forwarding from the local TCP port " << argv[1] << " to the local DBUS session at " << dbusAddress.toLocal8Bit().data() << std::endl;
        
        if(dbusAddress.isEmpty())
        {
            std::cerr << "The DBUS_SESSION_BUS_ADDRESS environment variable is not set" << std::endl;
            return 1;
        }
        
        // Open a TCP server
        
        QString abstractPrefix("unix:abstract=");
        
        if(!dbusAddress.startsWith(abstractPrefix))
        {
            std::cerr << "DBUS_SESSION_BUS_ADDRESS does not seem to use an abstract unix domain socket as expected" << std::endl;
            return 2;
        }
        
        path = dbusAddress.mid(abstractPrefix.size());
        if(path.contains(",guid="))
            path.truncate(path.indexOf(",guid="));
        
        // Mark it as an abstract unix domain socket
        path = path;
        
        serverfd = socket(AF_INET, SOCK_STREAM, 0);
        
        if (serverfd < 0) 
        {
            if(waitForClients)
                std::cerr << "ERROR opening server socket" << std::endl;
            return 3;
        }
        
        int portno = atoi(argv[1]);
        
        sockaddr_in server_addr;
        
        bzero((char *) &server_addr, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(portno);
        
        if (bind(serverfd, (struct sockaddr *) &server_addr,
                sizeof(server_addr)) < 0) 
        {
            if(waitForClients)
                std::cerr << "ERROR opening the server" << std::endl;
            return 7;
        }
    }else if(argc == 3)
    {
        if(waitForClients)
            std::cout << "forwarding from the local abstract unix domain socket " << argv[2] << " to the local TCP port " << argv[1] << std::endl;
        // Open a unix domain socket server
        serverfd = socket(AF_UNIX, SOCK_STREAM, 0);
        
        if (serverfd < 0) 
        {
            if(waitForClients)
                std::cerr << "ERROR opening server socket" << std::endl;
            return 3;
        }

        path = QString(argv[2]);

        sockaddr_un serv_addr;
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sun_family = AF_UNIX;
        serv_addr.sun_path[0] = '\0'; // Mark as an abstract socket
        strcpy(serv_addr.sun_path+1, path.toLocal8Bit().data());
        
        if(debug)
            std::cout << "opening at " << path.toLocal8Bit().data() << std::endl;
        
        if (bind(serverfd,(sockaddr *) &serv_addr, sizeof (serv_addr.sun_family) + 1 + path.toLocal8Bit().length()) < 0)
        {
            if(waitForClients)
                std::cerr << "ERROR opening the server" << std::endl;
            return 7;
        }
    }else{
        std::cerr << "Wrong arguments";
        return 1;
    }
    
    listen(serverfd, 10);

    while(waitForClients)
    {
        if(debug)
            std::cerr << "waiting for client" << std::endl;
        sockaddr_in cli_addr;
        socklen_t clilen = sizeof(cli_addr);
        int connectedclientsockfd = accept(serverfd, 
                    (struct sockaddr *) &cli_addr, 
                    &clilen);
        
        if(connectedclientsockfd < 0)
        {
            std::cerr << "ERROR on accept" << std::endl;
            return 8;
        }
        
        if(debug)
            std::cerr << "got client" << std::endl;

        int sockfd;
        
        int addrSize;
        sockaddr* useAddr = 0;
        sockaddr_un serv_addru;
        sockaddr_in serv_addrin;
        
        if(argc == 2)
        {
            sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
            if (sockfd < 0) 
            {
                std::cerr << "ERROR opening socket" << std::endl;
                return 3;
            }
            bzero((char *) &serv_addru, sizeof(serv_addru));
            serv_addru.sun_family = AF_UNIX;
            serv_addru.sun_path[0] = '\0'; // Mark as an abstract socket
            strcpy(serv_addru.sun_path+1, path.toLocal8Bit().data());
            addrSize = sizeof (serv_addru.sun_family) + 1 + path.toLocal8Bit().length();
            useAddr = (sockaddr*)&serv_addru;
            
            if(debug)
                std::cout << "connecting to " << path.toLocal8Bit().data() << std::endl;
        }else{
            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0) 
            {
                std::cerr << "ERROR opening socket" << std::endl;
                return 3;
            }
            int port = atoi(argv[1]);
            hostent *server = gethostbyname("localhost");
            if(server == NULL)
            {
                std::cerr << "failed to get server" << std::endl;
                return 5;
            }
            bzero((char *) &serv_addrin, sizeof(serv_addrin));
            serv_addrin.sin_family = AF_INET;
            serv_addrin.sin_addr.s_addr = INADDR_ANY;
            serv_addrin.sin_port = htons(port);
            bcopy((char *)server->h_addr,  (char *)&serv_addrin.sin_addr.s_addr, server->h_length);
            addrSize = sizeof (serv_addrin);
            useAddr = (sockaddr*)&serv_addrin;
            
            if(debug)
                std::cout << "connecting to port " << port << std::endl;
        }
        
        if (connect(sockfd, useAddr, addrSize) < 0)
        {
            int res = errno;
            if(res == ECONNREFUSED)
                std::cerr << "ERROR while connecting: connection refused" << std::endl;
            else if(res == ENOENT)
                std::cerr << "ERROR while connecting: no such file or directory" << std::endl;
            else
                std::cerr << "ERROR while connecting" << std::endl;
            return 5;
        }

        shuffleBetweenStreams(connectedclientsockfd, sockfd);
        close(sockfd);
        close(connectedclientsockfd);
    }
    return 0;
}
