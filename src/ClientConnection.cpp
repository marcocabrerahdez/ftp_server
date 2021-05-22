//****************************************************************************
//                         REDES Y SISTEMAS DISTRIBUIDOS
//                      
//                     2º de grado de Ingeniería Informática
//                       
//              This class processes an FTP transaction.
// 
//****************************************************************************



#include <cstring>
#include <cstdarg>
#include <cstdio>
#include <cerrno>
#include <netdb.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <locale.h>
#include <langinfo.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/stat.h> 
#include <iostream>
#include <dirent.h>

#include "common.h"

#include "ClientConnection.h"




ClientConnection::ClientConnection(int s) {
    int sock = (int)(s);
  
    char buffer[MAX_BUFF];

    control_socket = s;
    // Check the Linux man pages to know what fdopen does.
    fd = fdopen(s, "a+");
    if (fd == NULL){
	std::cout << "Connection closed" << std::endl;

	fclose(fd);
	close(control_socket);
	ok = false;
	return ;
    }
    
    ok = true;
    data_socket = -1;
    parar = false;
   
  
  
};


ClientConnection::~ClientConnection() {
 	fclose(fd);
	close(control_socket); 
  
}


int connect_TCP( uint32_t address,  uint16_t  port) {
     // Implement your code to define a socket here
     // Create sockaddr_in struct
    struct sockaddr_in sin; 
    int s;

    // Fill out the sockaddr_in struct
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = address;
    sin.sin_port = htons(port);

    // Create socket and proccess error
    s = socket(AF_INET, SOCK_STREAM, 0);
    if(s < 0)
       errexit("Can't create socket: %s\n", strerror(errno));
    
    // Create a connection to the specified foreign association
    if(connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
       errexit("No se puede conectar con %s: %s\n", address, strerror(errno));

    return s; // You must return the socket descriptor.
}






void ClientConnection::stop() {
    close(data_socket);
    close(control_socket);
    parar = true;
  
}





    
#define COMMAND(cmd) strcmp(command, cmd)==0

// This method processes the requests.
// Here you should implement the actions related to the FTP commands.
// See the example for the USER command.
// If you think that you have to add other commands feel free to do so. You 
// are allowed to add auxiliary methods if necessary.

void ClientConnection::WaitForRequests() {
    if (!ok) {
	 return;
    }
    
    fprintf(fd, "220 Service ready\n");
  
    while(!parar) {
        fscanf(fd, "%s", command);
        if (COMMAND("USER")) {
            fscanf(fd, "%s", arg);
            fprintf(fd, "331 User name ok, need password\n");
        }

        else if (COMMAND("PWD")) {
            char buf[MAX_BUFF];

            if (getcwd(buf, MAX_BUFF))
                fprintf(fd, "257 %s current working directory\n", buf);
        }

        else if (COMMAND("PASS")) {
            fscanf(fd, "%s", arg);
            if(strcmp(arg,"1234") == 0){
                fprintf(fd, "230 User logged in\n");
            }
            else{
                fprintf(fd, "530 Not logged in.\n");
                parar = true;
            }
        
        }

        // HOST-PORT specification for the data port to be used in data connection
        else if (COMMAND("PORT")) {
        // To be implemented by students
            // Variables
            pasive = false; 
            int addr[4], port_[2];
            
            // Concatenation of a 32-bit internet host address and a 16-bit
            fscanf(fd, "%d, %d, %d, %d, %d, %d", &addr[0], &addr[1], &addr[2],
                &addr[3], &port_[0], &port_[1]);

            uint32_t address = addr[0] | addr[1] << 8 | addr[2] << 16 | addr[3] << 24;
            uint16_t port = port_[0] << 8 | port_[1];

            // Initialize client connection
            data_socket = connect_TCP(address, port);
            
            // Response
            fprintf(fd, "200 OK.\n");
        }

        // Requests the server-DTP to "listen" on a data port
        else if (COMMAND("PASV")) {
        // To be implemented by students
            pasive = true;

            // Create sockaddr_in structure and the socket
            struct sockaddr_in sin, sock_addr;
            socklen_t sock_addr_len = sizeof(sock_addr);
            int s = socket(AF_INET, SOCK_STREAM, 0);

            int sv_addr = 16777343; // Only connects from localhost

            // Fill out the sockaddr_in struct
            memset(&sin, 0, sizeof(sin));
            sin.sin_family = AF_INET;
            sin.sin_addr.s_addr = sv_addr;
            sin.sin_port = 0;

            if (s < 0) {
                errexit("Can't create socket: %s\n", strerror(errno));
            }

            // Bind the port
            if (bind(s, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
                errexit("Could not bind the port: %s\n",
                        strerror(errno));
            }

            // Listen function mark a connection-mode socket, 
            // specified by the socket argument, as accepting connection.
            if (listen(s, 5) < 0) {
                errexit("Fallo en listen: %s\n", strerror(errno));
            }

            // Retrieves the current name for the specified socket descriptor in name
            getsockname(s, (struct sockaddr *)&sock_addr, &sock_addr_len);

            // Response
            fprintf(fd, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).\n",
                    (unsigned int)(sv_addr & 0xff),
                    (unsigned int)((sv_addr >> 8) & 0xff),
                    (unsigned int)((sv_addr >> 16) & 0xff),
                    (unsigned int)((sv_addr >> 24) & 0xff),
                    (unsigned int)(sock_addr.sin_port & 0xff),
                    (unsigned int)(sock_addr.sin_port >> 8));

            data_socket = s;
        }

        // Causes the server-DTP to accept the data transferred via the data connection
        else if (COMMAND("STOR") ) {
            // To be implemented by students
            fscanf(fd, "%s", arg);

            // Open file descriptor
            FILE *file = fopen(arg, "wb");

            // Store the data as a file at the server site if exits at the server site
            if (!file) {
                fprintf(fd, "450 Requested file action not taken. File unavailable.\n");
                close(data_socket);
            } else {
                fprintf(fd, "150 File status okay; opening data connection.\n");
                fflush(fd);
                char buf[MAX_BUFF];
                size_t receive;

                do {
                    receive = recv(data_socket, buf, MAX_BUFF, 0); // Read incoming data on connection-oriented sockets
                    fwrite(buf,  1, receive, file); // Writes count number of objects, each of size bytes to the given output stream
                } while (receive == MAX_BUFF);

                // Response and closing con
                fprintf(fd, "226  Closing data connection.\n");
                fclose(file);
                close(data_socket);
            }
        }

        // Causes the server-DTP to transfer a copy of the
        // file, specified in the pathname, to the server- or user-DTP
        // at the other end of the data connection
        else if (COMMAND("RETR")) {
        // To be implemented by students
            // Process arguments and open file descriptor
            fscanf(fd, "%s", arg);
            FILE *file = fopen(arg, "rb");

            if (!file) {
                fprintf(fd, "450 Requested file action not taken. File unavailable.\n");
                close(data_socket);
            } else {
                fprintf(fd, "150 File status okay - About to open data connection.\n");

                struct sockaddr_in s_addr;
                socklen_t s_addr_len = sizeof(s_addr);
                char buf[MAX_BUFF];
                size_t receive;

                do {
                    receive = fread(buf, 1, MAX_BUFF, file); // Reads the block of data from the stream
                    send(data_socket, buf, receive, 0); // Initiate transmission of a message from the specified socket to its peer
                } while (receive == MAX_BUFF);

                fprintf(fd, "226  Closing data connection.\n");
                fclose(file);
                close(data_socket);
            }
        }

        // Causes a list to be sent from the server to the passive DTP
        else if (COMMAND("LIST")) {
            // To be implemented by students	
            fprintf(fd, "125 Data connection already open; transfer starting.\n");
            fflush(fd);

            // Create sockaddr_in structure and the socket
            struct sockaddr_in s_addr;
            socklen_t s_addr_len = sizeof(s_addr);
            char buf[MAX_BUFF];

            // Ls command
            std::string content = "";
            std::string ls = "ls";
            ls.append(" 2>&1"); // Appends characters in the end of string

            // Execute the command specified by the string command
            FILE *file = popen(ls.c_str(), "r");

            if (!file) {
                fprintf(fd, "450 Requested file action not taken. File unavailable.\n");
                close(data_socket); 
            } else {
                if (pasive)
                    data_socket = accept(data_socket, (struct sockaddr *)&s_addr, &s_addr_len); // Accept a connection request from a client
                
                while (!feof(file)) {
                    if (fgets(buf, MAX_BUFF, file) != NULL)
                        content.append(buf);
                }

                // Initiate transmission of a message from the specified socket to its peer
                send(data_socket, content.c_str(), content.size(), 0);

                fprintf(fd, "250 Closing data connection. Requested file action successful.\n");
                pclose(file);
                close(data_socket);
            }
        }
        
        else if (COMMAND("SYST")) {
            fprintf(fd, "215 UNIX Type: L8.\n");   
        }

        else if (COMMAND("TYPE")) {
        fscanf(fd, "%s", arg);
        fprintf(fd, "200 OK\n");   
        }
        
        else if (COMMAND("QUIT")) {
            fprintf(fd, "221 Service closing control connection. Logged out if appropriate.\n");
            close(data_socket);	
            parar=true;
            break;
        }
    
        else  {
            fprintf(fd, "502 Command not implemented.\n"); fflush(fd);
            printf("Comando : %s %s\n", command, arg);
            printf("Error interno del servidor\n");
        
        }
        
    }
        
        fclose(fd);

        
        return;
  
};
