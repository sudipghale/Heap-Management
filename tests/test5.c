#include <stdlib.h>
#include <stdio.h>

int main()
{
  printf("Running test 1 to test a simple malloc and free\n");

  char * ptr = ( char * ) calloc ( 10,10 );

  while(ptr)
  {
    printf("%d\n",*ptr );
  }

  free( ptr );

  return 0;
}
