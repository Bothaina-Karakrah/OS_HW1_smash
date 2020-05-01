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

void SmallShell::set_prompt(char *new_prompt) {
    strcpy(this->shell_prompt, new_prompt);
}
/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
bool isBuiltInCommand(const char* cmd_s){
    return strcmp(cmd_s, "chprompt") == 0 || strcmp(cmd_s, "chprompt&") == 0 ||
           strcmp(cmd_s, "showpid") == 0 || strcmp(cmd_s, "showpid&") == 0 ||
           strcmp(cmd_s, "pwd") == 0 || strcmp(cmd_s, "pwd&") == 0 ||
           strcmp(cmd_s, "cd") == 0 || strcmp(cmd_s, "cd&") == 0 ||
           strcmp(cmd_s, "jobs") == 0 || strcmp(cmd_s, "jobs&") == 0 ||
           strcmp(cmd_s, "kill") == 0 || strcmp(cmd_s, "kill&") == 0 ||
           strcmp(cmd_s, "fg") == 0 || strcmp(cmd_s, "fg&") == 0 ||
           strcmp(cmd_s, "bg") == 0 || strcmp(cmd_s, "bg&") == 0 ||
           strcmp(cmd_s, "quit") == 0 || strcmp(cmd_s, "quit&") == 0;
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line, char* smash_prompt) {
    string str = string(cmd_line,strlen(cmd_line)+1);
    char *args[COMMAND_MAX_ARGS];
    int command_len =_parseCommandLine(str.c_str(), args);

    //copy first argument
    int len = strlen(args[0])+1;
    char cmd_s[len];
    strcpy(cmd_s, args[0]);

    //cmd_line is const so copy to remove & if exists
    char command_for_BuiltIn [strlen(cmd_line)+1];
    strcpy(command_for_BuiltIn, cmd_line);

    //check if is built-in command
    if (isBuiltInCommand(cmd_s)) {
        _removeBackgroundSign(command_for_BuiltIn);
    }

    if(*cmd_line == 0) {
        free_args(args, command_len);
        return nullptr;
    }
    else if(str.find("|") != string::npos){
        free_args(args, command_len);
        return  new PipeCommand(cmd_line, &smash_prompt);
    }
    else if(str.find(">") != string::npos)
    {
        free_args(args, command_len);
        return new RedirectionCommand(cmd_line,&smash_prompt);
    }
    else if(strcmp(cmd_s, "chprompt") == 0 || strcmp(cmd_s, "chprompt&") == 0){
        free_args(args, command_len);
        return new chprompt(command_for_BuiltIn, &smash_prompt);

    }
    else if(strcmp(cmd_s, "showpid") == 0 || strcmp(cmd_s, "showpid&") == 0){
        free_args(args, command_len);
        return new ShowPidCommand(command_for_BuiltIn);
    }
    else if(strcmp(cmd_s, "pwd") == 0 || strcmp(cmd_s, "pwd&") == 0){
        free_args(args, command_len);
        return new GetCurrDirCommand(command_for_BuiltIn);
    }
    else if((strcmp(cmd_s, "cd") == 0 || strcmp(cmd_s, "cd&") == 0) && command_len > 1){
        free_args(args, command_len);
        return  new ChangeDirCommand(command_for_BuiltIn, &plast);
    }
    else if(strcmp(cmd_s, "jobs") == 0 || strcmp(cmd_s, "jobs&") == 0){
        free_args(args, command_len);
        return new JobsCommand(command_for_BuiltIn, &jobslist);
    }
    else if (strcmp(cmd_s, "kill") == 0 || strcmp(cmd_s, "kill&") == 0){
        free_args(args, command_len);
        return new KillCommand(command_for_BuiltIn, &jobslist);
    }
    else if(strcmp(cmd_s, "fg") == 0 || strcmp(cmd_s, "fg&") == 0){
        free_args(args, command_len);
        return new ForegroundCommand(command_for_BuiltIn, &jobslist);
    }
    else if(strcmp(cmd_s, "bg") == 0 || strcmp(cmd_s, "bg&") == 0){
        free_args(args, command_len);
        return new BackgroundCommand(command_for_BuiltIn, &jobslist);
    }
    else if(strcmp(cmd_s, "quit") == 0 || strcmp(cmd_s, "quit&") == 0){
        free_args(args, command_len);
        return new QuitCommand(command_for_BuiltIn, &jobslist);
    }
    else if(strcmp(cmd_s,"cp") == 0){
        free_args(args, command_len);
        return new CopyCommand(cmd_line);
    }

    free_args(args, command_len);
    return new ExternalCommand(cmd_line);
}


void SmallShell::executeCommand(const char *cmd_line, char* smash_prompt) {
    // TODO: Add your implementation here
    Command* cmd = CreateCommand(cmd_line, smash_prompt);
    if(cmd == nullptr) {
        return;
    }
    cmd->execute();
    this->set_prompt(cmd->get_command_prompt());
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
    int neg_flag = 0;
    std::string::const_iterator it = s.begin();
    //check if first char is '-' meaning might be a negative number
    if (!s.empty() && (*it == '-')){
        it++;
    }
    while (it != s.end() && std::isdigit(*it)){
        ++it;
        neg_flag = 1;
    }
    return !s.empty() && it == s.end() && neg_flag == 1;
}


///command///
Command::Command(const char* cmd_line): pid(-1),command_prompt("smash") {

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

char * Command::get_command_prompt() {
    return this->command_prompt;
}

void Command::set_command_prompt(char *str) {
    strcpy(this->command_prompt, str);
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
        this->set_command_prompt(args[1]);
        //strcpy(*prompt_, args[1]);///////////just args[1]???
    } else{
        char str[6]= "smash";
        this->set_command_prompt(str);
        //strcpy(*prompt_, "smash");
    }
    free_args(args, command_len);
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
        return;
    }
    cout << cwd <<endl;
    return;
}



///cd
void ChangeDirCommand::execute() {
    string str = string(this->get_cmd_line(),strlen(this->get_cmd_line())+1);
    char *args[COMMAND_MAX_ARGS];
    int command_len = _parseCommandLine(str.c_str(), args);

    if(command_len < 2)
        return;

    if (command_len > 2) {
        cerr << "smash error: cd: too many arguments" << endl;
        free_args(args, command_len);
        return;
    }
    else{
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
                    perror("smash error: chdir failed");
                    free_args(args, command_len);
                    return;
                } else {
                    strcpy(*plast, new_path);
                }
                free(new_path);
            }
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

    int new_id = 1;

    if(this->jobs.size() > 0){
        //check if job exist in job list
        for (size_t i = 0; i < jobs.size(); ++i) {
            if(jobs[i].get_cmd()->get_pid() == cmd->get_pid()){
                time_t new_time;
                time(&new_time);
                jobs[i].set_time(new_time);
                jobs[i].get_cmd()->set_state(cmd->get_state());
                return;
            }
        }
        //if we here, job is not in the list
        new_id += ((this->jobs).back().get_job_id());
    }
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
                 << " : " << jobs[i].get_cmd()->get_pid() << " " << elapsed_time << " secs (stopped)" << endl;
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
    return NULL;
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

    for (int i = (this->jobs).size()-1; i >= 0; i--) {
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
        if (kill(jobs[i].get_cmd()->get_pid(), SIGKILL) != 0) {
            perror("smash error: kill failed");
        }
    }
    return;
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
    for(size_t i = 0; i < (this->jobs).size(); ++i) {

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

    //no need to check command arguments
    jobs->printJobsList();
    jobs->removeFinishedJobs();
}


///kill command
void KillCommand::execute() {
    jobs->removeFinishedJobs();

    string str = string(this->get_cmd_line(),strlen(this->get_cmd_line())+1);
    char *args[COMMAND_MAX_ARGS];
    int command_len = _parseCommandLine(str.c_str(), args);

    //check arguments_number
    if (command_len != 3){
        cout << "smash error: kill: invalid arguments" <<endl;
        free_args(args, command_len);
        return;
    }
    //check arguments_format
    if (!is_number(args[1]) || !is_number(args[2])){
        cout << "smash error: kill: invalid arguments" <<endl;
        free_args(args, command_len);
        return;
    }

    //got numbers
    //check validity of signum
    int signum = atoi((args[1]));
    signum = -1* signum;
    if (signum < 1 || signum > 31){
        cout << "smash error: kill: invalid arguments" <<endl;
        free_args(args, command_len);
        return;
    }


    if (_isBackgroundComamnd(this->get_cmd_line()) == 1) {
        _removeBackgroundSign(args[2]);
    }
    int job_id = atoi(args[2]);

    //check if job-id exists
    if (!(jobs->is_job_exist(job_id))){
        cout << "smash error: kill: job-id " << job_id<< " does not exist" << endl;
        free_args(args, command_len);
        return;
    }

    JobsList::JobEntry* jobEntry = jobs->getJobById(job_id);

    if (kill(jobEntry->get_cmd()->get_pid(), signum) != 0) {
        perror("smash error: kill failed");
    }
    else{
        cout << "signal number " << signum << " was sent to pid " << jobEntry->get_cmd()->get_pid() << endl;
    }
    free_args(args,command_len);
}



///ForeGround command
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
            perror("smash error: fg: invalid arguments");
            free_args(args,command_len);
            return;
        }

        //it is a number
        id = atoi(args[1]);

        //check if job-id does not exist
        if (!(jobs->is_job_exist(id))){
            cout << "smash error: fg: job-id " << id << " does not exist" << endl;
            free_args(args,command_len);
            return;
        }
    }
    //check if no argument was sent, id still not updated
    else if (command_len == 1){
        if (jobs->isEmpty()){
            free_args(args,command_len);
            cout << "smash error: fg: jobs list is empty" << endl;
            return;
        }
        else{
            jobs->getLastJob(&id);
        }
    }

    //if got here, then value in id is valid
    free_args(args,command_len);

    JobsList::JobEntry* jobEntry = jobs->getJobById(id);
    cout << jobEntry->get_cmd()->get_cmd_line() << " : " << jobEntry->get_cmd()->get_pid() << endl;

    if (kill(jobEntry->get_cmd()->get_pid(), SIGCONT) < 0){
        perror("smash error: kill failed");
        return;
    }

    SmallShell &smallShell = smallShell.getInstance();
    smallShell.set_curr_pid(jobEntry->get_cmd()->get_pid()) ;
    this->jobs->set_curr_fg_job(jobEntry->get_cmd(),jobEntry->get_job_id());
    jobEntry->get_cmd()->set_state(Foregroung);


    int state;
    if(waitpid(jobEntry->get_cmd()->get_pid(), &state, WUNTRACED) > 0){
        if(WIFSTOPPED(state)){
            jobEntry->get_cmd()->set_state(Stopped);
            return;
        }
    }
    else{
        perror("smash error: waitpid failed");
        return;
    }
    smallShell.set_curr_pid(-1) ;
    this->jobs->set_curr_fg_job(jobEntry->get_cmd(),jobEntry->get_job_id());
    return;
}


///bg command
void BackgroundCommand::execute() {
    jobs->removeFinishedJobs();

    string str = string(this->get_cmd_line(),strlen(this->get_cmd_line())+1);
    char *args[COMMAND_MAX_ARGS];
    int command_len = _parseCommandLine(str.c_str(), args);

    if (command_len > 2) {
        perror("smash error: bg: invalid arguments");
        free_args(args,command_len);
        return;
    }

    int id=-1;

    //if an argument was sent
    if (command_len == 2) {
        //check format
        if (!(is_number(args[1]))) {
            perror("smash error: bg: invalid arguments");
            free_args(args,command_len);
            return;
        }
        //is a number
        id = atoi(args[1]);

        //check if job-id does not exist
        if (!(jobs->is_job_exist(id))){
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
    else if (command_len == 1) {
        JobsList::JobEntry *jobEntry = jobs->getLastStoppedJob(&id);
        //check if list empty || no stopped job
        if (jobEntry == NULL && id <= 0) {
            perror("smash error: bg: there is no stopped jobs to resume");
            free_args(args,command_len);
            return;
        }
    }
    //if got here, then value in id is valid
    free_args(args,command_len);

    JobsList::JobEntry* jobEntry = jobs->getJobById(id);

    if(_isBackgroundComamnd(jobEntry->get_cmd()->get_cmd_line()))
    {
        _removeBackgroundSign(jobEntry->get_cmd()->get_cmd_line());
    }
    cout << jobEntry->get_cmd()->get_cmd_line() << " : " << jobEntry->get_cmd()->get_pid() << endl;

    if (kill(jobEntry->get_cmd()->get_pid(), SIGCONT)!=0){
        perror("smash error: kill failed");
        return;
    }
    //kill success
    jobEntry->get_cmd()->set_state(Background);
}


///quit
void QuitCommand::execute() {

    jobs->removeFinishedJobs();

    string str = string(this->get_cmd_line(),strlen(this->get_cmd_line())+1);
    char *args[COMMAND_MAX_ARGS];
    int command_len = _parseCommandLine(str.c_str(), args);

    if (command_len > 1 && strcmp("kill",args[1]) == 0) {
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

    char* cmdline =(char*)malloc(sizeof(char) * COMMAND_ARGS_MAX_LENGTH);
    strcpy(cmdline,this->get_cmd_line());
    _removeBackgroundSign(cmdline);
    char *argv [] = {(char *) "/bin/bash", (char *) "-c", cmdline, NULL};

    pid_t pid = fork();
    //if fork failed
    if (pid < 0) {
        perror("smash error: fork failed");
        return;
    }
    //son:
    if (pid == 0) {
        setpgrp();
        execv("/bin/bash", argv);
        //if we here execv failed
        perror("smash error: execv failed");
        return;
    }
        //father
    else {
        this->set_pid(pid);
        SmallShell &smallShell = smallShell.getInstance();
        if (_isBackgroundComamnd(this->get_cmd_line())) {
            smallShell.get_job_list()->addJob(this, false);
        }
        else {
            smallShell.set_curr_pid(pid);
            smallShell.get_job_list()->set_curr_fg_job(this, 0);
            int status;
            if (waitpid(pid, &status, WUNTRACED) < 0 ) {
                perror("smash error: waitpid failed");
            }
        }
    }
    return;
}

////redirection
void RedirectionCommand::execute() {
    string str = string(this->get_cmd_line(), strlen(this->get_cmd_line()) + 1);
    char *args[COMMAND_MAX_ARGS];
    int command_len = _parseCommandLine(str.c_str(), args);

    int RD_index = string(this->get_cmd_line()).find('>');
    bool is_append = false;
    if (this->get_cmd_line()[RD_index + 1] == '>') {
        is_append = true;
    }
    int file_output;

    //check if not is_append -> need to override
    if (!is_append) {
        //enable create if does not exist + override if exists
        file_output = open(args[command_len - 1], O_CREAT | O_WRONLY | O_TRUNC,0644);
        if (file_output == -1) {
            free_args(args, command_len);
            perror("smash error: open failed");
            return;
        }
    }
    else {
        //enable create if not exists + append if exists
        file_output = open(args[command_len - 1], O_APPEND | O_CREAT | O_WRONLY , 0644);
        if (file_output == -1) {
            free_args(args, command_len);
            perror("smash error: open failed");
            return;
        }
    }

    SmallShell &smallShell = smallShell.getInstance();
    string cmdLine = string(get_cmd_line());
    Command *command = smallShell.CreateCommand(cmdLine.substr(0, RD_index).c_str(), *prompt_);

    //save stdout
    int command_stdout = dup(1);
    stdout_fd = command_stdout;

    if (command_stdout == -1) {
        free_args(args, command_len);
        perror("smash error: dup failed");
        if (close(file_output) == -1){
            perror("smash error: close failed");
        }
        return;
    }

    if (dup2(file_output, 1) == -1) {
        free_args(args, command_len);
        perror("smash error: dup2 failed");
        if (close(file_output) == -1){
            perror("smash error: close failed");
        }
        return;
    }

    command->execute();

    if (dup2(command_stdout, 1) == -1) {
        free_args(args, command_len);
        perror("smash error: dup2 failed");
        if (close(file_output) == -1){
            perror("smash error: close failed");
        }
        return;
    }

    free_args(args, command_len);
    if (close(file_output) == -1){
        perror("smash error: close failed");
    }


}

///pipe
void PipeCommand::execute() {
    string cmdLine = string(get_cmd_line());
    int pipe_index = cmdLine.find('|');
    bool is_stderr = cmdLine[pipe_index + 1] && this->get_cmd_line()[pipe_index + 1] == '&';

    SmallShell &smallShell = smallShell.getInstance();

    auto source = smallShell.CreateCommand(cmdLine.substr(0, pipe_index).c_str(), *prompt_);
    auto target = smallShell.CreateCommand((cmdLine.substr(pipe_index + (int) is_stderr + 1)).c_str(), *prompt_);

    int fd[2];
    pipe(fd);

    pid_t pid = fork();

    if (pid == -1) {
        perror("smash error: fork failed");
        exit(1);
    }

    //source
    if(fork() == 0){
        setpgrp();
        close(fd[0]);

        if(!is_stderr){
            auto source_stdout = dup(1);
            dup2(fd[1],1);
            close(fd[1]);
        }
        //if stderr
        else{
            auto source_stderr = dup(2);
            dup2(fd[1],2);
            close(fd[1]);
        }
    }

    //target
    if (fork() == 0) {
        setpgrp();

        auto target_stdin = dup(0);
        dup2(fd[0],0);
        close(fd[0]);
        close(fd[1]);
    }

    close(fd[0]);
    close(fd[1]);
}


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

    int file_out = open(args[2],  O_CREAT | O_TRUNC | O_WRONLY ,0644);
    if (file_out == -1) {
        close(file_in);
        perror("smash error: open failed");
        return;
    }

    char buf[1024];
    while (true) {
        auto read_ret = read(file_in, &buf, 1024);

        if (read_ret == -1) {
            perror("smash error: read failed");
            break;
        } else if (read_ret == 0) {
            //we reach EOF
            break;
        }

        auto write_ret = write(file_out, &buf, read_ret);
        if (write_ret == -1) {
            perror("smash error: write failed");
            break;
        }
    }

    if(close(file_in) < 0 || close(file_out) < 0)
        perror("smash error: close failed");
    else
        cout << "smash: " << args[1] << " was copied to " << args[2] << endl;
    free_args(args, command_len);
    return;
}
