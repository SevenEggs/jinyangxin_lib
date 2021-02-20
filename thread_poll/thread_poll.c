#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include <pthread.h>

typedef struct 
{
	size_t size;
	void* (*ctor)(void *_self, va_list *params);
	void* (*dtor)(void *_self);
}AbstractClass;

#define LL_ADD(item, list) do		\
{									\
	item->pre = NULL;				\
	item->next = list;				\
	list->pre = item;				\
	list = item;					\
}while(0)

#define LL_REMOVE(item, list) do							\
{															\
	if(item->pre != NULL) item->pre->next = item->next;		\
	if(item->next != NULL) item->next->pre = item->pre;		\
	if(item == list) list = item->next;						\
	item->pre = item->next = NULL;							\
}while(0)

typedef struct NWROKER
{
	pthread_t thread;
	struct NWORKQUEUE *workqueue;
	int terminate;
	nWork *pre;
	nWork *next;
}nWorker;

typedef struct NJOB
{
	void (*nJob_function)(struct NJOB *job);
	void *user_data;
	nJob *pre;
	nJob *next;
}nJob;

typedef struct NWORKQUEUE
{
	nWorker *workers;				//线程
	nJob *waiting_jobs;
	pthread_mutex_t jobs_mtx;
	pthread_cond_t jobs_cond;
}nWorkQueue;

typedef nWorkQueue nThreadPool;

static int ntyWorkerThread(void *ptr)
{
	nWorker *worker = (nWorker*)ptr;
	while(1)
	{
		pthread_mutex_lock(&worker->workqueue->jobs_mtx);

		while(worker->workqueue->waiting_jobs == NULL)
		{
			if(worker->terminate)
			{
				break;
			}
			pthread_cond_wait(&worker->workqueue->jobs_cond, &worker->workqueue->jobs_cond);
		}

		if(worker->terminate)
		{
			pthread_mutex_unlock(&worker->workqueue->jobs_mtx);
			break;
		}

		//删除job节点
		nJob *job = worker->workqueue->waiting_jobs;
		if(job != NULL)
		{
			LL_REMOVE(job, worker->workqueue->waiting_jobs);
		}
		
		pthread_mutex_unlock(&worker->workqueue->jobs_mtx);

		if(job == NULL)
		{
			continue;
		}

		job->job_function(job);
		
	}
	
	return 0;
}

int ntyThreadPoolCreate(nThreadPool *workqueue, int numWorkers)
{
	if(numWorkers < 1)
	{
		numWorkers = 1;
	}

	memset(workqueue, 0, sizeof(nThreadPool));
	
	pthread_cond_t blank_cond = PTHREAD_COND_INITIALIZER;
	memcpy(&workqueue->jobs_cond, blank_cond, sizeof(workqueue->jobs_cond));

	pthread_mutex_t blank_mtx = PTHREAD_MUTEX_INITIALIZER;
	memcpy(%workqueue->jobs_mtx, blank_mtx, sizeof(workqueue->jobs_mtx));

	int i = 0;
	for(i = 0; i < numWorkers; i++)
	{
		nWorker *worker = (nWorker*)malloc(sizeof(nWorker));
		if(worker == NULL)
		{
			perror("func %s, malloc failed \n", __FUNCTION__);
			return 1;
		}

		memset(worker, 0, sizeof(nWorker));
		worker->workqueue = workqueue;
		int ret = pthread_create(worker->thread, NULL, ntyWorkerThread, (void*)worker);
		if(ret)
		{
			perror("func %s, create thread failed !\n", __FUNCTION);
			free(worker);
			return 1;
		}

		LL_ADD(worker, worker->workqueue->workers);
	}
	
	return 0;
}

//线程池退出
void ntyThreadPollThreadShutDown(nThreadPool *workqueue)
{
	nWorker *worker = NULL;
	//线程池退出信号置位
	for(worker = workqueue->workers; worker != NULL; worker = worker->next)
	{
		worker->terminate = 1;
	}

	//线程池唤醒等待
	pthread_mutex_lock(&workqueue->jobs_mtx);

	//线程池任务、线程指正置空
	workqueue->workers = NULL;
	workqueue->waiting_jobs = NULL;

	pthread_cond_broadcast(&workqueue->jobs_cond, &workqueue->jobs_mtx);
	
	pthread_mutex_unlock(&workqueue->jobs_mtx);
}

void ntyThreadPoolQueueJobAdd(nThreadPool *workqueue, nJob *job)
{
	pthread_mutex_lock(&workqueue->jobs_mtx);
	
	LL_ADD(job, workqueue->waiting_jobs);
	
	pthread_cond_signal(&workqueue->jobs_cond);
	pthread_mutex_unlock(&workqueue->jobs_mtx);
}

#ifdef 0

#define KING_MAX_THREAD			80
#define KING_COUNTER_SIZE		1000

void king_counter(nJob *job) {

	int index = *(int*)job->user_data;

	printf("index : %d, selfid : %lu\n", index, pthread_self());
	
	free(job->user_data);
	free(job);
}

int main(int argc, int *argv[])
{
	nThreadPool pool;

	ntyThreadPoolCreate(&pool, KING_MAX_THREAD);
	
	int i = 0;
	for (i = 0;i < KING_COUNTER_SIZE;i ++) {
		nJob *job = (nJob*)malloc(sizeof(nJob));
		if (job == NULL) {
			perror("malloc");
			exit(1);
		}
		
		job->job_function = king_counter;
		job->user_data = malloc(sizeof(int));
		*(int*)job->user_data = i;

		ntyThreadPoolQueueJobAdd(&pool, job);
		
	}

	getchar();
	printf("\n");
	
	return 0;
}


#endif


void *New(const void *_class, ...)
{
	const AbstractClass *class = _class;
	void *p = calloc(1, class->size);
	memset(p, 0, class->size);

	assert(p);
	*(const AbstractClass**)p = class;

	if(class->ctor)
	{
		va_list params;
		va_start(params, _class);
		p = class->ctor(p,params);
		va_end(params);
	}

	return p;
}

void Delete(const void *_class)
{
	const AbstractClass **class = _class;
	if(_class && (*class) && (*class)->dtor)
	{
		_class = (*class)->dtor(_class);
	}

	free(_class);
}

typedef struct _ThreadPool
{
	const void *_;
	nThreadPool *wq;
}ThreadPool;

//封装线程池操作结构体
typedef struct _ThreadPoolOpera
{
	size_t size;
	void* (*ctor)(void *_self, va_list *params);
	void* (*dtor)(void *_self);
	void (*addJob)(void *_self, void *task);
}ThreadPoolOpera;

void* ntyThreadPoolCtor(void* _self, va_list *params)
{
	ThreadPool *pool = (ThreadPool*)_self;

	pool->wq = (nThreadPool*)malloc(sizeof(nThreadPool));

	pool->wq->workers = (nWorker*)malloc(sizeof(nWorker));
	memset(pool->wq->workers, 0, sizeof(nWorker));

	pool->wq->waiting_jobs = (nJob*)malloc(sizeof(nJob));
	memset(pool->wq->waiting_jobs, 0, sizeof(nJob));

	int arg = va_arg(params, int);

	printf("func: %s, arg = [%d]\n", arg);
	ntyThreadPoolCreate(pool->wq, arg);

	return pool;
}


void* ntyThreadPoolDtor(void* _self)
{
	ThreadPool *pool = (ThreadPool*)_self;
	ntyThreadPoolShutdown(pool->wq);
	free(pool->wq);

	return pool;
}

void ntyThreadPoolAddJob(void *_self, void *task)
{
	ThreadPool *pool = (ThreadPool*)_self;

	nJob *job = task;
	
	ntyThreadPoolQueueJobAdd(pool, job);
}

const ThreadPoolOpera ntyThreadPoolOpera = {
	sizeof(ThreadPool),
	ntyThreadPoolCtor;
	ntyThreadPoolDtor;
	ntyThreadPoolAddJob;
};

const void *pNtyThreadPoolOpera = &ntyThreadPoolOpera;
static void *pThreadPool = NULL;

void *ntyThreadPoolInstance(int nWorker)
{
	if(pThreadPool == NULL)
	{
		pThreadPool = New(pNtyThreadPoolOpera, nWorker);
	}

	return pThreadPool;
}

void* ntyGetInstance(void)
{
	if(pThreadPool != NULL)
	{
		return pThreadPool;
	}
}

void ntyThreadPoolRelease(void)
{
	Delete(pThreadPool);

	pThreadPool = NULL;
}

int ntyThreadPoolPush(void *_self, void *task)
{
	ThreadPoolOpera **pThreadPoolOpera = _self;

	if(_self && (pThreadPoolOpera) && (*pThreadPoolOpera)->addJob)
	{
		(*pThreadPoolOpera)->addJob(_self, task);
		return 0;
	}
	return 1;
}

#if 1

#define KING_MAX_THREAD			80
#define KING_COUNTER_SIZE		1000

void king_counter(nJob *job) {

	int index = *(int*)job->user_data;

	printf("index : %d, selfid : %lu\n", index, pthread_self());
	
	free(job->user_data);
	free(job);
}


int main(int argc, char *argv[]) {

	
    int numWorkers = 20;
    void *pool = ntyThreadPoolInstance(numWorkers);

    int i = 0;
    for (i = 0;i < KING_COUNTER_SIZE;i ++) {
		nJob *job = (nJob*)malloc(sizeof(nJob));
		if (job == NULL) {
			perror("malloc");
			exit(1);
		}
		
		job->job_function = king_counter;
		job->user_data = malloc(sizeof(int));
		*(int*)job->user_data = i;

		ntyThreadPoolPush(pool, job);
		
	}

	getchar();
	printf("\n");

	
}

