/** @file Pipeline.cc
 * Implementation of class Pipeline.
 */
#include <iostream>
#include <unistd.h>		// for: pipe(), fork(), dup2(), close()
#include "asserts.h"
#include "unix_error.h"
#include "Pipeline.h"
#define READ_END 0                // MYCODE
#define WRITE_END 1               // MYCODE
#include "Command.h"              // MYCODE
using namespace std;


Pipeline::Pipeline()
	: background(false)
{

}

std::vector<Command*>	commands;     // MYCODE

void	Pipeline::addCommand(Command *cp)
{
	require(cp != 0);
	commands.push_back(cp);
}


Pipeline::~Pipeline()
{
	for (vector<Command*>::iterator  i = commands.begin() ; i != commands.end() ; ++i)
		delete  *i;
}


bool	Pipeline::isEmpty()	const
{
	return commands.empty();
}


// Execute the commands in this pipeline in parallel
void	Pipeline::execute()
{
	//cerr << "Pipeline::execute\n";//DEBUG

	// Because we want the shell to wait on the rightmost process only
	// we must created the various child processes from the right to the left.
	// Also see: pipe(2), fork(2), dup2(2), dup(2), close(2), open(2).
	// And maybe usefull for debugging: getpid(2), getppid(2).
	size_t	 j = commands.size();		// for count-down
	// TODO
	for (vector<Command*>::reverse_iterator  i = commands.rbegin() ; i != commands.rend() ; ++i, --j)
	{
		Command  *cp = *i;

		//start thread
		pid_t child_pid;
		int status;

		/* get and print my pid and my parent's pid. */
		child_pid = 0;
		//maak pipe
		if (j > 1) {
			int fd[2];
			pipe(fd);
            //fork my process
			if((child_pid = fork()) < 0 ) {
				cerr << "fork failure";
				exit(1);
			}

			if(child_pid == 0) {
			    //if child_pid == 0 then I am the child and should be writing to the pipe
			    //my parent created. after i duplicated the pipe's write end to my outputstream
			    //i can close both the reading end and the writing end, otherwise my parent will never
			    //continue because it is still possible for input to enter the pipe
				dup2(fd[WRITE_END], STDOUT_FILENO);
				close(fd[0]);
				close(fd[1]);
			} else {
			    // i am the parent should do the same as my child only with my read end.
				dup2(fd[READ_END], STDIN_FILENO);
				close(fd[0]);
				close(fd[1]);

				cp->execute();				// Execute command
				notreached();
			}
		} else {
			cp->execute();
			notreached();
		}    
    
	}
}

// vim:ai:aw:ts=4:sw=4:
