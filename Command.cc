/** @file Command.cc
 * Implementation of class Command.
 */
#include <iostream>
#include <cstdlib>    // for: getenv()
#include <unistd.h>    // for: getcwd(), close(), execv(), access()
#include <limits.h>    // for: PATH_MAX
#include <fcntl.h>    // for: O_RDONLY, O_CREAT, O_WRONLY, O_APPEND
#include "asserts.h"
#include "unix_error.h"
#include "Command.h"

#include <sstream>      // MYCODE
#include <stdio.h>      // MYCODE
#include <stdlib.h>     // MYCODE
#include <sys/types.h>  // MYCODE
#include <unistd.h>     // MYCODE
#include <dirent.h>     // MYCODE
#include <string.h>     // MYCODE
#include <cstring>      // MYCODE
using namespace std;




// Iff PATH_MAX is not defined in limits.h
#ifndef  PATH_MAX
# define  PATH_MAX (4096)  /* i.e. virtual pagesize */
#endif


Command::Command()
  : append(false)
{
}



void  Command::addWord(string& word)
{
  words.push_back(word);
}


void  Command::setInput(std::string& the_input)
{
  require(input.empty());    // catch multiple inputs
  input = the_input;
}

void  Command::setOutput(std::string& the_output)
{
  require(output.empty());  // catch multiple outputs
  output = the_output;
  append = false;
}

void  Command::setAppend(std::string& the_output)
{
  require(output.empty());  // catch multiple outputs
  output = the_output;
  append = true;
}

// A true "no-nothing" command?
bool  Command::isEmpty() const
{
  return input.empty() && output.empty() && words.empty();
}


std::string Command::getCommand()
{
    if(!isEmpty())
    {
        return words[0];
    }
    return "";
}

std::string Command::getArg(int index)
{
    if(words.size() > index)
    {
        return words[index];
    }
    return "";
}


// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ===========================================================

vector<string> split(const string &str, char delimiter, vector<string> &elements)
{
    stringstream strstr(str);
    string item;
    while(getline(strstr, item, delimiter))
    {
        elements.push_back(item);
    }
    return elements;
}

vector<string> split(const string &str, char delimiter)
{
    vector<string> elements;
    split(str, delimiter, elements);
    return elements;
}

//creates the full path for the command name
std::string *getFullPath(string name)
{
    if(name[0] == '/'){
        return new string( name);
    }
    
    if( (name.size() > 1) && 
        (name[0] == '.' ) && 
        (name[1] == '/' ) )
    {
        return new string( name);
    }

    char const *tempChar = getenv("PATH");
    vector<string> dirs = split(tempChar, ':');
    vector<string>::iterator i = dirs.begin();
    bool bFound = false;

    for(i == dirs.begin(); i != dirs.end(); ++i)          // Loop through all the filesystem
    {
        DIR *directory;
        struct dirent *ent;
        if ((directory = opendir ((*i).c_str())) != NULL) // if directory is not open
        {
            while ((ent = readdir (directory)) != NULL)
            {
                string str(ent->d_name);
                if(str.compare(name) == 0)                // check if command is bFound
                {
                  bFound=true;
                  break;                                  // When found exit the While loop
                }
            }
            
            if(bFound)                                    // If command file stop loop through filesystem
            {
              cerr << "Found a file" << endl;           
              break;
            }
        }
        else
        {
            cerr << "No directory to open" << endl;
        }
    }
    
    if( i != dirs.end() )
    {
        cerr << "i != dirs.end" << endl;
        return new string ( ((*i) + '/' + name) );
    }
    throw unix_error("no existing command");
}

// Execute a command
void  Command::execute()
{
  //cerr << "Command:execute\n";//DEBUG

  // TODO:  Handle I/O redirections.
  // TODO:  Convert the words vector<string> to: array of (char*) as expected by 'execv'.
  //      NOTE: Make sure the last element of that array will be a 0 pointer!
  // TODO:  Determine the full name of the program to be executed.
  //       If the name contains a '/' it already is an full name.
  //       Otherwise it is to be searched for using the PATH
  //       environment variable.
  // TODO:  Execute the program passing the arguments array.
  // Also see: close(2), open(2), getcwd(3), getenv(3), access(2), execv(2), exit(2)

#if 0  /* DEBUG code: Set to 0 to turn off the next block of code */
  cerr <<"Command::execute ";
  // Show the I/O redirections first
  if (!input.empty())
    cerr << " <" << input;
  if (!output.empty()){
    if (append)
      cerr << " >>" << output;
    else
      cerr << " >" << output;
  }
  // Then show the command & parameters to execute
  if (words.empty())
    cerr << "\t(DO_NOTHING)";
  else
    cerr << "\t";
  for (vector<string>::iterator  i = words.begin() ; i != words.end() ; ++i)
    cerr << " " << *i;
  cerr << endl;
#endif  /* end DEBUG code */

    /*
    Basically we wait for a command, once received we search for the corresponding program to execute it
    if the program is bFound we execute the command using execv
    */

    cerr << "number of words: " << words.size() << endl;
    if (words.size() >0)
    {
        const char **argv = new const char* [words.size()+1];
        string *commandPath = getFullPath(words.front()); // Get the full path of the command to execute

        argv[0] = (char *) commandPath->c_str();
        argv[words.size() +1] = NULL;

        for (int i = 1 ; i < words.size() ; ++i)
        {
            argv[i] = (char*) words[i].c_str();
        }

        if(hasInput())
        {
            freopen((char*)input.c_str(),"r",stdin);
        }

        execv(commandPath->c_str(), (char **)argv); // execute the command
    }
    else
    {
        cerr << "No command entered" <<endl;
    }
}
// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



// vim:ai:aw:ts=4:sw=4:
