#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <cstdio>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define HISTORY_MAX_RECORDS (50)
//this for the state of the job

typedef enum{Foregroung, Background, Stopped}State;

///Command///
class Command {
// TODO: Add your data members
//every command has the command line, pid and state
char* cmd_line;
pid_t pid;
State state;

 public:
  Command(const char* cmd_line);
  virtual ~Command();
  virtual void execute() = 0;
  // TODO: Add your extra methods if needed
  char* get_cmd_line();
  void set_pid(pid_t new_pid);
  pid_t get_pid();
  void set_state(State new_state);
  State get_state();
};

class BuiltInCommand : public Command {
 public:
  BuiltInCommand(const char* cmd_line) : Command(cmd_line){};
  virtual ~BuiltInCommand() {}
};

///External command///
class ExternalCommand : public Command {
 public:
  ExternalCommand(const char* cmd_line) : Command(cmd_line){};
  virtual ~ExternalCommand() {}
  void execute() override;
};

///pipe command///
class PipeCommand : public Command {
  // TODO: Add your data members
 public:
  PipeCommand(const char* cmd_line) : Command(cmd_line){};
  virtual ~PipeCommand() {}
  void execute() override;
};

///cd///
class RedirectionCommand : public Command {
 // TODO: Add your data members
 public:
  explicit RedirectionCommand(const char* cmd_line): Command(cmd_line){};
  virtual ~RedirectionCommand() {}
  void execute() override;
};

class ChangeDirCommand : public BuiltInCommand {
    char** plast;
// TODO: Add your data members
public:
  ChangeDirCommand(const char* cmd_line, char** plastPwd) : BuiltInCommand(cmd_line), plast(plastPwd){};
  virtual ~ChangeDirCommand() {}
  void execute() override;
};

///pwd///
class GetCurrDirCommand : public BuiltInCommand {
 public:
  GetCurrDirCommand(const char* cmd_line) : BuiltInCommand(cmd_line){};
  virtual ~GetCurrDirCommand() {}
  void execute() override;
};

///show pid///
class ShowPidCommand : public BuiltInCommand {
 public:
  ShowPidCommand(const char* cmd_line) : BuiltInCommand(cmd_line){};
  virtual ~ShowPidCommand() {};
  void execute() override;
};


/////why is this?///
class CommandsHistory {
 protected:
  class CommandHistoryEntry {
	  // TODO: Add your data members
  };
 // TODO: Add your data members
 public:
  CommandsHistory();
  ~CommandsHistory() {}
  void addRecord(const char* cmd_line);
  void printHistory();
};

class HistoryCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  HistoryCommand(const char* cmd_line, CommandsHistory* history);
  virtual ~HistoryCommand() {}
  void execute() override;
};

///jobs///
class JobsList {
 public:
  class JobEntry {
   // TODO: Add your data members
   Command *cmd;
      int job_id;
      time_t init_time;
  public:
      JobEntry(Command *new_cmd, int new_job_id) : cmd(new_cmd), job_id(new_job_id){
          if(time(&init_time) == (time_t)-1){
              perror("smash error: time failed");
          }
      }

      Command* get_cmd() const{
          return this->cmd;
      }

      void set_cmd(Command *new_cmd,int new_job_id){
          this->cmd = new_cmd;
          this->job_id = new_job_id;
      }

      void set_job_id(int new_job_id){
          this->job_id = new_job_id;
          return;
      }

      int get_job_id() const{
          return this->job_id;
      }

      time_t get_time() const{
          return this->init_time;
      }
  };


 // TODO: Add your data members
 public:
  JobsList();
  ~JobsList(){};
  void addJob(Command* cmd, bool isStopped = false);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry * getJobById(int jobId);
  void removeJobById(int jobId);
  JobEntry * getLastJob(int* lastJobId);
  JobEntry *getLastStoppedJob(int *jobId);
  // TODO: Add extra methods or modify exisitng ones as needed
  bool is_job_exist(int id);
  void print_before_quit();

    void delete_jobs_vector() {
        jobs.clear();
    }

private:
    std::vector<JobEntry> jobs;
};

class JobsCommand : public BuiltInCommand {
 // TODO: Add your data members
 JobsList* jobs;
 public:
  JobsCommand(const char* cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line), jobs(jobs){};
  virtual ~JobsCommand() {}
  void execute() override;
};

class KillCommand : public BuiltInCommand {
 // TODO: Add your data members
 JobsList* jobs;
 public:
  KillCommand(const char* cmd_line, JobsList* jobs): BuiltInCommand(cmd_line), jobs(jobs){};
  virtual ~KillCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 JobsList* jobs;
 public:
  ForegroundCommand(const char* cmd_line, JobsList* jobs): BuiltInCommand(cmd_line), jobs(jobs){};
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 JobsList* jobs;
 public:
  BackgroundCommand(const char* cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line), jobs(jobs){};
  virtual ~BackgroundCommand() {}
  void execute() override;
};

///kill///
class QuitCommand : public BuiltInCommand {
    JobsList* jobs;
// TODO: Add your data members

public:
    QuitCommand(const char* cmd_line, JobsList* jobs): BuiltInCommand(cmd_line), jobs(jobs){};
    virtual ~QuitCommand() {}
    void execute() override;
};

// TODO: should it really inhirit from BuiltInCommand ?
///cp///
class CopyCommand : public BuiltInCommand {
 public:
  CopyCommand(const char* cmd_line): BuiltInCommand(cmd_line){};
  virtual ~CopyCommand() {}
  void execute() override;
};

// TODO: add more classes if needed

// maybe chprompt , timeout ?

class SmallShell {

 private:
  // TODO: Add your data members
    JobsList jobslist;
    char * plast;
  SmallShell();

 public:
  Command *CreateCommand(const char* cmd_line);
  SmallShell(SmallShell const&)      = delete; // disable copy ctor
  void operator=(SmallShell const&)  = delete; // disable = operator
  static SmallShell& getInstance() // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
    ~SmallShell(){

        if(plast != nullptr){
            delete(plast);}
    };
  void executeCommand(const char* cmd_line);
  // TODO: add extra methods as needed

};

#endif //SMASH_COMMAND_H_
