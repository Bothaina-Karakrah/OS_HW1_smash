#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
    cout << "smash: got ctrl-Z" << endl;
    SmallShell &smallShell = smallShell.getInstance();
    int temp = smallShell.get_curr_pid();

    if (temp != -1) {
        smallShell.get_job_list()->addJob(smallShell.get_job_list()->get_curr_fg()->get_cmd() ,true);
        smallShell.get_job_list()->get_curr_fg()->get_cmd()->set_state(Stopped);
        if (kill(smallShell.get_curr_pid(), SIGSTOP) != 0) {
            perror("smash error: kill failed");
        }
        smallShell.set_curr_pid(-1) ;
        smallShell.get_job_list()->set_curr_fg(nullptr,0);
        cout << "smash: process " << temp << " was stopped" << endl;
    }
}

void ctrlCHandler(int sig_num) {
    cout << "smash: got ctrl-C" << endl;
    SmallShell &smallShell = smallShell.getInstance();
    int temp = smallShell.get_curr_pid();
    if (temp!= -1) {
        if (kill(smallShell.get_curr_pid(), SIGKILL) != 0) {
            perror("smash error: kill failed");
        }
        smallShell.set_curr_pid(-1) ;
        smallShell.get_job_list()->set_curr_fg(nullptr,0);
        cout << "smash: process " << temp << " was killed" << endl;
    }
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}

