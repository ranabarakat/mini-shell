
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

const char * usage = ""
"Usage:\n"
"    ls_output outfile\n"
"\n"
"    It does something similar to the shell command:\n"
"        csh> ls -l > outfile\n"
"\n"
"Example:\n"
"    ls_output outfile\n"
"    cat outputfile\n\n";

const char *ls = "ls";

int
main(int argc, char **argv, char **envp)
{
	if (argc < 2) {
		fprintf(stderr, "%s", usage );
		exit(1);
	}

	// Save default input, output, and error because we will
	// change them during redirection and we will need to restore them
	// at the end.
	// The dup() system call creates a copy of a file descriptor.
	int defaultin = dup( 0 );
	int defaultout = dup( 1 );
	int defaulterr = dup( 2 );

	//////////////////  ls  //////////////////////////

	// Input:    defaultin
	// Output:   file
	// Error:    defaulterr

	// Create file descriptor 
	int outfd = creat( argv[ 1 ], 0666 );
	

	if ( outfd < 0 ) {
		perror( "ls : create outfile" );
		exit( 2 );
	}
	// Redirect output to the created utfile instead off printing to stdout 
	dup2( outfd, 1 );
	close( outfd );

	// Redirect input
	dup2( defaultin, 0 );
	
	// Redirect output to file
	dup2( outfd, 1 );

	// Redirect err
	dup2( defaulterr, 2 );

	// Create new process for "ls"
	int pid = fork();
	if ( pid == -1 ) {
		perror( "ls: fork\n");
		exit( 2 );
	}

	if (pid == 0) {
		//Child
		
		// close file descriptors that are not needed
		close(outfd);
		close( defaultin );
		close( defaultout );
		close( defaulterr );

		// You can use execvp() instead if the arguments are stored in an array
		execlp(ls, ls, "-l", (char *) 0);

		// exec() is not suppose to return, something went wrong
		perror( "ls: exec ls");
		exit( 2 );
	}

	// Restore input, output, and error

	dup2( defaultin, 0 );
	dup2( defaultout, 1 );
	dup2( defaulterr, 2 );

	// Close file descriptors that are not needed
	close( outfd );
	close( defaultin );
	close( defaultout );
	close( defaulterr );

	// Wait for last process in the pipe line
	waitpid( pid, 0, 0 );

	exit( 2 );
}
