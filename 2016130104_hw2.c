#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#define FLAG_HELLO               ((unsigned char)(0x01 << 7))
#define FLAG_INSTRUCTION         ((unsigned char)(0x01 << 6))
#define FLAG_RESPONSE            ((unsigned char)(0x01 << 5))
#define FLAG_TERMINATE           ((unsigned char)(0x01 << 4))

#define OP_ECHO                  ((unsigned char)(0x00))
#define OP_INCREMENT             ((unsigned char)(0x01))
#define OP_DECREMENT             ((unsigned char)(0x02))

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
  
  while (buf_struct_rcv.flag != FLAG_TERMINATE) {

    /*if server sent ECHO request instruction*/
    if (buf_struct_rcv.operation == OP_ECHO) {
   
      struct hw_packet buf_struct;
      buf_struct.flag = FLAG_RESPONSE;
      buf_struct.operation = OP_ECHO;
      buf_struct.data_len = buf_struct_rcv.data_len;
      buf_struct.seq_num = buf_struct_rcv.seq_num;

      strcpy(buf_struct.data, buf_struct_rcv.data);

      printf("received instruction message! received data_len : %hu bytes\n", buf_struct_rcv.data_len);
      printf("operation type is echo.\n");
      printf("echo : %s\n", buf_struct.data);

      send(client_sockfd, &buf_struct, sizeof(struct hw_packet), 0);
      printf("sent response msg with seq.num. %u to server.\n\n", buf_struct.seq_num);
    }

    /*if server sent INCREMENT request instruction*/
    else if (buf_struct_rcv.operation == OP_INCREMENT) {
    
      struct hw_packet buf_struct;
      buf_struct.flag = FLAG_RESPONSE;
      buf_struct.operation = OP_ECHO;
      buf_struct.data_len = buf_struct_rcv.data_len;
      buf_struct.seq_num = buf_struct_rcv.seq_num;

      unsigned int value;
      memcpy(&value, buf_struct_rcv.data, sizeof(unsigned int));
      value = value + 1;
      
      printf("received instruction message! received data_len : %hu bytes\n", buf_struct_rcv.data_len);
      printf("operation type is increment.\n");
      printf("increment : %u\n", value);

      memcpy(buf_struct.data, &value, sizeof(unsigned int));
      send(client_sockfd, &buf_struct, sizeof(struct hw_packet), 0);
      printf("sent response msg with seq.num. %u to server.\n\n", buf_struct.seq_num);
    }  

    /*if server sent DECREMENT request instruction*/
    else if (buf_struct_rcv.operation == OP_DECREMENT) {
    
      struct hw_packet buf_struct;
      buf_struct.flag = FLAG_RESPONSE;
      buf_struct.operation = OP_ECHO;
      buf_struct.data_len = buf_struct_rcv.data_len;
      buf_struct.seq_num = buf_struct_rcv.seq_num;
      
      unsigned int value;
      memcpy(&value, buf_struct_rcv.data, sizeof(unsigned int));
      value = value - 1;      

      printf("received instruction message! received data_len : %hu bytes\n", buf_struct_rcv.data_len);
      printf("operation type is decrement.\n");
      printf("decrement : %u\n", value);

      memcpy(buf_struct.data, &value, sizeof(unsigned int));
      send(client_sockfd, &buf_struct, sizeof(struct hw_packet), 0);
      printf("sent response msg with seq.num. %u to server.\n\n", buf_struct.seq_num);
    }    
  
    recv(client_sockfd, &buf_struct_rcv, sizeof(struct hw_packet), 0);
  }

  /*if server send a terminate message*/
  printf("received terminate msg! terminating...\n");

  close(client_sockfd);
  return 0;
}


