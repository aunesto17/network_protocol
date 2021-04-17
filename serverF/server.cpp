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
 #define BUSIZ 1500


struct header{
  long data_lenght;
};


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

  char *file_path = "output.pdf";
  //int filefd;
  //ssize_t read_return;

  int SocketFD;
  //char buffer[1500];
  int n;
  cout<<"Hi"<<endl;
  Connection( SocketFD, 45001);
  
  cout<<"Connection initialized"<<endl;
    
    int ConnectFD = accept(SocketFD, NULL, NULL);

    if(0 > ConnectFD)
    {
      perror("error accept failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
    clients.push_back(ConnectFD);
    

    FILE* fd = fopen(file_path, "wb");
    
    header hdr;

    read(clients[0], &hdr, sizeof(hdr));
    int filesize = hdr.data_lenght;
    cout << "file size to be recieved: " << hdr.data_lenght << endl;
    char *buffer = (char*) malloc(sizeof(char)*hdr.data_lenght);

    int loc = 0; //buffer position to write
    int bytesread = 0; // how much data read so far
    
    while(hdr.data_lenght > 1024){
      bytesread = read(clients[0],&buffer[loc],1024);
      cout << "bytes recieved: " << bytesread << endl;
      if(bytesread < 0){ 
        // handle error
      }
      loc += bytesread;
      hdr.data_lenght = hdr.data_lenght - bytesread;
    }
    cout << "File recieved..." << endl;
    if (hdr.data_lenght > 0)
    {
      bytesread = read(clients[0],&buffer[loc],hdr.data_lenght);
    }
    
    
    
    //write to file
    fwrite(buffer, sizeof(char), filesize, fd);
    fclose(fd);

  close(SocketFD);
  cout<<"Bye"<<endl;
  return 0;
}
