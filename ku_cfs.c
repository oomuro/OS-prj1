#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "ku_cfs.h"

int main(int argc, char *argv[]) {

  // 프로세스 개수 할당
  int proc_num[5] = {0,};
  int total_proc_num = 0;
  for (int i = 0; i < 5; i++) {
    proc_num[i] = strtol(argv[i + 1], (char **) NULL, 10);
    total_proc_num += proc_num[i];
  }

  // time slice 할당
  int ts = strtol(argv[6], (char **) NULL, 10);

  // process 저장할 큐 생성 및 초기화
  queue *procs = malloc(sizeof(queue));
  init(procs);

  // ku_app에서 출력할 문자 ('A'부터 할당될 때마다 증가)
  char *output = calloc(2, sizeof(char));
  output[0] = 'A';
  output[1] = '\0';

  // 노드별로 프로세스 할당 (args에서 앞에 있는거부터 순서대로 넣음)
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < proc_num[i]; j++) {
      pcb *p = calloc(1, sizeof(pcb));
      p->pid = fork();
      if (p->pid == 0) {
        // child면 프로세스 소환
        execl("./ku_app", "ku_app", output, (char *) NULL);
      }
      output[0]++;
      p->nice = i - 2;
      enqueue(procs, *p);
    }
  }
  // 오류 방지를 위해 5초간 휴식
  sleep(5);

  // 스케쥴러 실행
  run_ts(procs, ts);

  // ts만큼 실헹 후에 다 끝나면 모든 프로세스 임의로 종료
  node *fin = procs->front;
  for (int i = 0; i < total_proc_num; i++) {
    kill(fin->this.pid, SIGKILL);
    fin = fin->next;
  }

  return 0;
}