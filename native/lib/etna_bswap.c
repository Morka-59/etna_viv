/*
 * Copyright (c) 2012-2013 Etnaviv Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#include "etna_bswap.h"

#include "viv.h"
#include "etna.h"

#include <stdio.h>

static int etna_bswap_init_buffer(etna_bswap_buffer *buf)
{
    pthread_mutex_init(&buf->available_mutex, NULL);
    pthread_cond_init(&buf->available_cond, NULL);

    buf->is_available = 1;
    if(viv_user_signal_create(0, &buf->sig_id_ready) != 0)
    {
#ifdef DEBUG
        fprintf(stderr, "Cannot create user signal for framebuffer sync\n");
#endif
        return ETNA_INTERNAL_ERROR;
    }
    return ETNA_OK;
}

static void etna_bswap_destroy_buffer(etna_bswap_buffer *buf)
{
    (void)pthread_mutex_destroy(&buf->available_mutex);
    (void)pthread_cond_destroy(&buf->available_cond);
    (void)viv_user_signal_destroy(buf->sig_id_ready);
}

static void etna_bswap_thread(etna_bswap_buffers *bufs)
{
    int cur = 0;
    while(!bufs->terminate)
    {
        /* wait for "buffer ready" signal for buffer X (and clear it) */
        if(viv_user_signal_wait(bufs->buf[cur].sig_id_ready, SIG_WAIT_INDEFINITE) != 0)
        {
#ifdef DEBUG
            fprintf(stderr, "Error waiting for framebuffer sync signal\n");
#endif
            return; // ?
        }
        /* switch buffers */
        bufs->set_buffer(bufs->userptr, cur);
        bufs->frontbuffer = cur;
        /* X = (X+1)%buffers */
        cur = (cur+1) % ETNA_BSWAP_NUM_BUFFERS;
        /* set "buffer available" for buffer X, signal condition */
        pthread_mutex_lock(&bufs->buf[cur].available_mutex);
        bufs->buf[cur].is_available = 1;
        pthread_cond_signal(&bufs->buf[cur].available_cond);
        pthread_mutex_unlock(&bufs->buf[cur].available_mutex);
    }
}

int etna_bswap_create(etna_bswap_buffers **bufs_out, int (*set_buffer)(void *, int), void *userptr)
{
    if(bufs_out == NULL)
        return ETNA_INVALID_ADDR;
    etna_bswap_buffers *bufs = ETNA_NEW(etna_bswap_buffers);
    if(bufs == NULL)
        return ETNA_INTERNAL_ERROR;
    bufs->set_buffer = set_buffer;
    bufs->userptr = userptr;
    bufs->backbuffer = bufs->frontbuffer = 0;
    bufs->terminate = false;
    for(int idx=0; idx<ETNA_BSWAP_NUM_BUFFERS; ++idx)
        etna_bswap_init_buffer(&bufs->buf[idx]);
    pthread_create(&bufs->thread, NULL, (void * (*)(void *))&etna_bswap_thread, bufs);

    *bufs_out = bufs;
    return ETNA_OK;
}

int etna_bswap_free(etna_bswap_buffers *bufs)
{
    bufs->terminate = true;
    /* signal ready signals, to prevent thread from waiting forever for buffer to become ready */
    for(int idx=0; idx<ETNA_BSWAP_NUM_BUFFERS; ++idx)
        (void)viv_user_signal_signal(bufs->buf[idx].sig_id_ready, 1); 
    (void)pthread_join(bufs->thread, NULL);
    for(int idx=0; idx<ETNA_BSWAP_NUM_BUFFERS; ++idx)
        etna_bswap_destroy_buffer(&bufs->buf[idx]);
    ETNA_FREE(bufs);
    return ETNA_OK;
}

/* wait until current backbuffer is available to render to */
int etna_bswap_wait_available(etna_bswap_buffers *bufs)
{
    etna_bswap_buffer *buf = &bufs->buf[bufs->backbuffer];
    /* Wait until buffer buf is available */
    pthread_mutex_lock(&buf->available_mutex);
    while(!buf->is_available)
    {
        pthread_cond_wait(&buf->available_cond, &buf->available_mutex);
    }
    pthread_mutex_unlock(&buf->available_mutex);
    return ETNA_OK;
}

/* queue buffer swap when GPU ready with rendering to buf */
int etna_bswap_queue_swap(etna_bswap_buffers *bufs)
{
    if(viv_event_queue_signal(bufs->buf[bufs->backbuffer].sig_id_ready, gcvKERNEL_PIXEL) != 0)
    {
#ifdef DEBUG
        fprintf(stderr, "Unable to queue framebuffer sync signal\n");
#endif
        return ETNA_INTERNAL_ERROR;
    }
    bufs->backbuffer = (bufs->backbuffer + 1) % ETNA_BSWAP_NUM_BUFFERS;
    return ETNA_OK;
}


