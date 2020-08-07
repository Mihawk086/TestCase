#include <iostream>
#include <thread>
#include <vector>
#include <memory>
#include <boost/asio.hpp>
#include <chrono>
#include <string>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


using namespace std;
using boost::asio::ip::tcp;
using boost::asio::ip::udp;

class Server{
    public:
        Server();
        void start();
    private:
        void do_accept();

        tcp::acceptor _acceptor;
        tcp::socket _socket;
        tcp::endpoint _endpoint;
};

class UdpServer{
    public:
    UdpServer(boost::asio::io_service* io):_socket(*io,udp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"),8080)),_io_service(io){
        
    }
    ~UdpServer(){

    };
    void start(){
        do_read();
    }
    void do_read(){
        _socket.async_receive_from(boost::asio::buffer(_buf),_ed,[this](const boost::system::error_code &err, size_t bytes){
            if(!err){
                string buf(_buf,bytes);
                std::cout<< "receive msg:" << buf <<" thread id:"<<this_thread::get_id()<<std::endl;
                _io_service->post([this](){
                    do_read();
                });
                this_thread::sleep_for(chrono::milliseconds(10000));
                //do_read();
            }

        });
    }
    public:
    char _buf[1500];
    udp::endpoint _ed;
    boost::asio::io_service* _io_service;
    udp::socket _socket;
};

int main(){

    std::cout<< "main thread id:"<<this_thread::get_id()<<std::endl;

    boost::asio::io_service io_service;
    boost::asio::io_service::work work(io_service);
    std::vector<thread*> v;
    UdpServer server(&io_service);
    server.start();

    #if 1
    int threadnum = 2;
    for (int i = 0 ; i < threadnum ; ++i){
        v.push_back(new thread([&io_service](){
            std::cout<< "sub thread id:"<<this_thread::get_id()<<std::endl;
            io_service.run();
        }));
    }
    #endif

    int udp_socket_fd = ::socket(AF_INET,SOCK_DGRAM,0);
    if(udp_socket_fd == -1){
        cout<<"udp_socket_fd error"<<endl;
    }
    struct sockaddr_in dest_addr = {0};
    dest_addr.sin_family = AF_INET;//使用IPv4协议
    dest_addr.sin_port = htons(8080);//设置接收方端口号
    dest_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    this_thread::sleep_for(chrono::milliseconds(2000));

    thread* th2 = new thread([&](){
        while (true)
        {
            static int num = 0;
            string buf(to_string(num++));
            ::sendto(udp_socket_fd,buf.c_str(),buf.size(),0,(struct sockaddr *)&dest_addr,sizeof(dest_addr));
            
            /*
            io_service.post([buf](){
                std::cout<< "post " << buf <<" thread id:"<<this_thread::get_id()<<std::endl;
            });
            */
            this_thread::sleep_for(chrono::milliseconds(1000));
        }
    });

    while (true)
    {
        this_thread::sleep_for(chrono::milliseconds(1000));
    }
    
}