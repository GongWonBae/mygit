#include "pcap.h"
#include <stdio.h>

#define WPCAP
#define HAVE_REMOTE

dsadsa

typedef struct ip_address{
	u_char byte1;
	u_char byte2;
	u_char byte3;
	u_char byte4;
}ip_address;
typedef struct ip_header
{
	u_char ver_ihl; // Version (4 bits) + Internet header length (4 bits)
	u_char tos; // Type of service 
	u_short tlen; // Total length 
	u_short identification; // Identification
	u_short flags_fo; // Flags (3 bits) + Fragment offset (13 bits)
	u_char ttl; // Time to live
	u_char proto; // Protocol
	u_short crc; // Header checksum
	ip_address saddr; // Source address
	ip_address daddr; // Destination address
	u_int op_pad; // Option + Padding
}ip_header;

/* UDP header*/
typedef struct udp_header{
	u_short sport;   // Source port
	u_short dport;   // Destination port
	u_short len;   // Datagram length
	u_short crc;   // Checksum
}udp_header;

/* eternet header */
struct ether_header
{
	u_char dst_host[6];
	u_char src_host[6];
	u_short frame_type;
}ether_header;

typedef struct tcp_header
{
	u_short sport; // Source port
	u_short dport; // Destination port
	u_int seqnum; // Sequence Number
	u_int acknum; // Acknowledgement number
	u_char hlen; // Header length
	u_char flags; // packet flags
	u_short win; // Window size
	u_short crc; // Header Checksum
	u_short urgptr; // Urgent pointer...still don't know what this is...
}tcp_header;

/* ��Ŷ�� ĸó ������, ȣ��Ǵ� �ݹ� �Լ� */
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data);

char link[100];
char woonbe[100];
int img_ip[4];
int img_pt;

FILE *fp = NULL;
int main()
{
	
	fp=fopen("test.txt","a");
	pcap_if_t *alldevs;
	pcap_if_t *d;
	int inum;
	int i=0;
	pcap_t *adhandle;
	char errbuf[PCAP_ERRBUF_SIZE];

	//���ͷ� ���� 
	char *filter = "port 80";
	struct bpf_program fcode;
	bpf_u_int32 NetMask;

	/* ��Ʈ��ũ �ٹ��̽� ����� �����´�. */
	/* alldevs�� ����Ʈ ���·� ����Ǹ�, ������ errbuf�� ���� ���� ���� */
	if(pcap_findalldevs(&alldevs, errbuf) == -1)
	{
		fprintf(stderr,"Error in pcap_findalldevs: %s\n", errbuf);
		exit(1);
	}

	/* ��Ʈ��ũ �ٹ��̽����� ����Ѵ�. */
	for(d=alldevs; d; d=d->next)
	{
		printf("%d. %s", ++i, d->name);
		if (d->description)
			printf("\n\t(%s)\n\n", d->description);
		else
			printf(" (No description available)\n");
	}

	/* ���� ó�� */
	if(i==0)
	{
		printf("\nNo interfaces found! Make sure WinPcap is installed.\n");
		return -1;
	}

	/* ĸó�� ��Ʈ��ũ ����̽� ���� */
	printf("��Ʈ��ũ ��� ���� : (1-%d):",i);
	scanf("%d", &inum);

	/* �Է°��� ��ȿ�� �Ǵ� */
	if(inum < 1 || inum > i)
	{
		printf("\nInterface number out of range.\n");
		/* ��ġ ��� ���� */
		pcap_freealldevs(alldevs);
		return -1;
	}

	/* ����ڰ� ������ ��ġ��� ���� */
	for(d=alldevs, i=0; i< inum-1 ;d=d->next, i++);

	/* ���� ��Ʈ��ũ ����̽��� ���� */
	if ((adhandle= pcap_open_live(d->name, // ����̽���
		65536,   // �ִ� ĸó���� 
		// 65536 -> ĸó�ɼ� �ִ� ��ü ���� 
		1,    // 0: �ڽſ��� �ش�Ǵ� ��Ŷ�� ĸó
		// 1: ������ ��� ��Ŷ ĸó
		10,   // read timeout 
		errbuf   // �������� ���庯�� 
		)) == NULL)
	{
		fprintf(stderr,"\nUnable to open the adapter. %s is not supported by WinPcap\n", d->name);
		/* ��ġ ��� ���� */
		pcap_freealldevs(alldevs);
		return -1;
	}

	printf("\nlistening on %s...\n", d->description);

	/* �ݸ���ũ ����, �̺κ��� ���� �� �𸣰��� */
	NetMask=0xffffff;
	// ����ڰ� ������ ���ͷ� ������
	if(pcap_compile(adhandle, &fcode, filter, 1, NetMask) < 0)
	{
		fprintf(stderr,"\nError compiling filter: wrong syntax.\n");
		pcap_close(adhandle);
		return -3;
	}
	// ����ڰ� ������ ���ͷ� ����
	if(pcap_setfilter(adhandle, &fcode)<0)
	{
		fprintf(stderr,"\nError setting the filter\n");
		pcap_close(adhandle);
		return -4;
	}
	/* ��ġ ��� ���� */
	pcap_freealldevs(alldevs);

	/* ĸó ���� */
	pcap_loop(adhandle,      // pcap_open_live���� ���� ��Ʈ��ũ ����̽� �ڵ�
		100,     // 0 : ���ѷ���
		 // �������� : ĸó�� ��Ŷ�� 
		packet_handler,  // ��Ŷ�� ĸó������, ȣ��� �Լ� �ڵ鷯 
		NULL);           // �ݹ��Լ��� �Ѱ��� �Ķ���� 

	pcap_close(adhandle);    // ��Ʈ��ũ ����̽� �ڵ� ����

	printf("\n��ũ�ּ� : %s\n",woonbe);
	
	fclose(fp);
	printf("%s",link);
	return 0;
}

/* ��Ŷ�� ĸó ������, ȣ��Ǵ� �ݹ� �Լ� */
void packet_handler(u_char *param,                    //�Ķ���ͷ� �Ѱܹ��� �� 
	const struct pcap_pkthdr *header, //��Ŷ ���� 
	const u_char *pkt_data)           //���� ĸó�� ��Ŷ ������
{
	int i;

	// ������, ��Ʈ�� ���ϱ� ���� ����
	ip_header *ih;
	udp_header *uh;
	tcp_header *th;
	u_int ip_len;

	/* retireve the position of the ip header */
	ih = (ip_header *) (pkt_data + 14); //length of ethernet header
	/* retireve the position of the udp header */
	ip_len = (ih->ver_ihl & 0xf) * 4;
	uh = (udp_header *) ((u_char*)ih + ip_len);



	if(ih->proto==6)
	{
		th = (tcp_header*) (ih + ip_len);
		printf("TCP\n");
	}
	else if(ih->proto==17)
	{
		uh=(udp_header*) (uh + ip_len);
		printf("UDP\n");
	}
	th->dport;

	//printf("������ %d.%d.%d.%d\n",ih->daddr.byte1,ih->daddr.byte2,ih->daddr.byte3,ih->daddr.byte4);
	int token=0,j=0;
	
	/* Print the packet */
	for (i=1; (i < header->caplen + 1 ) ; i++)
	{
		if(token==1){	
			//link[j++] = pkt_data[i-1];
			printf("%.2x ", pkt_data[i-1]);
			fputc(pkt_data[i-1] ,fp);
			if ( (i % 16) == 0) printf("\n");
	
			//printf("%c",pkt_data[i-1]);
		}
		if(pkt_data[i-5]=='P' && pkt_data[i-4] == 'O' && pkt_data[i-3] == 'S' && pkt_data[i-2] == 'T')
		{
			token=1;
		}
		//	printf("%.2x ", pkt_data[i-1]);
		
	}

	printf("\n\n");
}