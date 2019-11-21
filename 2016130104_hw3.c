/*before running the code: gcc -o openfilename 2016130104c. libsha1.a*/
/*libsha1.a must be inside the same folder with 2016130104.c*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>

#define FLAG_HELLO               ((unsigned char)(0x01 << 7))
#define FLAG_INSTRUCTION         ((unsigned char)(0x01 << 6))
#define FLAG_RESPONSE            ((unsigned char)(0x01 << 5))
#define FLAG_TERMINATE           ((unsigned char)(0x01 << 4))

#define OP_ECHO                  ((unsigned char)(0x00))
#define OP_PUSH                  ((unsigned char)(0x03))
#define OP_DIGEST                ((unsigned char)(0x04))

#define SERVER_PORT 47500

struct hw_packet {
  unsigned char flag;      //HIRT-4bits, reserved-4bits
  unsigned char operation; //8-bits operation
  unsigned short data_len; // 16bits (2 bytes) data length
  unsigned int seq_num;    // 32 bits (4 bytes) sequence number
  char data[1024];         // optional data
};

int main(int argc, char** argv) {
    
  printf("*** starting ***\n\n");

  struct sockaddr_in serv_addr;
  int client_sockfd;
  
  /*localhost*/
  serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  
  
  /*build address for data structure*/  
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port=htons(SERVER_PORT);

  /*connects to server*/
  client_sockfd = socket(PF_INET, SOCK_STREAM, 0);
  connect(client_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

  /*contain hello message with student id into the packet*/
  unsigned int value;
  struct hw_packet buf_struct;
  buf_struct.flag = FLAG_HELLO;
  buf_struct.operation = OP_ECHO;
  buf_struct.data_len = 4;
  buf_struct.seq_num = 0;
  value = 2016130104;

  /*array for store segmented data*/
  char data[12288];

  /*convert unsigned integer type to 4-bytes character type data*/
  memcpy(buf_struct.data, &value, sizeof(unsigned int));

  /*send hello message which contains my student id to server*/
  printf("sending first hello msg...\n");  
  send(client_sockfd, &buf_struct, sizeof(struct hw_packet), 0);

  /*receive hello message from server */
  struct hw_packet buf_struct_rcv;
  
  if (recv(client_sockfd, &buf_struct, sizeof(struct hw_packet), 0) != 0) {
    printf("received hello message from the server!\n");
  }

  /*respond to each instruction from server*/
  printf("waiting for the first instruction message...\n");

  recv(client_sockfd, &buf_struct_rcv, sizeof(struct hw_packet), 0);
  
  while (buf_struct_rcv.flag == FLAG_INSTRUCTION) {

    /*if server sent PUSH instruction*/

    if (buf_struct_rcv.operation == OP_PUSH) {

      struct hw_packet buf_struct;
      buf_struct.flag = FLAG_RESPONSE;
      buf_struct.operation = buf_struct_rcv.operation;
      buf_struct.data_len = 0;
      buf_struct.seq_num = 0;

      /*create new variable received_data to prevent overlapping received data*/
      char received_data[1024];

      /*initialize received_data*/
      memset(received_data, '\0', sizeof(received_data));  

      /*client should store the received data for future digest calculation*/
      memcpy(received_data, &buf_struct_rcv.data, sizeof(buf_struct_rcv.data));       
      strcat(data, received_data);

      /*data field must be empty*/
      strcpy(buf_struct.data, "\0");

      printf("received push instruction!!\n");
      printf("received seq_num : %u\n", buf_struct_rcv.seq_num);
      printf("received data_len: %hd\n", buf_struct_rcv.data_len);
      printf("saved bytes from idex %u to %u\n", buf_struct_rcv.seq_num, buf_struct_rcv.seq_num + buf_struct_rcv.data_len - 1);
      printf("saved byte stream (character representation) : %s\n", buf_struct_rcv.data);
      printf("current file size is : %u!\n\n", buf_struct_rcv.seq_num + buf_struct_rcv.data_len);

      /*prevent overlapped data from server into buf_struct_rcv.data*/
      memset(buf_struct_rcv.data, '\0', sizeof(buf_struct_rcv.data));

      send(client_sockfd, &buf_struct, sizeof(struct hw_packet), 0);

    }

    /*if server sent digest instruction message*/

    if (buf_struct_rcv.operation == OP_DIGEST) {
   
      struct hw_packet buf_struct;
      buf_struct.flag = FLAG_RESPONSE;
      buf_struct.operation = OP_DIGEST;
      buf_struct.data_len = 20;
      buf_struct.seq_num = 0;
 
      size_t length = strlen(data);
      unsigned char message_digest[20];

      /*client calculate SHA-1 output of data sent by series of push instruction*/
      SHA1(message_digest, data, length);

      printf("received digest instruction!!\n");
      printf("********** calculated digest **********\n");
      printf("%s\n", message_digest);
      printf("***************************************\n");

      memcpy(buf_struct.data, &message_digest, sizeof(message_digest));

      /*prevent overlapped data from server into buf_struct_rcv.data*/
      memset(buf_struct_rcv.data, '\0', sizeof(buf_struct_rcv.data));

      send(client_sockfd, &buf_struct, sizeof(struct hw_packet), 0);
      printf("sent digest message to server!\n\n");

    }   
  
    recv(client_sockfd, &buf_struct_rcv, sizeof(struct hw_packet), 0);

  }

  /*if server send a terminate message*/
  printf("received terminate msg! terminating...\n");

  close(client_sockfd);
  return 0;
}
