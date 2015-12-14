/*Exercise 1: Heuristic Search

Implement A* for the 8-puzzle.
Measure and compare the runtimes and the # of nodes expanded using the following 3 heuristics:
a) No heuristic (  h(s) = 0 for all states )
b) Misplaced tile count heuristic
c) Manhattan distance

Evaluate each of the above heuristics using at least 100 randomly generated sliding puzzles.
Note that not all initial states are solvable.
To generate a solvable, random instance, start with a solved puzzle, then apply a sequence of random steps, which will result in a jumbled state for which a path back to the goal state is guaranteed (in general, the larger the # of random steps,  the harder the resulting puzzle).

* Be careful about using efficient data structures for representing the OPEN/CLOSED sets in A*.
A reasonably efficient implementation should be able to solve *any* randomly geneated, solvable instance of the 8-puzzle within 1 second.
*/

/* Lin Gengxian Shunji 
 * 08-144505
 * 25th August 2015
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <time.h>
#include "dbg.h"


#define RANDOM_STEPS 100000
#define MAX_MOVES 50 /* max f_score discovered is 37 for manhattan distance on hardest puzzle (31 moves) */
#define PERMUTATIONS 196613 /* total permutations: 9! = 181440; has to be prime */
#define PRIME 24593
#define END_STATE 0x087654321 /* hexadecimal encoding of final state */
#define TEST_STATE1 0x123058746 /* 31 moves */
#define TEST_STATE2 0x103452768 /* 31 moves */
#define ITERATIONS 500 /* number of generated puzzles */

 /**********************************************
   *  END_STATE:
   *
   *  | 1 | 2 | 3 |
   *  | 4 | 5 | 6 |
   *  | 7 | 8 | 0 |
   *
   *  corresponding to the hexadecimal encoding:
   *  0x087654321
   **********************************************/

typedef struct puzzle { 
  unsigned long state; /* current state */
  unsigned long parent; /* parent state */
  int nmoves; /* number of moves made */
  int nchildren; /* number of children nodes */
  unsigned long child[4]; /* array cointaining children states */
  struct puzzle *next; /* pointer to next board (for prioirtyQ) */
} puzzle;

typedef struct priorityQ {
  int nelements; /* current number of queue elements */
  int min_index; /* index of current minimum f-score */
  puzzle *queue[MAX_MOVES]; /* priority Q indexed by f-score */
} priorityQ;

typedef struct hash_node {
  /* hash table stores information of state, parent state, 
     whether state is discovered, processed */
  unsigned long state;
  unsigned long parent;
  bool processed; /* processed upon extracting from priority queue */
  int f_score; /* if f_score set, state is discovered */
} hash_node;



/****************************************
 *       OPERATIONS FOR PUZZLE          *
 ****************************************/


void enum_states();

puzzle *board_init(unsigned long state, unsigned long parent, int nmoves)
{ /* initialize board */
  int i;
  puzzle *boardp = malloc(sizeof(*boardp));
  check_mem(boardp);
  boardp->state = state;
  boardp->parent = parent;
  boardp->nmoves = nmoves;
  boardp->nchildren = 0;
  
  for (i = 0; i < 4; i++) {
    boardp->child[i] = 0;
  }
  
  boardp->next = NULL;
  enum_states(boardp); /* enumerate and insert child states */
  
  return boardp;
 error:
  log_info("error allocating memory for board with state: %lx", state);
  return NULL;
}
  

unsigned long swap_tiles(unsigned long state, int index1, int index2)
{ /* swap tiles in index1 and index2, generating new state */
  unsigned high_shift; /* bits to shift for higher index */
  unsigned low_shift; /* bits to shift for lower index */
  unsigned shift_diff; /* difference in bits of 2 positions */
  unsigned long shift_mask = 0xF;
  if (index1 == index2) {
    log_info("same index");
    return state;
  }
  if (index1 < 0 || index1 > 8) {

    log_info("invalid swap");
    return state;
  }
  if (index2 < 0 || index2 > 8) {
    log_info("invalid swap");
    return state;
  }
  if (index1 > index2) {
    high_shift = index1 * 4;
    low_shift = index2 * 4;
  } else {
    low_shift = index1 * 4;
    high_shift = index2 * 4;
  }
  shift_diff = high_shift - low_shift;
  
  /* extract higher index */
  unsigned long extract_high = state & (shift_mask << high_shift);
  
  /* extract lower index */
  unsigned long extract_low = state & (shift_mask << low_shift);

   /* swap 4-bits groups */
  return ((state & (state ^ (extract_high + extract_low))) | (extract_high >> shift_diff) | (extract_low << shift_diff));
}


void insert_child(puzzle *boardp, unsigned long new_state)
{ // insert new state into child array of current board
   boardp->child[boardp->nchildren] = new_state;
   boardp->nchildren++; /* increment number of children */
}



void enum_states(puzzle *boardp)
{ /* enumerate states from current state, and add them into array of children states */
  
  /**********************************************
   * Indexed as follows:
   *
   *  | 0 | 1 | 2 |
   *  | 3 | 4 | 5 |
   *  | 6 | 7 | 8 |
   *
   *  corresponding to the hexadecimal encoding:
   *  0x876543210
   **********************************************/
  
  unsigned long state = boardp->state;
  unsigned blank_index; /* index with no tile */
  unsigned i = 0;

  while (true) {
    if (((state >> (4*i)) & 0xF) == 0) { /* find out whether index of blank is */
      blank_index = i; /* assign blank index */
      break;
    }
    i++;
  }

  switch(blank_index) {
  case 0 :
    insert_child(boardp, swap_tiles(state, 0, 1));
    insert_child(boardp, swap_tiles(state, 0, 3));
    break;
  case 1 :
    insert_child(boardp, swap_tiles(state, 1, 0));
    insert_child(boardp, swap_tiles(state, 1, 2));
    insert_child(boardp, swap_tiles(state, 1, 4));
    break;
  case 2:
    insert_child(boardp, swap_tiles(state, 2, 1));
    insert_child(boardp, swap_tiles(state, 2, 5));
    break;
  case 3:
    insert_child(boardp, swap_tiles(state, 3, 0));
    insert_child(boardp, swap_tiles(state, 3, 4));
    insert_child(boardp, swap_tiles(state, 3, 6));
    break;
  case 4:
    insert_child(boardp, swap_tiles(state, 4, 1));
    insert_child(boardp, swap_tiles(state, 4, 3));
    insert_child(boardp, swap_tiles(state, 4, 5));
    insert_child(boardp, swap_tiles(state, 4, 7));
    break;
  case 5:
    insert_child(boardp, swap_tiles(state, 5, 2));
    insert_child(boardp, swap_tiles(state, 5, 4));
    insert_child(boardp, swap_tiles(state, 5, 8));
    break;
  case 6:
    insert_child(boardp, swap_tiles(state, 6, 3));
    insert_child(boardp, swap_tiles(state, 6, 7));
    break;
  case 7:
    insert_child(boardp, swap_tiles(state, 7, 4));
    insert_child(boardp, swap_tiles(state, 7, 6));
    insert_child(boardp, swap_tiles(state, 7, 8));
    break;
  case 8:
    insert_child(boardp, swap_tiles(state, 8, 5));
    insert_child(boardp, swap_tiles(state, 8, 7));
    break;
  default: log_info("invalid blank index %d", blank_index);
  }
    
}

/**************************************************
 *              PRINTING/TRACING BOARD            *
 **************************************************/

void print_state(unsigned long state)
{ /* function to print state */
  int i;
  unsigned long temp;
  unsigned long mask = 0xF;
  printf("\n|");
  for (i = 0; i < 9; i++) {
    if ((i == 3) || (i == 6)) {
      printf("\n|");
    }
    temp = state >> (4 * i);
    temp = (temp & mask); /* extract value of least-significant 4 bits */
    printf(" %lu |", temp);
    
  }
  printf("\n");
}

int hash_f(unsigned long state, int i);

void trace(hash_node closed[], unsigned long state)
{  /* trace a final state to its initial state using information from the closed set 
      prints out states in order of moves made */
  
  unsigned long trace_array[MAX_MOVES];
  unsigned long current_state = state;
  int j = 0; /* trace array index */
  int move_count = 0;
  while (current_state != 0) {
    int i = 0; /* counter hash function */
    int index;

    trace_array[j] = current_state; /* insert into array in reverse order */
    j++;

    while (true) {
      index = hash_f(current_state, i); /* hash function */
      if (closed[index].state == current_state) {
	current_state = closed[index].parent;
	break;
      }
      i++;
    }
  }
  
  while (j > 0) { /* trace backwards */
    j--; 
    log_info("move %d:", move_count);
    print_state(trace_array[j]);
    move_count++;
  }
}


/**************************************************
 *          GENERATING RANDOM BOARD               *
 **************************************************/

puzzle *random_child(puzzle *boardp)
{ /* generate random child and returns pointer to child board */
  int rand_index = rand() % boardp->nchildren;
  unsigned long child_state = boardp->child[rand_index];
  puzzle *child_boardp;
  child_boardp = board_init(child_state, boardp->state, boardp->nmoves + 1);
  
  return child_boardp;
}


unsigned long random_state()
{ /* generate random board, and returns state of that board */
  int i;
  unsigned long state;
  puzzle *random_boardp = board_init(END_STATE, 0, 0); 
  puzzle *next_boardp; /* temporary pointer */
  for (i = 0; i < RANDOM_STEPS; i++) {
    next_boardp = random_child(random_boardp);
    free(random_boardp);
    random_boardp = next_boardp;
    } 
  state = random_boardp->state;
  free(random_boardp);
  return state;
}


/**************************************************
 * DATA STRUCTURE & OPERATIONS FOR PRIORITY QUEUE *
 **************************************************/
  
priorityQ *priorityQ_init(void)
{ /* initialize priority queue,
     priority queue is an array of linked-lists, indexed by f_score */
  int i;
  priorityQ *priorityQp = malloc(sizeof (*priorityQp));
  check_mem(priorityQp);
  priorityQp->nelements = 0;
  priorityQp->min_index = -1; /* -1 indicates that there are no elements on the queue */
  for (i = 0; i < MAX_MOVES; i++) {
    priorityQp->queue[i] = NULL; /* initialize null pointers */
  }
  return priorityQp;
 error:
  log_info("error allocating memory for priority queue");
  return NULL;
}

void priorityQ_insert(priorityQ *priorityQp, puzzle *boardp, int f_score)
{ /* given f_score, insert board into priority queue */

  if (priorityQp->min_index > f_score || /* if inserting state with lower f_score */
      priorityQp->min_index == -1) { /* or if min_index not yet set */
    priorityQp->min_index = f_score; /* set min_index to f_score */
  }
  
  puzzle *temp =  priorityQp->queue[f_score];/* temporary pointer */

  if (temp == NULL) { /* empty slot */
    priorityQp->queue[f_score] = boardp; /* insert into empty slot */
  } else {
    priorityQp->queue[f_score] = boardp; /* insert into head of linked-list */
    boardp->next = temp; 
  }
  priorityQp->nelements++; /* increment element counter */
}

puzzle *priorityQ_extract_min(priorityQ *priorityQp)
{ /* extract minimum board from priority queue, and update priority queue */

  if (priorityQp->nelements == 0 || priorityQp->min_index == -1) {
    log_info("error: no elements to extract");
    return NULL; /* no elements to extract */
  }
  
  while (priorityQp->queue[priorityQp->min_index] == NULL) { /* if index is empty */
      priorityQp->min_index++;
  }
  
  puzzle *min_boardp = priorityQp->queue[priorityQp->min_index]; /* extract first node of linked list */
  priorityQp->queue[priorityQp->min_index] = min_boardp->next; 
 
  priorityQp->nelements --; /* decrease number of elements */

  if (priorityQp->nelements == 0) {
    priorityQp->min_index = -1; /* reset min_index if priority queue is empty */
  }
  return min_boardp; 
}

bool priorityQ_remove(priorityQ *priorityQp, unsigned long state, int f_score)
{ /* remove state from priority queue */
  puzzle *current = priorityQp->queue[f_score];
  puzzle *temp; /* temporary pointer */
  if (current == NULL) {
    log_info("error removing, empty linked list");
    return false;
  }
  if (current->state == state) { /* if state at top of the linked list */
    priorityQp->queue[f_score] = current->next;
    free(current);
    priorityQp->nelements--; /* decrease number of elements */
    if (priorityQp->nelements == 0) priorityQp->min_index = -1; /* if no elements in priority queue, reset min index */
    return true;
  }

  while (current->next->state != state) { /* if state not at the top, traverse linked list */
    if (current->next == NULL) {
      log_info("error removing, state not in f_score index");
      return false;
    }
    current = current->next;
  }
  temp = current->next;
  current->next = temp->next;
  free(temp);
  priorityQp->min_index--; /* decrease number of elements */

  if (priorityQp->nelements == 0) priorityQp->min_index = -1; /* if no elements in priority queue, reset min index */
  return true;
}
  


void priorityQ_free(priorityQ *priorityQp)
{ /* free priority queue */
  if (priorityQp->nelements == 0) {
    log_info("priority queue is empty");
  } else {
    int i = priorityQp->min_index;
    puzzle *temp; /* temporary pointer */
    for (; i < MAX_MOVES; i++) {
      while ( priorityQp->queue[i] != NULL)  {
	/* successively free head of linked list, until index is empty */
	temp = priorityQp->queue[i]; 
	priorityQp->queue[i] = temp->next;
	free(temp);
      }
    }
  }
  free(priorityQp); /* free priority queue array */
}


/********************************************
 *      OPERATIONS FOR CLOSED SET           *
 ********************************************/

hash_node *closed_init(void)
{ /* initialize closed set: array of hash_nodes */
  int i;
  hash_node *closed = malloc(PERMUTATIONS * sizeof(hash_node));
  check_mem(closed);
  
  for (i = 0; i < PERMUTATIONS; i++) {
    closed[i].state = 0;
    closed[i].parent = 0;
    closed[i].processed = false;
    closed[i].f_score = INT_MAX; /* INT_MAX indicates state is not yet discovered */
  }
  return closed;
 error:
  log_info("error in allocating memory for closed set");
  return NULL;
}


int hash_f(unsigned long state, int i)
{ /* returns hash value, given state and count (double probing) */
  return ((state % PERMUTATIONS) + (i * ( PRIME - (state % PRIME)))) % PERMUTATIONS;
}

int closed_discover(hash_node closed[], unsigned long state, unsigned long parent, int f_score)
{ /* search closed set for state:
   * if not found, set to discovered, update state, parent and update f_score. return f_score
   * if found and processed, do nothing. return INT_MAX.
   * if found and not processed, compare f_scores:
   *  - if f_score lower than existing, update f_score, and return old f_score
   *  - if f_score higher or equivalent to existing, do nothing. return INT_MAX.
   */

  int i = 0; /* counter for hash function */
  int old_f_score;
  int index;

  while (true) {
    index = hash_f(state, i);  /* hash function */
    if (closed[index].state == 0) {
      closed[index].state = state; /* discover state */
      closed[index].parent = parent;
      closed[index].f_score = f_score;
      return f_score;
    } else if (closed[index].state == state) { /* if found */
      if (closed[index].processed == true) { /* if processed */
	return INT_MAX; /* do nothing */
      } else {
	if (f_score >= closed[index].f_score) { /* f_score higher than existing */
	  return INT_MAX; /* do nothing */
	} else {
	  old_f_score = closed[index].f_score;
	  closed[index].f_score = f_score; /* update f_score */
	  //log_info("updated state with f_score: %d ", f_score);
	  return old_f_score;
	}
      }
    }
    i++;
  }
}

bool closed_process(hash_node closed[], unsigned long state)
{ /* search closed set for state and set to processed */

  int i = 0; /* counter hash function */
  int index;

  while (true) {
    index = hash_f(state, i); /* hash function */
    if (closed[index].state == 0) {
      log_info("state not yet discovered");
      return false;
    } else if (closed[index].state == state) {
      if (closed[index].processed == true) log_info("state already processed");
      closed[index].processed = true;
      return true;
    }
    i++;
  }
}


int closed_free(hash_node closed[])
{ /* free closed set and
     return count of states that have been discovered/processed */
  int i;
  int count = 0;
  for (i = 0; i < PERMUTATIONS; i++) {
    if (closed[i].state != 0) {
      count++;
    }
  } 

  free(closed);
  return count;
}

/********************************************
 *       OPERATIONS FOR A* SEARCH           *
 ********************************************/

int no_heuristic(unsigned long state, int nmoves)
{ /* f_score = number of moves made */
  return nmoves;
}

int misplaced_tile_heuristic(long unsigned state, int nmoves)
{ /* f_score = number of misplaced tiles + number of moves made */
  /* remember to exclude blank tile */
  int i;
  int misplaced = 0; /* number of misplaced tiles */
  bool blank_misplaced = true;
  
  unsigned long mask = 0xF;
  unsigned long extract_same = state ^ END_STATE; /* returns 0000 (binary) where sequence matches */
  for (i = 0; i < 8; i++) {
    if (blank_misplaced == true) {
	if (((state & mask) == 0) && ((END_STATE & mask)== 0)) { /* if blank matches */
	  blank_misplaced = false;
	}
      }
    if (!((extract_same & mask) == 0)) { /* if 4-bit pattern do not match */
	misplaced ++;
      }
    mask = mask << 4;
  }

  if (blank_misplaced == true) misplaced--; /* do not include count for misplaced blank */

  return misplaced + nmoves;
}

int manhattan_distance_heuristic(unsigned long state, int nmoves)
{ /* f_score =  manhattan distance + nmoves */
  
  if (state == END_STATE) return 0 + nmoves;
  
  int i;
  unsigned long tile; /* current tile */
  int index1, index2; /* tile index for state and end state */
  int x1, x2, y1, y2; /* x and y coordinates for tile */
  unsigned long mask = 0xF;
  int distance = 0;

  
  for (tile = 1; tile < 9;tile++) {
    i = 0;
    while (i < 9) {
      if ((((state >> (4 * i))^tile) & mask) == 0) index1 = i; /* extract tile index for state */
      if ((((END_STATE >> (4 * i))^tile) & mask) == 0) index2 = i; /* extract tile index for end state */
      i++;
    }
    
    if (index1 == index2) continue;
    
    x1 = index1 / 3;
    x2 = index2 / 3;

    y1 = index1 % 3;
    y2 = index2 % 3;

    distance += abs(y1 - y2) + abs(x1 - x2);
  }

  return distance + nmoves;
}


puzzle *a_star_step(priorityQ *priorityQp, hash_node closed[], int (*heuristic)(unsigned long int state, int nmoves))
{ /* performs one step of a_star */  
  int i;
  puzzle *candidate;
  puzzle *next_boardp = priorityQ_extract_min(priorityQp);
 
  if (next_boardp == NULL) log_info("error, failed to extract from priority queue");

  if (next_boardp->state == END_STATE) { /* solved */
    return next_boardp;
  }

  int f_score;
  int aux_f_score; /* f_score to determine whether priority queue needs replacement */
 
  for (i = 0; i < next_boardp->nchildren; i++) { /* for children of extracted state */
    f_score = heuristic(next_boardp->child[i], next_boardp->nmoves + 1); /* calculate f_score */
    aux_f_score = closed_discover(closed, next_boardp->child[i], next_boardp->state, f_score);
    if (aux_f_score != INT_MAX) { 
      if (f_score != aux_f_score) { /* need to replace in priority queue */
	priorityQ_remove(priorityQp, next_boardp->child[i], aux_f_score);
      }
      candidate = board_init(next_boardp->child[i], next_boardp->state, next_boardp->nmoves + 1); /* initialize child board */
      priorityQ_insert(priorityQp, candidate, f_score); /* insert into priority queue */
    }    
  }
  closed_process(closed, next_boardp->state); /* process state */
  return next_boardp;
}

puzzle *a_star(priorityQ *priorityQp, hash_node closed[], int (*heuristic)(unsigned long int state, int nmoves))
{
  if (priorityQp->nelements == 0) { /* no elements to extract */
    log_info("error: no elements in the priority queue");
    return NULL;
  }
  puzzle *boardp = a_star_step(priorityQp, closed, heuristic); /* extract first processed state */
  puzzle *temp = boardp;
  while (boardp->state != END_STATE)
    {
      if (priorityQp->nelements == 0) { /* no elements to extract */
	log_info("error: no elements in the priority queue");
	return NULL;
      }
      boardp= a_star_step(priorityQp, closed, heuristic);
      free(temp); /* free previous board */
      temp = boardp;
    }
  return boardp;
}


int main(int argc, char *argv[])
{
 
  srand(time(NULL));

  int iterations;
  double timings[ITERATIONS]; /* array to store timings */
  int expanded[ITERATIONS]; /* array to store number of expanded (discovered / processed) states */
  double total = 0; /* total time taken */
  int total_expanded = 0; /* total expanded states */
 
  for (iterations = 0; iterations < ITERATIONS; iterations++) {
    
    unsigned long initial_state = random_state(); /* initialize random state */
    puzzle *boardp = board_init(initial_state, 0, 0); /* initialize board */
    priorityQ *priorityQp = priorityQ_init(); /* initialize priority queue */
    hash_node *closed = closed_init(); /* initialize closed set */
  
    clock_t start, end;
    long double cpu_time_used;
  
    priorityQ_insert(priorityQp, boardp, 0);
    closed_discover(closed, initial_state, 0, 0);

    start = clock();
    boardp = a_star(priorityQp, closed, manhattan_distance_heuristic); /* solve the board */
    end = clock();
    cpu_time_used = ((long double)(end - start))/ CLOCKS_PER_SEC; /* in seconds */

    priorityQ_free(priorityQp);
  
    //trace(closed, boardp->state); /* for tracing optimal move sequence */
    //log_info("number of moves is %d", boardp->nmoves);
    //log_info("time take ins %Lf", cpu_time_used);
    
    expanded[iterations] = closed_free(closed);
    timings[iterations] = cpu_time_used;
    free(boardp); 
  
  }

  for (iterations = 0; iterations < ITERATIONS; iterations ++) {
    total += timings[iterations];
    total_expanded += expanded[iterations];
  }
  
  log_info("average time taken is %lfs", (double)(total/ITERATIONS));
  log_info("average expanded is %d", total_expanded/ITERATIONS);
  
  return 0;
}
