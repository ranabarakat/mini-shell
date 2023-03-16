
/*
 * CS354: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <glob.h>
#include <sys/resource.h>

#include "command.h"

SimpleCommand::SimpleCommand()
{
	// Creat available space for 5 arguments
	_numberOfAvailableArguments = 5;
	_numberOfArguments = 0;
	_wildcard_index = -1;
	_arguments = (char **) malloc( _numberOfAvailableArguments * sizeof( char * ) );
}

void
SimpleCommand::insertArgument( char * argument )
{
	if ( _numberOfAvailableArguments == _numberOfArguments  + 1 ) {
		// Double the available space
		_numberOfAvailableArguments *= 2;
		_arguments = (char **) realloc( _arguments,
				  _numberOfAvailableArguments * sizeof( char * ) );
	}
	
	_arguments[ _numberOfArguments ] = argument;

	// Add NULL argument at the end
	_arguments[ _numberOfArguments + 1] = NULL;
	
	_numberOfArguments++;
}

Command::Command()
{
	// Create available space for one simple command
	_numberOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)
		malloc( _numberOfSimpleCommands * sizeof( SimpleCommand * ) );

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
	_append = 0;
}

void
Command::insertSimpleCommand( SimpleCommand * simpleCommand )
{
	if ( _numberOfAvailableSimpleCommands == _numberOfSimpleCommands ) {
		_numberOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **) realloc( _simpleCommands,
			 _numberOfAvailableSimpleCommands * sizeof( SimpleCommand * ) );
	}
	
	_simpleCommands[ _numberOfSimpleCommands ] = simpleCommand;
	_numberOfSimpleCommands++;
}

void
Command:: clear()
{
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		for ( int j = 0; j < _simpleCommands[ i ]->_numberOfArguments; j ++ ) {
			free ( _simpleCommands[ i ]->_arguments[ j ] );
		}
		
		free ( _simpleCommands[ i ]->_arguments );
		free ( _simpleCommands[ i ] );
	}

	if ( _outFile ) {
		free( _outFile );
	}

	if ( _inputFile ) {
		free( _inputFile );
	}

	if ( _errFile ) {
		free( _errFile );
	}

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
	_append = 0;
}

void
Command::print()
{
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");
	
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		printf("  %-3d ", i );
		for ( int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++ ) {
			printf("\"%s\" \t", _simpleCommands[i]->_arguments[ j ] );
		}
	}

	printf( "\n\n" );
	printf( "  Output       Input        Error        Background\n" );
	printf( "  ------------ ------------ ------------ ------------\n" );
	printf( "  %-12s %-12s %-12s %-12s\n", _outFile?_outFile:"default",
		_inputFile?_inputFile:"default", _errFile?_errFile:"default",
		_background?"YES":"NO");
	printf( "\n\n" );
	
}
void sighandler(int sig_num){
	// signal(SIGINT, sighandler);
	printf("\n");
	Command::_currentCommand.clear();
	Command::_currentCommand.prompt();
	fflush(stdout);
}

void proc_exit(int n)
{
		FILE *file = fopen("logfile.log", "a");
		time_t t;
		time(&t);
		fprintf(file, "child process died at %s\n", ctime(&t));
		fclose(file);
}


void
Command::execute()
{
	// Don't do anything if there are no simple commands
	if ( _numberOfSimpleCommands == 0 ) {
		prompt();
		return;
	}
	// exit the shell
	if(strcmp(_simpleCommands[0]->_arguments[0],"exit")==0){
		printf("Good bye!!\n");
		exit(0);
	}


		
	// Print contents of Command data structure
	print();

	int defaultin = dup( 0 ); // Default file Descriptor for stdin
	int defaultout = dup( 1 ); // Default file Descriptor for stdout
	int defaulterr = dup( 2 ); // Default file Descriptor for stderr
	int fdpipe[2];
	int fd_in,fd_out,fd_err;
	pid_t pid;

	if(_inputFile)
		fd_in = open(_inputFile,O_RDONLY);
	else
		fd_in = dup(defaultin);
	for(int i = 0; i < _numberOfSimpleCommands; i++ ) {

		//for every new command, issue its input feed from fd_in
		dup2(fd_in, 0); 
		close(fd_in);

			if(i < _numberOfSimpleCommands-1) { // not last command, redirect output to fdpipe[1], get input from fdpipe[0]
				pipe(fdpipe);
				fd_in = fdpipe[0];
				fd_out = fdpipe[1];
			}

			else {  // last command
				if(_append)
					fd_out = open( _outFile, O_RDWR|O_APPEND|O_CREAT); //readwrite, append and create if file doesn't exist
				else if( _outFile)
					fd_out = open( _outFile,O_RDWR|O_TRUNC|O_CREAT); //readwrite, truncate to overwrite, create if it doesn't exist
				else
					fd_out = dup( defaultout ); //write to standard output

				if( _errFile)
					fd_err = dup(fd_out);
				else
					fd_err = dup(defaulterr); 

				dup2(fd_err, 2); //fd_err is the new std err
				close(fd_err);
			}

			dup2(fd_out,1);  //fd_out is the new std out
			close(fd_out);
		/*----------------------------------------------------BONUS------------------------------------------------------------------*/
		if(_simpleCommands[i]->_wildcard_index>=0){
			int idx = _simpleCommands[i]->_wildcard_index;
			glob_t g;
			g.gl_offs = idx;
			glob(_simpleCommands[i]->_arguments[idx], GLOB_ERR, NULL, &g);
			for(int x=0;x<idx;x++){
				g.gl_pathv[x] = _simpleCommands[i]->_arguments[x];
			}
			_simpleCommands[i]->_arguments = g.gl_pathv;
		}
		/*--------------------------------------------------END BONUS---------------------------------------------------------------*/

			//change directory
		/*if(strcmp(_simpleCommands[0]->_arguments[0],"cd")==0){
			int x;
			if (_simpleCommands[0]->_numberOfArguments>1){
				if(chdir(_simpleCommands[0]->_arguments[1])){
					printf("Directory does not exist!\n");
				}
			}else{
				chdir(getenv("HOME"));
			}
			clear();
			prompt();
			return;
		}*/
			pid = fork(); 
			if(pid == -1){ //no child spawned
				perror("fork failed\n");
				exit(2);
			} 
			else if (pid == 0){
				execvp( _simpleCommands[i]->_arguments[0], _simpleCommands[i]->_arguments );
				perror("execvp failed\n"); //execvp doesn't return, reaching this line means an error happened	
				exit(2);
		}
		
	}
	//retrieve default standard input, output and error to prepare for next command
	dup2(defaultin, 0);
	dup2(defaultout, 1);
	dup2(defaulterr, 2);
	close(defaultin);
	close(defaultout);
	close(defaulterr);

	if (!_background){
		waitpid( pid, 0, 0 );
	}
	// Clear to prepare for next command
	clear();
	// Print new prompt
	prompt();

}

// Shell implementation

void
Command::prompt()
{
	printf("myshell>");
	fflush(stdout);
}

Command Command::_currentCommand;
SimpleCommand * Command::_currentSimpleCommand;

int yyparse(void);

int 
main()
{
	signal (SIGCHLD, proc_exit);
	signal(SIGINT, sighandler);
	Command::_currentCommand.prompt();
	yyparse();
	return 0;
}
