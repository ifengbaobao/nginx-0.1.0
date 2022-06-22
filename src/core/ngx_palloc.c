
/*
 * Copyright (C) Igor Sysoev
 内存池的管理。
 */


#include <ngx_config.h>
#include <ngx_core.h>

//创建一个内存池。内存池刚开始的地方就是ngx_pool_t结构体。
ngx_pool_t *ngx_create_pool(size_t size, ngx_log_t *log)
{
    ngx_pool_t  *p;

    if (!(p = ngx_alloc(size, log))) {
       return NULL;
    }

    p->last = (char *) p + sizeof(ngx_pool_t);
    p->end = (char *) p + size;
    p->next = NULL;
    p->large = NULL;
    p->log = log;

    return p;
}

//销毁内存池。
void ngx_destroy_pool(ngx_pool_t *pool)
{
    ngx_pool_t        *p, *n;
    ngx_pool_large_t  *l;
//所有的大内存快都释放掉。
    for (l = pool->large; l; l = l->next) {

        ngx_log_debug1(NGX_LOG_DEBUG_ALLOC, pool->log, 0,
                       "free: " PTR_FMT, l->alloc);

        if (l->alloc) {
            free(l->alloc);
        }
    }

#if (NGX_DEBUG)

    /*
     * we could allocate the pool->log from this pool
     * so we can not use this log while the free()ing the pool
     */

    for (p = pool, n = pool->next; /* void */; p = n, n = n->next) {
        ngx_log_debug2(NGX_LOG_DEBUG_ALLOC, pool->log, 0,
                       "free: " PTR_FMT ", unused: " SIZE_T_FMT,
                       p, p->end - p->last);

        if (n == NULL) {
            break;
        }
    }

#endif

//把pool和next都释放掉。
    for (p = pool, n = pool->next; /* void */; p = n, n = n->next) {
        free(p);

        if (n == NULL) {
            break;
        }
    }
}

//内存池中申请一块空间返回回去。
void *ngx_palloc(ngx_pool_t *pool, size_t size)
{
    char              *m;
    ngx_pool_t        *p, *n;
    ngx_pool_large_t  *large, *last;

//size 小于等于 4095 并且 size 小于等于 当前内存池空的状态下的值。 
    if (size <= (size_t) NGX_MAX_ALLOC_FROM_POOL  //linux 下 NGX_MAX_ALLOC_FROM_POOL 是 4095 .
        && size <= (size_t) (pool->end - (char *) pool) - sizeof(ngx_pool_t))
    {
        for (p = pool, n = pool->next; /* void */; p = n, n = n->next) {//死循环，对p和n复制。
            m = ngx_align(p->last);//可用的内存，并且内存对齐。

            if ((size_t) (p->end - m) >= size) {//当前内存中的空间够用，返回地址。并把当前的pool 的last向后移动。
                p->last = m + size ;

                return m;
            }

            if (n == NULL) {
                break;
            }
        }

        /* allocate a new pool block */

        if (!(n = ngx_create_pool((size_t) (p->end - (char *) p), p->log))) {
            return NULL;
        }

        p->next = n;
        m = n->last;
        n->last += size;

        return m;
    }

    /* allocate a large block */

    large = NULL;
    last = NULL;

    if (pool->large) {
        for (last = pool->large; /* void */ ; last = last->next) {
            if (last->alloc == NULL) {
                large = last;
                last = NULL;
                break;
            }

            if (last->next == NULL) {
                break;
            }
        }
    }

    if (large == NULL) {
        if (!(large = ngx_palloc(pool, sizeof(ngx_pool_large_t)))) {
            return NULL;
        }

        large->next = NULL;
    }

#if 0
    if (!(p = ngx_memalign(ngx_pagesize, size, pool->log))) {
        return NULL;
    }
#else
    if (!(p = ngx_alloc(size, pool->log))) {
        return NULL;
    }
#endif

    if (pool->large == NULL) {
        pool->large = large;

    } else if (last) {
        last->next = large;
    }

    large->alloc = p;

    return p;
}

//如果p的地址是大块内存申请的，释放掉。否则，忽略。
ngx_int_t ngx_pfree(ngx_pool_t *pool, void *p)
{
    ngx_pool_large_t  *l;

    for (l = pool->large; l; l = l->next) {
        if (p == l->alloc) {
            ngx_log_debug1(NGX_LOG_DEBUG_ALLOC, pool->log, 0,
                           "free: " PTR_FMT, l->alloc);
            free(l->alloc);
            l->alloc = NULL;

            return NGX_OK;
        }
    }

    return NGX_DECLINED;
}

//内存池中申请一块内存，并且把申请的内存初始化为0.
void *ngx_pcalloc(ngx_pool_t *pool, size_t size)
{
    void *p;

    p = ngx_palloc(pool, size);
    if (p) {
        ngx_memzero(p, size);
    }

    return p;
}

#if 0

static void *ngx_get_cached_block(size_t size)
{
    void                     *p;
    ngx_cached_block_slot_t  *slot;

    if (ngx_cycle->cache == NULL) {
        return NULL;
    }

    slot = &ngx_cycle->cache[(size + ngx_pagesize - 1) / ngx_pagesize];

    slot->tries++;

    if (slot->number) {
        p = slot->block;
        slot->block = slot->block->next;
        slot->number--;
        return p;
    }

    return NULL;
}

#endif
