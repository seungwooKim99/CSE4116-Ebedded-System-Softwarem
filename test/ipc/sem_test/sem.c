#include <sys/types.h> 
#include <sys/ipc.h> 
#include <sys/sem.h> 
#include <errno.h> 

union semun { 
     int val; 
     struct semid_ds *buf; 
     unsigned short *array; 
 }; 

 int initsem(key_t semkey) { 
     union semun semunarg; 
     int status = 0, semid; 

     semid = semget(semkey, 1, IPC_CREAT | IPC_EXCL | 0600); 
     if(semid == -1) { 
         if(errno == EEXIST) 
             semid = semget(semkey, 1, 0); 
     } 
     else { 
         semunarg.val = 1; 
         status = semctl(semid, 0, SETVAL, semunarg); 
     } 
     if(semid == -1 || status == -1) { 
         perror("initsem"); 
         return (-1); 
     } 
     return semid; 
 }
int semlock(int semid) {
    // p
     struct sembuf buf; 

     buf.sem_num = 0; 
     buf.sem_op = -1; 
     buf.sem_flg = SEM_UNDO; 
     if(semop(semid, &buf, 1) == -1) { 
         perror("semlock failed"); 
         exit(1); 
     } 
     return (0); 
 } 

 int semunlock(int semid) { 
    // v
     struct sembuf buf; 

     buf.sem_num = 0; 
     buf.sem_op = 1; 
     buf.sem_flg = SEM_UNDO; 
     if(semop(semid, &buf, 1) == -1) { 
         perror("semunlock failed"); 
         exit(1); 
     } 
     return (0); 
}
void semhandle() { 
    int semid; 
    pid_t pid = getpid(); 
    printf("Process %d : Before Lockn\n", pid); 
    if((semid = initsem(IPC_PRIVATE)) < 0) exit(1); 
    semlock(semid); 
    printf("In Lock Moden\n"); 
    semunlock(semid); 
    printf("Process %d : After Lockn\n", pid); 
    exit(0); 
 } 

int main() { 
     int a; 
     for(a=0; a<3; a++) 
         if(fork() == 0) semhandle(); 
 }