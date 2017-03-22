/** @file Sequence.cc
 * Implementation of class Sequence.
 */
#include <iostream>
#include <sys/wait.h>    // for: wait(), WIF...(status)
#include <unistd.h>      // for: fork(), nice()
#include <fcntl.h>      // for: O_RDONLY, O_CREAT, O_WRONLY, O_APPEND
#include <signal.h>      // for: signal(), SIG*
#include <cstring>      // for: strsignal()
#include "asserts.h"
#include "unix_error.h"
#include "Sequence.h"
#include <signal.h>     // MY CODE - pid_t

using namespace std;


void  Sequence::addPipeline(Pipeline* pp)
{
  require(pp != 0);
  commands.push_back(pp);
}


Sequence::~Sequence()
{
  for(vector<Pipeline*>::iterator  i = commands.begin() ; i != commands.end() ; ++i)
    delete  *i;
}


bool  Sequence::isEmpty()  const
{
  return commands.empty();
}


pid_t Sequence::currentCommand = 0;


// TODO:  Optional:
//      Lookout somewhere for special commands such as 'exit',
//      'logout', 'cd', etc, which may have to be
//      done by the original shell process itself.

// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Execute the pipelines in this sequence one by one
void  Sequence::execute()
{
  //cerr << "Sequence::execute\n";//DEBUG

  // Execute each pipeline in turn.
  // Also see: fork(2), nice(2), wait(2), WIF...(2), strsignal(3)
  size_t  j = commands.size();      // for count-down
  for (vector<Pipeline*>::iterator  i = commands.begin() ; i != commands.end() ; ++i, --j) {
    Pipeline  *pp = *i;
    if ( ! pp->isEmpty() )
    {
      if(pp->getFirstCommand()->getCommand() == "cd")
      {
        if(chdir(pp->getFirstCommand()->getArg(1).c_str()) == 0)
        {
            char cCurrentPath[4096]; 
            getcwd(cCurrentPath, sizeof(cCurrentPath));
            cout << "directory is now: " << cCurrentPath << "\n";
        }
        else
        {
            cerr << "Unable to change directory\n";
        }
      }
      else if(pp->getFirstCommand()->getCommand() == "pwd")
      {
          char cCurrentPath[4096];
          getcwd(cCurrentPath, sizeof(cCurrentPath));
          cout << "Directory is now: " << cCurrentPath << "\n";
      }
      else
      {
        int status = 0;
        currentCommand = 0;

        // Fork and save the ID of child as currentCommand
        if((currentCommand = fork()) < 0 )
        {
          cerr << "fork failure";
        }

        if(currentCommand == 0)
        {
          // i am child should execute command
          if(pp->isBackground())
          {
              // lower priority of this process
              nice(10);
              close(0);
          }
          else
          {
              signal(SIGINT, SIG_DFL);
              signal(SIGQUIT, SIG_DFL);
          }

          pp->execute();
        }
        else
        {
          if( ! pp->isBackground())
          {
              // i am parrent should wait till command is
              // executed
              waitpid(currentCommand,&status,0);
          }

          //command executed no current command
          currentCommand = 0;
        }

        if(status != 0) {
          cerr << "ERROR code: " << status << "\n";
        }

      }
    }
  }
}
// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// vim:ai:aw:ts=4:sw=4:
