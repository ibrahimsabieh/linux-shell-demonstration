#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFFERSIZE 34

void *watchProcess(*void *pid);

struct Process{
    pid_t pid;
    int priority;
    char * status;
    char * program;
};

struct Process processes[5];

void runProgram(int processIndex)
{
    pid_t pid = fork();

    if(pid != 0){
        strcpy(processes[processIndex].status, "Running");
        processes[processIndex].pid = pid;
        pthread_t thread;
        pthread_create(&thread, NULL, (void *)watchProcess, (void *)&pid);
    }
    else{
        char *arguments[] = {NULL};
        execve(processes[processIndex].program, arguments, NULL);
    }

}


int maxPriority(){
    int max = processes[0].priority;
    int maxIndex = 0;

    for(int i=1; i<5; i++){
        if(processes[i].priority > max){
            max = processes[i].priority;
            maxIndex = i;
        }
    }

    return maxIndex;
}

void *watchProcess(*void * pid)
{
    pid_t * process_pid = (pid_t *) pid;

    waitpid(*process_pid, NULL, 0);

    for(int i=0; i<5; i++){
        if(processes[i].pid == *process_pid){
            processes[i].priority = -1;
            break;
        }
    }

    int processToRun = maxPriority();

    for(int i=0; i<5; i++){
        if(processes[i].priority != -1 && strcmp(processes[i].status, "Stopped") == 0 && processes[i].priority == processToRun){
            kill(processes[i].pid, SIGCONT);
            break;
        }
    }

    processToRun = maxPriority();

    runProgram(processToRun);
}

void print_status()
{
    printf("PID\tPriority\tStatus\tProgram\n");
    for (int i = 0; i < 5; i++)
    {
        if (processes[i].pid != -1)
        {
            printf("%d\t%d\t%s\t%s\n", (int)processes[i].pid, processes[i].priority, processes[i].status, processes[i].program);
        }
    }
}

int main()
{
    char INSTR[BUFFERSIZE];
    int entryPriority;
    char progName[BUFFERSIZE];
    int available_index;

    for(int i=0; i<5; i++){
        processes[i].priority = -1;
    }

    while(1)
    {
        available_index = -1;

        printf("pshell: ");
        fgets(INSTR, BUFFERSIZE, stdin);

        if(strcmp(INSTR, "status\n") == 0){
            print_status();
        }

        int i=0;
        while(INSTR[i] != ' '){
            progName[i] = INSTR[i];
            i++;
        }
        progName[i] = '\0';
        entryPriority = (int)INSTR[i+1] - 48;

        for(int i=0; i<5; i++){
            if(processes[i].priority == -1){
                available_index = i;
                break;
            }
        }

        if(available_index == -1){
            printf("Table is full!\n");
        }
        else{
            processes[available_index].priority = entryPriority;
            strcpy(processes[available_index].program, progName);
            strcpy(processes[available_index].status, "Ready");
        }

        for(int i=0; i<5; i++){
            if(processes[i].priority != -1 && strcmp(processes[i].status, "Running") == 0 && processes[i].priority < entryPriority){
                kill(processes[i].pid, SIGSTOP);
                strcpy(processes[i].status, "Stopped");
                runProgram(entryPriority);
                break;
            }
        }

        for(int i=0; i<5; i++){
            if(processes[i].priority != -1 && strcmp(processes[i].status, "Running") != 0){
                int processToRun = maxPriority();
                runProgram(processToRun);
                break;
            }
        }


    }

    return 0;
}