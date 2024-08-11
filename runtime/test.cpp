#include <cstdio>
#include <cstdlib>
#include <libunwind.h>

extern "C" {

void unwind() {
  fprintf(stderr, "stack: \n");

  unw_cursor_t cursor;
  unw_context_t context;

  unw_getcontext(&context);
  unw_init_local(&cursor, &context);

  while (unw_step(&cursor) > 0) {
    unw_word_t offset, pc, sp;
    char sym[256];
    unw_get_reg(&cursor, UNW_REG_IP, &pc);
    unw_get_reg(&cursor, UNW_REG_SP, &sp);

    if (unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0) {
      fprintf(stderr, " 0x%lx: (%s+0x%lx) sp: 0x%lx\n", pc, sym, offset, sp);
    } else {
      fprintf(stderr, " 0x%lx: sp:0x%lx\n", pc, sp);
    }
  }
}

int add(int a, int b) {
    int c = a + b;
    unwind();
    return c;
}

int main() {
    int c = add(1, 2);
    fprintf(stdout, "result: %d\n", c);
    return 0;
} 

}
