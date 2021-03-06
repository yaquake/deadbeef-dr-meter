#include <stdlib.h>
#include "thread_runner.h"
#include "thread_data.h"

static unsigned number_of_batches(thread_data_t* thread_data, unsigned max_threads)
{
    unsigned batches = thread_data->items / max_threads;
    if(thread_data->items % max_threads) batches += 1;
    return batches;
}

thread_runner_t make_thread_runner(thread_data_t* thread_data, unsigned max_threads)
{
    thread_runner_t runner =
    {.threads = max_threads, .batches = number_of_batches(thread_data, max_threads), .thread_data = thread_data};
    runner.pids = malloc(runner.threads * sizeof(pthread_t));
    return runner;
}

static unsigned first_item_id(thread_runner_t* self, unsigned batch_id)
{
    return batch_id * self->threads;
}

static unsigned last_item_id(thread_runner_t* self, unsigned batch_id)
{
    unsigned next = batch_id + 1;
    return next == self->batches ? self->thread_data->items : next * self->threads;
}

static void join_threads(thread_runner_t* self, int spawned_threads)
{
    for(int j = 0; j < spawned_threads; ++j)
        pthread_join(self->pids[j], NULL);
}

static void run_batch(thread_runner_t* self, thread_worker_t worker, unsigned batch_id)
{
    unsigned first_item = first_item_id(self, batch_id);
    unsigned last_item = last_item_id(self, batch_id);
    for(unsigned k = first_item; k < last_item; ++k)
        pthread_create(&self->pids[k % self->threads], NULL, worker, (void*)&self->thread_data->data[k]);
    join_threads(self, last_item - first_item);
}

void run_batches(thread_runner_t* self, thread_worker_t worker)
{
    for(unsigned batch_id = 0; batch_id < self->batches; ++batch_id)
        run_batch(self, worker, batch_id);
}

void free_thread_runner(thread_runner_t* self)
{
    free(self->pids);
}
