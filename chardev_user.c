#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>



#define WR_VALUE _IOW('a' , 'a' ,int32_t *)
#define BUFLEN 100
	//read from the module
char rbuf[BUFLEN];
char wbuf[BUFLEN];
int q = 0;
int fd;
void *reader(){
	while (1){
		if(read(fd, rbuf, BUFLEN) < 0) {
			perror("Read failed: ");
			exit(2);
		}
		if(rbuf[0] != '\n'){
			printf("from the kfifo:%s",rbuf);
			//buf[0] = '\0';
		}
	}
}

void *writer(){
	//printf("enter mesg");
	while(1){
		 fgets(wbuf, sizeof(wbuf), stdin);
		 if(wbuf[0]=='\n')
		 	printf("enter");
		if (!strcmp(wbuf,"quit\n")){
			strcpy(wbuf,"Byebye");
			
			if(write(fd, wbuf, strlen(wbuf)+1) < 0) {
				perror("write failed: ");
				exit(3);
			}
			exit(0);
		
		}	
			if( write(fd, wbuf, strlen(wbuf)+1) < 0) {
				perror("write failed: ");
				exit(3);
			}
	}

}

int main(int argc, char *argv[]) {
	int i = atoi(argv[1]);
	fd = open("/dev/chatroom", O_RDWR);
	if( fd < 0) {
		perror("Open failed: ");
		exit(1);
	}

	ioctl(fd,WR_VALUE,&i);
	//while(1);
	if(read(fd, rbuf, BUFLEN) < 0) {
		perror("Read failed: ");
		exit(2);
	}
	if(rbuf[0] != '\n'){
		printf("from the kfifo:%s",rbuf);
		//buf[0] = '\0';
	}
	pthread_t t1,t2;
	pthread_create(&t1, NULL, reader, NULL);
	pthread_create(&t2, NULL, writer,NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	// while(1){
	// 	if(read(fd, rbuf, BUFLEN) < 0) {
	// 		perror("Read failed: ");
	// 		exit(2);
	// 	}
	// 	if(rbuf[0] != '\n'){
	// 		printf("from the kfifo:%s",rbuf);
	// 		//buf[0] = '\0';
	// 	}
	// }	
	return 0;
}