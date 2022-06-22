
/*
 * Copyright (C) Igor Sysoev
 对数组的创建销毁和存放数据。
 */


#ifndef _NGX_ARRAY_H_INCLUDED_
#define _NGX_ARRAY_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>

//存放数组的结构体。
struct ngx_array_s {
    void        *elts;//存放数据的指针。
    ngx_uint_t   nelts;//已经使用的数组的个数。
    size_t       size;//数组的每个值多大。
    ngx_uint_t   nalloc;//数组的总大小，总的能放多少个。
    ngx_pool_t  *pool;//内存池。
};

//创建一个数组。
ngx_array_t *ngx_create_array(ngx_pool_t *p, ngx_uint_t n, size_t size);
//销毁数组，如果申请的空间，正好在内存池最后的位置，直接从内存池中回收。
void ngx_destroy_array(ngx_array_t *a);
//返回未使用的空间的指针地址。数组的空间不够会自动扩充。
void *ngx_push_array(ngx_array_t *a);

//数组初始化。
ngx_inline static ngx_int_t ngx_array_init(ngx_array_t *array, ngx_pool_t *pool,
                                           ngx_uint_t n, size_t size)
{
    if (!(array->elts = ngx_palloc(pool, n * size))) {
        return NGX_ERROR;
    }

    array->nelts = 0;
    array->size = size;
    array->nalloc = n;
    array->pool = pool;

    return NGX_OK;
}


//数组初始化。
#define ngx_init_array(a, p, n, s, rc)                                       \
    ngx_test_null(a.elts, ngx_palloc(p, n * s), rc);                         \
    a.nelts = 0; a.size = s; a.nalloc = n; a.pool = p;

#define ngx_array_create  ngx_create_array
#define ngx_array_push    ngx_push_array


#endif /* _NGX_ARRAY_H_INCLUDED_ */
