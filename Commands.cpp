#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include <fcntl.h>
#include "Commands.h"

using namespace std;

const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY()  \
  cerr << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cerr << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

#define DEBUG_PRINT cerr << "DEBUG: "

#define EXEC(path, arg) \
  execvp((path), (arg));

string _ltrim(const std::string& s)
{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
    return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for(std::string s; iss >> s; ) {
        args[i] = (char*)malloc(s.length()+1);
        memset(args[i], 0, s.length()+1);
        strcpy(args[i], s.c_str());
        args[++i] = NULL;
    }
    return i;

    FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
    const string str(cmd_line);
    // find last character other than spaces
    unsigned int idx = str.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == string::npos) {
        return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&') {
        return;
    }
    // replace the & (background sign) with space and then remove all tailing spaces.
    cmd_line[idx] = ' ';
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}


SmallShell::SmallShell() {
// TODO: add your implementation
}


/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line, char* smash_prompt) {
    string cmd_s = string(cmd_line);
    if(cmd_s.find("chprompt") == 0){
        return new chprompt(cmd_line, &smash_prompt);
    } else if(cmd_s.find("showpid") == 0){
        return new ShowPidCommand(cmd_line);
    }
    else if(cmd_s.find("pwd") == 0){
        return new GetCurrDirCommand(cmd_line);
    }
    else if(cmd_s.find("cd") == 0){
        return  new ChangeDirCommand(cmd_line, &plast);
    }
    else if(cmd_s.find("jobs") == 0){
        return new JobsCommand(cmd_line, &jobslist);
    }
    else if (cmd_s.find("kill") == 0){
        return new KillCommand(cmd_line, &jobslist);
    }
    else if(cmd_s.find("fg") == 0){
        return new ForegroundCommand(cmd_line, &jobslist);
    }
    else if(cmd_s.find("bg") == 0){
        return new BackgroundCommand(cmd_line, &jobslist);
    }
    else if(cmd_s.find("quit") == 0){
        return new QuitCommand(cmd_line, &jobslist);
    }
//    else if(cmd_s.find("pipe") == 0){
//        return  new PipeCommand(cmd_line);
//    }
    else{
        return new ExternalCommand(cmd_line);
    }
    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line, char* smash_prompt) {
    // TODO: Add your implementation here
     Command* cmd = CreateCommand(cmd_line, smash_prompt);
     cmd->execute();
}


// TODO: Add your implementation for classes in Commands.h

void free_args(char* args [], int command_len){
    for (int i = 0;i<command_len;i++) {
        free(args[i]);
    }
}


//////checking if a string is a number/////
bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}


///command///
Command::Command(const char* cmd_line): pid(-1) {

    if (_isBackgroundComamnd(cmd_line))
        state = Background;
    else
        state = Foregroung;

    this->cmd_line = (char *) malloc(sizeof(char) * strlen(cmd_line) + 1);
    strcpy(this->cmd_line, cmd_line);
}

Command::~Command() {
    free(cmd_line);
}

char * Command::get_cmd_line() {
    return this->cmd_line;
}

void Command::set_pid(pid_t new_pid) {
    this->pid = new_pid;
    return;
}

pid_t Command::get_pid() {
    return this->pid;
}

void Command::set_state(State new_state) {
    this->state = new_state;
    return;
}

State Command::get_state() {
    return this->state;
}



///chprompt
void chprompt::execute() {
    string str = string(this->get_cmd_line(),strlen(this->get_cmd_line())+1);
    char *args[COMMAND_MAX_ARGS];
    int command_len = _parseCommandLine(str.c_str(), args);

    if(command_len > 1){
        strcpy(*prompt_, args[1]);///////////just args[1]???
    }
    free_args(args, command_len);
    return;
}

///showpid///
void ShowPidCommand::execute() {
    cout << "smash pid is " << getpid() << endl;
}


///pwd///
void GetCurrDirCommand::execute() {
    char cwd[1024];
    if(getcwd(cwd, sizeof(cwd)) == NULL){
        perror("smash error: getcwd failed");
    }
    cout << cwd <<endl;
    return;
}



///cd
void ChangeDirCommand::execute() {
    string str = string(this->get_cmd_line(),strlen(this->get_cmd_line())+1);
    char *args[COMMAND_MAX_ARGS];
    int command_len = _parseCommandLine(str.c_str(), args);

    if (command_len > 2) {
        cerr << "smash error: cd: too many arguments" << endl;
        free_args(args, command_len);
        return;
    } else{
        if (strcmp(args[1], "-") == 0) {
            if (*plast == nullptr) {
                cerr << "smash error: cd: OLDPWD not set" << endl;
                free_args(args, command_len);
                return;
            }
            //plast not null
            else {
                char* new_path = (char *) malloc(sizeof(char) * 1024);
                getcwd(new_path, 1024);
                if (chdir(*plast) != 0) {
                    cerr << "smash error: chdir failed" << endl;
                } else {
                    strcpy(*plast, new_path);
                }
                free(new_path);
            }
        }else if(strcmp(args[1], "..") == 0){
            char* new_path = (char *) malloc(sizeof(char) * 1024);
            getcwd(new_path, 1024);
            if ( chdir("/home") != 0) {
                perror("smash error: chdir failed");
                free_args(args, command_len);
                return;
            } else {
                if(*plast == nullptr){
                    *plast = (char*)malloc(sizeof(char)*1024);
                }
                strcpy(*plast, new_path);
            }
            free(new_path);
        }
        else {
            char* new_path = (char *) malloc(sizeof(char) * 1024);
            getcwd(new_path, 1024);
            if ( chdir(args[1]) != 0) {
                perror("smash error: chdir failed");
                free_args(args, command_len);
                return;
            } else {
                if(*plast == nullptr){
                    *plast = (char*)malloc(sizeof(char)*1024);
                }
                strcpy(*plast, new_path);
            }
            free(new_path);
        }
    }
    free_args(args, command_len);
}


//////jobs//////
void JobsList::addJob(Command *cmd, bool isStopped) {
    if (!cmd) return;
    removeFinishedJobs();

    if (isStopped){
        cmd->set_state(Stopped);
    }
    else{
        cmd->set_state(Background);
    }

    int new_id = ((this->jobs).back().get_job_id()) + 1;
    JobEntry new_job = JobEntry(cmd,new_id);
    this->jobs.push_back(new_job);
}

void JobsList::printJobsList() {
    removeFinishedJobs();

    time_t time2;
    if(time(&time2) == (time_t)-1){
        perror("smash error: time failed");
    }

    for(size_t i = 0; i < (this->jobs).size(); ++i){
        double elapsed_time = difftime(time2, jobs[i].get_time());

        if (jobs[i].get_cmd()->get_state() == Stopped){

            cout << "[" << jobs[i].get_job_id() << "] "<< jobs[i].get_cmd()->get_cmd_line()
                 << " : " << jobs[i].get_cmd()->get_pid() << " " << elapsed_time << "secs (stopped)" << endl;
        }
        else{
            cout << "[" << jobs[i].get_job_id() << "] "<< jobs[i].get_cmd()->get_cmd_line()
                 << " : " << jobs[i].get_cmd()->get_pid() << " " << elapsed_time << " secs" << endl;
        }
    }
}


JobsList::JobEntry* JobsList::getJobById(int jobId) {
    removeFinishedJobs();

    for (size_t i = 0; i < (this->jobs).size(); ++i) {
        if (jobs[i].get_job_id() == jobId){
            return &(jobs[i]);
        }
    }
}

void JobsList::removeJobById(int jobId) {
    removeFinishedJobs();

    for (size_t i = 0; i < (this->jobs).size(); ++i) {
        if (jobs[i].get_job_id() == jobId){
            jobs.erase(jobs.begin() + i);
            return;
        }
    }
}

JobsList::JobEntry* JobsList::getLastJob(int *lastJobId) {
    removeFinishedJobs();

    if (!(jobs.empty())) {
        *lastJobId = jobs.back().get_job_id();
        return &(jobs.back());
    }
    *lastJobId = 0;
    return NULL;
}

JobsList::JobEntry* JobsList::getLastStoppedJob(int *jobId) {
    removeFinishedJobs();

    for (size_t i = (this->jobs).size()-1; i >= 0; --i) {
        if (jobs[i].get_cmd()->get_state() == Stopped){
            *jobId = jobs[i].get_job_id();
            return  &(jobs[i]);
        }
    }
    //if the list is empty || there's no stopped job
    *jobId = 0;
    return NULL;
}

bool JobsList::is_job_exist(int id) {
    removeFinishedJobs();

    for (size_t i = 0; i < (this->jobs).size(); ++i) {
        if (jobs[i].get_job_id() == id) {
            return true;
        }
    }

    return false;
}

void JobsList::killAllJobs() {

    removeFinishedJobs();

    for (size_t i = 0; i < (this->jobs).size(); ++i) {
        int ret = kill(jobs[i].get_cmd()->get_pid(), SIGKILL);
        if (ret != 0) {
            perror("smash error: kill failed");
        }
    }
}

void JobsList::print_before_quit() {
    int size = jobs.size();
    cout << "smash: sending SIGKILL signal to " << size << " jobs:" << endl;
    for (int i = 0; i < size; i++) {
        pid_t pid = jobs[i].get_cmd()->get_pid();
        const char* cmd_line = jobs[i].get_cmd()->get_cmd_line();
        cout << pid << ": " << cmd_line << endl;
    }
}

void JobsList::removeFinishedJobs() {
    for(int i=0;i<jobs.size();i++) {

        int waitpid_res = waitpid(jobs[i].get_cmd()->get_pid() ,NULL ,WNOHANG);
         if (waitpid_res >0) {
            jobs.erase(jobs.begin()+i);
            i--;
        }
    }
}

bool JobsList::isEmpty() {
    return jobs.empty();
}

void JobsCommand::execute() {
    jobs->removeFinishedJobs();

    string str = string(this->get_cmd_line(),strlen(this->get_cmd_line())+1);
    char *args[COMMAND_MAX_ARGS];
    int command_len = _parseCommandLine(str.c_str(), args);

    //no need to check command length since it ignores other arguments
    jobs->printJobsList();
    free_args(args, command_len);
}


///kill command
void KillCommand::execute() {
    jobs->removeFinishedJobs();

    string str = string(this->get_cmd_line(),strlen(this->get_cmd_line())+1);
    char *args[COMMAND_MAX_ARGS];
    int command_len = _parseCommandLine(str.c_str(), args);
    //check arguments_number
    if (command_len != 3){
        cout << "smash error: kill: invalid arguments" << endl;
        free_args(args, command_len);
        return;
    }
    //check arguments_format
    if (!is_number(args[1]) || !is_number(args[2])){
        cout << "smash error: kill: invalid arguments" << endl;
        free_args(args, command_len);
        return;
    }

    //converting to an int
    stringstream str1(args[1]);
    int signum = 0;
    str1 >> signum;

    stringstream str2(args[2]);
    int job_id =0;
    str2 >> job_id;

    //check if job-id exists
    if (!(jobs->is_job_exist(job_id))){
        cout << "smash error: kill: job-id " << job_id<< " does not exist" << endl;
        free_args(args, command_len);
        return;
    }

    JobsList::JobEntry* jobEntry = jobs->getJobById(job_id);
    int ret = kill(jobEntry->get_cmd()->get_pid(), signum);

    if (ret != 0) {
        perror("smash error: kill failed");
    }
    else{
        cout << "signal number " << signum << " was sent to pid " << job_id << endl;
    }
    free_args(args,command_len);
}


///fg command
void ForegroundCommand::execute() {
    jobs->removeFinishedJobs();

    string str = string(this->get_cmd_line(), strlen(this->get_cmd_line()) + 1);
    char *args[COMMAND_MAX_ARGS];
    int command_len = _parseCommandLine(str.c_str(), args);

    //check arguments_number
    if (command_len > 2) {
        cout << "smash error: fg: invalid arguments" << endl;
        free_args(args,command_len);
        return;
    }

    int id=-1;

    //if an argument was sent
    if (command_len == 2) {
        //check format
        if (!(is_number(args[1]))) {
            cout << "smash error: fg: invalid arguments" << endl;
            free_args(args,command_len);
            return;
        }

        //it is a number
        stringstream str1(args[1]);
        str1 >> id;
        //job-id does not exist
        if (!(jobs->is_job_exist(id))){
            cout << "smash error: fg: job-id " << id << " does not exist" << endl;
            free_args(args,command_len);
            return;
        }
    }
    //check if no argument was sent, id still not updated
    if (id == -1){
        if (jobs->isEmpty()){
            free_args(args,command_len);
            cout << "smash error: fg: jobs list is empty" << endl;

        }
        else{
            jobs->getLastJob(&id);
        }
    }

    //if got here, then value in id is valid
    free_args(args,command_len);

    JobsList::JobEntry* jobEntry = jobs->getJobById(id);
    cout << jobEntry->get_cmd()->get_cmd_line() << " : " << id << endl;
    jobs->removeJobById(id);

    if (kill(jobEntry->get_cmd()->get_pid(), SIGCONT)!=0){
        perror("smash error: kill failed");
    }

    waitpid(jobEntry->get_cmd()->get_pid(), nullptr, WNOHANG);
    return;
}


///bg command
void BackgroundCommand::execute() {
    jobs->removeFinishedJobs();

    string str = string(this->get_cmd_line(),strlen(this->get_cmd_line())+1);
    char *args[COMMAND_MAX_ARGS];
    int command_len = _parseCommandLine(str.c_str(), args);

    if (command_len > 2) {
        cerr << "smash error: bg: invalid arguments" << endl;
        free_args(args,command_len);
        return;
    }

    int id=-1;

    //if an argument was sent
    if (command_len == 2) {
        //check format
        if (!(is_number(args[1]))) {
            cout << "smash error: bg: invalid arguments" << endl;
            free_args(args,command_len);
            return;
        }
        //is a number
        stringstream str1(args[1]);
        str1 >> id;
        //job-id does not exist
        if (!(jobs->is_job_exist(id))) {
            cout << "smash error: bg: job-id " << id << " does not exist" << endl;
            free_args(args,command_len);
            return;
        }
        //exists but not stopped
        if (jobs->getJobById(id)->get_cmd()->get_state() != Stopped){
            cout << "smash error: bg: job-id " << id << " is already running in the background" << endl;
            free_args(args,command_len);
            return;
        }
    }
    //check if no argument was sent, id still not updated
    if (id == -1) {
        JobsList::JobEntry *jobEntry = jobs->getLastStoppedJob(&id);
        //check if list empty || no stopped job
        if (jobEntry == NULL && id == 0) {
            cout << "smash error: bg: there is no stopped jobs to resume" << endl;
            free_args(args,command_len);
            return;
        }
    }
    //if got here, then value in id is valid
    free_args(args,command_len);

    JobsList::JobEntry* jobEntry = jobs->getJobById(id);
    cout << jobEntry->get_cmd()->get_cmd_line() << " : " << id << endl;
    jobEntry->get_cmd()->set_state(Background);

    int ret = kill(jobEntry->get_cmd()->get_pid(), SIGCONT);
    if (ret!=0){
        perror("smash error: kill failed");
    }
}


///quit
void QuitCommand::execute() {

    jobs->removeFinishedJobs();

    string str = string(this->get_cmd_line(),strlen(this->get_cmd_line())+1);
    char *args[COMMAND_MAX_ARGS];
    int command_len = _parseCommandLine(str.c_str(), args);

    if (command_len >= 2) {
        jobs->print_before_quit();
        jobs->killAllJobs();
        jobs->delete_jobs_vector();

    } else {
        jobs->delete_jobs_vector();
    }

    free_args(args, command_len);
    exit(0);
}


///external commands
void ExternalCommand::execute() {
    auto cmd_temp = string(this->get_cmd_line());
    char *args[] = {(char *) "/bin/bash", (char *) "-c", (char *) cmd_temp.c_str(), nullptr};

    pid_t pid = fork();

    if (pid == 0) {
        setpgrp();
        execv(args[0], args);
        //if we here execv failed
        perror("smash error: execv failed");
        exit(0);

    } else if (pid > 0) {
//        setFg(this, pid);
        waitpid(pid, NULL, WUNTRACED);
  //      setFg(this, NULL);

    } else {

        perror("smash error: fork failed");
    }
    return;
}



///pipe




///cp
void CopyCommand::execute() {
    string str = string(this->get_cmd_line(),strlen(this->get_cmd_line())+1);
    char *args[COMMAND_MAX_ARGS];
    int command_len = _parseCommandLine(str.c_str(), args);

    int file_in = open(args[1], O_RDONLY);
    if (file_in == -1) {
        perror("smash error: open failed");
        return;
    }

    int fd2 = open(args[2],  O_CREAT | O_TRUNC | O_WRONLY ,0644);
    if (fd2 == -1) {
        close(file_in);
        perror("smash error: open failed");
        return;
    }

    char temp[256];
    int res = read(file_in, temp, sizeof(temp));
    if (res == -1) {
        close(file_in);
        close(fd2);
        perror("smash error: read failed");
        return;
    }

    if (write(fd2, temp, res) == -1) {
        close(file_in);
        close(fd2);
        perror("smash error: write failed");
        return;
    }

    while (res != 0) {
        res = read(file_in, temp, sizeof(temp));
        if (res == -1) {
            perror("smash error: read failed");
            return;
        }

        if(res != 0) {
            if (write(fd2, temp, res) == -1) {
                close(file_in);
                close(fd2);
                perror("smash error: write failed");
                return;
            }
        }
    }

    close(file_in);
    close(fd2);
    free_args(args, command_len);
    cout << "smash: " << args[1] << " was copied to " << args[2] << endl;
    return;
}
