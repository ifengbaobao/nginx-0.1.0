
/*
 * Copyright (C) Igor Sysoev
 内存池的管理。
 */


#ifndef _NGX_PALLOC_H_INCLUDED_
#define _NGX_PALLOC_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


/*
 * NGX_MAX_ALLOC_FROM_POOL should be (ngx_page_size - 1), i.e. 4095 on x86.
 * On FreeBSD 5.x it allows to use the zero copy sending.
 * On Windows NT it decreases a number of locked pages in a kernel.
 */
#define NGX_MAX_ALLOC_FROM_POOL  (ngx_pagesize - 1)

#define NGX_DEFAULT_POOL_SIZE   (16 * 1024)

#define ngx_test_null(p, alloc, rc)  if ((p = alloc) == NULL) { return rc; }


typedef struct ngx_pool_large_s  ngx_pool_large_t;

//大块内存的内存池。
struct ngx_pool_large_s {
    ngx_pool_large_t  *next;
    void              *alloc;
};

//内存池。需要的空间小，就在自己的空间中分配，需要的空间大就用打开内存的结构体 large 。
struct ngx_pool_s {
    char              *last;//可用的内存的起始位置。
    char              *end;//当前内存池的最后一个字节的下一个字节的内存地址。大于或等于这个值，说明内存就越界了。
    ngx_pool_t        *next;//当前内存池同样大小的下一个内存池的地址。
    ngx_pool_large_t  *large;//大块内存空间的结构体。
    ngx_log_t         *log;
};

//malloc 申请空间，并输出日志。
void *ngx_alloc(size_t size, ngx_log_t *log);
//申请空间，并对申请的空间初始化为0.
void *ngx_calloc(size_t size, ngx_log_t *log);

//创建一个内存池。内存池刚开始的地方就是ngx_pool_t结构体。
ngx_pool_t *ngx_create_pool(size_t size, ngx_log_t *log);
//销毁内存池。
void ngx_destroy_pool(ngx_pool_t *pool);
//内存池中申请一块空间返回回去。
void *ngx_palloc(ngx_pool_t *pool, size_t size);
//内存池中申请一块内存，并且把申请的内存初始化为0.
void *ngx_pcalloc(ngx_pool_t *pool, size_t size);
//如果p的地址是大块内存申请的，释放掉。否则，忽略。
ngx_int_t ngx_pfree(ngx_pool_t *pool, void *p);


#endif /* _NGX_PALLOC_H_INCLUDED_ */
