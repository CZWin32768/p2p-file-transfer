#include<netinet/in.h>  
#include<sys/types.h>  
#include<sys/socket.h>  
#include<stdio.h>  
#include<stdlib.h>  
#include<string.h>  
#include<pthread.h>
#include <arpa/inet.h>

//char message[]="Hello World";

#define HELLO_WORLD_SERVER_PORT    9898
#define LENGTH_OF_LISTEN_QUEUE     20  
#define BUFFER_SIZE                1024  
#define FILE_NAME_MAX_SIZE         512 

int is_file_accepted() {
    return 1;
}

void *serve_thread_fun() {
    struct sockaddr_in   server_addr;  
    bzero(&server_addr, sizeof(server_addr));  
    server_addr.sin_family = AF_INET;  
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);  
    server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);  
  
    // create a stream socket  
    // 创建用于internet的流协议(TCP)socket，用server_socket代表服务器向客户端提供服务的接口  
    int server_socket = socket(PF_INET, SOCK_STREAM, 0);  
    if (server_socket < 0)  
    {  
        printf("Create Socket Failed!\n");  
        exit(1);  
    }  
  
    // 把socket和socket地址结构绑定  
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)))  
    {  
        printf("Server Bind Port: %d Failed!\n", HELLO_WORLD_SERVER_PORT);  
        exit(1);  
    }  
  
    // server_socket用于监听  
    // （文件描述符， 隊列長度）
    if (listen(server_socket, LENGTH_OF_LISTEN_QUEUE))  
    {  
        printf("Server Listen Failed!\n");  
        exit(1);  
    }  
    printf("server init ok\n");
    // 服务器端一直运行用以持续为客户端提供服务  
    while(1)  
    {  
        // 定义客户端的socket地址结构client_addr，当收到来自客户端的请求后，调用accept  
        // 接受此请求，同时将client端的地址和端口等信息写入client_addr中  
        struct sockaddr_in client_addr;  
        socklen_t          length = sizeof(client_addr );  
  
        // 接受一个从client端到达server端的连接请求,将客户端的信息保存在client_addr中  
        // 如果没有连接请求，则一直等待直到有连接请求为止，这是accept函数的特性，可以  
        // 用select()来实现超时检测  
        // accpet返回一个新的socket,这个socket用来与此次连接到server的client进行通信  
        // 这里的new_server_socket代表了这个通信通道  
        int new_server_socket = accept(server_socket, (struct sockaddr*)&client_addr, &length);  
        if (new_server_socket < 0)  
        {  
            printf("Server Accept Failed!\n");  
            break;  
        }  
  
        char buffer[BUFFER_SIZE];  
        bzero(buffer, sizeof(buffer));  
        length = recv(new_server_socket, buffer, BUFFER_SIZE, 0);  
        if (length < 0)  
        {  
            printf("Server Recieve Data Failed!\n");  
            break;  
        }  

        char file_name[FILE_NAME_MAX_SIZE + 1];  
        bzero(file_name, sizeof(file_name));  
        strncpy(file_name, buffer,  
                strlen(buffer) > FILE_NAME_MAX_SIZE ? FILE_NAME_MAX_SIZE : strlen(buffer));  
        
        if(!is_file_accepted()) {
            close(new_server_socket);
            printf("Rejected!\n");
            continue;
        }
        strncpy(buffer, "OK",  2);
        send(new_server_socket, buffer, BUFFER_SIZE, 0);

        FILE *fp = fopen(file_name, "w");  
          
        bzero(buffer, sizeof(buffer));  
        int length2 = 0;  
        while(length2 = recv(new_server_socket, buffer, BUFFER_SIZE, 0))  
        {  
            if (length2 < 0)  
            {  
                printf("Recieve Data From Server Failed!\n");  
                break;  
            }  
    
            int write_length = fwrite(buffer, sizeof(char), length2, fp);  
            if (write_length < length2)  
            {  
                printf("File:\t%s Write Failed!\n", file_name);  
                break;  
            }  
            bzero(buffer, BUFFER_SIZE);  
        }  
    
        printf("Recieve File:\t %s From Server Finished!\n", file_name);
        close(new_server_socket);  
        fclose(fp);
    }
    close(server_socket);
}

void send_main(char* target_ip) {
    struct sockaddr_in client_addr;  
    bzero(&client_addr, sizeof(client_addr));  
    client_addr.sin_family = AF_INET; // internet协议族  
    client_addr.sin_addr.s_addr = htons(INADDR_ANY); // INADDR_ANY表示自动获取本机地址  
    client_addr.sin_port = htons(0); // auto allocated, 让系统自动分配一个空闲端口  
  
    // 创建用于internet的流协议(TCP)类型socket，用client_socket代表客户端socket  
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);  
    if (client_socket < 0)  
    {  
        printf("Create Socket Failed!\n");  
        exit(1);  
    }  
  
    // 把客户端的socket和客户端的socket地址结构绑定   
    if (bind(client_socket, (struct sockaddr*)&client_addr, sizeof(client_addr)))  
    {  
        printf("Client Bind Port Failed!\n");  
        exit(1);  
    }  
  
    // 设置一个socket地址结构server_addr,代表服务器的internet地址和端口  
    struct sockaddr_in  server_addr;  
    bzero(&server_addr, sizeof(server_addr));  
    server_addr.sin_family = AF_INET;  
  
    // 服务器的IP地址来自程序的参数   
    if (inet_aton(target_ip, &server_addr.sin_addr) == 0)  
    {  
        printf("Server IP Address Error!\n");  
        exit(1);  
    }  
  
    server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);  
    socklen_t server_addr_length = sizeof(server_addr);  
  
    // 向服务器发起连接请求，连接成功后client_socket代表客户端和服务器端的一个socket连接  
    if (connect(client_socket, (struct sockaddr*)&server_addr, server_addr_length) < 0)  
    {  
        printf("Can Not Connect To %s!\n", target_ip);  
        exit(1);  
    }  
  
    char file_name[FILE_NAME_MAX_SIZE + 1];  
    bzero(file_name, sizeof(file_name));  
    printf("Please Input File Name That You Want To Send.\t");  
    scanf("%s", file_name);  
    
    char buffer[BUFFER_SIZE];  
    bzero(buffer, sizeof(buffer));  
    strncpy(buffer, file_name, strlen(file_name) > BUFFER_SIZE ? BUFFER_SIZE : strlen(file_name));  
    // 向服务器发送buffer中的数据，此时buffer中存放的是客户端需要接收的文件的名字  
    send(client_socket, buffer, BUFFER_SIZE, 0);
    
    int length = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if(buffer[0] == 'O' && buffer[1] == 'K') {
        FILE *fp = fopen(file_name, "r");  
        if (fp == NULL)  
        {  
            printf("File:\t%s Not Found!\n", file_name);  
        }  
        else  
        {  
            bzero(buffer, BUFFER_SIZE);  
            int file_block_length = 0;  
            while( (file_block_length = fread(buffer, sizeof(char), BUFFER_SIZE, fp)) > 0)  
            {  
                printf("file_block_length = %d\n", file_block_length);  
  
                // 发送buffer中的字符串到client_socket,实际上就是发送给客户端  
                if (send(client_socket, buffer, file_block_length, 0) < 0)  
                {  
                    printf("Send File:\t%s Failed!\n", file_name);  
                    break;  
                }  
  
                bzero(buffer, sizeof(buffer));  
            }  
            fclose(fp);  
            printf("File:\t%s Transfer Finished!\n", file_name);  
        }  
  
        close(client_socket);  
    }
    else {
        close(client_socket);
        printf("send req rej\n");
    }
}

int main(int argc, char *argv) {
    pthread_t serve_thread; 
    int serve_thread_res = pthread_create(&serve_thread, NULL, serve_thread_fun, NULL);
    while(1) {
        char ip[100];
        printf("please input the target ip:");
        scanf("%s",ip);
        send_main(ip);
    }
}
