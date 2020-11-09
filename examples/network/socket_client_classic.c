#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define buffersize 1024




int main(int argc, char *argv[]){
 
	if(argc != 2) {
		fprintf(stderr, “Usage: %s string\n”, argv[0]);
		exit(EXIT_FAILURE);
	}
 
    int sock = socket(AF_INET, SOCK_STREAM, 0);
  
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));  
    serv_addr.sin_family = AF_INET;  
	
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  
    //inet_pton(AF_INET, “localhost”, &serveraddr.sin_addr);
	
	serv_addr.sin_port = htons(5566); 
    connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
   
    char *message = (char*) calloc(strlen(argv[1])+1, sizeof(char)) ;
    strcpy(message, argv[1]) ;
    send(sock, message, strlen(message)+1, 0) ;

    char *buffer = (char*) calloc(buffersize, sizeof(char)) ;
    read(sock, buffer, buffersize);
    printf("result form server: %s\n", buffer);
   
    close(sock);
    return 0;
}

/* API info
IPV4:
in_addr_t inet_addr(const char *cp); // in_addr_t s_addr = inet_addr("127.0.0.1")
char *inet_ntoa(struct in_addr in);  // char* ip = inet_ntoa(s_addr);

IPV4&6:
int inet_pton(int af, const char *src, void *dst); // inet_pton(AF_INET, "localhost”, &serveraddr.sin_addr);
const char *inet_ntop(int af, const void *src, char *dst, socklen_t size); // inet_ntop(AF_INET, &serveraddr.sin_addr, str, INET_ADDRSTRLEN); INET6_ADDRSTRLEN

*/




