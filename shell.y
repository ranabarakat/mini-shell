
/*
 * CS-413 Spring 98
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%token	<string_val> WORD WILDCARD

%token 	NOTOKEN GREAT NEWLINE GREATGREAT LESS GREATAMPERSAND AMPERSAND PIPE CD

%union	{
		char   *string_val;
	}

%{
extern "C" 
{
	int yylex();
	void yyerror (char const *s);
}
#define yylex yylex
#include <stdio.h>
#include <unistd.h>
#include "command.h"
%}

%%

goal:	
	commands
	;

commands: 
	command
	| commands command 
	;

command: simple_command
        ;

simple_command:	
	pipe_input mod_list background NEWLINE {
		printf("   Yacc: Execute command\n");
		Command::_currentCommand.execute();
	}
	| CD WORD NEWLINE {
		if(chdir($2)>=0){
		Command::_currentCommand.prompt();
		printf("/%s ",$2);
		}
		else {
		printf("no directory exists ! \n");
		//chdir("..");
		Command::_currentCommand.prompt();
		}
		
	}
	| CD NEWLINE {
	 chdir("/home");
	}
	| NEWLINE 
	| error NEWLINE { yyerrok; }
	;

command_and_args:
	command_word arg_list {
		Command::_currentCommand.
			insertSimpleCommand( Command::_currentSimpleCommand );
			//printf("insertSimpleCommand \n");
	}
	
	;

arg_list:
	arg_list argument
	| /* can be empty */
	;
mod_list:
	mod_list iomodifier_opt
	| /* can be empty */
	;
argument:
	WORD {
               printf("   Yacc: insert argument \"%s\"\n", $1);

	       Command::_currentSimpleCommand->insertArgument( $1 );
	}
	| WILDCARD {
               printf("   Yacc: insert wildcard \"%s\"\n", $1);

		   Command::_currentSimpleCommand->_wildcard_index = Command::_currentSimpleCommand->_numberOfArguments;
	       Command::_currentSimpleCommand->insertArgument( $1 );
	}
	;

pipe_input:
	pipe_input PIPE command_and_args{
		printf("   Yacc: pipe \n");
	}
	|command_and_args;
	;
background: 
	AMPERSAND {
		//Command::_currentCommand.clear();
		//printf("reached here backgrnd");
		Command::_currentCommand._background = 1;
	}
	|
	;
command_word:
	WORD {
               printf("   Yacc: insert command \"%s\"\n", $1);
	       
	       Command::_currentSimpleCommand = new SimpleCommand();
	       Command::_currentSimpleCommand->insertArgument( $1 );
	}
	
	
	;

iomodifier_opt:
	GREAT WORD {
		printf("   Yacc: insert output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
	}
	| GREATAMPERSAND WORD {
		printf("   Yacc: insert output and error \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._errFile = $2; 
	}
	| LESS WORD {
		printf("   Yacc: insert input \"%s\"\n", $2);
		Command::_currentCommand._inputFile = $2;
	}
	| GREATGREAT WORD {
		printf("   Yacc: insert output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._append = 1;
	}
	
	;

%%

void
yyerror(const char * s)
{
	fprintf(stderr,"%s", s);
}

#if 0
main()
{
	yyparse();
}
#endif