// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <SlobAllocator.h>

#include <Console.h>
#include <Kernel.h>
#include <Math.h>

namespace valkyrie::kernel {

SlobAllocator::SlobAllocator(PageFrameAllocator* page_frame_allocator)
    : _page_frame_allocator(page_frame_allocator),
      _free_lists(),
      _top_chunk(page_frame_allocator->allocate_one_page_frame()) {}


void* SlobAllocator::allocate(size_t requested_size) {
  if (!requested_size) {
    return nullptr;
  }

  requested_size = round_up_to_pow2(requested_size + sizeof(Slob));
  int order = size_to_order(requested_size);

  if (order >= MAX_ORDER) {
    Kernel::panic("order >= CHUNK_MAX_ORDER\n");
  }

  Slob* victim = _free_lists[order];

  if (victim && victim->order == size_to_order(requested_size)) {
    printf("freelist hit!\n");
    free_list_del_head(victim);
    dump_slob_info();
    return victim + 1;
  }

  printf("no existing free block available\n");

  if (is_top_chunk_used_up()) {
    _top_chunk = _page_frame_allocator->allocate_one_page_frame();
  }

  victim = split_from_top_chunk(requested_size);
  dump_slob_info();
  return victim + 1;  // skip the header
}

void SlobAllocator::deallocate(void* p) {
  if (!p) {
    return;
  }

  Slob* slob = reinterpret_cast<Slob*>(p) - 1;  // 1 is for the header
  free_list_add_head(slob);
  dump_slob_info();
}

void SlobAllocator::dump_slob_info() const {
  puts("--- dumping slob free lists ---");

  for (int i = 0; i < SLOB_MAX_ORDER; i++) {
    printf("_free_lists[%d]: ", i);
    Slob* ptr = _free_lists[i];
    while (ptr) {
      printf("[%d] -> ", ptr->order);
      ptr = ptr->next;
    }
    printf("[nil]\n");
  }

  puts("--- end dumping slob free lists ---");
}


SlobAllocator::Slob* SlobAllocator::split_from_top_chunk(size_t requested_size) {
  Slob* chunk = reinterpret_cast<Slob*>(_top_chunk);

  size_t top_chunk = reinterpret_cast<size_t>(_top_chunk);
  top_chunk += requested_size;
  _top_chunk = reinterpret_cast<void*>(top_chunk);

  return chunk;
}

bool SlobAllocator::is_top_chunk_used_up() const {
  size_t top_chunk = reinterpret_cast<size_t>(_top_chunk);
  return top_chunk >= top_chunk + PAGE_SIZE;
}


void SlobAllocator::free_list_del_head(Slob* chunk) {
  if (!_free_lists[chunk->order]) {
    return;
  }
  _free_lists[chunk->order] = _free_lists[chunk->order]->next;
  chunk->next = nullptr;
}

void SlobAllocator::free_list_add_head(Slob* chunk) {
  if (_free_lists[chunk->order] == chunk) {
    return;
  }

  // If the list is empty
  if (!_free_lists[chunk->order]) {
    _free_lists[chunk->order] = chunk;
    chunk->next = nullptr;
  } else {
    chunk->next = _free_lists[chunk->order];
    _free_lists[chunk->order] = chunk;
  }
}

int SlobAllocator::size_to_order(const size_t size) {
  return log2(size / CHUNK_SIZE);
}

}  // namespace valkyrie::kernel
