#include <sys/time.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

// 1.25^nice
#define NICE_MINUS_2 0.6400
#define NICE_MINUS_1 0.8000
#define NICE_ZERO    1.0000
#define NICE_PLUS_1  1.2500
#define NICE_PLUS_2  1.5625

// PCB struct
typedef struct PCB {
  int pid;
  int nice;
  double vruntime;
} pcb;

// PCB linked list 구현을 위한 노드 struct
typedef struct NODE {
  pcb this;
  struct NODE *next;
} node;

// node를 담고 있는 queue
typedef struct QUEUE {
  int count;
  node *front;
  node *rear;
} queue;

// queue 초기화
void init(queue *q) {
  q->count = 0;
  q->front = NULL;
  q->rear = NULL;
}

// queue가 비었는지 확인
int is_empty(queue *q) {
  return q->count == 0;
}

// queue에 pcb값을 가지고 node 생성 후 저장
void enqueue(queue *q, pcb value) {
  node *tmp = calloc(1, sizeof(node));
  tmp->this = value;
  tmp->next = NULL;
  if (!is_empty(q)) {
    q->rear->next = tmp;
    q->rear = tmp;
  } else {
    q->front = q->rear = tmp;
  }
  q->count++;
}

// queue에서 맨 처음 node의 pcb값 얻어옴
pcb dequeue(queue *q) {
  node *tmp = q->front;
  pcb p = q->front->this;
  q->front = q->front->next;
  q->count--;
  if (is_empty(q)) {
    q->rear = NULL;
  }
  free(tmp);
  return p;
}

// queue에 vruntime 값에 따라 끝난 프로세스 저장
void insert(queue *q, pcb value) {
  node *tmp = q->front;
  node *ins = calloc(1, sizeof(node));
  ins->this = value;

  if (is_empty(q)) {
    q->front = q->rear = ins;
    q->count++;
  } else if (q->count == 1) {
    if (tmp->this.vruntime > value.vruntime) {
      ins->next = tmp;
      q->front = ins;
      q->count++;
    } else {
      tmp->next = ins;
      q->rear = ins;
      q->count++;
    }
  } else if (q->count == 2) {
    if (tmp->this.vruntime > value.vruntime) {
      ins->next = tmp;
      q->front = ins;
      q->count++;
    } else if (tmp->next->this.vruntime > value.vruntime) {
      ins->next = tmp->next;
      tmp->next = ins;
      q->count++;
    } else {
      tmp->next->next = ins;
      ins->next = NULL;
      q->rear = ins;
      q->count++;
    }
  } else {
    for (int i = 0; i < q->count; i++) {
      if (i == 0) {
        if (tmp->this.vruntime > value.vruntime) {
          ins->next = tmp;
          q->front = ins;
          q->count++;
          break;
        } else if (tmp->next->this.vruntime > value.vruntime) {
          ins->next = tmp->next;
          tmp->next = ins;
          q->count++;
          break;
        } else {
          tmp = tmp->next;
        }
      } else {
        if (tmp->next == NULL) {
          tmp->next = ins;
          q->rear = ins;
          ins->next = NULL;
          q->count++;
          break;
        } else {
          if (tmp->next->this.vruntime > value.vruntime) {
            ins->next = tmp->next;
            tmp->next = ins;
            q->count++;
            break;
          } else {
            tmp = tmp->next;
          }
        }
      }
    }
  }
}

// 시그널 활용하기 위한 함수
void sigalrm(int signum) {}

// ts만큼 스케쥴러 실행
void run_ts(queue *q, int ts) {
  struct sigaction sa;
  struct itimerval timer;

  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = &sigalrm;
  sigaction(SIGALRM, &sa, NULL);

  // itimer 값을 1초로 설정
  timer.it_value.tv_sec = 1;
  timer.it_value.tv_usec = 0;
  timer.it_interval.tv_sec = 1;
  timer.it_interval.tv_usec = 0;

  setitimer(ITIMER_REAL, &timer, NULL);

  for (int i = 0; i < ts; i++) {
    pcb least = dequeue(q);
    kill(least.pid, SIGCONT);
    pause();
    kill(least.pid, SIGSTOP);
    switch (least.nice) {
    case -2:least.vruntime += NICE_MINUS_2;
      break;
    case -1:least.vruntime += NICE_MINUS_1;
      break;
    case 0:least.vruntime += NICE_ZERO;
      break;
    case 1:least.vruntime += NICE_PLUS_1;
      break;
    case 2:least.vruntime += NICE_PLUS_2;
      break;
    }
    insert(q, least);
  }
}