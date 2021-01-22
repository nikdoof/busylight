struct status
{
  char name[20];
  int r;
  int g;
  int b;
  int blink;
};

struct status statuses[] = {
  {"busy", 255, 0, 0, 0},
  {"available", 0, 255, 0, 0},
  {"away", 150, 255, 0, 0},
  {"ooo", 255, 0, 255, 0},
  {"offline", 0, 0, 0, 0},
  {"blue", 0, 0, 255, 10}
};
#define STATUSES_COUNT 6