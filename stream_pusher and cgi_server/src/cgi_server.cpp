// Server stress test program
// P151 优化原理

#include "processpool.h"
#include <random>

// static const char* request = "res:";

struct str
{
    std::string date;
    std::string msgid;
};

std::random_device rd;                              

//处理客户CGI请求的类，它可以作为processpool 类的模板参数
class cgi_conn
{
 public:
  cgi_conn(){}
  ~cgi_conn(){}

  //初始化客户连接，清空读缓冲区
  void init( int epollfd, int sockfd, const sockaddr_in &client_addr )
  {
      m_epollfd = epollfd;
      m_sockfd = sockfd;
      m_address = client_addr;
      memset( m_buf, '\0', BUFFER_SIZE);
      m_read_idx = 0;
      fork_flag = true;
  }
  //事间戳
  std::time_t getTimeStamp()
  {
    std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp = 
        std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
    std::time_t timestamp = tmp.count();
    return timestamp;
  }
  //获取时间 年-月-日 时-分-秒 毫秒
  std::tm* gettm(uint64_t timestamp)
  {
    uint64_t milli = timestamp + (uint64_t)8 * 60 * 60 * 1000;
    auto mTime = std::chrono::milliseconds(milli);
    auto tp = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>(mTime);
    auto tt = std::chrono::system_clock::to_time_t(tp);
    std::tm* now = std::gmtime(&tt);
    return now;
  }
  struct str now_str()
  {  
    time_t timep;
    timep = getTimeStamp();
    struct tm* info;
    info = gettm(timep);
    char tmp[64];
    char msgid_temp[40];
    
    timep = timep % 1000000;
    timep = timep % 1000;
    sprintf(tmp, "[%4d-%02d-%02d %02d:%02d:%02d.%03ld]", info->tm_year + 1900, info->tm_mon + 1, info->tm_mday, 
                                                        info->tm_hour, info->tm_min, info->tm_sec, timep);	
    sprintf(msgid_temp, "%02d%02d%02d%03ld%03d", info->tm_hour, info->tm_min, info->tm_sec, timep, rd() % 1000);     
    
    struct str ret_str;
    ret_str.date = tmp;
    ret_str.msgid = msgid_temp;
    return ret_str;
  }

  //向客户端回应len字节的数据
  bool write_nbytes( int sockfd, const char* buffer, int len )
  {
    int bytes_write = 0;
    printf( "write out %d bytes to socket %d\n", len, sockfd );
    while(1)
    {
        bytes_write = send( sockfd, buffer, len, 0 );
        if( bytes_write == -1 )
        {
            return false;
        }
        else if( bytes_write == 0)
        {
            return false;
        }

        len -= bytes_write;

        buffer = buffer + bytes_write;
        if(len <= 0)
        {
            return true;
        }
    }
  }

  void process()
  {
      int idx = 0;
      int ret = -1;

      //循环读取、分析客户数据
      while (true)
      {
          idx = m_read_idx;
        //   std::cout<< "This is befor recv.\n";
        //   ret = recv( m_sockfd, m_buf + idx, BUFFER_SIZE -1 -idx, 0 );
          ret = recv( m_sockfd, m_buf + idx, BUFFER_SIZE -1 -idx, 0 );
        
          //如果该操作发生错误，则关闭客户连接。
          //但如果是暂时无数据可读，则退出循环。
          if( ret < 0)
          {
              if(errno != EAGAIN)
              {
                  removefd( m_epollfd, m_sockfd );
              }
              break;
          }
          //如果对方关闭连接，则服务器也要关闭连接
          else if( ret == 0)
          {
            removefd( m_epollfd, m_sockfd );
            break;
          }
          else
          {
                bool pipe_flag;
                int write_pipe[2] = {0};

                m_read_idx += ret;
                struct str ret_str = now_str();
                std::cout<< ret_str.date << " recv " << inet_ntoa(m_address.sin_addr) << 
                            " msg, msgid:" << ret_str.msgid << ", msg content:" << m_buf + idx << std::endl;     
                
                m_buf[idx - 1] = '\0';

                std::string str = m_buf + idx;

                // Print the read message
                // printf("User msg: %s\n", str.c_str());

                if( str == "start")
                {
                    pipe_flag = true;
                    printf("start: %d\n", pipe_flag);

                    std::string request = "Begin Streaming.";            
                    write_nbytes(m_sockfd, request.c_str(), request.length());                         
                }
                else if( str == "stop")
                {
                    pipe_flag = false;
                    printf("stop: %d\n", pipe_flag);
                    
                    std::string request = "Stop streaming.";                    
                    write_nbytes(m_sockfd, request.c_str(), request.length());     
                }else
                {                   
                    std::string request = "Hello! Group-9!";    
                    write_nbytes(m_sockfd, request.c_str(), request.length());                    
                }
                write_pipe[0] = pipe_flag;

                pid_t pid;
                if(fork_flag)
                {
                    pid = fork();
                    fork_flag = false;
                }

                if(pid == -1)
                {
                    std::cout<< "fork error.\n";
                }
                else if(pid == 0)
                {
                    // std::cout<< "This is named pipe.\n";
                    int fd1;
                    // FIFO file path
                    char * myfifo = "./myfifo";
                    // mkfifo(<pathname>,<permission>)
                    
                    mkfifo(myfifo, 0666);
                    // char str1[80], str2[80];
                    fd1 = open(myfifo,O_WRONLY);
                    // write(fd1, str2, strlen(str2)+1);
                    // write(fd1, m_buf + idx, strlen(m_buf)+1);
                    write(fd1, write_pipe, sizeof(write_pipe));
                    
                    // std::cout<< "m_buff + idx = " << m_buf + idx <<std::endl;
                    close(fd1);  
                }
                else 
                {
                    // std::cout<< "This is parent pid.\n";
                }


            //   // 如果遇到字符“\r\n”,则开始处理客户请求
            //   for( ; idx < m_read_idx; ++idx )
            //   {
            //       if((idx >= 1) && (m_buf[idx-1] == '\r') && (m_buf[idx] == '\n'))
            //       {
            //           break;
            //       }
            //   }

            //   //如果没有遇到字符“\r\n”，则需要读取更多的客户数据
            //   if(idx == m_read_idx )
            //   {
            //       continue;
            //   }

            // //   m_buf[idx - 1] = '\0';
            //   char* file_name = m_buf;
            //   // 判断客户要运行的CGI程序是否存在
            //   if(access( file_name, F_OK ) == -1)
            //   {
            //       removefd( m_epollfd, m_sockfd );
            //       break;
            //   }

            //   std::cout<< "This is before fork()." <<std::endl;
            //   // 创建子进程来执行CGI程序
            //   ret = fork();
            //   if(ret == -1)     // fork调用失败返回-1
            //   {
            //     removefd( m_epollfd, m_sockfd );
            //     std::cout<< "fork error.\n";
            //     break;
            //   }
            //   else if( ret > 0) // 父进程中返回子进程PID，在子进程中返回0
            //   {
            //     //父进程只需关闭连接
            //     std::cout<< "This is Head_CGI.\n";
            //     removefd( m_epollfd, m_sockfd );
            //     break;
            //   }
            //   else  // 子进程将标准输出定向到m_sockfd，并执行CGI程序
            //   {
            //     //   close( STDOUT_FILENO );
            //     //   dup( m_sockfd );
            //     //   execl( m_buf, m_buf, NULL);
            //     //   // exec函数族的函数执行成功后不会返回，调用失败时，会设置errno并返回-1，然后从原程序的调用点接着往下执行。
            //     //   exit(0);
            //     // std::cout<< "This is named pipe.\n";
            //     // int fd1;
            //     // // FIFO file path
            //     // char * myfifo = "./myfifo";
            //     // // Creating the named file(FIFO)
            //     // // mkfifo(<pathname>,<permission>)
            //     // mkfifo(myfifo, 0666);
            //     // // char str1[80], str2[80];
                
            //     // // Now open in write mode and write
            //     // // string taken from user.
            //     // fd1 = open(myfifo,O_WRONLY);
            //     // // fgets(str2, 80, stdin);
            //     // // write(fd1, str2, strlen(str2)+1);
            //     // write(fd1, m_buf + idx, strlen(m_buf)+1);
            //     // std::cout<< "m_buff + idx = " << m_buf + idx <<std::endl;
            //     // close(fd1);  

            //     exit(0);
            //   }
          }
      }      
  }

 private:
  //读缓冲区的大小
  static const int BUFFER_SIZE = 1024;
  static int m_epollfd;
  int m_sockfd;
  sockaddr_in m_address;
  char m_buf[ BUFFER_SIZE ];

  //标记度缓冲区中已经读入的客户数据的最后一个字节的下一个位置
  int m_read_idx;

  bool fork_flag;
};

int cgi_conn::m_epollfd = -1;

int main(int argc, char *argv[])
{   
    if( argc <= 1)
    {
        printf("usage: %s port_nunber\n", basename( argv[0]));
        return 1;
    }
    // const char* ip = argv[1];
    // const char* ip = "121.5.5.221";    
    int port = atoi( argv[1] );
    std::cout<< "port=" << port <<std::endl;
    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert( listenfd >= 0);

    int ret = 0;
    struct sockaddr_in address;
    bzero( &address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons( port );
    // inet_pton( AF_INET, ip, &address.sin_addr );
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    // 12.13 modify
    int opt = 1;
    setsockopt( listenfd, SOL_SOCKET,SO_REUSEADDR, (const void *)&opt, sizeof(opt) );
    // 12.13 modify

    ret = bind( listenfd, (struct sockaddr*) &address, sizeof(address));
    assert( ret != -1);

    ret = listen( listenfd, 5);
    assert( ret != -1);

    processpool< cgi_conn >* pool = processpool< cgi_conn >::create( listenfd );
    if( pool )
    {
        pool->run();
        delete pool;
    }
    close( listenfd );  //正如前文提到的，main函数创建了文件描述符listened，那么就由它亲自来关闭   
    return 0;
}