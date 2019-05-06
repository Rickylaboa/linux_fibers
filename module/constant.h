#define ACTIVE_FIBER 1
#define INACTIVE_FIBER 0


struct fiber_info {

  void *stack;
  void (*routine)(void *);
  void *args;
};
