#include        <netinet/in.h>
#include        <errno.h>
#include        <stdio.h>
#include        <stdlib.h>
#include        <sys/socket.h>
#include        <unistd.h>
#include	<dirent.h>
#define MAXLINE                  4096
#define LISTENQ                  1024    /* 2nd argument to listen() */
#define SERV_PORT                9877
#define SA      struct sockaddr

static int sockfd;
void dg_echo(int, SA *, socklen_t);

int 
main(int argc, char ** argv)
{
	struct sockaddr_in servaddr, cliaddr;	
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0))<0) {
		printf("socket error.\n");
		exit(1);
	}
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);

	bind(sockfd, (SA *)&servaddr, sizeof(servaddr));

	dg_echo(sockfd, (SA *)&cliaddr, sizeof(cliaddr));
}

int readFileList(char *basePath,SA *pcliaddr,socklen_t clilen){
	DIR *dir;
	struct dirent *ptr;
	char base[1000];
	char path[1000];
	socklen_t len;
	len = clilen;
	if((dir = opendir(basePath))==NULL){
		perror("Open dir error...");
		exit(1);
	}
	
	while((ptr = readdir(dir)) != NULL){
		if(strcmp(ptr->d_name,".")==0||strcmp(ptr->d_name,"..")==0)
			continue;
		else if(ptr->d_type == 8){
			memset(path,'\0',sizeof(path));
			strcpy(path,basePath);
			strcat(path,"/");
			strcat(path,ptr->d_name);
			//path=basePath;
			sendto(sockfd,path, sizeof(path), 0, pcliaddr, len);
			//printf("d_name:%s/%s\n",basePath,ptr->d_name);
			}
		else if(ptr->d_type == 10){
			memset(path,'\0',sizeof(path));
			strcpy(path,basePath);
			strcat(path,"/");
			strcat(path,ptr->d_name);
			sendto(sockfd,path, sizeof(path), 0, pcliaddr, len);
			//printf("d_name:%s/%s\n",basePath,ptr->d_name);
			}
		else if(ptr->d_type == 4){
			memset(base,'\0',sizeof(base));
			strcpy(base,basePath);
			strcat(base,"/");
			strcat(base,ptr->d_name);
			readFileList(base,pcliaddr,clilen);
		}
	}
	closedir(dir);
	return 1;
}

void 
dg_echo(int sockfd, SA *pcliaddr, socklen_t clilen)
{
	DIR *dir;
	char basePath[1000];
	const char *s1="1";
	const char *s2="2\n";
	char s3[1000]="resent.";
	char file_name_up[256];
	char errormessage[10]="error";
	//const char *s4="udpserv1.c";
	int n;
	socklen_t len;
	char mesg[MAXLINE];
	char mesg_n[MAXLINE];
	char buffer[80];
	char filebuffer[8000];
	char fileupbuffer[8000];
	char filetap[1000]="Please enter the file name: ";
	getcwd(buffer,sizeof(buffer));
	//printf("dir is:%s\n",buffer);
	if(buffer == NULL){
		perror("getcwd error");
	}
	else{
		//printf("%s\n",buffer);
		//free(buffer);
	}
	printf("dir is:%s\n",buffer);
	
		
	for ( ; ; ) {
		len = clilen;
		memset(mesg_n,'\0',sizeof(mesg_n));
		n = recvfrom(sockfd, mesg, MAXLINE, 0, pcliaddr, &len);
		for(int i=0;mesg[i]!='\n';i++){
			mesg_n[i]=mesg[i];
		}
		printf("mesg_n:%s\n",mesg_n);
		if(strcmp(mesg_n,s1)==0){
			printf("in");
			readFileList(buffer,pcliaddr,clilen);
		}
		else if(*mesg=='2'){
			char file_name[256];
			bzero(file_name,256);
			//sendto(sockfd, s3, n, 0, pcliaddr, len);
			
			for(int i=1;mesg[i]!='\n';i++){
				file_name[i-1]=mesg[i];
			}
			printf("change to %s\n",file_name);
			FILE*fp = fopen(file_name,"r");
			if(fp==NULL){
				printf("FILE:%s Not Found\n",file_name);
				sendto(sockfd, errormessage, sizeof(errormessage), 0, pcliaddr, len);
			}
			else{
				sendto(sockfd, s3, n, 0, pcliaddr, len);
				bzero(filebuffer,0);
				int length=0;
				while((length = fread(filebuffer,sizeof(char),8000,fp))>0){
					printf("length=%d\n",length);
					if(sendto(sockfd,filebuffer,length, 0, pcliaddr, len)<0){
						printf("Send File:%s Failed.\n",file_name);
						break;
					}
					bzero(filebuffer,8000);
				}
				fclose(fp);
				printf("File:%s Transfer Successful!\n",file_name);
			}
			//sendto(sockfd, mesg, n, 0, pcliaddr, len);			
		}
		else if(*mesg=='3'){
			//printf("intter 3\n");
			for(int i=1;mesg[i]!='\n';i++){
				file_name_up[i-1]=mesg[i];
			}
			FILE *fp1=fopen(file_name_up,"w");
			if(fp1==NULL){
				printf("file can not open\n");
				exit(1);
			}
			bzero(fileupbuffer,MAXLINE+1);
			int lengthup=0;
			lengthup = recvfrom(sockfd, fileupbuffer, MAXLINE, 0, NULL, NULL);
				/*if(lengthup<=10){
					break;
				}*/
				//printf("intter\n");
				printf("%d\n",lengthup);
				if(fwrite(fileupbuffer,sizeof(char),lengthup,fp1)<lengthup){
					printf("file write faile\n");
					break;
				}
				fputs(fileupbuffer, stdout);
				bzero(fileupbuffer,MAXLINE+1);
				//fputs(fileupbuffer,stdout);
				//mesg[lengthup] = 0; 
					
			//}
			printf("receive file successful\n");
			fclose(fp1);
		}
		printf("recv\n");
		sendto(sockfd, mesg, n, 0, pcliaddr, len);
		printf("recv2\n");
	}
}
