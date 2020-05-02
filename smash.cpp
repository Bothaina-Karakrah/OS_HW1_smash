#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <cstring>
#include "Commands.h"
#include "signals.h"

int main(int argc, char* argv[]) {
    if(signal(SIGTSTP , ctrlZHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-Z handler");
    }
    if(signal(SIGINT , ctrlCHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }

    //TODO: setup sig alarm handler
    stdout_fd=1;
   // char smash_prompt[1024];
 //   strcpy(smash_prompt, "smash");

    SmallShell& smash = SmallShell::getInstance();
    char str[6]= "smash";
    smash.set_prompt(str);
    while(true) {
        std::cout << smash.get_prompt() <<"> "; // TODO: change this (why?)
        std::string cmd_line;
        std::getline(std::cin, cmd_line);
        smash.executeCommand(cmd_line.c_str(), smash.get_prompt());
    }
    return 0;
}
