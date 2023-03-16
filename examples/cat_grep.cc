
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

const char * usage = ""
"Usage:\n"
"    cat_grep file-name word \n"
"\n"
"    It does something similar to the shell command:\n"
"        csh> cat file | grep word \n"
"\n"
"Example:\n"
"    cat_grep filename keyword \n";

const char *cat = "cat";
const char *grep = "grep";

int
main(int argc, char **argv, char **envp)
{
	if (argc < 3) {
		fprintf(stderr, "%s", usage );
		exit(1);
	}

	// Save default input, output, and error because we will
	// change them during redirection and we will need to restore them
	// at the end.
	// The dup() system call creates a copy of a file descriptor.
	int defaultin = dup( 0 ); // Default file Descriptor for stdin
	int defaultout = dup( 1 ); // Default file Descriptor for stdout
	int defaulterr = dup( 2 ); // Default file Descriptor for stderr

	//////////////////  cat //////////////////////////

	// Input:    defaultin
	// Output:   pipe
	// Error:    defaulterr

	// Create new pipe 
	// Conceptually, a pipe is a connection between two processes, 
	// such that the standard output from one process becomes the standard input of the other process.
	// so if a process writes to fdpipe[1] process be can read from fdpipe[0] 
	int fdpipe[2];
	if ( pipe(fdpipe) == -1) {
		perror( "cat_grep: pipe");
		exit( 2 );
	}

	// Redirect input (use sdtin)
	dup2( defaultin, 0 );
	
	// Redirect output to pipe (write the output to pipefile[1] instead od stdout)
	dup2( fdpipe[ 1 ], 1 );

	// Redirect err (use stderr)
	dup2( defaulterr, 2 );

	// Create new process for "cat"
	int pid = fork();
	if ( pid == -1 ) {
		perror( "cat_grep: fork\n");
		exit( 2 );
	}

	if (pid == 0) {
		//Child
		
		// close file descriptors that are not needed
		close(fdpipe[0]);
		close(fdpipe[1]);
		close( defaultin );
		close( defaultout );
		close( defaulterr );

		// You can use execvp() instead if the arguments are stored in an array
		execlp(cat, cat, argv[1], (char *) 0);

		// exec() is not suppose to return, something went wrong
		perror( "cat_grep: exec cat");
		exit( 2 );
	}
	printf("test");
	//////////////////  grep //////////////////////////

	// Input:    pipe
	// Output:   outfile
	// Error:    defaulterr

	// Redirect input for grep feed it with the output of cat which is writen by the child in fdpipe[1] 
	// so parent should read it from fdpipe[0]
	dup2( fdpipe[0], 0);
	
	// Redirect Output to the Default (stdout)
	dup2( defaultout ,1);
	
	// Redirect err
	dup2( defaulterr, 2 );

	pid = fork();
	if (pid == -1 ) {
		perror( "cat_grep: fork");
		exit( 2 );
	}
	
	if (pid == 0) {
		//Child

		// close file descriptors that are not needed
		close(fdpipe[0]);
		close(fdpipe[1]);
		close( defaultin );
		close( defaultout );
		close( defaulterr );
		
		// You can use execvp() instead if the arguments are stored in an array
		execlp(grep, grep , argv[2], (char *) 0);

		// exec() is not suppose to return, something went wrong
		perror( "cat_grep: exec grep");
		exit( 2 );

	}

	// Restore input, output, and error

	dup2( defaultin, 0 );
	dup2( defaultout, 1 );
	dup2( defaulterr, 2 );

	// Close file descriptors that are not needed
	close(fdpipe[0]);
	close(fdpipe[1]);
	close( defaultin );
	close( defaultout );
	close( defaulterr );

	// Wait for last process in the pipe line
	waitpid( pid, 0, 0 );

	exit( 2 );
}
