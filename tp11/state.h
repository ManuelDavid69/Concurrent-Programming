#ifndef TP11_STATE_H
#define TP11_STATE_H

#define TP11_CALC_SEM_KEY 0x0B0B0001
#define TP11_CALC_SHM_KEY 0x0B0B0002
#define TP11_IMG_SEM_KEY 0x0B0B0003
#define TP11_IMG_SHM_KEY 0x0B0B0004

#define CALC_MAX_CLIENTS 8
#define CALC_QUEUE_CAPACITY 16
#define IMG_TASK_SLOTS 5
#define IMG_PATH_MAX 10000

enum {
    CALC_MUTEX = 0,
    CALC_SLOTS = 1,
    CALC_ITEMS = 2,
    CALC_RESULT_BASE = 3,
    CALC_SEM_COUNT = CALC_RESULT_BASE + CALC_MAX_CLIENTS
};

enum {
    IMG_MUTEX = 0,
    IMG_SLOTS = 1,
    IMG_ITEMS = 2,
    IMG_SEM_COUNT = 3
};

typedef struct {
    int client_id;
    int left;
    int right;
    char operator;
} CalcRequest;

typedef struct {
    int left;
    int right;
    int result;
    char operator;
} CalcResponse;

typedef struct {
    int head;
    int tail;
    int count;
    CalcRequest queue[CALC_QUEUE_CAPACITY];
    CalcResponse results[CALC_MAX_CLIENTS];
    int client_pid[CALC_MAX_CLIENTS];
} CalcShared;

typedef struct {
    int head;
    int tail;
    int count;
    int active_clients;
    int finished;
    char tasks[IMG_TASK_SLOTS][IMG_PATH_MAX];
} ImageShared;

#endif
