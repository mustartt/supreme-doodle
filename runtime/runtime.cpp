#include "llvm-statepoint-tablegen.h"
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <libunwind.h>
#include <new>

extern "C" {

extern uint8_t _LLVM_StackMaps[];
extern int program_entry();
statepoint_table_t *table = nullptr;

void *runtime_allocate(size_t size) noexcept {
  auto ptr = operator new(size, std::nothrow);
  assert(ptr && "Runtime: panic failed to allocate");
  std::cerr << "Runtime: allocate " << ptr << " " << size << std::endl;
  return ptr;
}

void runtime_deallocate(void *ptr) noexcept {
  std::cerr << "Runtime: deallocate " << ptr << std::endl;
  operator delete(ptr, std::nothrow);
}

void runtime_gc_poll() {
  std::cerr << "Runtime: gc_poll" << std::endl;
  std::cerr << "--------------------------------" << std::endl;
  std::cerr << "  ret addr: " << __builtin_return_address(0) << " "
            << __builtin_return_address(1) << std::endl;

  std::cerr << "  stack: " << std::endl;

  unw_cursor_t cursor;
  unw_context_t context;

  unw_getcontext(&context);
  unw_init_local(&cursor, &context);

  do {
    unw_word_t offset, pc, sp;
    char sym[256];
    unw_get_reg(&cursor, UNW_REG_IP, &pc);
    unw_get_reg(&cursor, UNW_REG_SP, &sp);

    if (unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0) {
      fprintf(stderr, "    (%s+0x%lx): pc 0x%lx\n", sym, offset, pc);
    } else {
      fprintf(stderr, "    (unknown): pc 0x%lx\n", pc);
    }
    frame_info_t *frame_info = lookup_return_address(table, pc);
    if (frame_info) {
      char *stack_pointer = reinterpret_cast<char *>(sp);
      char *base_pointer = stack_pointer + (frame_info->frameSize - 8);

      std::cerr << "      frame: size " << frame_info->frameSize << " count "
                << frame_info->numSlots << " sp "
                << static_cast<void *>(stack_pointer) << " base "
                << static_cast<void *>(base_pointer) << std::endl;

      for (unsigned slot = 0; slot < frame_info->numSlots; ++slot) {
        pointer_slot_t ptr_slot = frame_info->slots[slot];
        char *pointer_addr = stack_pointer + ptr_slot.offset;

        std::cerr << "        Live Root " << static_cast<void *>(pointer_addr) << " offset "
                  << ptr_slot.offset << ": "
                  << *(reinterpret_cast<void **>(pointer_addr)) << std::endl;
      }
    }
  } while (unw_step(&cursor) > 0);

  std::cerr << "--------------------------------" << std::endl << std::endl;
}

void runtime_inspect_ptr(void *ptr) noexcept {
  std::cerr << "Runtime: inspect_ptr " << ptr << std::endl;
}
}

int main(int argc, char *argv[]) {
  std::cerr << "Runtime: starting up runtime..." << std::endl;

  table = generate_table(_LLVM_StackMaps, 1.0);
  print_table(stderr, table, true);

  std::cerr << "Runtime: entering program entry" << std::endl;

  // main program
  int rc = program_entry();

  std::cerr << "Runtime: Exit Code " << rc << std::endl;
  destroy_table(table);

  return 0;
}
