#ifndef STRUCTURE_H
#define STRUCTURE_H

#ifdef _METRO
typedef struct {
  vertex* sample;
  int nbsamples;
}sample;

typedef struct {
  int *cube;
  int nbcube;
}cellules;
#endif

#endif
