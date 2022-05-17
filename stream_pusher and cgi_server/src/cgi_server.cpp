#include "processpool.h"
#include <random>

struct str
{
    std::string date;
    std::string msgid;
};
std::random_device rd;                              

// 处理客户CGI请求的类，它可以作为processpool类的模板参数
class cgi_conn
{
 public:
  cgi_conn(){}
  ~cgi_conn(){}

  // 初始化客户连接，清空读缓冲区
  void init( int epollfd, int sockfd, const sockaddr_in &client_addr )
  {
      m_epollfd = epollfd;
      m_sockfd = sockfd;
      m_address = client_addr;
      memset( m_buf, '\0', BUFFER_SIZE);
      m_read_idx = 0;
      fork_flag = true;
  }

  /************************************/
  /* 服务于类阿帕奇压测工具作QPS测试  */
  // 事间戳
  std::time_t getTimeStamp()
  {
    std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp = 
        std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
    std::time_t timestamp = tmp.count();
    return timestamp;
  }

  // 获取时间 年-月-日 时-分-秒 毫秒
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
  /*****************************/

  // 向客户端回应len字节的数据
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

      // 分析客户请求数据
      while (true)
      {
          idx = m_read_idx;
          ret = recv( m_sockfd, m_buf + idx, BUFFER_SIZE -1 -idx, 0 );
        
          // 如果该操作发生错误，则关闭客户连接。但如果是暂时无数据可读，则退出循环。
          if( ret < 0)
          {
              if(errno != EAGAIN)
              {
                  removefd( m_epollfd, m_sockfd );
              }
              break;
          }
          // 如果对方关闭连接，则服务器也要关闭连接
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
                    int fd1;
                    char * myfifo = "./myfifo";

                    // mkfifo(<pathname>,<permission>)
                    mkfifo(myfifo, 0666);
                    fd1 = open(myfifo,O_WRONLY);
                    write(fd1, write_pipe, sizeof(write_pipe));
                    close(fd1);  
                }
          }
      }      
  }

 private:
  static const int BUFFER_SIZE = 1024; // 读缓冲区的大小
  static int m_epollfd;
  int m_sockfd;
  sockaddr_in m_address;
  char m_buf[ BUFFER_SIZE ];
  int m_read_idx;                      // 标记读缓冲区中已经读入的客户数据的最后一个字节的下一个位置
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

    int opt = 1;
    setsockopt( listenfd, SOL_SOCKET,SO_REUSEADDR, (const void *)&opt, sizeof(opt) );

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
    close( listenfd );  // main函数创建了文件描述符listened，那么就由它亲自来关闭。  
    return 0;
}