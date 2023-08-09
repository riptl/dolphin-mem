/* Minimal program to dump MEM1 off Dolphin in C.
   Obvious improvements can make this more stable. */

#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

int
main( int     argc,
      char ** argv ) {
  pid_t pid = -1;
  for( int i=1; i<argc; i++ ) {
    if( 0==strcmp( argv[i], "--pid" ) ) {
      assert( i+1 < argc );
      pid = atol( argv[ i+1 ] );
    }
  }
  assert( pid>0L );

  char buf[ PATH_MAX ];
  sprintf( buf, "/proc/%d/fd", pid );
  DIR * dir = opendir( buf );
  assert( dir );
  struct dirent * dent;
  while( (dent=readdir( dir )) ) {
    long sz = readlinkat( dirfd( dir ), dent->d_name, buf, PATH_MAX );
    buf[ sz ]='\0';
    if( 0==strncmp( buf, "/dev/shm/", 9UL )
     && !strstr ( buf, "-jitblock" ) ) {
      fprintf( stderr, "Opening %s\n", buf );
      int shm_fd = openat( dirfd( dir ), dent->d_name, O_RDWR );
      assert( shm_fd>=0 );

      struct stat file_stat[1];
      int err = fstat( shm_fd, file_stat );
      assert( err==0 );
      fprintf( stderr, "st_ino: %lu\n", file_stat->st_ino );

      void * mem = mmap( NULL, (1UL<<25), PROT_READ|PROT_WRITE, MAP_SHARED, shm_fd, 0L );
      fwrite( mem, (1UL<<25), 1UL, stdout );
    }
  }
}
