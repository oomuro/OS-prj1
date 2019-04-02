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
  double vruntime;
  int nice;
} pcb;

// PCB linked list 구현을 위한 노드 struct
typedef struct NODE {
  pcb *this;
  struct NODE *next;
} node;

// PCB들 중에서 vruntime이 가장 작은 struct 찾아서 리턴
pcb *search_least(node *n) {
  node *cur = n;
  double least = cur->this->vruntime;
  while (n->next != NULL) {
    n = n->next;
    if (least > n->this->vruntime) {
      cur = n;
      least = cur->this->vruntime;
    }
  }
  return cur->this;
}

// 시그널 활용하기 위한 함수
void sigalrm(int signum) {}

// ts만큼 스케쥴러 실행
void run_ts(node *n, int ts) {
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
    pcb *least = search_least(n);
    kill(least->pid, SIGCONT);
    pause();
    kill(least->pid, SIGSTOP);
    switch (least->nice) {
    case -2:least->vruntime += NICE_MINUS_2;
      break;
    case -1:least->vruntime += NICE_MINUS_1;
      break;
    case 0:least->vruntime += NICE_ZERO;
      break;
    case 1:least->vruntime += NICE_PLUS_1;
      break;
    case 2:least->vruntime += NICE_PLUS_2;
      break;
    }
  }
}