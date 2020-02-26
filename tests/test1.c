#include <stdlib.h>

#include <stdio.h>



int main()

{

  printf("Running test 1 to test a simple malloc and free\n");



  char * ptr = ( char * ) malloc ( 600 );
  realloc(ptr, 6000);

  //free( ptr );



  return 0;

}
