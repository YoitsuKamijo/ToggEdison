#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <time.h>

int main(int argc, char *argv[])
{
    int sockfd = 0, n = 0;
    char sendBuff[1025];
    char recvBuff[1024];
    struct sockaddr_in serv_addr;
    time_t ticks;  

    if(argc != 2)
    {
        printf("\n Usage: %s <ip of server> \n",argv[0]);
        return 1;
    } 

    memset(sendBuff, '0', sizeof(sendBuff)); 
    memset(recvBuff, '0',sizeof(recvBuff));

    memset(&serv_addr, '0', sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5000); 

    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return 1;
    } 
    while (1)
    {
        if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            printf("\n Error : Could not create socket \n");
            continue;
        } 

        if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            printf("\n Error : Connect Failed \n");
            continue;
        }
        ticks = time(NULL);	
        snprintf(sendBuff, sizeof(sendBuff), "%.24s\r\n", ctime(&ticks));
        write(sockfd, sendBuff, strlen(sendBuff));
        close(sockfd);
	usleep(350 * 1000);

    } 

    if(n < 0)
    {
        printf("\n Read error \n");
    } 
    return 0;
}
