/*
 * Copyright 2025 Samuel "Lord-Windy" Brown
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "chained_adapter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct ChainedPage {
  struct ChainedPage *next;
  uint64_t size;
  uint64_t used;
  uint8_t data[];
} ChainedPage;

typedef struct {
  ChainedPage *first_page;
  ChainedPage *current_page;
  uint64_t page_size;
  uint64_t growth_pages;
  uint64_t total_allocated;
  uint64_t total_capacity;
  bool enable_stats;
  bool enable_debug;
} ChainedContext;

static SamrenaError chained_create(void **context, const void *config) {
  if (!context || !config) {
    return SAMRENA_ERROR_NULL_POINTER;
  }

  const SamrenaConfig *cfg = (const SamrenaConfig *)config;

  ChainedContext *ctx = malloc(sizeof(ChainedContext));
  if (!ctx) {
    return SAMRENA_ERROR_OUT_OF_MEMORY;
  }

  ctx->page_size = cfg->page_size > 0 ? cfg->page_size : NEAT_INITIAL_PAGE_SIZE;
  ctx->growth_pages = cfg->growth_pages > 0 ? cfg->growth_pages : 1;
  ctx->total_allocated = 0;
  ctx->total_capacity = 0;
  ctx->enable_stats = cfg->enable_stats;
  ctx->enable_debug = cfg->enable_debug;
  ctx->first_page = NULL;
  ctx->current_page = NULL;

  // Allocate initial pages
  uint64_t initial_pages = cfg->initial_pages;
  for (uint64_t i = 0; i < initial_pages; i++) {
    ChainedPage *page = malloc(sizeof(ChainedPage) + ctx->page_size);
    if (!page) {
      // Cleanup on failure
      while (ctx->first_page) {
        ChainedPage *next = ctx->first_page->next;
        free(ctx->first_page);
        ctx->first_page = next;
      }
      free(ctx);
      return SAMRENA_ERROR_OUT_OF_MEMORY;
    }

    page->next = NULL;
    page->size = ctx->page_size;
    page->used = 0;

    if (!ctx->first_page) {
      ctx->first_page = page;
      ctx->current_page = page;
    } else {
      ctx->current_page->next = page;
      ctx->current_page = page;
    }

    ctx->total_capacity += ctx->page_size;
  }

  // Reset current page to first for allocation
  ctx->current_page = ctx->first_page;

  *context = ctx;
  return SAMRENA_SUCCESS;
}

static void chained_destroy(void *context) {
  if (!context)
    return;

  ChainedContext *ctx = (ChainedContext *)context;
  ChainedPage *page = ctx->first_page;

  while (page) {
    ChainedPage *next = page->next;
    free(page);
    page = next;
  }

  free(ctx);
}

static ChainedPage *chained_find_available_page(ChainedContext *ctx, uint64_t size) {
  ChainedPage *page = ctx->current_page;

  // Search from current page forward
  while (page) {
    if (page->size - page->used >= size) {
      return page;
    }
    page = page->next;
  }

  // Search from beginning to current page
  page = ctx->first_page;
  while (page && page != ctx->current_page) {
    if (page->size - page->used >= size) {
      return page;
    }
    page = page->next;
  }

  return NULL;
}

static SamrenaError chained_add_pages(ChainedContext *ctx, uint64_t min_size) {
  ChainedPage *last_page = ctx->first_page;
  while (last_page && last_page->next) {
    last_page = last_page->next;
  }

  // If the allocation is larger than our standard page size, create a single large page
  if (min_size > ctx->page_size) {
    ChainedPage *page = malloc(sizeof(ChainedPage) + min_size);
    if (!page) {
      return SAMRENA_ERROR_OUT_OF_MEMORY;
    }

    page->next = NULL;
    page->size = min_size;
    page->used = 0;

    if (last_page) {
      last_page->next = page;
    } else {
      ctx->first_page = page;
      ctx->current_page = page;
    }

    ctx->total_capacity += min_size;
    return SAMRENA_SUCCESS;
  }

  // Otherwise, add standard-sized pages
  uint64_t pages_needed = (min_size + ctx->page_size - 1) / ctx->page_size;
  if (pages_needed < ctx->growth_pages) {
    pages_needed = ctx->growth_pages;
  }

  for (uint64_t i = 0; i < pages_needed; i++) {
    ChainedPage *page = malloc(sizeof(ChainedPage) + ctx->page_size);
    if (!page) {
      return SAMRENA_ERROR_OUT_OF_MEMORY;
    }

    page->next = NULL;
    page->size = ctx->page_size;
    page->used = 0;

    if (last_page) {
      last_page->next = page;
    } else {
      ctx->first_page = page;
      ctx->current_page = page;
    }
    last_page = page;

    ctx->total_capacity += ctx->page_size;
  }

  return SAMRENA_SUCCESS;
}

static void *chained_push(void *context, uint64_t size) {
  if (!context || size == 0) {
    return NULL;
  }

  ChainedContext *ctx = (ChainedContext *)context;

  // Align size to 8-byte boundary
  size = (size + 7) & ~7;

  ChainedPage *page = chained_find_available_page(ctx, size);
  if (!page) {
    // Need to add more pages
    SamrenaError err = chained_add_pages(ctx, size);
    if (err != SAMRENA_SUCCESS) {
      return NULL;
    }
    page = chained_find_available_page(ctx, size);
    if (!page) {
      return NULL;
    }
  }

  void *result = page->data + page->used;
  page->used += size;
  ctx->total_allocated += size;
  ctx->current_page = page;

  return result;
}

static void *chained_push_zero(void *context, uint64_t size) {
  void *result = chained_push(context, size);
  if (result) {
    memset(result, 0, size);
  }
  return result;
}

static uint64_t chained_allocated(void *context) {
  if (!context)
    return 0;

  ChainedContext *ctx = (ChainedContext *)context;
  return ctx->total_allocated;
}

static uint64_t chained_capacity(void *context) {
  if (!context)
    return 0;

  ChainedContext *ctx = (ChainedContext *)context;
  return ctx->total_capacity;
}

static SamrenaError chained_reserve(void *context, uint64_t min_capacity) {
  if (!context) {
    return SAMRENA_ERROR_NULL_POINTER;
  }

  ChainedContext *ctx = (ChainedContext *)context;
  if (ctx->total_capacity >= min_capacity) {
    return SAMRENA_SUCCESS;
  }

  uint64_t needed = min_capacity - ctx->total_capacity;
  return chained_add_pages(ctx, needed);
}

static void chained_reset(void *context) {
  if (!context)
    return;

  ChainedContext *ctx = (ChainedContext *)context;
  ChainedPage *page = ctx->first_page;

  while (page) {
    page->used = 0;
    page = page->next;
  }

  ctx->total_allocated = 0;
  ctx->current_page = ctx->first_page;
}

static void chained_dump_stats(void *context, FILE *out) {
  if (!context || !out)
    return;

  ChainedContext *ctx = (ChainedContext *)context;

  fprintf(out, "=== Chained Adapter Statistics ===\n");
  fprintf(out, "Page size: %lu bytes\n", ctx->page_size);
  fprintf(out, "Growth pages: %lu\n", ctx->growth_pages);
  fprintf(out, "Total allocated: %lu bytes\n", ctx->total_allocated);
  fprintf(out, "Total capacity: %lu bytes\n", ctx->total_capacity);
  fprintf(out, "Utilization: %.2f%%\n",
          ctx->total_capacity > 0 ? (double)ctx->total_allocated / ctx->total_capacity * 100.0
                                  : 0.0);

  // Count pages
  uint64_t page_count = 0;
  ChainedPage *page = ctx->first_page;
  while (page) {
    page_count++;
    page = page->next;
  }
  fprintf(out, "Page count: %lu\n", page_count);

  if (ctx->enable_debug) {
    fprintf(out, "\n=== Page Details ===\n");
    page = ctx->first_page;
    uint64_t i = 0;
    while (page) {
      fprintf(out, "Page %lu: %lu/%lu bytes used (%.2f%%)\n", i, page->used, page->size,
              page->size > 0 ? (double)page->used / page->size * 100.0 : 0.0);
      page = page->next;
      i++;
    }
  }
}

// Chained adapter capabilities
static const SamrenaCapabilities chained_capabilities = {.flags = SAMRENA_CAP_ZERO_COPY_GROWTH |
                                                                  SAMRENA_CAP_RESET |
                                                                  SAMRENA_CAP_RESERVE,
                                                         .max_allocation_size = UINT64_MAX,
                                                         .alignment_guarantee = sizeof(void *)};

static const SamrenaCapabilities *chained_get_capabilities(void *context) {
  (void)context; // Unused for static capabilities
  return &chained_capabilities;
}

const SamrenaOps chained_adapter_ops = {.name = "chained",
                                        .create = chained_create,
                                        .destroy = chained_destroy,
                                        .push = chained_push,
                                        .push_zero = chained_push_zero,
                                        .allocated = chained_allocated,
                                        .capacity = chained_capacity,
                                        .reserve = chained_reserve,
                                        .reset = chained_reset,
                                        .get_capabilities = chained_get_capabilities,
                                        .save_point = NULL,    // Not supported
                                        .restore_point = NULL, // Not supported
                                        .prefetch = NULL,      // Not needed
                                        .dump_stats = chained_dump_stats};