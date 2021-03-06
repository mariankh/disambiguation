#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <unistd.h>
#include <time.h>

#include "../graph/rgraph.h"

using namespace mico::graph;

void usage(char *cmd) {
  printf("Usage: %s -i restorefile -e reserve_edges -v reserve_vertices\n", cmd);
  exit(1);
}


void filter(rgraph *g, igraph_vector_t *edges, int sid, int pid, int oid) {
  int i, eid, from, to;
  for(i = 0; i<igraph_vector_size(edges); i++) {
    eid = igraph_vector_e(edges,i);

    igraph_edge(g->graph, eid, &from, &to);

    printf("%d %d %d\n",from,g->labels[eid],to);

    if((sid < 0 || sid == from) && (pid < 0 || pid == g->labels[eid]) && (oid < 0 || oid == to)) {
      const char *s = g->vertices[from];
      const char *p = g->vertices[g->labels[eid]];
      const char *o = g->vertices[to];

      printf("%s --- %s --> %s\n", s, p, o);
    }
  }
}


void query(rgraph *g, const char *s, const char *p, const char* o) {
  int sid = -1, pid = -1, oid = -1;



  if(strcmp("*",s) != 0) {
    sid = g->get_vertice_id(s);
  }
  if(strcmp("*",p) != 0) {
    pid = g->get_vertice_id(p);
  }
  if(strcmp("*",o) != 0) {
    oid = g->get_vertice_id(o);
  }

  printf("querying %s=%d, %s=%d, %s=%d\n",s,sid,p,pid,o,oid);

  igraph_vector_t edges;
  igraph_vector_init(&edges,16);

  // first filter by subject OR object
  if(sid >= 0) {
    igraph_incident(g->graph,&edges,sid,IGRAPH_OUT);
  } else if(oid >= 0) {
    igraph_incident(g->graph,&edges,oid,IGRAPH_IN);
  } else {
    printf("filtering by predicate not supported!\n");
  }
  filter(g,&edges,sid,pid,oid);

  igraph_vector_destroy(&edges);

}


int main(int argc, char** argv) {
  int opt;
  char *ifile = NULL;
  long int reserve_edges = 1<<16;
  long int reserve_vertices = 1<<12;

  // read options from command line
  while( (opt = getopt(argc,argv,"i:e:v:")) != -1) {
    switch(opt) {
    case 'i':
      ifile = optarg;
      break;
    case 'e':
      sscanf(optarg,"%ld",&reserve_edges);;
      break;
    case 'v':
      sscanf(optarg,"%ld",&reserve_vertices);;
      break;
    default:
      usage(argv[0]);
    }
  }

  if(ifile) {
    rgraph_complete graph;


    // first restore existing dump in case -i is given
    graph.restore_file(ifile);

    // read from stdin pairs of vertices and compute relatedness
    char *line    = NULL;
    size_t len    = 0;
    char *s, *p, *o;
    s = new char[256];
    p = new char[256];
    o = new char[256];
    while((getline(&line,&len,stdin) != -1)) {
      sscanf(line,"%s %s %s\n", s, p, o);

      query(&graph,s,p,o);
    }

    free(line);

    return 0;
  } else {
    usage(argv[0]);
    return 1;
  }

}


