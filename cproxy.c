
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#ifdef _WIN64
#include <winsock.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#endif


// static bool recv_data(int sock, char* data, int num_bytes) {
//     int bytes_recv;
//     do {
//         bytes_recv = recv(sock, data, num_bytes, 0);
//         if (bytes_recv <= 0) {
//             return true;
//         } else {
//             data += bytes_recv;
//             num_bytes -= bytes_recv;
//         }
//     } while(num_bytes > 0);
//     return false;
// }

/*
 * Takes in a socket, some data, and the length of the data and ensures that
 * the entire chunk of data is transfered
 */
static void send_data(int sock, char* data, int num_bytes) {
    int bytes_sent;
    do {
        bytes_sent = send(sock, data, num_bytes, 0);
        if (bytes_sent == -1) {
            perror("client failed recv data:");
            exit(1);
        } else {
            data += bytes_sent;
            num_bytes -= bytes_sent;
        }
    } while(num_bytes > 0);
}
/* 
 * Awaits a connection from cprocy when connected conncts to telnet deamon
 */
void cproxy(int port, char* ipText , char* portText) {
    char buff[1024];
    char buff2[1024];
    int MAX_LEN = 1024;
    int acc, b, localSock;
    struct sockaddr_in sproxyAddr; // address to connect to
    struct sockaddr_in cproxyAddr, telnetAddr;

    int telnetSock = socket(PF_INET, SOCK_STREAM, 0);
    cproxyAddr.sin_family = AF_INET;
    cproxyAddr.sin_addr.s_addr = INADDR_ANY; // htonl INADDR_ANY ;
    cproxyAddr.sin_port = htons(port);// added local to hold spot 
    if (telnetSock < 0){
        fprintf(stderr,"Unable to create socket");
        exit(1);
    }
    if (bind(telnetSock, (struct sockaddr*)&cproxyAddr, sizeof(cproxyAddr)) < 0){
        fprintf(stderr,"Unable to Bind");
        exit(1);
    }
    if (listen(telnetSock, 5) < 0){
        fprintf(stderr,"Unable to Listen");
        exit(1);
    }
    socklen_t telnetLen;
    telnetLen  = sizeof(telnetAddr);
    int telnetCon = accept(telnetSock, (struct sockaddr *)&telnetAddr, &telnetLen);

    // Connect to sproxy
    int sproxySock = socket(PF_INET, SOCK_STREAM, 0);
    if (sproxySock == -1) {
        perror("client failed creating socket");
        exit(1);
    }
    sproxyAddr.sin_family = AF_INET;
    sproxyAddr.sin_port = htons(atoi(portText));
    inet_pton(AF_INET, ipText, &sproxyAddr.sin_addr);
    if (connect(sproxySock, (struct sockaddr*)&sproxyAddr, sizeof(sproxyAddr)) == -1) {
        perror("client failed connecting socket");
        exit(1);
    }

    //accept the conection to telnet
    while(1) {
        if(acc < 0){
            fprintf(stderr,"Unable to accept connection");
            exit(1);
        }
/////////////////////////////////////////////////////////////////////////////////////////////////////telnet daemon// 
        int rest = 1;
        while(rest){
            int n , rv;
            struct timeval tv;
            fd_set readfds;

            FD_SET(acc, &readfds);
            FD_SET(sproxySock, &readfds);
            if(acc > sproxySock) n = acc + 1;
            else n = sproxySock +1;

            tv.tv_sec = 10;
            tv.tv_usec = 500000;
            
            rv = select(n, &readfds, NULL, NULL, &tv);
            if(rv < 0){
                fprintf(stderr,"Error in select");
                exit(1);
            }
            int rev, rev2;
            if(FD_ISSET(acc, &readfds)){
                rev = recv(acc, buff, MAX_LEN,0);
                if(rev <= 0){
                    break;
                }
                send_data(sproxySock, buff, rev);
            }
            if (FD_ISSET(sproxySock, &readfds)){
                rev2 = recv(sproxySock, buff2, MAX_LEN,0);
                if (rev <= 0){
                    break;
                }
                send_data(acc, buff2, rev2);
            }
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////  
int main(int argc, char* argv[]) {
    if(argc == 4){
       cproxy(atoi(argv[1]),argv[2],argv[3]);
    }else{
       fprintf(stderr,"Error missing sport arg");
    }
    return 0;
}