#include        <netinet/in.h>
#include        <errno.h>
#include        <stdio.h>
#include        <stdlib.h>
#include        <sys/socket.h>
#include        <unistd.h>
#include        <string.h>
#define MAXLINE                  4096
#define LISTENQ                  1024    /* 2nd argument to listen() */
#define SERV_PORT                9877
#define SA      struct sockaddr


void dg_cli(FILE *, int, const SA *, socklen_t);

main(int argc, char **argv)
{
	int sockfd;
	struct sockaddr_in servaddr;

	if (argc != 2) {
		printf("usage:udpcli01sigio <IPaddress>\n");
		exit(1);
	}

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);

	inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0))<0) {
		printf("socket error.");
		exit(1);
	}

	dg_cli(stdin, sockfd, (SA *)&servaddr, sizeof(servaddr));
	
	exit(0);
}

void
dg_cli(FILE *fp, int sockfd, const SA *pservaddr, socklen_t servlen)
{
	int  n;
	char sendline[MAXLINE], recvline[MAXLINE + 1], sendline_n[MAXLINE + 1],recvline_n[MAXLINE + 1];
	const char *s="1\n";
	const char *s2="2\n";
	const char *s3="resent.";
	char buffer[MAXLINE+1];
	char filebuffer[8000];
	char file_name[256];
	char errormessage[10]="error\n";
	char successmesg[20]="successful\n";
	
	printf("--MENU--\n");
    printf("1.list\n");
    printf("2.download file. eg: down:text.txt\n");
    printf("3.up file. eg: up:text.txt\n");
    printf("4.else echo\n");
    printf("--------\n");

	while (fgets(sendline, MAXLINE,fp) != NULL) {
		
		if(sendline[0]=='u'&&sendline[1]=='p'&&sendline[2]==':'){
			char file_name_up[256];
			bzero(file_name_up,256);
			for(int i=3;sendline[i]!='\n';i++){
				file_name_up[i-3]=sendline[i];
			}
			printf("filename:%s\n",file_name_up);
			FILE *fp1=fopen(file_name_up,"r");
			if(fp1==NULL){
				printf("FILE:%s Not Found\n",file_name_up);
				continue;
			}
			else{
				sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen);
				bzero(filebuffer,0);
				int length=0;
				if((length = fread(filebuffer,sizeof(char),8000,fp1))>0){
						//printf("length=%d\n",length);
						
					if(sendto(sockfd,filebuffer,length, 0, pservaddr, servlen)<0){
						printf("Send File:%s Failed.\n",file_name_up);
						//break;
					}
					fputs(filebuffer, stdout);
					bzero(filebuffer,8000);
					//break;
				}
				fclose(fp1);
				printf("File:%s Transfer Successful!\n",file_name_up);
			}		
		}
		else if(sendline[0]=='d'&&sendline[1]=='o'&&sendline[2]=='w'&&sendline[3]=='n'&&sendline[4]==':'){
			for(int i=5;sendline[i]!='\n';i++){
				file_name[i-5]=sendline[i];
			}
			sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen);
		}
		else{
			sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen);
		}
			//if(sendline=)
	
		while(n = recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL)){
			//printf("%d\n",n);
			//printf("%s\n",recvline);
			if(strcmp(recvline,successmesg)==0){
				break;
			}
			if(strcmp(recvline,errormessage)==0){
				printf("FILE:%s Not Found\n",file_name);
				//recvfrom(sockfd, buffer, MAXLINE, 0, NULL, NULL);
				//recvfrom(sockfd, buffer, MAXLINE, 0, NULL, NULL);
				//fputs(buffer, stdout);
				break;
			}
			else if(strcmp(recvline,s3)==0){
				FILE *fp=fopen(file_name,"w");
				if(fp==NULL){
					printf("file can not open\n");
					exit(1);
				}
				bzero(buffer,MAXLINE+1);
				int length=0;
				if(length = recvfrom(sockfd, buffer, MAXLINE, 0, NULL, NULL)){
					printf("length=%d\n",length);
					if(fwrite(buffer,sizeof(char),length,fp)<length){
						printf("file write faile\n");
						//break;
					}
					fputs(buffer, stdout);
					bzero(buffer,MAXLINE+1);
					//fputs(buffer,stdout);
					recvline[n] = 0; 
					//break;
				}
				printf("receive file successful\n");
				fclose(fp);
			}
			recvline[n] = 0; /* null terminate */
			fputs(recvline, stdout);
			printf("\n");

		}
	}
}
 
