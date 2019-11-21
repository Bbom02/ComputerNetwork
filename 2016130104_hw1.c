#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>

#define SERVER_PORT 47500

int 
main()
{
 struct sockaddr_in sin;
 int s;

 /* localhost */
 sin.sin_addr.s_addr = inet_addr("127.0.0.1");

 /* build address data structure */
 sin.sin_family = AF_INET;
 sin.sin_port=htons(SERVER_PORT);

 /* action open */ 
 s = socket(PF_INET, SOCK_STREAM, 0);
 connect(s, (struct sockaddr *)&sin, sizeof(sin));

 /* send data */
 char data[] = "2016130104";
 send(s, data, strlen(data)+1, 0);
 close(s);
 return 0 ;
}
