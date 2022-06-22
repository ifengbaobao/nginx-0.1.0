
/*
 * Copyright (C) Igor Sysoev
  对malloc 和 calloc 这两个函数的封装。
 */


#ifndef _NGX_ALLOC_H_INCLUDED_
#define _NGX_ALLOC_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>

//malloc 申请空间，并输出日志。
void *ngx_alloc(size_t size, ngx_log_t *log);
//申请空间，并对申请的空间初始化为0.
void *ngx_calloc(size_t size, ngx_log_t *log);

#define ngx_free          free


/*
 * Linux has memalign() or posix_memalign()
 * Solaris has memalign()
 * FreeBSD has not memalign() or posix_memalign() but its malloc() alignes
 * allocations bigger than page size at the page boundary.
 */

#if (HAVE_POSIX_MEMALIGN || HAVE_MEMALIGN)

//内存对齐分配内存。
void *ngx_memalign(size_t alignment, size_t size, ngx_log_t *log);

#else

//内存对齐分配内存。
#define ngx_memalign(alignment, size, log)  ngx_alloc(size, log)

#endif

//内存的分页大小。
extern int ngx_pagesize;


#endif /* _NGX_ALLOC_H_INCLUDED_ */
