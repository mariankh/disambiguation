#include <iostream>
#include <vector>

#include <float.h>
#include <limits.h>

#include "../threading/thread.h"
#include "wsd_relatedness_worker.h"

void mico::disambiguation::wsd::relatedness_worker::run() {

  // loop until queue of thread pool is empty
  while(1) {
    pthread_mutex_lock(&pool->tsk_mutex);
    if(!pool->tasks.empty()) {
      rtask t = pool->tasks.front();
      pool->tasks.pop();
      pthread_mutex_unlock(&pool->tsk_mutex);

      t.relatedness = pool->states[id]->relatedness(t.from, t.to);

      if(t.relatedness < DBL_MAX) {
	pthread_mutex_lock(&pool->wsd_mutex);
	igraph_add_edge(&pool->wsd_graph,t.fromId,t.toId);
	igraph_vector_push_back (&pool->wsd_weights, t.relatedness);
	pthread_mutex_unlock(&pool->wsd_mutex);
      }

      
    } else {
      pthread_mutex_unlock(&pool->tsk_mutex);
      break;
    } 
    
  }


}


mico::disambiguation::wsd::relatedness_threadpool_base::relatedness_threadpool_base(rgraph* graph, igraph_t& wsd_graph, igraph_vector_t& wsd_weights, int max_dist) 
  : graph(graph), wsd_graph(wsd_graph), wsd_weights(wsd_weights), max_dist(max_dist) {
  pthread_mutex_init(&wsd_mutex,NULL);
  pthread_mutex_init(&tsk_mutex,NULL);
    
  initialised = false; // indicate threads still need to be initialised

};


mico::disambiguation::wsd::relatedness_threadpool_base::~relatedness_threadpool_base()  {
  // destroy threads and states
  if(initialised) {
    for(int i=0; i<NUM_THREADS; i++) {
      delete pool[i];
      delete states[i];
    }
  }

  pthread_mutex_destroy(&wsd_mutex);
  pthread_mutex_destroy(&tsk_mutex);

};


void mico::disambiguation::wsd::relatedness_threadpool_base::start()  {
  if(!initialised) {
    // create threads and states
    for(int i=0; i<NUM_THREADS; i++) {
      states[i] = create_algorithm();
      pool[i]   = new relatedness_worker(i,this);
    }
    initialised = true;
  }

  // destroy threads and states
  for(int i=0; i<NUM_THREADS; i++) {
    pool[i]->start();
  }

};


void mico::disambiguation::wsd::relatedness_threadpool_base::reset()  {
  // destroy threads and states
  if(initialised) {
    for(int i=0; i<NUM_THREADS; i++) {
      pool[i]->reset();
    }
  }

};

void mico::disambiguation::wsd::relatedness_threadpool_base::join()  {
  // destroy threads and states
  if(initialised) {
    for(int i=0; i<NUM_THREADS; i++) {
      pool[i]->join();
    }
  }

};