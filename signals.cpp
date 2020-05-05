#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"
#include "common.h"

using namespace std;

void ctrlZHandler(int sig_num) {


    ///fix stdout to the standard one.
    if (from_redirect == true) {
        from_redirect = false;
        if (close(new_fd_copy) < 0) {
            perror("smash error: close failed");
        }
        if (dup2(stdout_fd_copy, STDOUT_FILENO) == -1) {
            perror("smash error: dup2 failed");
        }
    }

    // TODO: Add your implementation
    cout << "smash: got ctrl-Z" << endl;
    SmallShell &smallShell = smallShell.getInstance();
    int temp = smallShell.get_curr_pid();
    //TODO: setup sig alarm handler

    if(temp < 0){
        return;
    }

    if(temp > 0){
        if (killpg(smallShell.get_curr_pid(), SIGSTOP) != 0) {
            perror("smash error: kill failed");
            return;
        }
        smallShell.get_job_list()->addJob(smallShell.get_job_list()->get_curr_fg_job()->get_cmd() ,true);
//        smallShell.get_job_list()->get_curr_fg_job()->get_cmd()->set_state(Stopped);
        smallShell.set_curr_pid(-1) ;
        smallShell.get_job_list()->set_curr_fg_job(nullptr);
        cout << "smash: process " << temp << " was stopped" << endl;
    }
}

void ctrlCHandler(int sig_num) {

    ///fix stdout to the standard one.
    if (from_redirect == true) {
        from_redirect = false;
        if (close(new_fd_copy) < 0) {
            perror("smash error: close failed");
        }
        if (dup2(stdout_fd_copy, STDOUT_FILENO) == -1) {
            perror("smash error: dup2 failed");
        }
    }


    // TODO: Add your implementation
    cout << "smash: got ctrl-C" << endl;
    SmallShell &smallShell = smallShell.getInstance();
    int temp = smallShell.get_curr_pid();

    if(temp < 0){
        return;
    }
    if(temp > 0){
        if (killpg(smallShell.get_curr_pid(), SIGKILL) != 0) {
            perror("smash error: kill failed");
            return;
        }
        if(smallShell.get_job_list()->pid_exist(temp)){
            smallShell.get_job_list()->removeJobBypid(temp);
        }
        smallShell.set_curr_pid(-1);
        smallShell.get_job_list()->set_curr_fg_job(nullptr);
        cout << "smash: process " << temp << " was killed" << endl;
    }

}


void alarmHandler(int sig_num) {
    // TODO: Add your implementation
}

