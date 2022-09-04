// Rahul Eaga 19CS10029
// Uday Kiran 19CS10017

// Standard includes
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <conio.h>
#include <pwd.h>
#include <dirent.h>
#ifndef CTRL
#define CTRL(c) ((c) & 037)
#endif

// Max no. of characters for the command string input
#define CMD_SIZE 1000
using namespace std;


vector<string> hist;
pid_t pid = -1;
bool bg;

string ltrim(string s)
{
    string te;
    for(int i=0;i<s.size();i++)
        if(!isspace(s[i])) //Checking for space
            return s.substr(i);
    return te;
}

// Trims all the whitespaces to the right of the string
string rtrim(string s)
{
    string te;
    int n=s.size();
    for(int i=0;i<n;i++)
        if(!isspace(s[n-1-i]))
            return s.substr(0, n-i);
    return te;
}
// Function to print Current Directory.
void printDir()
{
	char cwd[1024];
	getcwd(cwd, sizeof(cwd));
	printf("\nDir: %s", cwd);
}

// Function to parse and remove spaces in a string
string removeSpace(string str)
{
	for(int i=0;i<str.size();i++){
		if(str[i]==' '){
			// If a space is found, erase that character in the string
			str.erase(str.begin()+i);
		}
	}
	return str;
}

/* 
Function to break the given string at the delimeter '|'
and return a vector containing individual commands
*/
vector<string> parsePipe(string cmd)
{
	vector<string> piped_cmds;
	string delim = "|";
	while(cmd.length()>0){
		// Find the first occurence of '|'
		int pos = cmd.find(delim);
		// If the return value of find is npos, it is the end of string, i.e. not found
		if(pos!=string::npos){
			// If '|' is found,
			piped_cmds.push_back(cmd.substr(0,pos)); 	// Push the first command in the vector,
			cmd.erase(0,cmd.find(delim)+delim.length()); // And erase the characters upto the '|' for next iteration
		}
		else break; // If on more pipes are present in the string, break the loop
	}
	piped_cmds.push_back(cmd); // Push the command after the last pipe in the vector
	return piped_cmds;
}


// Check for the quit command
int check_qut(string cmd){
	fflush(stdout);
	cmd = ltrim(rtrim(cmd));
	if(cmd == "quit")//If the given command is quit then leaving the shell
	{ 

		cout<<"Exiting the shell"<<endl;
		return 1;

	}
	
	return 0;
	
	
}

// Check if the command is a cd and change directory using chdir syscall
int check_dir(string cmd) {
    cmd = ltrim(rtrim(cmd));
    if (cmd.substr(0,2) != "cd")
    {
        return 0;
    }
    cmd.replace(0, 3, "");
    cmd.erase(remove(cmd.begin(), cmd.end(), '\"' ), cmd.end());  

    if(!cmd.size())
    {
        chdir(getenv("HOME"));    
        return 1;
    }
    cout<<cmd<<"\n";
    int status = chdir(cmd.c_str());
    if(status == -1)
    {
        cout<<"virtual-bash: cd: "+cmd+": No such file or directory\n";
        return 1;
    }
    return 1;
}


// Checking for the history
int check_hist(string cmd){
    
	int k;
	string line;
	vector<string> tempa;

    
    cmd = ltrim(rtrim(cmd));
    if(cmd != "history")
    {
        return 0;
    }


    cout<< "History:" << endl;

	ifstream file("history_storing.txt");

     while (getline(file, line))
    {
        tempa.push_back(line);
    }
    k = 1;
    
	if(tempa.size() > 1000){
		for (int i = 999; i >= 0; i--)
    	{
        	cout << "\t" << k << " . " << tempa[i] << endl;
			k++;
    	}
	}
	else{
		for (int i = tempa.size() - 1; i >= 0; i--)
    	{
        	cout << "\t" << k << " . " << tempa[i] << endl;
			k++;
    	}
		
	}
    
    return 1;
}

void add_history(string s)
{
	const string file_name = "history_storing.txt";
	fstream processed_file(file_name.c_str());
	stringstream filedata;

	filedata << s;
	filedata <<"\n";

	filedata << processed_file.rdbuf();
	processed_file.close();

	processed_file.open(file_name.c_str(),fstream::out | fstream::trunc);
	processed_file << filedata.rdbuf();
}





/* 
Function to take a string, parse it and call execvp 
to execute the command along with its arguments
*/
void execute_ext_cmd(string cmd)
{
	string orig = cmd;
	vector<string> args;
	string delim = " ";
	while(cmd.size()>0){
		// Find the first occurence of space in cmd
		int pos = cmd.find(delim);
		// If space exists,
		if(pos!=string::npos){
			try{
				// Take substring of the cmd upto the occurence of space and 
				// strip any spaces left in the command in the beginning or the end
				string argument = removeSpace(cmd.substr(0,pos)); 
				if(argument.size()!=0){
					// If is not a blank string, i.e the command didn't have only spaces in it
					args.push_back(argument); // Add the substring to list of arguments
				}
			}
			catch(...){
			}
			try{
				// Erase the substring upto that space
				cmd.erase(0,cmd.find(delim)+delim.length());
			}
			catch(...){

			}
		}
		else {
			break; // If no spaces are present, break the loop
		}
	}
	// If any argument is left over after the last space, add it to the list
	if(cmd.size()!=0){
		args.push_back(removeSpace(cmd));
	}
	// Create a array of C strings for execvp
	char** exec_args = new char* [args.size()+1];
	// Add the arguments to the array
	for(int i=0;i<args.size();i++){
		exec_args[i] = new char[args[i].size()+1];
		strcpy(exec_args[i],args[i].c_str());
	}
	   
    // Append NULL to the array of arguments
	exec_args[args.size()] = NULL;

	// Call execvp with the arguments
	execvp(exec_args[0],exec_args);
	// If execvp returns, it is an invalid command. Exit after printing the error.
	if(int pstat = 	execvp(exec_args[0],exec_args))
    {
        perror("Invalid Command");
    }
}


/*
Function to parse a command for presence of '>' or '<'
and redirecting input and outputs accordingly
*/
void parseInputOutput(string cmd)
{
	int lessThan = cmd.find("<"); 
	int greaterThan = cmd.find(">");
	// If there is no input redirection
	if(lessThan==string::npos){
		// Also no output redirection
		if(greaterThan==string::npos){
			// Simply execute the command
			execute_ext_cmd(cmd);
		}
		// If there is only output redirection
		else{
			// Separate the command from the input file name
			string exec_cmd = cmd.substr(0,greaterThan);
			string filename = cmd.substr(greaterThan+1);
			// Parse the filename to remove any leading or trailing spaces
			filename = removeSpace(filename);
			// Create/Open the file in write only mode.
			// If file exists, we need to overwrite the data in it.
			int filefd = open(filename.c_str(),O_WRONLY|O_CREAT|O_TRUNC,S_IRWXU);
			// Replace the standard output file descriptor with the output file's descriptor
			dup2(filefd,STDOUT_FILENO);
			// Execute the command
			execute_ext_cmd(exec_cmd);
		}
	}
	// There is input redirection
	else{
		// No output redirection => Only input redirection
		if(greaterThan==string::npos){
			// Extract the command and the input file name
			string exec_cmd = cmd.substr(0,lessThan);
			string filename = cmd.substr(lessThan+1);
			// Parse the filename to remove any extra spaces
			filename = removeSpace(filename);
			// Open the file in read only mode
			int fileid = open(filename.c_str(),O_RDONLY);
			// In case of invalid filename, print the error and exit.
			if(fileid<0){
				perror("Unable to open file");
				exit(0);
			}
			// Replace the standard input file descriptor with the input file's descriptor
			dup2(fileid,STDIN_FILENO);
			// Execute the command
			execute_ext_cmd(exec_cmd);
		}
		else{
			// If there are both input and output redirection
			string inputfilename,outputfilename,exec_cmd;
			// If first input file is mentioned then output,
			if(lessThan<greaterThan){
				// Extract the command, and corresponding file names
				// Parse the filenames to remove spaces
				exec_cmd = cmd.substr(0,lessThan);
				inputfilename = cmd.substr(lessThan+1,greaterThan-lessThan-1);
				inputfilename = removeSpace(inputfilename);
				outputfilename = cmd.substr(greaterThan+1);
				outputfilename = removeSpace(outputfilename);
			}
			// If first output file mentioned then input,
			else{
				// Extract the command, and corresponding file names
				// Parse the filenames to remove spaces
				exec_cmd = cmd.substr(0,greaterThan);
				outputfilename = cmd.substr(greaterThan+1,lessThan-greaterThan-1);
				inputfilename = cmd.substr(lessThan+1);
				inputfilename = removeSpace(inputfilename);
				outputfilename = removeSpace(outputfilename);
			}
			// Open the files, input file in readonly mode 
			// And output file in write only mode, create if not found
			int inputfileid = open(inputfilename.c_str(),O_RDONLY);
			int outputfileid = open(outputfilename.c_str(),O_WRONLY|O_CREAT|O_TRUNC,S_IRWXU);
			// Replace the corresponding file descriptors of standard I/O with file ids
			dup2(inputfileid,STDIN_FILENO);
			dup2(outputfileid,STDOUT_FILENO);
			// Execute the command
			execute_ext_cmd(exec_cmd);
		}
	}
}

void signal_handlr(int sig_num){
    if(sig_num = SIGINT){
        if(pid != -1){
            kill(pid, SIGKILL);
        }
    }
    if(sig_num = SIGTSTP){
        if(!bg){
            bg = true;
        }
    }
}


int main()
{
	// character array for reading input from user
	string cmd;

	cout << "\n" << endl;
    cout << "Welcome to virtual shell  "<<endl;
    hist.clear();

    signal(SIGINT, signal_handlr);
    signal(SIGTSTP, signal_handlr);

	while(true)
	{
		// Print shell prompt
        // print shell line
        //cout<< "\n";
		printDir();
		cout << " >>> ";
		cmd = "";

		// Using getline to take input with spaces at once
		//cin.getline(command,CMD_SIZE);


		char ch = getch();

		if(ch == CTRL('r')){
				cout << "Entrered the ctrl r" << endl;
				cout << "Enter search term: " << endl;
				continue;
			}

		while(ch!='\n'){
			
			if(ch >=' ' && ch <='~'){
				cout << ch;
				fflush(stdout);
				cmd.push_back(ch);
				ch = getch();
			}
			
			else if(ch =='\t'){
				string s;
				for(int i=0;i<cmd.size();i++){
					if(cmd[i] == ' ' || cmd[i] == '|' || cmd[i] == '>' || cmd[i] == '<'){
						s="";
					}
					else{
						s=s+cmd[i];
					}
				}

				int j;
				for(j=s.size()-1;j>=0;j--){
					if(s[j] == '/'){
						break;
					}
				}

				string dirr = s.substr(0,j+1);
				string file_name = s.substr(j+1,s.size());
				chdir(dirr.c_str());
				DIR *cdir = opendir(".");

				int choices = 0;
				vector<string> names;
				struct dirent *sdir;

				while((sdir = readdir(cdir)) != NULL){
					if(strlen(sdir->d_name) < file_name.size()){
						continue;
					}
					else{
						if(0 == strncmp(sdir->d_name, file_name.c_str(), file_name.size())){
							choices++;
							names.push_back(sdir->d_name);
						}
					}
				}

				if(choices == 1){
					cout << names[0].substr(file_name.size(),names[0].size());
					cmd = cmd + names[0].substr(file_name.size(), names[0].size());
				}
				else if(choices > 1){
					for(int k=0;k<choices;k++){
						cout << " \n"<< k <<" . "<< cmd << names[k].substr(file_name.size(),names[k].size());
					}
					ch = getch();
					int k; 
					k = ch-char('0');
					
					while (k>choices){
						cout << "\nEnter some number which is less than " << choices << endl;
						ch = getch();
						k = ch-char('0');
						continue;

					}

					if (k<choices){
						cmd = cmd + names[k].substr(file_name.size(), names[k].size());
						// cout << "error handling" << endl;
						cout << endl << cmd;
						
					}
					
					
				}
				ch = getch();	
				continue;

			}

			else if(ch == 127){
				if(cmd.size() > 0){
					cout<< "\b \b";	
				}
				if(cmd.size()!=0){
					cmd.pop_back();
				}
				ch = getch();
				continue;

			}

			else{
			ch = getch();
			}

		}

		cout << "\n";
		cmd = ltrim(rtrim(cmd));
        //hist.push_back(cmd);
		if(cmd.size()!=0){
			add_history(cmd);
		}
		
        
		// If the command is quit exit the shell
		if(check_qut(cmd)){
			
			exit(0);
			return 0;
		}

		// If the command is history  
        if(check_hist(cmd)){
            continue;
        }

       // If the command is cd
        if(check_dir(cmd)){
            continue;
        }

		// Flag for checking background processes
		int background=0;
		// If there is an & in the command change the flag and erase the &
		if(cmd.find("&")!=string::npos){
			background = 1;
			cmd.erase(cmd.find("&"));
		}
		// Parse the given string for '|' and break into separate commands
		vector<string> piped_cmds = parsePipe(cmd);
		// Number of pipes in the command
		int num_pipes = piped_cmds.size()-1;
		// If there are no pipes, simply execute the command
		if(num_pipes==0){
			// Create a child process
			pid_t pid = fork();
			if(pid==0){
				// Parse the command for I/O redirection and then execute it.
				parseInputOutput(piped_cmds[0]);
			}
			else{
				// Make the parent process wait only if it is not a background command
				if(!background)	
					waitpid(pid,NULL,0);
			}
				
		}
		// If there are pipes in the command given
		else{
			// Create pipes equal to no. of commands - 1
			int pipes[num_pipes][2];
			for(int i=0;i<num_pipes;i++){
				if(pipe2(pipes[i],0)<0){
					perror("");
					exit(1);
				}
			}
			// For each of the commands in the parsed list
			for(int i=0;i<=num_pipes;i++){
				// Create a child process, to parse for I/O redirection and execute it
				pid_t pid = fork();	
				if(pid == 0){
					// For first command, only redirect output to next pipe.
					if(i==0){
						dup2(pipes[i][1],STDOUT_FILENO);
						close(pipes[i][1]);
						parseInputOutput(piped_cmds[i]);
					}
					// For last command, only redirect input from the previous pipe
					else if(i==num_pipes){
						dup2(pipes[i-1][0],STDIN_FILENO);
						close(pipes[i-1][0]);
						parseInputOutput(piped_cmds[i]);
					}
					// For intermediate commands, redirect input and output
					// to read from previous pipe and write to its pipe for the next process
					else{
						dup2(pipes[i-1][0],STDIN_FILENO);
						dup2(pipes[i][1],STDOUT_FILENO);
						close(pipes[i-1][0]);
						close(pipes[i][1]);
						parseInputOutput(piped_cmds[i]);
					}
				}
				// Parent process
				else{
					// If it is not a background process,
					if(!background){
						// Wait for termination of current child process
						waitpid(pid,NULL,0);
						// For all commands except last one, close the write end of the pipe
						// as the process writing to it has terminated
						if(i!=num_pipes){
							close(pipes[i][1]);
						}
					}
					// If it is background process, make the wait call as non blocking
					else{
						// Don't wait
						// Close the write end of the pipe
						if(i!=num_pipes){
							close(pipes[i][1]);
						}
					}
				}
			}
		}
	}
}



