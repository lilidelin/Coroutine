#include"IOManager.h"
#include<iostream>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include<sys/un.h>
using namespace sylar;
void fun1(int fd){
    char rev_data[1024] = {0};
    int n = recv(fd,rev_data,sizeof(rev_data),0);
    std::cout<<"recv data: "<<rev_data<<" len="<<n<<std::endl;
}
void fun2(int fd){
    char send_data[1024] = "hello from test";
    std::cout<<"send data: "<<send_data<<std::endl;
    send(fd,send_data,sizeof(send_data),0);
}
int main(){
    int fd[2];
    if(socketpair(AF_UNIX, SOCK_STREAM, 0, fd)<0){
        std::cout<<"socketpair error"<<std::endl;
        return -1;
    }
    fcntl(fd[0],F_SETFL,O_NONBLOCK);
    fcntl(fd[1],F_SETFL,O_NONBLOCK);

    IOManager ioManager(1);
    // fd[1] 写，fd[0] 读
    ioManager.addEvent(fd[1],IOManager::WRITE,std::bind(&fun2,fd[1]));
    ioManager.addEvent(fd[0],IOManager::READ,std::bind(&fun1,fd[0]));
    sleep(1);

    ioManager.Stop();
    std::cout<<"event has been posted"<<std::endl;
    close(fd[0]);
    close(fd[1]);
    return 0;
}