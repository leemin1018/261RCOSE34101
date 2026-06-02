#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#define SIZE 100
/*Create_Process(): 실행할 프로세스를 생성하고 각 프로세스에 데이터 설정
(Random 으로 data 입력)
o Process ID
o Arrival time / CPU burst time / IO burst time 등
o Priority
• Config(): 시스템 환경 설정
o Ready Queue / Waiting Queue
• Schedule(): CPU 스케줄링 알고리즘 구현
o FCFS(First Come First Served)
o SJF(Shortest Job First)
o Priority
o RR(Round Robin)
o Preemptive 방식 적용 – SJF, Priority
• Evaluation(): 각 CPU 스케줄링 알고리즘들간 비교 평가 및 분석
o Average waiting time
o Average turnaround tim*/
typedef enum {
    ALGO_FCFS,
    ALGO_SJF,
    ALGO_SJF_PREEMPTIVE,
    ALGO_PRIORITY,
    ALGO_PRIORITY_PREEMPTIVE,
    ALGO_RR
} Algorithm;
typedef struct {
    int pid;        // 실행된 프로세스 ID (CPU가 쉬었으면 -1 또는 0)
    int start_time; // 실행 시작 시간
    int end_time;   // 실행 종료 시간
} GanttRecord; //일단 AI 써써 임시로 만듬 나중애ㅔ 수정하

//일단 ai로 간트 출력함수 아래 깃코드 바탕으로 만들어봄, 나중에 수정하기 
void print_gantt_chart(GanttRecord records[], int count) {
    int i, j;

    printf("\n=== Gantt Chart ===\n");

    // 1. Top Bar 그리기
    printf(" ");
    for(i = 0; i < count; i++) {
        int duration = records[i].end_time - records[i].start_time;
        for(j = 0; j < duration; j++) printf("--");
        printf(" ");
    }
    printf("\n|");

    // 2. Middle (Process ID) 그리기
    for(i = 0; i < count; i++) {
        int duration = records[i].end_time - records[i].start_time;
        for(j = 0; j < duration - 1; j++) printf(" ");
        
        if (records[i].pid > 0) {
            printf("P%d", records[i].pid);
        } else {
            printf("ID"); // Idle 상태 (CPU가 쉼)
        }
        
        for(j = 0; j < duration - 1; j++) printf(" ");
        printf("|");
    }
    printf("\n ");

    // 3. Bottom Bar 그리기
    for(i = 0; i < count; i++) {
        int duration = records[i].end_time - records[i].start_time;
        for(j = 0; j < duration; j++) printf("--");
        printf(" ");
    }
    printf("\n");

    // 4. Timeline 그리기
    printf("%d", records[0].start_time); // 시작 시간
    for(i = 0; i < count; i++) {
        int duration = records[i].end_time - records[i].start_time;
        for(j = 0; j < duration; j++) printf("  ");
        
        // 자릿수에 따른 백스페이스 조절 (칸 맞추기)
        if(records[i].end_time > 9) printf("\b"); 
        printf("%d", records[i].end_time);
    }
    printf("\n");
}
typedef struct {
    // 1. 기본 식별 정보 (명세서 필수 요건)
    int pid;                // 프로세스 ID (예: 1, 2, 3...)
    int arrival_time;       // 도착 시간 (Ready Queue에 들어온 시간)
    
    // 2. 작업량 정보 (명세서 필수 요건)
    int cpu_burst_time;     // 총 필요한 CPU 실행 시간
    int io_burst_time;      // I/O 작업에 필요한 시간 (Random 또는 고정)
    int priority;           // 우선순위 (Priority 스케줄링 알고리즘용)

    // 3. 시뮬레이션 상태 추적을 위한 변수 (선점형 구현 시 매우 중요!)
    int remaining_time;     // 남은 CPU 실행 시간 (초기값 = cpu_burst_tim
    // 4. 결과 분석용 변수 (Average waiting/turnaround time 계산용)
    int completion_time;    // 작업이 완전히 끝난 시간
    int waiting_time;       // 총 대기 시간
    int turnaround_time;    // 반환 시간 (완료 시간 - 도착 시간)
    int srandom;            // 랜덤값 (동점 처리용)
    int cpu_used;          // CPU 사용 시간 (I/O 요청 시점 체크용)
    int io_request_time;    // CPU 사용 중 I/O 요청 시점 (랜덤으로 설정)
    int io_done_time;       // I/O 작업이 완료된 시간 (I
} Process;
typedef struct {
    Process *data[SIZE];
    int currenop; //큐의 프로세스 수
} Queue;
// 1. 큐 초기화
void init_queue(Queue *q) {
    q->currenop = 0;
}

// 2. 비어있는지
int is_empty(Queue *q) {
    return q->currenop == 0;
}

// 3. 가득 찼는지
int is_full(Queue *q) {
    return q->currenop == SIZE;
}

// 4. 뒤에 추가
void enqueue(Queue *q, Process *p) {
    if (is_full(q)) {
        printf("que full\n");
        return;
    }
    q->data[q->currenop] = p;
    q->currenop++;
}
// 5. 앞에서 빼기 (FCFS, RR용)
Process* dequeue(Queue *q) {
    if (is_empty(q)) return NULL;
    
    Process *p = q->data[0];  // 맨 앞 저장
    
    // 나머지를 한 칸씩 앞으로 당기기
    for (int i = 0; i < q->currenop - 1; i++) {
        q->data[i] = q->data[i + 1];
    }
    
    q->currenop--;
    return p;
}
// 6. 중간에서 빼기 (SJF, Priority의 핵심)
Process* remove_at(Queue *q, int idx) {
    if (idx < 0 || idx >= q->currenop) return NULL;
    
    Process *p = q->data[idx];  // 해당 위치 저장
    
    // idx 다음 것들을 한 칸씩 앞으로 당기기
    for (int i = idx; i < q->currenop - 1; i++) {
        q->data[i] = q->data[i + 1];
    }
    
    q->currenop--;
    return p;
}

// 7. 현재 개수
int size(Queue *q) {
    return q->currenop;
}
Process* sjf_remove(Queue *q) {
    if (is_empty(q)) return NULL;
    //큐 빈
    int min_idx = 0;
    for (int i = 1; i < q->currenop; i++) {
        if (q->data[i]->remaining_time < q->data[min_idx]->remaining_time) {
            min_idx = i;
        }
    }
    return remove_at(q, min_idx);
}
Process* check_sjf(Queue *q) {
    if (is_empty(q)) return NULL;
    //큐 빈
    int min_idx = 0;
    for (int i = 1; i < q->currenop; i++) {
        if (q->data[i]->remaining_time < q->data[min_idx]->remaining_time) {
            min_idx = i;
        }
    }
    return q->data[min_idx];
}

Process* priority_remove(Queue *q) {
    if (is_empty(q)) return NULL;
    //큐 빈
    int min_idx = 0;
    for (int i = 1; i < q->currenop; i++) {
        if (q->data[i]->priority < q->data[min_idx]->priority) {
            min_idx = i;
        }
    }
    return remove_at(q, min_idx);
}

Process* check_priority(Queue *q) {
    if (is_empty(q)) return NULL;
    //큐 빈
    int min_idx = 0;
    for (int i = 1; i < q->currenop; i++) {
        if (q->data[i]->priority < q->data[min_idx]->priority) {
            min_idx = i;
        }
    }
    return q->data[min_idx];
}
//input empty pointer or array and int num_processes
void create_process(Process processarray[], int processnum){
    for (int i=0; i<processnum; i++){
        processarray[i].pid = i+1;
        processarray[i].arrival_time = rand() % 10;
        processarray[i].cpu_burst_time = (rand() % 10) + 1;
        processarray[i].io_burst_time = (rand() % 5); 
        processarray[i].priority = rand() % 5;
        processarray[i].remaining_time = processarray[i].cpu_burst_time; 
        processarray[i].completion_time = 0;
        processarray[i].waiting_time = 0;
        processarray[i].turnaround_time = 0;
        processarray[i].srandom = rand() % 1000;//랜덤값ㅔ
        processarray[i].cpu_used = 0;
        processarray[i].io_request_time = (rand() % processarray[i].cpu_burst_time) + 1; // CPU 사용 중 랜덤한 시점에 I/O 요청
        processarray[i].io_done_time = 0; // I/O 완료 시간 초기화

    }
}

//매 틱마다로 변경하기 
/*
void FIFO(Process processarray[], int processnum){
    int currentime = 0
    sortbyarrival(processarray, processnum,1 );
    for (int i=0; i<processnum; i++){
        if (currentime < processarray[i].arrival_time) {
            currentime = processarray[i].arrival_time; 
        }
        processarray[i].waiting_time = currentime - processarray[i].arrival_time;
        printf("[Time %3d - %3d] : Process %d 실행\n", currentime, currentime + processarray[i].cpu_burst_time, processarray[i].pid);
        currentime += processarray[i].cpu_burst_time;
        processarray[i].completion_time = currentime;
        processarray[i].turnaround_time = processarray[i].completion_time - processarray[i].arrival_time;
        
    }
}*/
//어짜피 코드 비슷하니까 걍 같은 알고리즘 기반에 정렬 큐만 다르게 ㄱㄱ
//Process* sjf_remove( Process* check_sjf(Queue *q
// Process* priority_remove(Queue *q) Process* check_priority(Queue *q)7
void unitedsort(Process processarray[], int processnum, Algorithm algo){
    int currentime = 0;
    int completedprocessprocess =0;
    //int inprocessnadready =0;
    //int inprocess =0;
    //int leftprocess=processnum;
    Queue ready_queue;
    Queue waiting_queue;
    Process *running = NULL;
    init_queue(&ready_queue);
    init_queue(&waiting_queue);
    // ----------------------------------------------------
    // [추가 1] 간트 차트 기록용 변수 선언
    GanttRecord records[1000]; // 조각이 많아질 수 있으니 넉넉하게 잡습니다.
    int record_cnt = 0;
    int prev_pid = -2;         // 이전 틱에서 실행된 프로세스 ID (-2는 초기 상태)
    // ---------------------------------------------------- AI러 ㅇ;ㄹ단
    while (completedprocessprocess < processnum) {
        //프로세스 도착하면 준비큐로
        for (int i = 0; i < processnum; i++) {
            if (processarray[i].arrival_time == currentime) {
                enqueue(&ready_queue, &processarray[i]);
                }
        }
        ///IO체크
        for (int i = 0; i < waiting_queue.currenop; ) {
            if (waiting_queue.data[i]->io_done_time <= currentime) {
                Process *p = remove_at(&waiting_queue, i);
                enqueue(&ready_queue, p);
                 // i 증가 안 함 (큐 당겨졌으니)
            } else {
                i++;
            }
        }
        //이부분만 다른거 불러오게
        switch (algo){
            case ALGO_FCFS:
            //no running process and que no empty and bring prcs from que
                if (running == NULL && !is_empty(&ready_queue)) {
                    running = dequeue(&ready_queue);
                }
                break;
            case ALGO_SJF:
                if (running == NULL && !is_empty(&ready_queue)) {
                    running = sjf_remove(&ready_queue);
                }
                break;
            case ALGO_SJF_PREEMPTIVE:
                if (!is_empty(&ready_queue)) {
                   Process* shortest = check_sjf(&ready_queue);//chck que
                   if ( running ==NULL) {
                    running = sjf_remove(&ready_queue);
                   }else if ( shortest->remaining_time < running->remaining_time) {
                    //현재 실행중인거보다 짧은거 있으면 바꿔주기
                    enqueue(&ready_queue, running); //현재 실행중인거 큐에 넣고
                    running = sjf_remove(&ready_queue); //짧은거 빼서 실행
                   }
                }
                break;
            case ALGO_PRIORITY:
                if (running == NULL && !is_empty(&ready_queue)) {
                    running = priority_remove(&ready_queue);
                }
                break;
            case ALGO_PRIORITY_PREEMPTIVE:
                if (!is_empty(&ready_queue)) {
                   Process* highest = check_priority(&ready_queue);//chck que
                   if ( running ==NULL) {
                    running = priority_remove(&ready_queue);
                   }else if ( highest->priority < running->priority) {
                    //현재 실행중인거보다 짧은거 있으면 바꿔주기
                    enqueue(&ready_queue, running); //현재 실행중인거 큐에 넣고
                    running = priority_remove(&ready_queue); //짧은거 빼서 실행
                    //
                   }
                }
                break;
            }
        // 2. waiting_time 증가는 별도 (큐 크기만큼만!)
        for (int i = 0; i < ready_queue.currenop; i++) {
            ready_queue.data[i]->waiting_time++;
        }
        // ----------------------------------------------------
        // [추가 2] 매 틱마다 CPU 상태를 확인해서 변경될 때만 기록!
        int current_pid = (running != NULL) ? running->pid : 0; // 0은 CPU가 쉬는 상태(Idle)

        if (current_pid != prev_pid) {
            // CPU 점유가 바뀌었다면 (Context Switch 발생)
            if (prev_pid != -2) { 
                // 처음 시작이 아니라면, 방금 전까지 실행되던 조각의 끝나는 시간을 현재 시간으로 닫아줌
                records[record_cnt].end_time = currentime;
                record_cnt++;
            }
            // 새로운 조각 기록 시작
            records[record_cnt].pid = current_pid;
            records[record_cnt].start_time = currentime;
            prev_pid = current_pid;
        }
        // ----------------------------------------------------여기도 일단 ai로 나중에 수정하
        //1틱씩 실행하기
        if (running != NULL) {
            running->remaining_time--;
            running->cpu_used++;
            // 4. 완료 체크
            if (running->remaining_time == 0) {
                running->completion_time = currentime + 1;
                running->turnaround_time = running->completion_time - running->arrival_time;
                completedprocessprocess++;
                running = NULL;  // CPU 비움
            }else if (running->cpu_used == running->io_request_time) {
                //IO요청 실행 중인 프로세스가 I/O 모드로 전환되어 잠시 빠진다
                //실행중이던 작업은 일단 waiting 큐로 빼고 IO 시작 IO중에도 다른 프로세스는 계속 돌아가게 종료후 wait에서 다시 rady로
                running->io_done_time = currentime + running->io_burst_time; // I/O 완료 시간 계산
                if (running->remaining_time > 0) {
                    // io ㅅㅣ간
                    running->io_request_time = running->cpu_used + (rand() % running->remaining_time) + 1;
                }
                enqueue(&waiting_queue, running); // 현재 실행 중인 프로세스를 큐에
                running = NULL; // CPU 비움

            }
        }
        
        currentime++;
    }
    // ----------------------------------------------------
    // [추가 3] 시뮬레이션이 모두 끝나면 마지막 조각의 시간을 닫고 출력!
    if (prev_pid != -2) {
        records[record_cnt].end_time = currentime;
    }
    
    // 방금 전에 작성한 출력 함수 호출 (count는 인덱스 0부터 시작했으니 +1)
    print_gantt_chart(records, record_cnt + 1);
    // ---------------------------------------------------- AI러 ㅇ;ㄹ단
}
// FCFS+시간퀀텀
// FCFS+시간퀀텀
void RRsort(Process processarray[], int processnum, int time_quantum){
    int currentime = 0;
    int completedprocessprocess =0;
    //int inprocessnadready =0;
    //int inprocess =0;
    //int leftprocess=processnum;
    Queue ready_queue;
    Queue waiting_queue;
    Process *running = NULL;
    init_queue(&ready_queue);
    init_queue(&waiting_queue);
    int usedtime=0;

    // ----------------------------------------------------
    // [추가 1] 간트 차트 기록용 변수 선언
    GanttRecord records[1000]; 
    int record_cnt = 0;
    int prev_pid = -2;         
    // ----------------------------------------------------

    while (completedprocessprocess < processnum) {
        //프로세스 도착하면 준비큐로
        for (int i = 0; i < processnum; i++) {
            if (processarray[i].arrival_time == currentime) {
                enqueue(&ready_queue, &processarray[i]);
                }
        }
        //Io추가 ㄲRR
        for (int i = 0; i < waiting_queue.currenop; ) {
            if (waiting_queue.data[i]->io_done_time <= currentime) {
                Process *p = remove_at(&waiting_queue, i);
                enqueue(&ready_queue, p);
            } else {
                i++;
            }
        }
        if (running == NULL && !is_empty(&ready_queue)) {
            running = dequeue(&ready_queue);
            usedtime = 0;
        }
        for (int i = 0; i < ready_queue.currenop; i++) {
            ready_queue.data[i]->waiting_time++;
        }

        // ----------------------------------------------------
        // [추가 2] 매 틱마다 CPU 상태를 확인해서 변경될 때만 기록!
        int current_pid = (running != NULL) ? running->pid : 0; 

        if (current_pid != prev_pid) {
            if (prev_pid != -2) { 
                records[record_cnt].end_time = currentime;
                record_cnt++;
            }
            records[record_cnt].pid = current_pid;
            records[record_cnt].start_time = currentime;
            prev_pid = current_pid;
        }
        // ----------------------------------------------------

        //Run RR
        if (running != NULL) {
            running->remaining_time--;
            running->cpu_used++; // [버그 수정] I/O 체크를 위해 누락되었던 cpu 사용 시간 증가 추가!
            usedtime++;
            
            // 4. 완료 체크
            if (running->remaining_time == 0) {
                running->completion_time = currentime + 1;
                running->turnaround_time = running->completion_time - running->arrival_time;
                completedprocessprocess++;
                running = NULL;  // CPU 비움
                usedtime=0;
            } else if (running->cpu_used == running->io_request_time) {
                running->io_done_time = currentime + running->io_burst_time;
                
                // [추가 로직] 다음 I/O가 언제 발생할지 새로 랜덤 세팅!
                if (running->remaining_time > 0) {
                    running->io_request_time = running->cpu_used + (rand() % running->remaining_time) + 1;
                }
                
                enqueue(&waiting_queue, running);
                running = NULL;
                usedtime = 0;
            } else if (usedtime == time_quantum) {
                //시간 퀀텀 다 쓰면 다시 큐에 넣기
                enqueue(&ready_queue, running);
                running = NULL;
                usedtime=0;
            }
        }
        currentime++;   
    }

    // ----------------------------------------------------
    // [추가 3] 시뮬레이션이 모두 끝나면 마지막 조각의 시간을 닫고 출력!
    if (prev_pid != -2) {
        records[record_cnt].end_time = currentime;
    }
    
    print_gantt_chart(records, record_cnt + 1);
    // ----------------------------------------------------
}
int printgantt(){
    return 0;
}
void evaluatesort(Process processarray[], int processnum, const char *algo_name) {
    printf("\n=== [%s] Evaluation Results ===\n", algo_name);
    
    int total_wait = 0, total_turn = 0;
    
    for (int i = 0; i < processnum; i++) {
        printf("P%d: completion=%d, wait=%d, turnaround=%d\n",
               processarray[i].pid, processarray[i].completion_time,
               processarray[i].waiting_time, processarray[i].turnaround_time);
               
        total_wait += processarray[i].waiting_time;
        total_turn += processarray[i].turnaround_time;
    }
    
    printf(">> Avg Waiting: %.2f, Avg Turnaround: %.2f\n",
           (float)total_wait / processnum, 
           (float)total_turn / processnum);
}//이것도 일단 ai로 만듬 나중에 수정
//Io cpu일시정지 근데 이걸 랜덤 틱으로?
//프로세스 생성시에 랜덤으로 틱들오오는 시간 설정
int IOintrrupt(){
    return 0;
}

/*void FIFO(Process processarray[], int processnum){
    int currentime = 0;
    int completedprocessprocess =0;
    //int inprocessnadready =0;
    //int inprocess =0;
    //int leftprocess=processnum;
    Queue ready_queue;
    Process *running = NULL;
    init_queue(&ready_queue);
    while (completedprocessprocess < processnum) {
        //프로세스 도착하면 준비큐로
        for (int i = 0; i < processnum; i++) {
            if (processarray[i].arrival_time == currentime) {
                enqueue(&ready_queue, &processarray[i]);
            }
            ready_queue.data[i]->waiting_time++;
        }
        //process empty -> 큐에서 꺼내기 
        if (running == NULL && !is_empty(&ready_queue)) {
            running = dequeue(&ready_queue);
        }
        //1틱씩 실행하기
        if (running != NULL) {
            running->remaining_time--;
            
            // 4. 완료 체크
            if (running->remaining_time == 0) {
                running->completion_time = currentime + 1;
                running->turnaround_time = running->completion_time - running->arrival_time;
                completedprocessprocess++;
                running = NULL;  // CPU 비움
            }
        }
        
        currentime++;
    }
}*/
int main(void) {
    srand(time(NULL));
    int num_processes = 5;    // 또는 사용자 입력 받기
    
    Process backup[SIZE];
    Process processarray[SIZE];
    
    create_process(backup, num_processes);
    
    printf("=== Original Processes ===\n");
    for (int i = 0; i < num_processes; i++) {    // n → num_processes
        printf("P%d: arrival=%d, burst=%d, priority=%d\n",
               backup[i].pid, backup[i].arrival_time,
               backup[i].cpu_burst_time, backup[i].priority);
    }
    
    // 알고리즘 5개 실행 (FCFS ~ Priority Preemptive)
    const char *names[] = {"FCFS", "SJF", "SJF Preemptive", 
                           "Priority", "Priority Preemptive"};
    Algorithm algos[] = {ALGO_FCFS, ALGO_SJF, ALGO_SJF_PREEMPTIVE,
                         ALGO_PRIORITY, ALGO_PRIORITY_PREEMPTIVE};
    
    for (int a = 0; a < 5; a++) {
        memcpy(processarray, backup, sizeof(Process) * num_processes);
        
        // 1. 시뮬레이션 돌리고 (내부에서 간트 차트 출력)
        unitedsort(processarray, num_processes, algos[a]);
        
        // 2. 결과 넘겨서 평가하기
        evaluatesort(processarray, num_processes, names[a]); 
    }
    
    // RR 알고리즘 실행
    memcpy(processarray, backup, sizeof(Process) * num_processes);
    
    // 1. RR 시뮬레이션 돌리고
    RRsort(processarray, num_processes, 4);
    
    // 2. 결과 넘겨서 평가하기
    evaluatesort(processarray, num_processes, "Round Robin (q=4)");
    
    return 0;
}
/*void print_gantt_chart(Process p[], int n)
{
    int i, j;
    // print top bar
    printf(" ");
    for(i=0; i<n; i++) {
        for(j=0; j<p[i].burst_time; j++) printf("--");
        printf(" ");
    }
    printf("\n|");
 
    // printing process id in the middle
    for(i=0; i<n; i++) {
        for(j=0; j<p[i].burst_time - 1; j++) printf(" ");
        printf("P%d", p[i].pid);
        for(j=0; j<p[i].burst_time - 1; j++) printf(" ");
        printf("|");
    }
    printf("\n ");
    // printing bottom bar
    for(i=0; i<n; i++) {
        for(j=0; j<p[i].burst_time; j++) printf("--");
        printf(" ");
    }
    printf("\n");
 
    // printing the time line
    printf("0");
    for(i=0; i<n; i++) {
        for(j=0; j<p[i].burst_time; j++) printf("  ");
        if(p[i].turnaround_time > 9) printf("\b"); // backspace : remove 1 space
        printf("%d", p[i].turnaround_time);
 
    }
    printf("\n");
 
}https://gist.github.com/tienminhvy/239823742c8aca42649c951399c7e24a 여기 출처로 개조 ㄱㄱ헛*/