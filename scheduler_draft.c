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
    ALGO_RR,
    ALGO_EDF,
    ALGO_RMS
} Algorithm;
typedef struct {
    int pid;// 실행된 프로세스 ID (CPU가 쉬었으면 -1 또는 0)
    int start_time;// 실행 시작 시간
    int end_time;// 실행 종료 시간
} GanttRecord; //간트 차트 추적용 일단

void print_gantt_chart(GanttRecord records[], int count, char *algo_name) {
    printf("\n========= Gantt Chart [%s] =========\n", algo_name);  // 수정 1
    int i, j;
    printf("  ");
    for(i = 0; i < count; i++){
        int duration = records[i].end_time - records[i].start_time;
        for(j = 0; j < duration; j++) printf("---");
        printf(" ");
    }
    printf("\n|");   // 수정 2: 줄바꿈 추가
    
    // 2. Middle (Process ID)
    for(i = 0; i < count; i++) {
        int duration = records[i].end_time - records[i].start_time;
        for(j = 0; j < duration - 1; j++) printf(" ");
        
        if (records[i].pid > 0) {
            printf("P%d", records[i].pid);
        } else {
            printf("ID");
        }
        
        for(j = 0; j < duration - 1; j++) printf(" ");
        printf("|");
    }
    printf("\n ");
    
    // 3. Bottom Bar
    for(i = 0; i < count; i++) {
        int duration = records[i].end_time - records[i].start_time;
        for(j = 0; j < duration; j++) printf("--");
        printf(" ");
    }
    printf("\n");
    
    // 4. Timeline
    printf("%3d", records[0].start_time);
    for(i = 0; i < count; i++) {
        int duration = records[i].end_time - records[i].start_time;
        for(j = 0; j < duration; j++) printf("   ");
        printf("%3d", records[i].end_time);
    }
    printf("\n");
}
typedef struct {
    int pid;//프로세스 id
    int arrival_time;//도착시간(ready queue)
    int cpu_burst_time;//실행 시간
    int io_burst_time;//IO랜덤 시간
    int priority;//priority기반용 
    int remaining_time;//남은 bursttime
    int completion_time;//작업 종료시간 기록용
    int waiting_time;//기다린 시간
    int turnaround_time;//반환 시간 (완료 시간 - 도착 시간)
    int cpu_used;// CPU 사용 시간 (I/O 요청 시점 체크용)
    int io_request_time;    // CPU 사용 중 I/O 요청 시점 (랜덤으로 설정)
    int io_done_time;// I/O 작업이 완료된 시간 (I
    /////EDF RMS
    int deadline;// EDF용 마감 시간
    int period;// RMS용 주기
    //반복용
    int repetear;//반복횟수 RMS EMD 아니면 일단 1 이고 이거 둘은 한 5정도로
    int deadline_missed;//놓친거

} Process;
typedef struct {
    Process *data[SIZE];
    int currenop; //큐에 지금 있는 프로세스수
} Queue;
// 큐 초기화
void init_queue(Queue *q) {
    q->currenop = 0;
}

// 비어있는지
int is_empty(Queue *q) {
    return q->currenop == 0;
    //1/0 리턴 C에 bool없어사
}

// 가득 찼는지
int is_full(Queue *q) {
    return q->currenop == SIZE;
    //다참 1 아님 0
}

// 뒤에 추가
void enqueue(Queue *q, Process *p) {
    if (is_full(q)) {//큐가 가득차면 추가 못하고 근대ㅔ 이게 일어나나
        printf("que full\n");
        return;
    }
    q->data[q->currenop] = p;///0부터니까 현재 개수= 지금 다음위치에저장
    q->currenop++;//데이터 개수 업데이트
}
// 앞에서 빼기 (FCFS, RR용)
Process* dequeue(Queue *q) {
    if (is_empty(q)) return NULL;//빈큐에서 못뻄
    Process *p = q->data[0];//임시저장하기 안날려먹게
    // 나머지를 한 칸씩 앞으로 당기기 데이터 하나빠졌으니까 
    for (int i = 0; i < q->currenop - 1; i++) {
        q->data[i] = q->data[i + 1];
    }
    //그리고 개수 하나 줄이고 
    q->currenop--;
    return p;
}
// 중간에서 빼기 (SJF, Priority용)
Process* remove_at(Queue *q, int idx) {
    if (idx < 0 || idx >= q->currenop) return NULL;//인덱스 검사하고( 음수 이런거)
    Process *p = q->data[idx]; // 해당 위치 저장 위랑 같음 근데 이건 중강에서 빼니까
    //위랑 같은데 이건 빼는거 앞은 굳이 ㄴ
    for (int i = idx; i < q->currenop - 1; i++) {
        q->data[i] = q->data[i + 1];
    }
    q->currenop--;
    return p;
}  
// 현재 개수6;.;
int size(Queue *q) {
    return q->currenop;
}
Process* sjf_remove(Queue *q) {
    if (is_empty(q)) return NULL;
    //큐 빈
    int min_idx = 0;
    for (int i = 1; i < q->currenop; i++) { // 동점-앞에 있는 큐가 나옴
        if (q->data[i]->remaining_time < q->data[min_idx]->remaining_time) {//더 작으면 먼저 나오게 함
            min_idx = i; //교체하고
        }
    }
    return remove_at(q, min_idx); //그 위치에서 빼기
}
//check어쩌고는 다 비슷한데 제거 안하고 그냥 다시 가져다둠, 선점형용에서 쓰게
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
//SJF랑 같은데 이제 비교대상이 우선순위로
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
//똑같은 걍 선점형용 보기
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
////////EDF RMS
Process* check_edf(Queue *q) {
    if (is_empty(q)) return NULL;
    int min_idx = 0;
    for (int i = 1; i < q->currenop; i++) {
        if (q->data[i]->deadline < q->data[min_idx]->deadline) {
            min_idx = i;
        }
    }
    return q->data[min_idx];
}

Process* edf_remove(Queue *q) {
    if (is_empty(q)) return NULL;
    int min_idx = 0;
    for (int i = 1; i < q->currenop; i++) {
        if (q->data[i]->deadline < q->data[min_idx]->deadline) {
            min_idx = i;
        }
    }
    return remove_at(q, min_idx);
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
        //processarray[i].srandom = rand() % 1000;//랜덤값ㅔ
        processarray[i].cpu_used = 0;
        processarray[i].io_request_time = (rand() % processarray[i].cpu_burst_time) + 1; // CPU 사용 중 랜덤한 시점에 I/O 요청
        processarray[i].io_done_time = 0; // I/O 완료 시간 초기화
        //EDF RMS용인데 일단 넣어보고
        processarray[i].period = (rand() % 15) + 10;
        processarray[i].deadline = processarray[i].arrival_time + processarray[i].period;
        processarray[i].repetear = 1; //dlfeks 1dlsep
        processarray[i].deadline_missed = 0;
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
    // ---------------------------------------------------- AI추가
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
                //ㅁㅇㅇ EDF RMS
            /*case ALGO_EDF:
            if (!is_empty(&ready_queue)) {
                Process* earliest = check_edf(&ready_queue);
                if (running == NULL) {
                    running = edf_remove(&ready_queue);
                } else if (earliest->deadline < running->deadline) {
                    // 데드라인 먼저면
                    enqueue(&ready_queue, running);
                    running = edf_remove(&ready_queue);
                }
            }
            break;일단 RR기반으로 다시 만들어봄*/
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
    print_gantt_chart(records, record_cnt + 1, "not RR");
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
    if (prev_pid != -2) {
        records[record_cnt].end_time = currentime;
    }
    print_gantt_chart(records, record_cnt + 1, "RR");
}
//relative_deadline = period
//absolute_deadline = arrival_time + period
void EDFsort(Process processarray[], int processnum) {
    int currentime = 0;
    int completedprocessprocess = 0;
    Queue ready_queue;
    Queue waiting_queue;
    Process *running = NULL;
    init_queue(&ready_queue);
    init_queue(&waiting_queue);
    
    GanttRecord records[1000];
    int record_cnt = 0;
    int prev_pid = -2;
    
    while (completedprocessprocess < processnum) {
        // 1. 도착 처리
        for (int i = 0; i < processnum; i++) {
            if (processarray[i].arrival_time == currentime) {
                enqueue(&ready_queue, &processarray[i]);
            }
        }
        
        // 2. I/O 완료 체크
        for (int i = 0; i < waiting_queue.currenop; ) {
            if (waiting_queue.data[i]->io_done_time <= currentime) {
                Process *p = remove_at(&waiting_queue, i);
                enqueue(&ready_queue, p);
            } else {
                i++;
            }
        }
        
        // 3. ★ EDF 선택 (선점형!) ★
        if (!is_empty(&ready_queue)) {
            Process *earliest = check_edf(&ready_queue);
            if (running == NULL) {
                running = edf_remove(&ready_queue);
            } else if (earliest->deadline < running->deadline) {
                // 더 급한 작업이 있으면 선점
                enqueue(&ready_queue, running);
                running = edf_remove(&ready_queue);
            }
        }
        
        // 4. waiting_time 증가
        for (int i = 0; i < ready_queue.currenop; i++) {
            ready_queue.data[i]->waiting_time++;
        }
        
        // 5. Gantt 기록
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
        
        // 6. ★★★ 데드라인 체크 (하드 RT 핵심!) ★★★
        if (running != NULL && currentime >= running->deadline) {
            // 데드라인 도달했는데 아직 안 끝남 → 미스!
            running->deadline_missed++;
            printf("[Time %d] P%d DEADLINE MISS!\n", currentime, running->pid);
            
            if (running->repetear > 1) {
                // 다음 인스턴스 즉시 시작 (RR의 quantum 만료와 유사)
                running->repetear--;
                running->arrival_time = currentime;
                running->deadline = currentime + running->period;
                running->remaining_time = running->cpu_burst_time;
                running->cpu_used = 0;
                running->io_request_time = (rand() % running->cpu_burst_time) + 1;
                running->io_done_time = 0;
                // running 유지 (즉시 새 인스턴스 실행)
            } else {
                // 마지막 인스턴스 → 종료
                running->completion_time = currentime;
                completedprocessprocess++;
                running = NULL;
            }
            currentime++;
            continue;  // 이 tick 처리 끝
        }
        
        // 7. 실행
        if (running != NULL) {
            running->remaining_time--;
            running->cpu_used++;
            
            // 7-a. 완료 체크 (데드라인 안에 끝남!)
            if (running->remaining_time == 0) {
                running->completion_time = currentime + 1;
                running->turnaround_time += running->completion_time - running->arrival_time;
                
                if (running->repetear > 1) {
                    // 다음 인스턴스 예약 (period 후 도착)
                    running->repetear--;
                    running->arrival_time = running->deadline; //드리프트 방지
                    running->deadline = running->arrival_time + running->period;
                    running->remaining_time = running->cpu_burst_time;
                    running->cpu_used = 0;
                    running->io_request_time = (rand() % running->cpu_burst_time) + 1;
                    running->io_done_time = 0;
                    // CPU 비우고 도착 대기
                } else {
                    // 마지막 인스턴스 정상 완료
                    completedprocessprocess++;
                }
                running = NULL;
            }
            // 7-b. I/O 발생
            else if (running->cpu_used == running->io_request_time) {
                running->io_done_time = currentime + running->io_burst_time;
                if (running->remaining_time > 0) {
                    running->io_request_time = running->cpu_used + (rand() % running->remaining_time) + 1;
                }
                enqueue(&waiting_queue, running);
                running = NULL;
            }
        }
        
        currentime++;
    }
    
    if (prev_pid != -2) {
        records[record_cnt].end_time = currentime;
    }
    print_gantt_chart(records, record_cnt + 1, "EDF");
}
void RMSsort(Process processarray[], int processnum) {
    int currentime = 0;
    int completedprocessprocess = 0;
    Queue ready_queue;
    Queue waiting_queue;
    Process *running = NULL;
    init_queue(&ready_queue);
    init_queue(&waiting_queue);
    
    GanttRecord records[1000];
    int record_cnt = 0;
    int prev_pid = -2;
    
    while (completedprocessprocess < processnum) {
        // 1. 도착 처리
        for (int i = 0; i < processnum; i++) {
            if (processarray[i].arrival_time == currentime) {
                enqueue(&ready_queue, &processarray[i]);
            }
        }
        
        // 2. I/O 완료 체크
        for (int i = 0; i < waiting_queue.currenop; ) {
            if (waiting_queue.data[i]->io_done_time <= currentime) {
                Process *p = remove_at(&waiting_queue, i);
                enqueue(&ready_queue, p);
            } else {
                i++;
            }
        }
        
        // 3. ★ EDF 선택 (선점형!) ★
        if (!is_empty(&ready_queue)) {
            Process *highest = check_priority(&ready_queue);
            if (running == NULL) {
                running = priority_remove(&ready_queue);
            } else if (highest->priority < running->priority) {
                // 더 급한 작업이 있으면 선점
                enqueue(&ready_queue, running);
                running = priority_remove(&ready_queue);
            }
        }
        
        // 4. waiting_time 증가
        for (int i = 0; i < ready_queue.currenop; i++) {
            ready_queue.data[i]->waiting_time++;
        }
        
        // 5. Gantt 기록
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
        
        // 6. ★★★ 데드라인 체크 (하드 RT 핵심!) ★★★
        if (running != NULL && currentime >= running->deadline) {
            // 데드라인 도달했는데 아직 안 끝남 → 미스!
            running->deadline_missed++;
            printf("[Time %d] P%d DEADLINE MISS!\n", currentime, running->pid);
            
            if (running->repetear > 1) {
                // 다음 인스턴스 즉시 시작 (RR의 quantum 만료와 유사)
                running->repetear--;
                running->arrival_time = currentime;
                running->deadline = currentime + running->period;
                running->remaining_time = running->cpu_burst_time;
                running->cpu_used = 0;
                running->io_request_time = (rand() % running->cpu_burst_time) + 1;
                running->io_done_time = 0;
                // running 유지 (즉시 새 인스턴스 실행)
            } else {
                // 마지막 인스턴스 → 종료
                running->completion_time = currentime;
                completedprocessprocess++;
                running = NULL;
            }
            currentime++;
            continue;  // 이 tick 처리 끝
        }
        
        // 7. 실행
        if (running != NULL) {
            running->remaining_time--;
            running->cpu_used++;
            
            // 7-a. 완료 체크 (데드라인 안에 끝남!)
            if (running->remaining_time == 0) {
                running->completion_time = currentime + 1;
                running->turnaround_time += running->completion_time - running->arrival_time;
                
                if (running->repetear > 1) {
                    // 다음 인스턴스 예약 (period 후 도착)
                    running->repetear--;
                    running->arrival_time = running->deadline;//여기도 드리프트
                    running->deadline = running->arrival_time + running->period;
                    running->remaining_time = running->cpu_burst_time;
                    running->cpu_used = 0;
                    running->io_request_time = (rand() % running->cpu_burst_time) + 1;
                    running->io_done_time = 0;
                    // CPU 비우고 도착 대기
                } else {
                    // 마지막 인스턴스 정상 완료
                    completedprocessprocess++;
                }
                running = NULL;
            }
            // 7-b. I/O 발생
            else if (running->cpu_used == running->io_request_time) {
                running->io_done_time = currentime + running->io_burst_time;
                if (running->remaining_time > 0) {
                    running->io_request_time = running->cpu_used + (rand() % running->remaining_time) + 1;
                }
                enqueue(&waiting_queue, running);
                running = NULL;
            }
        }
        
        currentime++;
    }
    
    if (prev_pid != -2) {
        records[record_cnt].end_time = currentime;
    }
    print_gantt_chart(records, record_cnt + 1, "RMS");
}
int printgantt(){
    return 0;
}
void evaluatesort(Process processarray[], int processnum, const char *algo_name) {
    printf("\n=== [%s] Evaluation Results ===\n", algo_name);
    
    int total_wait = 0, total_turn = 0;
    int total_misses = 0;
    
    for (int i = 0; i < processnum; i++) {
        printf("P%d: completion=%d, wait=%d, turnaround=%d\n",
               processarray[i].pid, processarray[i].completion_time,
               processarray[i].waiting_time, processarray[i].turnaround_time);
               
        total_wait += processarray[i].waiting_time;
        total_turn += processarray[i].turnaround_time;
        total_misses += processarray[i].deadline_missed;
    }
    
    printf(">> Avg Waiting: %.2f, Avg Turnaround: %.2f\n",
           (float)total_wait / processnum, 
           (float)total_turn / processnum);
    if (total_misses > 0) {
        printf(">> Total Deadline Misses: %d\n", total_misses);
    } else {
        printf(">> All deadlines met!\n");
    }
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
    int is_generated = 0;
    int choice;
    while (1) {
        printf("\n================ CPU 스케줄링 시뮬레이터 ================\n");
        printf(" 1. 프로세스 랜덤 생성 (현재 상태: %s)\n", is_generated ? "생성 완료" : "미생성");
        printf(" 2. FCFS (First-Come First-Served) 실행\n");
        printf(" 3. SJF (Shortest Job First - 비선점/선점 선택) 실행\n");
        printf(" 4. Priority (우선순위 - 비선점/선점 선택) 실행\n");
        printf(" 5. Round Robin (RR) 실행\n");
        printf(" 6. Rate-Monotonic (RM) 실시간 스케줄링 실행\n");
        printf(" 7. Earliest Deadline First (EDF) 실시간 스케줄링 실행\n");
        printf(" 8. 시뮬레이터 종료\n");
        printf("========================================================\n");
        printf(" 메뉴를 선택하세요: ");
        
        if (scanf("%d", &choice) != 1) break;
        if (choice == 8) {
            printf("시뮬레이터를 종료합니다.\n");
            break;
        }

        if (choice == 1) {
            printf("생성할 프로세스 개수를 입력하세요 (최대 %d): ", SIZE);
            scanf("%d", &num_processes);
            create_process(backup, num_processes);
            is_generated = 1;
            
            printf("\n[생성된 원본 프로세스 목록]\n");
            for (int i = 0; i < num_processes; i++) {
                printf("P%d: Arrival=%d, Burst=%d, Priority=%d, Period=%d, Deadline=%d\n",
                       backup[i].pid, backup[i].arrival_time, backup[i].cpu_burst_time, 
                       backup[i].priority, backup[i].period, backup[i].deadline);
            }
            continue;
        }

        // 2~7번 메뉴는 프로세스가 먼저 생성되어 있어야 실행 가능
        if (!is_generated) {
            printf("오류: 프로세스를 먼저 생성(1번 메뉴)해야 시뮬레이션을 수행할 수 있습니다.\n");
            continue;
        }

        // 실행할 때마다 원본 데이터 백업본에서 복사하여 독립된 환경 보장
        memcpy(processarray, backup, sizeof(Process) * num_processes);

        switch (choice) {
            case 2:
                unitedsort(processarray, num_processes, ALGO_FCFS);
                evaluatesort(processarray, num_processes, "FCFS");
                break;
            case 3: {
                int sub_choice;
                printf("1. 비선점형 SJF  |  2. 선점형 SJF (SRTF)\n선택: ");
                scanf("%d", &sub_choice);
                if (sub_choice == 1) {
                    unitedsort(processarray, num_processes, ALGO_SJF);
                    evaluatesort(processarray, num_processes, "SJF (Non-preemptive)");
                } else {
                    unitedsort(processarray, num_processes, ALGO_SJF_PREEMPTIVE);
                    evaluatesort(processarray, num_processes, "SJF (Preemptive)");
                }
                break;
            }
            case 4: {
                int sub_choice;
                printf("1. 비선점형 Priority  |  2. 선점형 Priority\n선택: ");
                scanf("%d", &sub_choice);
                if (sub_choice == 1) {
                    unitedsort(processarray, num_processes, ALGO_PRIORITY);
                    evaluatesort(processarray, num_processes, "Priority (Non-preemptive)");
                } else {
                    unitedsort(processarray, num_processes, ALGO_PRIORITY_PREEMPTIVE);
                    evaluatesort(processarray, num_processes, "Priority (Preemptive)");
                }
                break;
            }
            case 5: {
                int quantum;
                printf("Time Quantum 값을 입력하세요: ");
                scanf("%d", &quantum);
                RRsort(processarray, num_processes, quantum);
                evaluatesort(processarray, num_processes, "Round Robin");
                break;
            }
            case 6://RMS
                for (int i = 0; i < num_processes; i++) {
                    processarray[i].priority = processarray[i].period; 
                    processarray[i].repetear = 5;
                    processarray[i].deadline_missed = 0;
                }
                RMSsort(processarray, num_processes); 
                evaluatesort(processarray, num_processes, "Rate-Monotonic (RM)");
                break;
            case 7://EDF
                for (int i = 0; i < num_processes; i++) {
                    processarray[i].repetear = 5;
                    processarray[i].deadline_missed = 0;
                }
                EDFsort(processarray, num_processes);
                evaluatesort(processarray, num_processes, "Earliest Deadline First (EDF)");
                break;
            default:
                printf("잘못된 번호입니다. 다시 선택해주세요.\n");
        }
    }
    return 0;
    /*
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
    
    return 0;&*/
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
/* 3. git 명령어
git add scheduler_draft.c
git commit -m "..."
git push*/