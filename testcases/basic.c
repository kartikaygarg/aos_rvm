/* basic.c - test that basic persistency works */

#include "../rvm.h"
#include "../rvm.cpp"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define TEST_STRING "hello, world"
#define OFFSET2 1000


/* proc1 writes some data, commits it, then exits */
void proc1() 
{
     rvm_t rvm;
     trans_t trans;
     char* segs[1];
     
     rvm = rvm_init("rvm_segments");
     printf("rvm_init done in proc1 \n");
     //rvm_destroy(rvm, "testseg");
     segs[0] = (char *) rvm_map(rvm, "testseg", 10000);
     printf("rvm_map done in proc1 \n");
     
     trans = rvm_begin_trans(rvm, 1, (void **) segs);
     printf("rvm_begin_trans done in proc1 \n");
     
     rvm_about_to_modify(trans, segs[0], 0, 100);
     printf("rvm_about_to_modify done in proc1 \n");
     sprintf(segs[0], TEST_STRING);
     
     rvm_about_to_modify(trans, segs[0], OFFSET2, 100);
     printf("rvm_about_to_modify in proc1 \n");
     sprintf(segs[0]+OFFSET2, TEST_STRING);
     
     rvm_commit_trans(trans);
     printf("rvm_commit_trans done in proc1 \n");

     abort();

}


/* proc2 opens the segments and reads from them */
void proc2() 
{
     char* segs[1];
     rvm_t rvm;
     
     rvm = rvm_init("rvm_segments");
     printf("rvm_init done \n");
     segs[0] = (char *) rvm_map(rvm, "testseg", 10000);
     printf("rvm_map done \n");
     if(strcmp(segs[0], TEST_STRING)) {
	  printf("ERROR: first hello not present\n");
	  exit(2);
     }
     if(strcmp(segs[0]+OFFSET2, TEST_STRING)) {
	  printf("ERROR: second hello not present\n");
	  exit(2);
     }

     printf("OK\n");
     exit(0);
}


int main(int argc, char **argv)
{
     int pid;

     pid = fork();
     if(pid < 0) {
	  perror("fork");
	  exit(2);
     }
     if(pid == 0) {
	  proc1();
       printf("proc1 successful ! \n");
	   fflush(stdout);
	  exit(0);
     }

     waitpid(pid, NULL, 0);
     printf("Before proc2 called \n");
     fflush(stdout);
	 proc2();
     printf("After proc2 called \n");
	fflush(stdout);
	
     return 0;
}