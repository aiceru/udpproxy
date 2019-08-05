#include <string>
#include <cstring>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_PACKET_SIZE     1024 * 1024     // 1 MByte

int main(void) {
  std::string mcast_address = "225.0.0.37";
  int mcast_port = 49111;
  std::string ucast_address = "192.168.100.10";
  int ucast_port = 49111;

  uint32_t recv_len;
  int mcast_sockfd;
  int ucast_sockfd;

  struct sockaddr_in    mcast_addr;
  struct sockaddr_in    ucast_addr;
  struct sockaddr_in    in_addr;
  socklen_t             in_addr_len;
  struct ip_mreq        mreq;

  char buf[MAX_PACKET_SIZE];

  if( (mcast_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ||
      (ucast_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    fprintf(stderr, "Failed to create socket\n");
    return -1;
  }

  int reuse = 1;
  if( setsockopt(mcast_sockfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(reuse)) < 0 ||
      setsockopt(mcast_sockfd, SOL_SOCKET, SO_REUSEPORT, (const char *)&reuse, sizeof(reuse)) < 0 ) {
    fprintf(stderr, "Failed to set socket options\n");
    return -1;
  }
  if( setsockopt(ucast_sockfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(reuse)) < 0 ||
      setsockopt(ucast_sockfd, SOL_SOCKET, SO_REUSEPORT, (const char *)&reuse, sizeof(reuse)) < 0 ) {
    fprintf(stderr, "Failed to set socket options\n");
    return -1;
  }

  std::memset(&mcast_addr, 0, sizeof(mcast_addr));
  mcast_addr.sin_family = AF_INET;
  mcast_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  mcast_addr.sin_port = htons(mcast_port);

  mreq.imr_multiaddr.s_addr = inet_addr(mcast_address.c_str());
  mreq.imr_interface.s_addr = INADDR_ANY;

  if( setsockopt(mcast_sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0 ) {
    fprintf(stderr, "Failed to join multicast group\n");
    return -1;
  }

  std::memset(&ucast_addr, 0, sizeof(ucast_addr));
  ucast_addr.sin_family = AF_INET;
  ucast_addr.sin_addr.s_addr = inet_addr(ucast_address.c_str());
  ucast_addr.sin_port = htons(ucast_port);

  std::memset(&in_addr, 0, sizeof(in_addr));

  while(1) {
    recv_len = recvfrom(mcast_sockfd, buf, MAX_PACKET_SIZE, 0, (struct sockaddr *)&in_addr, &in_addr_len);
    if(recv_len > 0) {
      sendto(ucast_sockfd, buf, recv_len, 0, (struct sockaddr *)&ucast_addr, sizeof(ucast_addr));
    }
    usleep(1);
  }

  return 0;
}
