#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include "netlink.h"


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andrey Tarasov");
MODULE_DESCRIPTION("Module for testing netlink sockets");


static struct sock *nl_sk = NULL;

/*
 * netlink_recv_msg - получение сообщения через netlink сокет
 * Принимаем сообщение из пространства пользователя и отправляем
 * его обратно, то есть выплняем эхо.
 */
static void netlink_recv_msg(struct sk_buff *skb)
{
	struct nlmsghdr* nlh = (struct nlmsghdr*) skb->data;
	char* msg = (char*) nlmsg_data(nlh);
	printk(KERN_INFO "Kernel received via netlink: %s\n", msg);

	struct sk_buff* skb_out = nlmsg_new(MAX_PAYLOAD, 0);
	if (! skb_out) {
		printk(KERN_ERR "Failed to allocate new skb.\n");
		return;
	}

	int pid = nlh->nlmsg_pid;
	nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, MAX_PAYLOAD, 0);
	NETLINK_CB(skb_out).dst_group = 0;
	sprintf(nlmsg_data(nlh), "Kernel got: %s", msg);

	if (nlmsg_unicast(nl_sk, skb_out, pid) < 0)
		printk(KERN_INFO "Error while sending back to user.\n");
}


static int __init netlink_init(void)
{
	struct netlink_kernel_cfg cfg = {
		.input = netlink_recv_msg,
	};

	nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
	if (! nl_sk) {
		printk(KERN_ALERT "Error creating socket.\n");
		return -10;
	} else
		printk(KERN_INFO "Netlink socket created succesfully.\n");

	return 0;
}


static void __exit netlink_exit(void) {
	printk(KERN_INFO "Exiting netlink module.\n");
	netlink_kernel_release(nl_sk);
}


module_init(netlink_init);
module_exit(netlink_exit);

