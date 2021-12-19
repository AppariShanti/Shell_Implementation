/********************************************************************************************
This is a template for assignment on writing a custom Shell. 

Students may change the return types and arguments of the functions given in this template,
but do not change the names of these functions.

Though use of any extra functions is not recommended, students may use new functions if they need to, 
but that should not make code unnecessorily complex to read.

Students should keep names of declared variable (and any new functions) self explanatory,
and add proper comments for every logical step.

Students need to be careful while forking a new process (no unnecessory process creations) 
or while inserting the single handler code (should be added at the correct places).

Finally, keep your filename as myshell.c, do not change this name (not even myshell.cpp, 
as you not need to use any features for this assignment that are supported by C++ but not by C).
*********************************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>			// exit()
#include <unistd.h>			// fork(), getpid(), exec()
#include <sys/wait.h>		// wait()
#include <signal.h>			// signal()
#include <fcntl.h>			// close(), open()

#define MAXSIZE 100
#define MAXDIR 2000
char currentWorkingDirectory[MAXDIR];
char* Parser[MAXSIZE];
int rc;
void parseInput(char* inputLine,char** parsedInputs)
{
	// This function will parse the input string into multiple commands or a single command with arguments depending on the delimiter (&&, ##, >, or spaces).
	int i,doneParsing=0;
	for(i=0;i<MAXSIZE && !doneParsing;i++)
	{
		parsedInputs[i]=strsep(&inputLine," ");
		if(parsedInputs[i]==NULL)
			doneParsing=1;
		else if(strlen(parsedInputs[i])==0)
			i--;
	}
}
void SIGTSTP_Handler()  // for handling ctrl+Z 
{
	if (rc != 0) 
	{
	    kill(rc, SIGTSTP);
    	}
}

//changing the directory
void changeDirectory(char* inputLine)
{
	char* pd=getcwd(currentWorkingDirectory,1000);
	size_t len=strlen(pd);
	int j=len-1,found=0;
	char* p[MAXSIZE];
	int i;
	int doneParsing=0;
	for(i=0;i<MAXSIZE && !doneParsing;i++)
	{
		p[i]=strsep(&inputLine," ");
		if(p[i]==NULL)
		{
			doneParsing=1;       		// reached end of inputLine
		}
		else if(strlen(p[i])==0)	// for ignoring the empty spaces in between
		{
			i--;
		}
	}
	while(j>=0 && !found)
	{
		if(pd[j]!='/')
		{
			pd[j]='\0';
			j--;
		}
		else
		{
			found=1;
		}	
	}
		
	if(strcmp(p[0],"cd")==0)
	{
		if(strcmp(p[1],"..")==0 || strlen(p[1])==0 || p[1]==NULL)
		{
			//go to parent directory
			int changed=chdir(pd);
		}
		else if(p[1]!=NULL)
		{
			int changed=chdir(p[1]);
			//if directory didn't change no such directory present
			if(changed!=0)
			{
				printf("bash: cd: %s: No such file or directory\n",p[1]);
			}
		}
	}
	else if(strncmp(p[0],"cd",2)==0)
	{
		printf("Shell: Incorrect command\n");
	}
}
void executeCommand(char** parsedInputs,int type)
{
	// This function will fork a new process to execute a command
	
	int status,wc1,w2,w3,w;
	if(type==0)
	{
		w=waitpid(rc, NULL, WUNTRACED | WCONTINUED );
	}
	rc=fork();
	if(rc<0)
	{
		fprintf(stderr,"fork failed\n");
		exit(1);
	}
	else if(rc==0)
	{
		signal(SIGINT,SIG_DFL);			//restore ctrl+c innterupt signal to terminate the child process 
		execvp(parsedInputs[0],parsedInputs);
		printf("Shell: Incorrect command\n");
		exit(0);
	}
	else
	{
		if(type==1)				// for sequential execution
		{
			wc1=wait(NULL);
		}
	}
}
void executeCommandFinal(char* inputLine)
{
	// This function will fork a new process to execute a command

	char* Commands[MAXSIZE];
	int i,j,k,l;
	int doneParsing=0;
	for(i=0;i<MAXSIZE && !doneParsing;i++)
	{
		Commands[i]=strsep(&inputLine," ");
		if(Commands[i]==NULL)
		{
			doneParsing=1;       		// reached end of inputLine
		}
		else if(strlen(Commands[i])==0)	// for ignoring the empty spaces in between
		{
			i--;
		}
	}
	rc=fork();
	int status,wc1;
	if(rc<0)
	{
		fprintf(stderr,"fork failed\n");
		exit(1);
	}
	else if(rc==0)
	{
		signal(SIGINT,SIG_DFL);
		execvp(Commands[0],Commands);
		printf("Shell: Incorrect command\n");
		exit(0);
	}
	else
	{
		int wc=waitpid(rc, NULL, WUNTRACED |WCONTINUED );
		return;
	}
	
}
void executeParallelCommands(char* inputLine)
{
	// This function will run multiple commands in parallel
	char* parallelCommands[MAXSIZE];
	int i,j,k,l;
	int doneParsing=0;
	
	// parsing for "&&" 
	for(i=0;i<MAXSIZE && !doneParsing;i++)
	{
		parallelCommands[i]=strsep(&inputLine,"&&");
		if(parallelCommands[i]==NULL)
		{
			doneParsing=1;       		// reached end of inputLine
		}
		else if(strlen(parallelCommands[i])==0)	// for ignoring the empty spaces in between
		{
			i--;
		}
	}
	k=i;
	i=0;
	// again parsing the parallelCommands to get individual commands and arguments
	for(l=0;l<k;l++)
	{
		char* finalCommands[MAXSIZE];
		char* p=parallelCommands[l];
		int done=0;
		for(i=0;i<MAXSIZE && !done;i++)
		{
			finalCommands[i]=strsep(&p," ");
			if(finalCommands[i]==NULL)
			{
				done=1;
			}
			else if(strlen(finalCommands[i])==0)
			{
				i--;
			}
		}
		rc=fork();
		int status;
		if(rc<0)
		{
			fprintf(stderr,"fork failed\n");
			exit(1);
		}
		else if(rc==0)
		{
			signal(SIGINT,SIG_DFL);
			executeCommand(finalCommands,0);
			exit(0);
			//int wc1=waitpid(rc, NULL, WUNTRACED | WCONTINUED );
		}
		else
		{
			int wc1=waitpid(rc, NULL, WUNTRACED | WCONTINUED );
			//int wc2=wait(NULL);
		}
	}
	return;
}

void executeSequentialCommands(char* inputLine)
{	
	// This function will run multiple commands in parallel
	char* sequentialCommands[MAXSIZE];
	int i,j,k,l;
	int doneParsing=0;
	
	//parsing for "##"
	for(i=0;i<MAXSIZE && !doneParsing;i++)
	{
		sequentialCommands[i]=strsep(&inputLine,"##");
		if(sequentialCommands[i]==NULL)
		{
			doneParsing=1;       			// reached end of inputLine
		}
		else if(strlen(sequentialCommands[i])==0)	// for ignoring the empty spaces in between
		{
			i--;
		}
	}
	k=i;
	i=0;
	
	// again parsing sequential commands to get individual commands and arguments
	for(l=0;l<k;l++)
	{
		char* finalCommands[MAXSIZE];
		char* p=sequentialCommands[l];
		int done=0;
		for(i=0;i<MAXSIZE && !done;i++)
		{
			finalCommands[i]=strsep(&p," ");
			if(finalCommands[i]==NULL)
			{
				done=1;
			}
			else if(strlen(finalCommands[i])==0)
			{
				i--;
			}
		}
		//rc=fork();
		//int status;
		//if(rc<0)
		//{
		//	fprintf(stderr,"fork failed\n");
		//	exit(1);
		//}
		//else if(rc==0)
		//{
			//signal(SIGINT,SIG_DFL);
			//if(strcmp(finalCommands[0],"cd")==0)
			//	changeDirectory(p);
			//else
				executeCommand(finalCommands,1);
				//printf("Shell: Incorrect command\n");
			//exit(0);
		//}
		//else
		//{
			//int wc1=wait(NULL);
			
	//	}
	}
	
}

void executeCommandRedirection(char* inputLine)
{
	// This function will run a single command with output redirected to an output file specificed by user
	char* parsedInputs[MAXSIZE];
	int i,doneParsing=0;

	// parsing for ">" 
	for(i=0;i<MAXSIZE && !doneParsing;i++)
	{
		parsedInputs[i]=strsep(&inputLine,">");
		if(parsedInputs[i]==NULL)
			doneParsing=1;
		else if(strlen(parsedInputs[i])==0)
			i--;
	}
	doneParsing=0;	
	
	// again parsing for induividual command and its arguments
	char* command[MAXSIZE];
	for(i=0;i<MAXSIZE && !doneParsing;i++)
	{
		command[i]=strsep(&parsedInputs[0]," ");
		if(command[i]==NULL)
			doneParsing=1;
		else if(strlen(command[i])==0)
			i--;
	}
	doneParsing=0;

	//parsing for proper file name
	char* filename[MAXSIZE];
	for(i=0;i<MAXSIZE && !doneParsing;i++)
	{
		filename[i]=strsep(&parsedInputs[1]," ");
		if(filename[i]==NULL)
			doneParsing=1;
		else if(strlen(filename[i])==0)
			i--;
	}
	
	rc=fork();
	if(rc<0)
	{
		fprintf(stderr,"fork failed\n");
		exit(1);
	}
	else if(rc==0)
	{
		signal(SIGINT,SIG_DFL);
		close(STDOUT_FILENO);
		//opening/created new file for output redirection
		open(filename[0],O_CREAT | O_WRONLY | O_TRUNC | S_IRWXU ,0666);
		execvp(command[0],command);
		printf("Shell: Incorrect command\n");
		exit(0);
	}
	else
	{
		int wc=waitpid(rc, NULL, WUNTRACED | WCONTINUED );
	}
}

int findCommandType(char* inputLine)
{
	// finding type of input command got on terminal
	int l=strlen(inputLine);
	int i,flag=0;			//default flag for space separated command - normal execution
	for(i=1;inputLine[i]!='\0';i++)
	{
		if(inputLine[i]=='&' && inputLine[i-1]=='&') 	  // for parallel execution
			flag=1;
		else if(inputLine[i]=='#' && inputLine[i-1]=='#') //for sequential execution 
			flag=2;
		else if(inputLine[i]=='>')			  //for output redirection
			flag=3;
	}
	return flag;
}


int main()
{
	// Initial declarations
	signal(SIGINT,SIG_IGN);
	signal(SIGTSTP,SIGTSTP_Handler);
	while(1)	// This loop will keep your shell running until user exits.
	{
		char* inputLine = NULL;
		char* parsedInputs[MAXSIZE];
         	size_t len = 0;
		// Print the prompt in format - currentWorkingDirectory$
		printf("%s$",getcwd(currentWorkingDirectory,2000));

		// accept input with 'getline()'
		getline(&inputLine,&len,stdin);
		int l=strlen(inputLine);
		inputLine[l-1]='\0';
		// Parse input with 'strsep()' for different symbols (&&, ##, >) and for spaces.
		//parseInput(inputLine,parsedInputs);
		if(strncmp(inputLine,"cd",2)==0)
		{
			changeDirectory(inputLine);
		}
		else if(strcmp(inputLine,"exit")==0)	// When user uses exit command.
		{
			printf("Exiting shell...\n");
			break;
		}
		else
		{
			
			int flag=findCommandType(inputLine);
			if(flag==1)
				executeParallelCommands(inputLine);		// This function is invoked when user wants to run multiple commands in parallel (commands separated by &&)
			else if(flag==2)
				executeSequentialCommands(inputLine);			// This function is invoked when user wants to run multiple commands sequentially (commands separated by ##)
			else if(flag==3)
				executeCommandRedirection(inputLine);			// This function is invoked when user wants redirect output of a single command to and output file specificed by user
			else
				executeCommandFinal(inputLine);				// This function is invoked when user wants to run a single commands
		}
				
	}
	
	return 0;
}


