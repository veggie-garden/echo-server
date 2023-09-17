// c++ header, for exit() call.
#include <iostream>
// network headers
#include <netdb.h>  // sockaddr_in
#include <string.h> // bzero
#include <sys/errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/unistd.h>

#include <stdio.h>

#define BACKLOG 0
#define MAX 2048

void error(const char *msg) {
  perror(msg);
  exit(-1);
}

class server {
private:
  int m_protocol;
  int m_listenfd, m_connfd, m_sockfd;
  struct sockaddr_in m_cliaddr, m_servaddr;
  socklen_t m_clilen, m_servlen;

  // int client[FD_SETSIZE], m_nready; //1024, num of clients that are ready.

public:
  server();
  server(const int family, int type, const in_addr_t ip, const char *port,
         int protocol);
  void start_server(const int family, int type, const in_addr_t ip,
                    const char *port, int protocol);
  // void Signal(int sig);

protected:
  void Socket(int __family, int __type, int __protocol);
  void Bind();
  void init_server(const int family, const in_addr_t ip, const char *port);
  void Listen(); // made as const. LOOK FOR ERRS.
  bool Accept();
  void echo_the_requests();
};

server ::server() {
  m_protocol = 0;
  m_listenfd = 0;
  m_connfd = 0;
  bzero(&m_cliaddr, sizeof(m_cliaddr));
  bzero(&m_servaddr, sizeof(m_servaddr));
  m_servlen = sizeof(m_servaddr);
  m_sockfd = 0;
}

server::server(const int family, int type, const in_addr_t ip, const char *port,
               int protocol) {
  m_protocol = 0;
  m_listenfd = 0;
  m_connfd = 0;
  bzero(&m_cliaddr, sizeof(m_cliaddr));
  bzero(&m_servaddr, sizeof(m_servaddr));
  m_servlen = sizeof(m_servaddr);
  m_sockfd = 0;

  start_server(family, type, ip, port, protocol);
}

void server ::start_server(const int family, int type, const in_addr_t ip,
                           const char *port, int protocol) {
  // Signal(SIGINT);
  Socket(family, type, protocol);
  init_server(family, ip, port);
  Bind();
  Listen();
  for (;;) {
    m_clilen = sizeof(m_cliaddr);
    if (Accept())
      continue;
    echo_the_requests();
  }
}

void server ::Socket(int __family, int __type, int __protocol) {
  if ((m_listenfd = socket(__family, __type, __protocol)) < 0)
    error("Socket error : ");
}

void server ::Bind() {
  if (bind(m_listenfd, (struct sockaddr *)&m_servaddr, m_servlen) < 0)
    error("bind error : ");
}

void server ::init_server(const int family, const in_addr_t ip,
                          const char *port) {
  bzero(&m_servaddr, sizeof(m_servaddr));
  m_servaddr.sin_family = family;
  m_servaddr.sin_addr.s_addr = htonl(ip);
  m_servaddr.sin_port = htons(atoi(port));
}

void server ::Listen() // made as const. LOOK FOR ERRS.
{
  if (listen(m_listenfd, BACKLOG) < 0)
    error("Listen error : ");
}

bool server ::Accept() {
  if ((m_connfd =
           accept(m_listenfd, (struct sockaddr *)&m_cliaddr, &m_clilen)) < 0) {
    if (errno == EINTR)
      return true;
    else {
      error("Accept error : ");
      return false;
    }
  }
  return false;
}

void server ::echo_the_requests() {
  int i, n;

  char buf[MAX];
  m_sockfd = m_connfd;
  bzero(&buf, MAX);

  /*
      handle the client
  */

again:
  while ((n = recv(m_sockfd, buf, MAX, 0)) > 0) {
    send(m_sockfd, buf, sizeof(buf), 0);
    bzero(&buf, sizeof(buf));
  }

  bzero(&buf, sizeof(buf)); // we do this to make sure there is a '%uFFFD' at
                            // the end, all the times.

  if (n < 0 && errno == EINTR) // Interrupt occured
  {
    if (errno == EINTR)
      goto again;
    else
      error("Echo error : ");
  }
  return;
}

int main(int argc, char **argv) {
  // signal(SIGINT, signalHandler);
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>", argv[0]);
    exit(1);
  }

  server obj(AF_INET, SOCK_STREAM, INADDR_ANY, argv[1],
             0); // family, type, ip, port, protocol.
  /*
  * We can also do the following :

    server obj;
    obj.start_server(AF_INET, SOCK_STREAM, INADDR_ANY, argv[1], 0);
  */
  return 0;
}

/*
Explanation : If you don't have a basic idea in socket programming, checkout
"resources" at the end .

              In main,
              we start the server using the
                  parametrized constructor.The other method is commented.Then,
              the control moves to the
              start_server() method,
              which,
              further invokes the essential methods,
              namely : Socket(),
                       init_server(),
                       Bind(),
                       Listen(),
                       Accept(),
                       echo_the_requests().

                       Note : init_server(),
                       is a function that initializes the structure,
                       "m_servaddr",
                       which is used later in communication.
*/
