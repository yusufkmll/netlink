#include <linux/module.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <net/sock.h>

#define NETLINK_TEST 17

MODULE_LICENSE("GPL");

struct sock *nl_sock = NULL;

static void netlink_test_recv_msg(struct sk_buff *skb)
{
    struct sk_buff *skb_out;
    struct nlmsghdr *nlh;
    int msg_size;
    char *msg;
    int pid;
    int res;

    nlh = (struct nlmsghdr *)skb->data;
    pid = nlh->nlmsg_pid; /* pid of sending process */
    msg = (char *)nlmsg_data(nlh);
    msg_size = strlen(msg);

    printk(KERN_INFO "netlink_test: Received from pid %d: %s\n", pid, msg);

    // create reply
    skb_out = nlmsg_new(msg_size, 0);
    if (!skb_out) {
      printk(KERN_ERR "netlink_test: Failed to allocate new skb\n");
      return;
    }

    // put received message into reply
    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
    NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
    strncpy(nlmsg_data(nlh), msg, msg_size);

    printk(KERN_INFO "netlink_test: Send %s\n", msg);

    res = nlmsg_unicast(nl_sock, skb_out, pid);
    if (res < 0)
      printk(KERN_INFO "netlink_test: Error while sending skb to user\n");
}

static int __init netlink_test_init(void)
{
  printk(KERN_INFO "netlink_test: Init module\n");

    //* Sanirim func ptr burada paslaniyor
  struct netlink_kernel_cfg cfg = {
    .input = netlink_test_recv_msg,
  };

  nl_sock = netlink_kernel_create(&init_net, NETLINK_TEST, &cfg);
  if (!nl_sock) {
    printk(KERN_ALERT "netlink_test: Error creating socket.\n");
    return -10;
  }

  return 0;
}

static void __exit netlink_test_exit(void)
{
  printk(KERN_INFO "netlink_test: Exit module\n");

  netlink_kernel_release(nl_sock);
}

module_init(netlink_test_init);
module_exit(netlink_test_exit);
