/* Client code in C++ */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <thread>
#include <fcntl.h>

using namespace std;

#define BUSIZ 1500

int SocketFD;

struct header{
  long data_lenght;
};

void Connection ( string IP, int port)
{
  struct sockaddr_in stSockAddr;
  SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

  int Res;
  int n;

  if (-1 == SocketFD)
    {
      perror("cannot create socket");
      exit(EXIT_FAILURE);
    }

   memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
 
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(port);
    Res = inet_pton(AF_INET, IP.c_str() , &stSockAddr.sin_addr);

    if (0 > Res)
    {
      perror("error: first parameter is not a valid address family");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
    else if (0 == Res)
    {
      perror("char string (second parameter does not contain valid ipaddress");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
 
    if (-1 == connect(SocketFD, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
    {
      perror("connect failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
}

void Listen()
{ 
  while(1)
  {
    int n;
    char buffer[256];
    bzero(buffer,256);
    n = read(SocketFD, buffer,256);
    if (n < 0) perror("ERROR reading from socket");
    cout<<buffer<<endl;
  }
}


int main(void)
{
  Connection("127.0.0.1",45001);

  ssize_t read_return;

  header hdr;

  FILE *filefd = fopen("10840.pdf", "rb");
  fseek(filefd, 0, SEEK_END);
  unsigned long filesize = ftell(filefd);
  char *buffer = (char*)malloc(sizeof(char)*filesize);
  rewind(filefd);
  hdr.data_lenght = filesize;

  fread(buffer, sizeof(char), filesize, filefd);
  cout << "buffer load: " << sizeof(buffer) << " filefdS: " << sizeof(filefd) <<endl;
  cout << "hrd.dl_sent: " << hdr.data_lenght << endl;

  //send size of the file first
  write(SocketFD, &hdr, sizeof(hdr));
  // send file
  int loc = 0;
  int bytesSent = 0;
  while (bytesSent < hdr.data_lenght)
  {
    bytesSent = write(SocketFD, &buffer[loc], BUFSIZ);
    if (bytesSent < 0)
    {
      /* handle error */
    }
    loc += bytesSent;
    hdr.data_lenght = hdr.data_lenght - bytesSent;
    cout << "bS: " << bytesSent << " loc: " << loc << " bytesLeft: " << hdr.data_lenght << endl;
  }
  cout << "File sent..." << endl;
  bytesSent = write(SocketFD, &buffer[loc], hdr.data_lenght);


  

  //thread(Listen).detach();

  /*
  while( 1 )
  { 
      string message;
      //cout<<"Type a message: ";
      message="";
      getline(cin,message);
      n = write(SocketFD,message.c_str(),message.length());      
  }
  */

  shutdown(SocketFD, SHUT_RDWR);
  //close(filefd);
  close(SocketFD);
  return 0;
}
