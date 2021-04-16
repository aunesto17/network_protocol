  /* Server code in C */
 
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
  #include <vector>
  #include <fcntl.h>   

  using namespace std;
 
vector<int> clients;

void Broadcast(int ConnectFD, string message) 
{
  //write(ConnectFD, message.c_str(),message.length());
  
  for (int i = 0; i < clients.size(); i++) {
    write(clients[i], message.c_str(),message.length());
  }
  

}

void Connection( int &SocketFD , int port)
{
    struct sockaddr_in stSockAddr;
    SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    if(-1 == SocketFD)
    {
      perror("can not create socket");
      exit(EXIT_FAILURE);
    }
    
    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
   
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(port);
    stSockAddr.sin_addr.s_addr = INADDR_ANY;
 
    if(-1 == bind(SocketFD,(const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
    {
      perror("error bind failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
    
    if(-1 == listen(SocketFD, 10))
    {
      perror("error listen failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
   
}

void listenClient(int ConnectFD)
{
    char buffer[256];
    int n;

    string message;
    while(1)
    {
      bzero(buffer,256); 
      n = read(ConnectFD,buffer,255);
      if (n < 0) perror("ERROR reading from socket");
      printf("Client %i: %s\n",ConnectFD,buffer);
      //cout<<"Type a message: ";
      message="Client " + to_string(ConnectFD) + ":" + buffer;
      //getline(cin,message);
      Broadcast(ConnectFD, message);
    }

    for(int i = 0; i < clients.size(); ++i) 
    {
      if (clients[i] == ConnectFD) 
      {
        shutdown(ConnectFD, SHUT_RDWR);
        close(ConnectFD);
        clients.erase(clients.begin() + i);
      }
    }
}

int main(void)
{
  struct sockaddr_in stSockAddr;

  char *file_path = "output.tmp";
  int filefd;
  ssize_t read_return;

  int SocketFD;
  char buffer[1500];
  int n;
  cout<<"Hi"<<endl;
  Connection( SocketFD, 45000);
  
  cout<<"Connection initialized"<<endl;
  
  while(1)
  {
    
    int ConnectFD = accept(SocketFD, NULL, NULL);

    if(0 > ConnectFD)
    {
      perror("error accept failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
    clients.push_back(ConnectFD);
    thread(listenClient, ConnectFD).detach();

    filefd = open(file_path,
                O_WRONLY | O_CREAT | O_TRUNC,
                S_IRUSR | S_IWUSR);
    if (filefd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    while (1) {
        read_return = read(SocketFD, buffer, 1500);
        if (read_return == 0)
            break;
        if (read_return == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        if (write(filefd, buffer, read_return) == -1) {
            perror("write");
            exit(EXIT_FAILURE);
        }
    }

    close(filefd);
    
  }

  close(SocketFD);
  cout<<"Bye"<<endl;
  return 0;
}
