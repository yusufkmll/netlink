#include <sys/socket.h>
#include <linux/netlink.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_PAYLOAD 1024
#define CONSTANT_MESSS "Hello from userspace process"
#define NETLINK_TEST 17

//* Source ve destination adresleri yapilari
struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh = NULL;
struct iovec iov;
struct msghdr msg;
int sock_fd;

void main() {
    //* user space'ten socket olusturma
    sock_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_TEST);

    printf("fd numara %d\n", sock_fd);

    //* source address yapisinin adresi sifirlanir
    //? bence gereksiz
    memset(&src_addr, 0, sizeof(src_addr));

    src_addr.nl_family = AF_NETLINK; //* netlink
    src_addr.nl_pid = getpid(); //* mevcut process
    src_addr.nl_groups = 0; //* unicast mesaj

    //* socketi bind'lama
    bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr));

    //? yine gereksiz
    memset(&dest_addr, 0, sizeof(dest_addr));
   
    dest_addr.nl_family = AF_NETLINK; //* netlink
    dest_addr.nl_pid = 0;   //* destination kernel, o yuzden 0
    dest_addr.nl_groups = 0; //* unicast mesaj

    //* Payload ve overhead icin gerekli yeri ayir
    //* netlink mesaji icin gereken header yapisi
    nlh = (struct nlmsghdr*) malloc(NLMSG_SPACE(MAX_PAYLOAD));

    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD); //* boyut
    nlh->nlmsg_pid = getpid(); //* mevcut process
    nlh->nlmsg_flags = 0; //? kernel'den ack iste

    strcpy(NLMSG_DATA(nlh), CONSTANT_MESSS);

    //* header ve payload io vektorune yazildi
    //? neden io vector
    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;

    //? bilmiyorum ama iov atamak için gerekli olabilir
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    //* mesaji gonder
    sendmsg(sock_fd, &msg, 0);

    printf("Mesaj bekleniyor..\n");

    //* Mesaj gelene kadar process blocking state'te durur
    recvmsg(sock_fd, &msg, 0);

    //* Mesaj geldi, oku
    printf("Gelen mesaj: %s\n", (char*)NLMSG_DATA(nlh));

    //* Haberlesme bitti, soketi kapat
    close(sock_fd);

    //* HAZIR
}