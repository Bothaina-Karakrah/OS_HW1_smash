#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {

    ///fix stdout to the standard one
    if (dup2(stdout_fd, 1) == -1) {
        perror("smash error: dup2 failed");
    }

	// TODO: Add your implementation
    cout << "smash: got ctrl-Z" << endl;
    SmallShell &smallShell = smallShell.getInstance();
    int temp = smallShell.get_curr_pid();
    //TODO: setup sig alarm handler

    if (temp != -1) {
        smallShell.get_job_list()->addJob(smallShell.get_job_list()->get_curr_fg_job()->get_cmd() ,true);
        smallShell.get_job_list()->get_curr_fg_job()->get_cmd()->set_state(Stopped);
        if (kill(smallShell.get_curr_pid(), SIGSTOP) != 0) {
            perror("smash error: kill failed");
        }
        smallShell.set_curr_pid(-1) ;
        smallShell.get_job_list()->set_curr_fg_job(nullptr,0);
        cout << "smash: process " << temp << " was stopped" << endl;
    }
}

void ctrlCHandler(int sig_num) {

    ///fix stdout to the standard one
    if (dup2(stdout_fd, 1) == -1) {
        perror("smash error: dup2 failed");
    }


    // TODO: Add your implementation
    cout << "smash: got ctrl-C" << endl;
    SmallShell &smallShell = smallShell.getInstance();
    int temp = smallShell.get_curr_pid();
    if (temp != -1) {
        if (kill(smallShell.get_curr_pid(), SIGKILL) != 0) {
            perror("smash error: kill failed");
        }
        smallShell.set_curr_pid(-1);
        smallShell.get_job_list()->set_curr_fg_job(nullptr, 0);
        cout << "smash: process " << temp << " was killed" << endl;
    }
}


    void alarmHandler(int sig_num) {
        // TODO: Add your implementation
    }

