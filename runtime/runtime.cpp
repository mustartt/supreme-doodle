#include "llvm-statepoint-tablegen.h"
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <libunwind.h>
#include <new>
#include <unordered_set>
#include <deque>

extern "C" {

extern uint8_t __LLVM_StackMaps[];
extern int program_entry();
statepoint_table_t *table = nullptr;

enum class object_gc_state { White = 0, Gray = 1, Black = 2 };

struct __attribute__((packed, aligned(8))) object_metadata {
  uint32_t object_size;
  object_gc_state state;
};

std::unordered_set<object_metadata *> gc_white_set;
std::deque<void *> gc_gray_worklist;

void *runtime_allocate(uint32_t size) noexcept {
  static_assert(sizeof(object_metadata) == 8);

  void *base_ptr = operator new(sizeof(object_gc_state) + size, std::nothrow);
  assert(base_ptr && "Runtime: panic failed to allocate");

  object_metadata *metadata_ptr = new (base_ptr) object_metadata{
      .object_size = size,
      .state = object_gc_state::White,
  };
  gc_white_set.insert(metadata_ptr);
  void *obj = reinterpret_cast<char *>(base_ptr) + sizeof(object_gc_state);

  std::cerr << "Runtime: allocate " << obj << " " << size << std::endl;
  return obj;
}

void runtime_deallocate(void *ptr) noexcept {
  static_assert(sizeof(object_metadata) == 8);

  std::cerr << "Runtime: deallocate " << ptr << std::endl;
  operator delete(reinterpret_cast<char *>(ptr) - sizeof(object_gc_state),
                  std::nothrow);
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

        std::cerr << "        Live Root " << static_cast<void *>(pointer_addr)
                  << " offset " << ptr_slot.offset << ": "
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

  table = generate_table(__LLVM_StackMaps, 1.0);
  print_table(stderr, table, true);

  std::cerr << "Runtime: entering program entry" << std::endl;

  // main program
  int rc = program_entry();

  std::cerr << "Runtime: Exit Code " << rc << std::endl;
  destroy_table(table);

  return 0;
}
