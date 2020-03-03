#include <semaphore.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

void test1(){
	
	sem_unlink("/sem_name");
#if 0	
	sem_t *sem = sem_open("/sem_name",O_CREAT,0666,1);
	if(sem == NULL){
		perror("sem_open");
		return ;
	}
	int val;
    sem_getvalue(sem, &val);
    printf("parent:semaphore value:%d\n",val);
	
	while(1){
		printf("sem_wait start:%ld\n",getpid());
		sem_wait(sem);
		printf("sem_wait end:%ld\n",getpid());
	}
#endif
}

int main(){
	test1();
	return 0;
}

