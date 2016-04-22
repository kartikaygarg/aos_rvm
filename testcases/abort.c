/* abort.c - test that aborting a modification returns the segment to
 * its initial state */

#include "../rvm.h"
#include "../rvm.cpp"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_STRING1 "hello, world"
#define TEST_STRING2 "bleg!"
#define OFFSET2 1000


int main(int argc, char **argv)
{
     rvm_t rvm;
     char *seg;
     char *segs[1];
     trans_t trans;
     
     rvm = rvm_init("rvm_segments");
     
     rvm_destroy(rvm, "testseg");
     
     segs[0] = (char *) rvm_map(rvm, "testseg", 10000);
     seg = segs[0];

     /* write some data and commit it */
     trans = rvm_begin_trans(rvm, 1, (void**) segs);
     printf("rvm_begin_trans_1 done !\n");
     rvm_about_to_modify(trans, seg, 0, 100);
     printf("rvm_about_to_modify_1.1 done !\n");
     sprintf(seg, TEST_STRING1);
     
     rvm_about_to_modify(trans, seg, OFFSET2, 100);
     printf("rvm_about_to_modify_1.2 done !\n");
     sprintf(seg+OFFSET2, TEST_STRING1);
     
     rvm_commit_trans(trans);
     printf("rvm_commit_trans done !\n");

     /* start writing some different data, but abort */
     trans = rvm_begin_trans(rvm, 1, (void**) segs);
     printf("rvm_begin_trans_2 done !\n");
     rvm_about_to_modify(trans, seg, 0, 100);
     printf("rvm_about_to_modify_2.1 done !\n");
     sprintf(seg, TEST_STRING2);
     
     rvm_about_to_modify(trans, seg, OFFSET2, 100);
     printf("rvm_about_to_modify_2.2 done !\n");
     sprintf(seg+OFFSET2, TEST_STRING2);

     rvm_abort_trans(trans);
     printf("rvm_abort_trans done !\n");


     /* test that the data was restored */
     if(strcmp(seg+OFFSET2, TEST_STRING1)) {
	  printf("ERROR: second hello is incorrect (%s)\n",
		 seg+OFFSET2);
	  exit(2);
     }

     if(strcmp(seg, TEST_STRING1)) {
	  printf("ERROR: first hello is incorrect (%s)\n",
		 seg);
	  exit(2);
     }
     

     rvm_unmap(rvm, seg);
     printf("OK\n");
     exit(0);
}

