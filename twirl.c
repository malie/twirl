#include <stdio.h>
#include <assert.h>
#include <sys/time.h>
#include <string.h>
#include "picosat.h"

#define S 9
#define S2 (S*S)
#define S3 (S*S*S)
#define S4 (S*S*S*S)

int field(int x, int y) {
  assert(x >= 0 && x < S && y >= 0 && y < S);
  return y*S+x; }

int field_col(int f) {
  assert(f >= 0 && f < S2);
  return f%S;}

int field_row(int f) {
  assert(f >= 0 && f < S2);
  return f/S;}

int white(int f)    { assert(f >= 0 && f < S2); return 1+f;}
int black(int f)    { assert(f >= 0 && f < S2); return 1+S2+f;}
int nblack(int f)   { assert(f >= 0 && f < S2); return 1+2*S2+f;}
int numfield(int f) { assert(f >= 0 && f < S2); return 1+3*S2+f;}
int xblack(int f)   { assert(f >= 0 && f < S2); return 1+4*S2+f;}

#define LS2OFFS (1+5*S2)
int num(int f, int d) {
  assert(f >= 0 && f < S2);
  assert(d >= 0 && d < S);
  return LS2OFFS+f*S+d;}

// f field, l length
int hstr(int f, int l) {
  assert(f >= 0 && f < S2);
  assert(l >= 2 && l <= S);
  return LS2OFFS+S3+f*S+(l-2);
}

int vstr(int f, int l) {
  assert(f >= 0 && f < S2);
  assert(l >= 2 && l <= S);
  return LS2OFFS+2*S3+f*S+(l-2);
}

#define LS3OFFS (LS2OFFS+3*S3)

// f field, l length, d smallest digit
int hstrd(int f, int l, int d) {
  assert(f >= 0 && f < S2);
  assert(l >= 2 && l <= S);
  assert(d >= 0 && d < S);
  return LS3OFFS+f*S2+(l-2)*S+d;}

int vstrd(int f, int l, int d) {
  assert(f >= 0 && f < S2);
  assert(l >= 2 && l <= S);
  assert(d >= 0 && d < S);
  return LS3OFFS+S4+f*S2+(l-2)*S+d;}

#define LASTVAR (LS3OFFS+2*S4)



////////////////////////////////////////

#define li(x) printf(#x " -> %i\n", (x))
#define lii(x,y) printf(#x ", " #y " -> %i, %i\n", (x), (y))
#define liii(x,y,z) printf(#x ", " #y ", " #z " -> %i, %i\n", \
			   (x), (y), (z))

////////////////////////////////////////

int same_col_but(int f, int row) {
  assert(f >= 0 && f < S2);
  assert(row >= 0 && row < S);
  int col = f % S;
  return field(col, row);
}

int same_row_but(int f, int col) {
  assert(f >= 0 && f < S2);
  assert(col >= 0 && col < S);
  int row = f / S;
  return field(col, row);
}

////////////////////////////////////////

void*
memdup(void* mem, size_t s) {
  void *res = malloc(s);
  memcpy(res, mem, s);
  return res;}

////////////////////////////////////////

struct clause {
  int id;
  // int desc;
  int size;
  int *lits;
};

#define MAXCLAUSELEN S+5
int current_clause[MAXCLAUSELEN];
int ccptr = 0;

#define NUM_CLAUSES (S2*10+S3*60+2*S4)
struct clause clauses[NUM_CLAUSES];
int clausesptr = 0;

void
add_lit(int lit) {
  if (lit) {
    assert(ccptr < MAXCLAUSELEN);
    current_clause[ccptr++] = lit;}
  else {
    assert(ccptr > 0);
    struct clause *c = clauses + clausesptr;
    c->id = clausesptr;
    // c->desc =  234;
    c->size = ccptr;
    c->lits = memdup(current_clause, ccptr*sizeof(int));
    ccptr = 0;
    clausesptr++;
    assert(clausesptr < NUM_CLAUSES);}}


int
firstMUSVar() {
  return LASTVAR;}

int
lastMUSVar() {
  return LASTVAR+clausesptr-1;}

void
feedClausesToPicosat(PicoSAT *p, int forMUS) {
  for (int cl = 0; cl < clausesptr; cl++)
    {
      struct clause *c = clauses + cl;

      if (forMUS)
	picosat_add(p, LASTVAR + cl);
      
      for (int i = 0; i < c->size; i++) {
	picosat_add(p, c->lits[i]);
      }
      picosat_add(p, 0);
    }
}

////////////////////////////////////////



void
printResults(PicoSAT *p, int* fields) {
  
  int verbose = 0;
  int y, x, d;
  for (y = 0; y < S; y++) {
    for (x = 0; x < S; x++) {
      int f = field(x, y);

      if (picosat_deref(p, black(f)) == 1)
	printf("B");
      else if (picosat_deref(p, nblack(f)) == 1)
	printf("N");
      /* else if (verbose && picosat_deref(p, numfield(f)) == 1) */
      /* 	printf("#"); */
      else if (verbose && picosat_deref(p, white(f)) == 1)
	printf("w");
      else 
	printf(" ");

      if (fields)
	printf("%c", fields[f] ? '.' : ' ');

      if (verbose) {
	if (picosat_deref(p, xblack(f)) == 1)
	  printf("x");
	else
	  printf(" ");}

      int fo = 0;
      for (d = 0; d < S; d++) {
	if (picosat_deref(p, num(f, d)) == 1) {
	  printf("%i", 1+d);
	  fo++;}}
      if (fo == 0) {
	printf("_");
	fo+=1;}
      while (fo < 2) {
	printf(" ");
	fo++;}
      
      printf(" ");
    }
    printf("\n");
  }
  return;
  for (int row = 0; row < S; row++) {
    for (int col = 0; col < S; col++) {
      for (int len = 2; len < S-col; len++) {
	if (picosat_deref(p, hstr(field(col, row), len)))
	  printf("hstr(field(%i,%i), %i) -> 1\n", col, row, len);
      }
    }
  }
  printf("\n");
  for (int row = 0; row < S; row++) {
    for (int col = 0; col < S; col++) {
      for (int len = 2; len < S-row; len++) {
	if (picosat_deref(p, vstr(field(col, row), len)))
	  printf("vstr(field(%i,%i), %i) -> 1\n", col, row, len);
      }
    }
  }
}



void
encodeToCNF() {

#define clause2(a,b)				\
  ({add_lit(a); add_lit(b); add_lit(0);})

#define clause3(a,b,c)					\
  ({add_lit(a); add_lit(b); add_lit(c); add_lit(0);})

#define implies(a,b)  clause2(-a, b);
#define implies_or(a,b,c) clause3(-a, b, c);
  
  int f, d, l, t, e, c, r;
  
  for (f = 0; f < S2; f++) {

    clause3(white(f), black(f), nblack(f));
    implies(white(f), -black(f));
    implies(white(f), -nblack(f));
    implies(nblack(f), -black(f));
    implies(nblack(f), -white(f));
    implies(black(f), -white(f));
    implies(black(f), -nblack(f));

    
    implies(white(f), numfield(f));
    implies(nblack(f), xblack(f));
    implies(nblack(f), numfield(f));
    implies(black(f), xblack(f));

    
    implies(white(f), -xblack(f));
    
    implies(black(f), -numfield(f));
    implies(xblack(f), -white(f));
    implies(numfield(f), -black(f));
    implies_or(numfield(f), white(f), nblack(f));
    implies_or(xblack(f), black(f), nblack(f));


    for (d = 0; d < S; d++) {
      implies(num(f,d), numfield(f));

      for (e = 0; e < S; e++) {
	// each digit at most once in each row
	t = same_row_but(f, e);
	if (t != f)
	  implies(num(f, d), -num(t, d));

	// each digit at most once in each column
	t = same_col_but(f, e);
	if (t != f)
	  implies(num(f, d), -num(t, d));

	// only one digit at most in each cell
	if (d != e)
	  implies(num(f, d), -num(f, e));
      }
    }
    // numfield(f) => exists d . num(f,d)
    add_lit(-numfield(f));
    for (d = 0; d < S; d++)
      add_lit(num(f, d));
    add_lit(0);

    // no single white fields
    int c = field_col(f);
    int r = field_row(f);
    add_lit(-white(f));
    if (c >= 1)
      add_lit(-xblack(field(c-1, r)));
    if (c < S-1)
      add_lit(-xblack(field(c+1, r)));
    if (r >= 1)
      add_lit(-xblack(field(c, r-1)));
    if (r < S-1)
      add_lit(-xblack(field(c, r+1)));
    add_lit(0);
  }

  
  
  // Encoding the straights:
  // A constellation like B W W W B is a straight of len 3,
  // when there are no blacks in between.
  // The two B's can also be the border.
  // Only straights of a length >= 2 are interesting.
  // The implications are encoded into clauses like this:
  //    a&b => x
  //    -(a&b) | x
  //    -a | -b | x

  for (f = 0; f < S2; f++) {
    int col = field_col(f);
    int row = field_row(f);
    for (c = col+1; c < S; c++) {
      // col is the first column
      // c is the last column (inclusively)
      int len = c-col+1;
      if (col > 0)
	add_lit(-xblack(same_row_but(f, col-1)));
      if (c <= S-2)
	add_lit(-xblack(same_row_but(f, c+1)));
      for (int s = 0; s < len; s++)
	add_lit(-white(same_row_but(f, col+s)));
      add_lit(hstr(f, len));
      add_lit(0);

      if (col > 0)
	implies(hstr(f,len), xblack(same_row_but(f, col-1)));
      if (c <= S-2)
	implies(hstr(f,len), xblack(same_row_but(f, c+1)));
      for (int s = 0; s < len; s++)
	implies(hstr(f,len), white(same_row_but(f, col+s)));

      // hstr(f,len) => hstrd(f,len,0) | hstrd(f,len,1) ...
      add_lit(-hstr(f, len));
      int largest_mind = S-len;
      for (int mind = 0; mind <= largest_mind; mind++)
	add_lit(hstrd(f, len, mind));
      add_lit(0);

      // hstrd(f,len,d) => num(f,d) | num(f,d+1) ...
      // for all the num()'s in the straight
      for (int sc = col; sc <= c; sc++) {
	int ff = same_row_but(f, sc);
	for (int mind = 0; mind <= largest_mind; mind++) {
	  add_lit(-hstrd(f, len, mind));
	  for (int dig = 0; dig < len; dig++)
	    add_lit(num(ff, mind+dig));
	  add_lit(0);
	}
      }
    }

    for (r = row+1; r < S; r++) {
      int len = r-row+1;
      if (row > 0)
	add_lit(-xblack(same_col_but(f, row-1)));
      if (r <= S-2)
	add_lit(-xblack(same_col_but(f, r+1)));
      for (int s = 0; s < len; s++)
	add_lit(-white(same_col_but(f, row+s)));
      add_lit(vstr(f, len));
      add_lit(0);

      if (row > 0)
	implies(vstr(f,len), xblack(same_col_but(f, row-1)));
      if (r <= S-2)
	implies(vstr(f,len), xblack(same_col_but(f, r+1)));
      for (int s = 0; s < len; s++)
	implies(vstr(f,len), white(same_col_but(f, row+s)));

      
      // and the vstr(f,len) def
      add_lit(-vstr(f, len));
      int largest_mind = S-len;
      for (int mind = 0; mind <= largest_mind; mind++)
	add_lit(vstrd(f, len, mind));
      add_lit(0);

      // vstrd(f,len,d) => num(f,d) | num(f,d+1) ...
      // for all the num()'s in the straight
      for (int sc = row; sc <= r; sc++) {
	int ff = same_col_but(f, sc);
	for (int mind = 0; mind <= largest_mind; mind++) {
	  add_lit(-vstrd(f, len, mind));
	  for (int dig = 0; dig < len; dig++)
	    add_lit(num(ff, mind+dig));
	  add_lit(0);
	}
      }
    }
  }
  printf("encoded into %i clauses\n", clausesptr);
}

unsigned long long seed = 9111111111111239ULL;
void
init_seed() {
  struct timeval t;
  gettimeofday(&t, NULL);
  seed += t.tv_usec;}

int randint(int n) {
  seed += (seed>>1) + (seed>>5) + (seed>>17) + (seed>>39) + (seed<<3);
  return seed%n;}

int mirror(int kind, int f) {
  if (kind == 1)
    return field(S-1-field_col(f),
		 S-1-field_row(f));
  else if (kind == 2)
    return field(S-1-field_col(f),
		 field_row(f));
  else abort();}
    


void
assume_others_white(PicoSAT *p, int*fields)
{
  for (int g = 0; g < S2; g++) {
    if (fields[g] == 0)
      picosat_assume(p, white(g));}}


int
find_digit(PicoSAT *p, int f) {
  int digit = -1;
  for (int d = 0; d < S; d++) {
    if (picosat_deref(p, num(f, d)) == 1) {
      digit = d;}}
  assert(digit != -1);
  return digit;}


int
has_second_solution(PicoSAT *p, int *fields) {
  int digits[S2];
  int nsolution[S2+1];
  int fillptr = 0;

  for (int f = 0; f < S2; f++)
    digits[f] = -1;

  int nadded = 0;
  for (int f = 0; f < S2; f++)
    {
      if (fields[f] == 0
	  && picosat_deref(p, white(f)) == 1)
	{
	  int digit = find_digit(p, f);
	  assert(digit != -1);
	  digits[f] = digit;
	  nsolution[fillptr++] = -num(f, digit);
	  nadded++;}}

  nsolution[fillptr++] = 0;

  picosat_push(p);
  picosat_add_lits(p, nsolution);

  printf("added a clause with %i literals for next solution...\n",
	 nadded);
  
  assume_others_white(p, fields);
  int res;
  if (picosat_sat(p, -1) == PICOSAT_SATISFIABLE) {
    printf("has second solution\n");
    if (0)
      for (int f = 0; f < S2; f++)
	{
	  int od = digits[f];
	  if (od != -1 && picosat_deref(p, num(f, od)) != 1) {
	    int digit = find_digit(p, f);
	    
	    printf("  diff %i,%i   %i -> %i\n",
		   field_col(f), field_row(f), 1+od, 1+digit);}}
    
    res = 1;}
  else {
    printf("NO SECOND SOLUTION\n");
    res = 0;
  }
  picosat_pop(p);
  return res;}

PicoSAT*
fill_new_instance(int *fields, int *digit, int forMUS)
{
  PicoSAT *p = picosat_init();
  feedClausesToPicosat(p, forMUS);
  
  for (int f = 0; f < S2; f++) {
    if (fields[f] == 1) {
      picosat_add_arg(p, white(f), 0);
      picosat_add_arg(p, num(f, digit[f]), 0);
    } else if (fields[f] == 2) {
      picosat_add_arg(p, nblack(f), 0);
      picosat_add_arg(p, num(f, digit[f]), 0);
    } else if (fields[f] == 3) {
      picosat_add_arg(p, black(f), 0);
    }
  }
  return p;
}



// This is exploring how determining a puzzles difficulty could work.
void
determine_difficulty(PicoSAT *pouter, int *fields, int *digit)
{
  int forMUS = 1;
  PicoSAT *p = fill_new_instance(fields, digit, forMUS);

  printf("firstMUSVAR() -> %i   lastMUSVAR() -> %i\n",
	 firstMUSVar(), lastMUSVar());

  int verbose_err = 5;
  for (int f = 0; f < S2; f++) {
    if (fields[f] == 0
	&& picosat_deref(pouter, white(f)) == 1)
      {
	// a white field assigned by the solver, not by the problem.
	int digit = find_digit(pouter, f);
	
	assume_others_white(p, fields);
	picosat_assume(p, -num(f, digit));

	int lastmv = lastMUSVar();
	for (int a = firstMUSVar(); a < lastmv; a++) {
	  picosat_assume(p, -a);}

	printf("start sat\n");
	int res = picosat_sat(p, -1);
	printf("end sat\n");
	assert(res == PICOSAT_UNSATISFIABLE);
	
	printf("how difficult is it to see %i,%i must be %i\n",
	       field_col(f), field_row(f), 1+digit);
	int* failed =
	  picosat_mus_assumptions(p, (void*)0, (void*)0, 0);
	printf("failed assuptions:\n  ");
	int *p = failed;
	int i = 0;
	while (*p) {
	  if ((i++%10) == 9)
	    printf("\n  ");
	  printf("%i ", *p);
	  p++;
	}
	printf("\n");
	
      }
  }
  picosat_reset(p);
}

void
showLinkToGame(PicoSAT *p, int *fields, int *digit) {
  char url64codes[] =
    "0123456789"
    "_-"
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  // scrumble it a bit, so you can't read
  // the solution from the url (easily)
  long cr = 1492UL;
#define next() ({cr = (cr*3UL+7UL)%13913131UL; cr%64; })
  
  char encodedGame[S2+1];
  int fillptr = 0;
  int nb = 0, ng = 0;
  
  for (int f = 0; f < S2; f++) {
    int val = 0;

    // xblack
    if (fields[f] == 2 || fields[f] == 3) {
      val += 1;
      nb++;}

    // digit shown
    int nf = fields[f] == 1 || fields[f] == 2;
    if (nf) {
      val += 2;
      ng++;}
    
    if (picosat_deref(p, white(f)) == 1
	|| picosat_deref(p, nblack(f)) == 1)
      val += 4*find_digit(p, f);

    int x = val ^ next();

    if (x < 0 || x > 63) {
      printf("bad code; val:%i, x:%i\n", val, x);
      assert(0);
    }
    encodedGame[fillptr++] = url64codes[x];}

  
  encodedGame[fillptr++] = 0;
  printf("\n\nURL to play this game:\n"
	 "http://malie.github.io/undiluted/play/str8ts.html"
	 "?p=%c%s&nb=%i&ng=%i\n\n",
	 url64codes[S],
	 encodedGame,
	 nb,
	 ng);
}


int
main()
{
  encodeToCNF();

  PicoSAT *p = picosat_init();
  feedClausesToPicosat(p, 0);
  
  init_seed();

  int fields[S2];
  int digit[S2];
  for (int i = 0; i < S2; i++) {
    fields[i] = 0;
    digit[i] = -1;}

  int num_black = S + randint(S) + 2;

  int mir = 1;

  if (!mir)
    num_black *= 2;

  for (int i = 0; i < 3000; i++)
    {
      printf("\n\n");
      int w =
	(num_black > 5) ? 1 + randint(2)
	: (num_black > 0) ? randint(3) : 0;
      if (w == 0) {
	int f = randint(S2);
	int d = randint(S);
	if (fields[f] != 0) continue;
	printf("white %i at %i,%i\n", d+1, field_col(f), field_row(f));

	picosat_push(p);
	picosat_add_arg(p, white(f), 0);
	picosat_add_arg(p, num(f, d), 0);
	fields[f] = 1;
	digit[f] = d;
	assume_others_white(p, fields);
	
	int res = picosat_sat(p, -1);
	if (res == PICOSAT_SATISFIABLE) {
	  printf("  => good\n");
	  printResults(p, fields);

	  if (!has_second_solution(p, fields)) {
	    goto success;}

	} else {
	  fields[f] = 0;
	  digit[f] = -1;
	  picosat_pop(p);
	  printf("  => bad\n");}}
      else if (w == 1) {
	int f = randint(S2);
	int d = randint(S);
	int mf = mirror(mir, f);
	if (fields[f] != 0
	    || (mir && fields[mf] != 0))
	  continue;
	printf("black with digit %i at %i,%i\n",
	       d+1, field_col(f), field_row(f));

	picosat_push(p);
	picosat_add_arg(p, nblack(f), 0);
	picosat_add_arg(p, num(f, d), 0);
	if (mir)
	  picosat_add_arg(p, black(mf), 0);

	fields[f] = 2;
	if (mir) fields[mf] = 3;
	digit[f] = d;
	assume_others_white(p, fields);
	
	int res = picosat_sat(p, -1);
	if (res == PICOSAT_SATISFIABLE) {
	  num_black--;
	  printf("  => good (%i black left)\n", num_black);
	  printResults(p, fields);
	  if (!has_second_solution(p, fields)) {
	    goto success;}
	  
	} else {
	  fields[f] = 0;
	  if (mir) fields[mf] = 0;
	  digit[f] = -1;
	  picosat_pop(p);
	  printf("  => bad\n");}}
      
      else if (w == 2) {
	int f = randint(S2);
	int d = randint(S);
	int mf = mirror(mir, f);
	if (fields[f] != 0
	    || (mir && fields[mf] != 0))
	  continue;

	printf("black at %i,%i\n", field_col(f), field_row(f));

	picosat_push(p);
	picosat_add_arg(p, black(f), 0);
	picosat_add_arg(p, black(mf), 0);
	fields[f] = 3;
	if (mir) fields[mf] = 3;
	assume_others_white(p, fields);
	
	int res = picosat_sat(p, -1);
	if (res == PICOSAT_SATISFIABLE) {
	  num_black--;
	  printf("  => good (%i black left)\n", num_black);
	  printResults(p, fields);
	  if (!has_second_solution(p, fields)) {
	    goto success;}

	} else {
	  fields[f] = 0;
	  if (mir) fields[mf] = 0;
	  picosat_pop(p);
	  printf("  => bad\n");}}
    }

  picosat_stats(p);
  printf("\nnothing found\n");
  return 1;

 success:
  
  printf("\nfound a game!\n");

  assume_others_white(p, fields);
  int res = picosat_sat(p, -1);
  assert(res == PICOSAT_SATISFIABLE);

  determine_difficulty(p, fields, digit);

  picosat_stats(p);

  showLinkToGame(p, fields, digit);
  return 0;
}
