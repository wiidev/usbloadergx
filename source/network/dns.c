#include "dns.h"

/**
 * Resolves a domainname to an ip address
 * It makes use of net_gethostbyname from libogc, which in turn makes use of a Wii BIOS function
 * Just like the net_gethostbyname function this function is NOT threadsafe!
 *
 * @param char* The domain name to resolve
 * @return u32 The ipaddress represented by four bytes inside an u32 (in network order)
 */
u32 getipbyname(char *domain)
{
	//Care should be taken when using net_gethostbyname,
	//it returns a static buffer which makes it not threadsafe
	//TODO: implement some locking mechanism to make below code atomic
	struct hostent *host = net_gethostbyname(domain);
	
	if(host == NULL) {
		return 0;
	}
	
	u32 *ip = (u32*)host->h_addr_list[0];
	return *ip;
}



//Defines how many DNS entries should be cached by getipbynamecached()
#define MAX_DNS_CACHE_ENTRIES 20

//The cache is defined as a linked list,
//The last resolved domain name will always be at the front
//This will allow heavily used domainnames to always stay cached
struct dnsentry {
	char *domain;
	u32 ip;
	struct dnsentry *nextnode;
} ;

static struct dnsentry *firstdnsentry = NULL;
static int dnsentrycount = 0;

/**
 * Performs the same function as getipbyname(),
 * except that it will prevent extremely expensive net_gethostbyname() calls by caching the result
 */
u32 getipbynamecached(char *domain)
{
	//Search if this domainname is already cached
	struct dnsentry *node = firstdnsentry;
	struct dnsentry *previousnode = NULL;
	
	while(node != NULL)
	{
		if(strcmp(node->domain, domain) == 0)
		{
			//DNS node found in the cache, move it to the front of the list
			if(previousnode != NULL)
				previousnode->nextnode = node->nextnode;
				
			if(node != firstdnsentry)
				node->nextnode = firstdnsentry;
			firstdnsentry = node;
			
			return node->ip;
		}
		//Go to the next element in the list
		previousnode = node;
		node = node->nextnode;
	}
	u32 ip = getipbyname(domain);
	
	//No cache of this domain could be found, create a cache node and add it to the front of the cache
	struct dnsentry *newnode = malloc(sizeof(struct dnsentry));
	if(newnode == NULL) {
		return ip;
	}
		
	newnode->ip = ip;
	newnode->domain = malloc(strlen(domain)+1);
	if(newnode->domain == NULL)
	{
		free(newnode);
		return ip;
	}
	strcpy(newnode->domain, domain);
	
	newnode->nextnode = firstdnsentry;
	firstdnsentry = newnode;
	dnsentrycount++;

	//If the cache grows too big delete the last (and probably least important) node of the list
	if(dnsentrycount > MAX_DNS_CACHE_ENTRIES)
	{
		struct dnsentry *node = firstdnsentry;
		struct dnsentry *previousnode = NULL;
		
		//Fetch the last two elements of the list
		while(node->nextnode != NULL)
		{
			previousnode = node;
			node = node->nextnode;
		}
		
		if(node == NULL)
		{
			printf("Configuration error, MAX_DNS_ENTRIES reached while the list is empty\n");
			exit(1);
		} else if(previousnode == NULL)
		{
			firstdnsentry = NULL;
		} else {
			previousnode->nextnode = NULL;
		}
		
		free(node->domain);
		free(node);
		dnsentrycount--;
	}

	return newnode->ip;
}
