#pragma once 
#include "headers.h"

AlgorithmType algo;
int processesCount;
PCB* runningProcess = NULL;
int test = 0;

// ================== Finished Processes Queue ================== //
typedef struct FinishedProcessNode
{
    PCB *data;
    struct FinishedProcessNode *next;
} FinishedProcessNode;
FinishedProcessNode *finished_process_front = NULL, *finished_process_rear = NULL;
int finished_process_qSize = 0;  //would equal total number of pcbs when the program exits
void finished_process_enqueue(PCB *val);
void finished_process_pop();     //throws front and frees memory

void finished_process_enqueue(PCB *val)
{
    finished_process_qSize++;
    FinishedProcessNode *newNode = malloc(sizeof(FinishedProcessNode));
    newNode->next = NULL;
    newNode->data = val;
    
    //First node to be added
    if(finished_process_front == NULL && finished_process_rear == NULL)
    {
        //make both front and rear points to newNode
        finished_process_front = newNode;
        finished_process_rear = newNode;
    }
    else //not the first
    {
        //add newNode in rear->next
        finished_process_rear->next = newNode;

        //make newNode as the rear Node
        finished_process_rear = newNode;
    }
}

void finished_process_pop()
{
    if(finished_process_front == NULL)
        return;
    if(finished_process_front == finished_process_rear)
        finished_process_rear = NULL;
    FinishedProcessNode* temp = finished_process_front;
    finished_process_front = finished_process_front->next;
    free(temp->data);   //First free data
    free(temp);
}

// ================== PCB Queue ================== //
typedef struct PCBNode
{
    PCB *data;
    struct PCBNode *next;
} PCBNode;
PCBNode *pcb_front = NULL, *pcb_rear = NULL;
int pcb_qSize = 0;  //would equal total number of pcbs when the program exits
void pcb_pop();     //throws front and frees memory
void pcb_enqueue(PCB *val);

void pcb_enqueue(PCB *val)
{
    pcb_qSize++;
    PCBNode *newNode = malloc(sizeof(PCBNode));
    newNode->next = NULL;
    newNode->data = val;
    
    //First node to be added
    if(pcb_front == NULL && pcb_rear == NULL)
    {
        //make both front and rear points to newNode
        pcb_front = newNode;
        pcb_rear = newNode;
    }
    else //not the first
    {
        //add newNode in rear->next
        pcb_rear->next = newNode;

        //make newNode as the rear Node
        pcb_rear = newNode;
    }
}

void pcb_pop()
{
    if(pcb_front == NULL)
        return;
    if(pcb_front == pcb_rear)
        pcb_rear = NULL;
    PCBNode* temp = pcb_front;
    pcb_front = pcb_front->next;
    free(temp->data);   //First free data
    free(temp);
}

//==================functions definations====================//

//============ Functions  Definations    =====================//

//process operations
int forkNewProcess();
void startProcess();
void stopProcess();



//communication with process_generator
void recvProcess();

//-------------- Scheduling Algorithms ------------//
void runPHPF();
void runSRTN();
void runSJF();
//------------------------------------------------//

void runAlgo(); // runs choosen algo by the user in process generator



//hanlder for handling processes arrival and finish
void handleProcessArrival(int signum);
void handleProcessFinish(int signum);

// for clearing on exit
void clearResources(); 


//================ Functions Implementations ===============//



int forkNewProcess(int execution_time){
    pid_t schedulerPid = getpid();
    pid_t processPid = fork();

    if(processPid==-1){
        perror("Error while forking process!");
        exit(FAILURE_CODE);
    }
    else if(processPid==0){
        char stringExecutionTime[5] = {0};
        char stringSchedulerPid[7] = {0};
        char stringClkValue[7] = {0};
        sprintf(stringSchedulerPid, "%d", schedulerPid);
        sprintf(stringExecutionTime, "%d", execution_time);
        sprintf(stringClkValue, "%d", getClk());
        char *const paramList[] = {"./process.out", stringExecutionTime, stringSchedulerPid, stringSchedulerPid,NULL};
        execv("./process.out", paramList);
        
        //if it executes what is under this line, then execv has failed
        perror("Error in execv'ing to clock");
        exit(EXIT_FAILURE);
    }

    return processPid;
}


void recvProcess(){
    ProcessParameters recPrc;
    PCB* prc;
    
    while(msgQueueRcvPrc(&recPrc) != -1){
        //allocating memory for prc
        prc = (PCB*) malloc(sizeof(PCB));

        //extrating PCB obj parameters from the recieved object
        prc->id = recPrc.process_id;
        printf("Rec Prc ID: %d\n", prc->id);
        prc->arrival_time =  recPrc.arrival_time;
        prc->priority = recPrc.priority;
        prc->execution_time = recPrc.execution_time;
        prc->remaining_time = recPrc.execution_time;
        prc->start_time = -1;


        //enqueue to the PCB queue
        pcb_enqueue(prc);

    }
}

void runPHPF(){


}


void runSRTN(){

}

void runSJF(){
    if(runningProcess==NULL && pcb_front!=NULL){
        runningProcess = pcb_front->data;
        pcb_pop();
        forkNewProcess(runningProcess->remaining_time);
        test = 1;
    }
    
}


void runAlgo(){

    printf("Running RunAlgo\n");
    switch(algo){
        case PHPF:
            runPHPF();
            break;
        case SRTN:
            runSRTN();
            break;
        case SJF:
            runSJF();
            break;
    }
}


void handleProcessArrival(int signum){
    recvProcess();
    runAlgo();
}


void FinalizeProcessParameters(PCB* p) {
    p->finish_time = getClk();
    p->waiting_time = p->finish_time - p->arrival_time - p->execution_time;
    p->turnaround_time = p->finish_time - p->arrival_time;
    p->weighted_turnaround_time = p->turnaround_time / (float) p->execution_time;
    p->weighted_turnaround_time = round (p->weighted_turnaround_time * 100) / 100;
}

void RegisterFinishedProcess(PCB* p) {
    finished_process_enqueue(p);
}

void handleProcessFinished(int signum){
    printf("Process with PID:%d finished!\n", runningProcess->process_id);
    FinalizeProcessParameters(runningProcess);
    RegisterFinishedProcess(runningProcess);   
    runningProcess = NULL;
    runAlgo();
}