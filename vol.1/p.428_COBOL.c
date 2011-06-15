/*
 * Algorithm 2.4A @ TAOCP::p.428
 * Algorithm 2.4B @ TAOCP::p.429
 * Algorithm 2.4C @ TAOCP::p.431
 *
 * author: Forrest Y. Yu <forrest.yu@gmail.com>, http://forrestyu.net/
 *
 * BUILD and RUN:
 *     $ gcc -std=c99 -Wall -o cobol p.428_COBOL.c
 *     $ ./cobol
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

/******************************************************************************
 * configuration
 ******************************************************************************/
#define INFO_COLOR		WHITE
#define FOUND_COLOR		YELLOW
#define STACK_PUSH_COLOR	CYAN
#define STACK_TOP_COLOR		MAGENTA
#define GRAPHVIZ_CMD		"neato"
#define FIG_FORMAT		"svg"
#define LOWEST_DOT_ID		0

/******************************************************************************
 * define
 ******************************************************************************/
#define MAX_DOTF_NR		1024
#define CMD_MAX			(FILENAME_MAX + 128)
#define DOT_LABEL_STR_LEN	1024
#define ______________________________	print_data_table
/******************************************************************************
 * global const
 ******************************************************************************/
/* colors */
const char BLACK[]  = {033, '[', '3', '0', 'm', 0};
const char RED[]    = {033, '[', '3', '1', 'm', 0};
const char GREEN[]  = {033, '[', '3', '2', 'm', 0};
const char YELLOW[] = {033, '[', '3', '3', 'm', 0};
const char BLUE[]   = {033, '[', '3', '4', 'm', 0};
const char MAGENTA[]= {033, '[', '3', '5', 'm', 0};
const char CYAN[]   = {033, '[', '3', '6', 'm', 0};
const char WHITE[]  = {033, '[', '3', '7', 'm', 0};
const char NOCOLOR[]= {033, '[',      '0', 'm', 0};

/* for graphviz */
const char U_ATTR[] = "arrowhead=\"open\", color=\"yellow\"";
const char D_ATTR[] = "arrowhead=\"open\", color=\"green\"";
const char L_ATTR[] = "arrowhead=\"normal\", color=\"blue\"";
const char R_ATTR[] = "arrowhead=\"normal\", color=\"red\"";

const char * subscript[]   = {"₀", "₁", "₂", "₃", "₄", "₅", "₆", "₇", "₈", "₉"};


/*
 * declaration
 */
struct symbol_table_entry;

/* 
 * constants
 */
enum {
	DATA_TABLE_SIZE = 128,
	STACK_SIZE      = 128,
	MAX_LEVEL	= 8
};

/*
 * enum & struct
 */
typedef enum symbol {A=0, B, C, D, E, F, G, H, SYMBOL_TABLE_SIZE} SYM;

typedef struct data_table_entry {
	struct data_table_entry * PREV;
	struct data_table_entry * PARENT;
	struct symbol_table_entry * NAME;
	struct data_table_entry * CHILD;
	struct data_table_entry * SIB;
} DTE;

typedef struct symbol_table_entry {
	int stuff;
	DTE * LINK;
} STE;

/* 
 * globals
 */
int id = 0;
/* int level_numbers[] = {1, 3, 7}; */
/* SYM symbols[]       = {A, B, C}; */

/*
 * 1 A             1 H	  
 *   3 B	     5 F  
 *     7 C	       8 G
 *     7 D	     5 B  
 *   3 E	     5 C  
 *   3 F	       9 E
 *     4 G	       9 D
 *		       9 G
 *
 *
 *           PREV    PARENT                 NAME     CHILD        SIB 
 * -----     ----    ------     ----------------    ------     ------
 * DT[0]        ^         ^     &symbol_table[A]    &DT[1]     &DT[7]      A1
 *   [1]        ^    &DT[0]     &symbol_table[B]    &DT[2]     &DT[4]      B3
 *   [2]        ^    &DT[1]     &symbol_table[C]         ^     &DT[3]      C7
 *   [3]        ^    &DT[1]     &symbol_table[D]         ^          ^      D7
 *   [4]        ^    &DT[0]     &symbol_table[E]         ^     &DT[5]      E3
 *   [5]        ^    &DT[0]     &symbol_table[F]    &DT[6]          ^      F3
 *   [6]        ^    &DT[5]     &symbol_table[G]         ^          ^      G4
 *   [7]        ^         ^     &symbol_table[H]    &DT[8]          ^      H1
 *   [8]   &DT[5]    &DT[7]     &symbol_table[F]    &DT[9]     &DT[10]     F5
 *   [9]   &DT[6]    &DT[8]     &symbol_table[G]         ^          ^      G8
 *  [10]   &DT[1]    &DT[7]     &symbol_table[B]         ^     &DT[11]     B5
 *  [11]   &DT[2]    &DT[7]     &symbol_table[C]    &DT[12]         ^      C5
 *  [12]   &DT[4]    &DT[11]    &symbol_table[E]         ^     &DT[13]     E9
 *  [13]   &DT[3]    &DT[11]    &symbol_table[D]         ^     &DT[14]     D9
 *  [14]   &DT[9]    &DT[11]    &symbol_table[G]         ^          ^      G9
 * ------------------------------------------------------------------------------------
 *  ^ is the abbreviation of (DTE*)0
 */
int level_numbers[] = {1, 3, 7, 7, 3, 3, 4, 1, 5, 8, 5, 5, 9, 9, 9};
SYM symbols[]       = {A, B, C, D, E, F, G, H, F, G, B, C, E, D, G};
STE symbol_table[SYMBOL_TABLE_SIZE];

/* 
 * functions
 */
/* routine via which the dot file is written */
void write_dot(int id, const char *fmt, ...)
{
	static int flag = 0;
	static FILE * dotf[MAX_DOTF_NR];
	static char dotfilename[MAX_DOTF_NR][FILENAME_MAX];
	char cmd[CMD_MAX];

	va_list args;
	va_start(args, fmt);

	assert(id >= 0 && id < MAX_DOTF_NR);
	if (id < LOWEST_DOT_ID)
		return;

	if (flag) {
		int i;
		for (i = 0; i < MAX_DOTF_NR; i++)
			dotf[i] = 0;
		flag = 1;
	}
	if (!dotf[id]) {
		char * p;
		strcpy(dotfilename[id], __FILE__);
		for (p = dotfilename[id] + strlen(dotfilename[id]); *p != '.'; p--);
		snprintf(p, (size_t)FILENAME_MAX, "%02d.dot", id);
		fprintf(stderr, "%sdot file: %s%s\n", INFO_COLOR,
			dotfilename[id], NOCOLOR);

		dotf[id] = fopen(dotfilename[id], "w");
		assert(dotf[id] != 0);

		fprintf(dotf[id], "digraph example {\n");

		fprintf(dotf[id],
			"\tnode [shape=circle"
			", fontname=Courier New"
			", penwidth=0.5"
			", height=0.3"
			", splines=\"true\""
			"];\n");
	}

	assert(dotf[id] != 0);

	/* if the dot file becomes big, something must be wrong */
	if (ftell(dotf[id]) > 16*1024) {
		fprintf(stderr,
			"%sdot file too big: %s"
			"%s%s%s"
			"%s, something must be wrong.\n%s",
			YELLOW, NOCOLOR,
			RED, dotfilename[id], NOCOLOR,
			YELLOW, NOCOLOR);
		assert(ftell(dotf[id]) <= 8*1024);
	}

	if (!fmt[0]) {
		fprintf(dotf[id], "}\n");
		fclose(dotf[id]);
		dotf[id] = 0;
		snprintf(cmd, (size_t)CMD_MAX, "%s -T%s \"%s\" -o "
			 "\"TAOCP - Algorithm 2.3.3A (%02d).%s\"",
			 GRAPHVIZ_CMD, FIG_FORMAT, dotfilename[id], id, FIG_FORMAT);
		printf("%s$ %s%s%s\n", WHITE, WHITE, cmd, NOCOLOR);
		/* printf("%s$ %s%s%s\n", WHITE, GREEN, cmd, NOCOLOR); */
		assert(system(cmd) == 0);
	} else {
		fprintf(dotf[id], "\t");
		vfprintf(dotf[id], fmt, args);
	}

	va_end(args);
}

void visit(DTE * p, double x, double y)
{
	write_dot(id, "\"%X\" [label=\"%c\", pos=\"%.5f,%.5f!\"];\n",
		  (unsigned int)p, p->NAME->stuff, x, y);
	if (p->PREV)
		write_dot(id, "\"%X\" -> \"%X\" [%s];\n", (int)p, (int)p->PREV, U_ATTR);
	if (p->SIB)
		write_dot(id, "\"%X\" -> \"%X\" [%s];\n", (int)p, (int)p->SIB, D_ATTR);
	if (p->PARENT)
		write_dot(id, "\"%X\" -> \"%X\" [%s];\n", (int)p, (int)p->PARENT, L_ATTR);
	if (p->CHILD)
		write_dot(id, "\"%X\" -> \"%X\" [%s];\n", (int)p, (int)p->CHILD, R_ATTR);

}
void traverse_data_table(DTE * DT, int depth, double x, double y)
{
	const double xshift[] = {1.0, 1.0, 1.0, 1.0, 1.0};
	const double yshift[] = {3.0, 1.0, 1.0, 1.0, 1.0};
	DTE * p = DT;
	if (!p)
		return;
	while (p) {
		traverse_data_table(p->CHILD, depth + 1, x + xshift[depth], y);
		visit(p, x, y);
		p = p->SIB;
		x += 0.2*depth;
		y -= yshift[depth];
	}
}
void print_data_table(DTE * DT, const char * label_fmt, ...)
{
	va_list args;
	va_start(args, label_fmt);

	char label[DOT_LABEL_STR_LEN];

	vsnprintf(label, DOT_LABEL_STR_LEN, label_fmt, args);

	/* printf("%sdot %02d : %s%s\n", INFO_COLOR, id, label, NOCOLOR); */
	write_dot(id, "label = \"p.428\\n%s\";\n", label);
	traverse_data_table(DT, 0, 0.0, 0.0);
	write_dot(id, "");
	id++;

	va_end(args);
}

int get_pair(int * pL, STE ** pP)
{
	static int i = 0;
	static int pair_cnt = sizeof(level_numbers) / sizeof(level_numbers[0]);

	assert(pair_cnt < DATA_TABLE_SIZE);

	if (i == 0) { /* initialize the symbol_table[]: set everything to 0 */
		int k;
                /*                    stuff        LINK
		 * ---------------    -----     -------
                 * symbol_table[0]      'A'     (DTE*)0
                 *             [1]      'B'     (DTE*)0
                 *             [2]      'C'     (DTE*)0
                 *             [3]      'D'     (DTE*)0
                 *             [4]      'E'     (DTE*)0
                 *             [5]      'F'     (DTE*)0
                 *             [6]      'G'     (DTE*)0
                 *             [7]      'H'     (DTE*)0
                 */
		for (k = 0; k < SYMBOL_TABLE_SIZE; k++) {
			symbol_table[k].stuff = k + 'A';
			symbol_table[k].LINK = 0;
		}
	}

	if (i >= pair_cnt)	/* no more pairs */
		return 0;

	*pL = level_numbers[i];
	symbol_table[symbols[i]].stuff = 'A'+symbols[i];
	*pP = &symbol_table[symbols[i]];
	printf("pair:%d, %c\n", level_numbers[i], symbol_table[symbols[i]].stuff);
	i++;

	return 1;
}

DTE * alloc_DTE(void)
{
	static DTE pool[DATA_TABLE_SIZE];
	static int i = 0;
	assert(i < DATA_TABLE_SIZE);
	return &pool[i++];
}

static int   stack_L[STACK_SIZE];
static DTE * stack_P[STACK_SIZE];
static int top = 0;	/* top is the top element */
void push(int L, DTE * P)
{
	++top;
	assert(top < STACK_SIZE);
	stack_L[top] = L;
	stack_P[top] = P;
	printf("%s(%d,%c) <-- pushed%s\n", STACK_PUSH_COLOR,
	       L, P ? P->NAME->stuff : '^', NOCOLOR);
}
void get_top(int * pL, DTE ** pP)
{
	assert(top > 0);
	*pL = stack_L[top];
	*pP = stack_P[top];
	printf("%s(%d,%c)%s\n", STACK_TOP_COLOR,
	       *pL, *pP ? (*pP)->NAME->stuff : '^', NOCOLOR);
}
void remove_top(void)
{
	--top;
	printf("%spoped%s\n", STACK_TOP_COLOR, NOCOLOR);
}
void pop(int * pL, DTE ** pP)
{
	get_top(pL, pP);
	remove_top();
}

/*
 * Algorithm 2.4 A
 */
DTE * Algorithm_A(void)
{
	DTE * FIRST = 0;
	int L;
	STE * P;

	/* A1. [Initialize.] */
	/* ______________________________(FIRST, "A1"); */
	push(0, 0);

	while (1) {
		/* A2. [Next item.] */
		/* ______________________________(FIRST, "A2"); */
		if (!get_pair(&L, &P))
			break;
		DTE * Q = alloc_DTE();

		/* A3. [Set name links.] */
		/* ______________________________(FIRST, "A3"); */
		Q->PREV = P->LINK;
		P->LINK = Q;
		Q->NAME = P;

		/* A4. Compare levels.] */
		/* ______________________________(FIRST, "A4"); */
		int L1;
		DTE * P1;
		get_top(&L1, &P1);
		if (P1 == 0) {
			FIRST = Q;
		} else if (L1 < L) {
			P1->CHILD = Q;
		} else {
			/* ______________________________(FIRST, "A5"); */
			/* A5. [Remove top level.] */
			while (L1 > L) {
				remove_top();
				get_top(&L1, &P1);
			}
			if (L1 == L) {
				P1->SIB = Q;
				remove_top();
				get_top(&L1, &P1);
			} else {
				fprintf(stderr, "%sError: mixed numbers have"
					" occurred on the same level!%s\n",
					RED, NOCOLOR);
				assert(L1 == L);
			}
		}

		/* A6. [Set family links.] */
		/* ______________________________(FIRST, "A6"); */
		Q->PARENT = P1;
		Q->CHILD = 0;
		Q->SIB = 0;

		/* A7. [Add to stack.] */
		/* ______________________________(FIRST, "A7"); */
		push(L, Q);
	}
	return FIRST;
}

int s2Pk(const char * s, STE * st[])
{
	const char * p = s;
	int n = 0;
	while (*p) {
		if (*p >= 'A' && *p <= 'Z') {
			assert(n < MAX_LEVEL);
			st[n++] = &symbol_table[*p++ - 'A'];
		} else if (*p == ' ') {	     /* skip " of" */
			const char c = *++p;
			if (c == 'o') {
				assert(*++p == 'f');
				assert(*++p == ' ');
			} else {
				assert(c >= 'A' && c <= 'Z');
			}
		} else {
			printf("unexpected char: '%c'\n", *p);
			assert(0);
		}
	}
	assert(n < sizeof(subscript) / sizeof(subscript[0]));
	for (int i = 0; i < n; i++) {
		printf("P%s: %c\n", subscript[i], st[i]->stuff);
	}

	return n;
}

int my_Algorithm_B(DTE * FIRST, const char * s, DTE * qualified[])
{
	STE * st[MAX_LEVEL];
	int n = s2Pk(s, st);

	if (!n)			/* no input at all */
		return 0;

	int qualified_nr = 0;

	/*
	 * Algorithm 2.4B @ p.429~p.430
	 */
	DTE * Q;
	DTE * P;
	DTE * S;
	int k;
	Q = 0;

	for (P = st[0]->LINK; P; P = P->PREV) {
		for (k = 0, S = P; S; S = S->PARENT) {
			if (S->NAME == st[k]) {
				printf("k: %d, P%s: %c\n",
				       k, subscript[k], st[k]->stuff);
				k++;
				if (k == n)
					break;
			}
		}
		if (k == n) {
			qualified[qualified_nr++] = P;
			if (Q == 0) {
				Q = P;
				printf("%sfound one:%s\n", FOUND_COLOR, NOCOLOR);
				for (DTE * x = Q; x; x = x->PARENT)
					printf("%s%c%s ", FOUND_COLOR, x->NAME->stuff, NOCOLOR);
				printf("\n");
			} else {
				printf("%sfound more than one:%s\n", FOUND_COLOR, NOCOLOR);
				for (DTE * x = P; x; x = x->PARENT)
					printf("%s%c%s ", FOUND_COLOR, x->NAME->stuff, NOCOLOR);
				printf("\n");
			}
		}
	}

	return qualified_nr;
}

int TAOCP_Algorithm_B(DTE * FIRST, const char * s, DTE * qualified[])
{
	STE * st[MAX_LEVEL];
	int n = s2Pk(s, st);

	printf("n:%d\n", n);

	if (!n)			/* no input at all */
		return 0;

	int qualified_nr = 0;

	/*
	 * Algorithm 2.4B @ p.429~p.430
	 */
	DTE * Q;
	DTE * P;
	DTE * S;
	int k;

	/* B1. [Initialize.] */
	Q = 0;
	P = st[0]->LINK;

B2:
	/* B2. [Done?] */
	if (P == 0)
		return qualified_nr;
	S = P;
	k = 0;

B3:
	/* B3. [Match complete?] */
	if (k < n - 1) {
		goto B4;
	} else {
		qualified[qualified_nr++] = P;

		if (Q != 0)
			printf("more than one.\n");
		Q = P;
		P = P->PREV;
		goto B2;
	}

B4:
	/* B4. [Increase k.] */
	k++;

B5:
	/* B5. [Move up tree.] */
	S = S->PARENT;
	if (S == 0) {
		P = P->PREV;
		goto B2;
	}

	/* B6. [A_k match?] */
	/* printf("%sB%d.%s\n", RED, 6, NOCOLOR); */
	/* printf("S: %c, P%s: %c\n", S->NAME->stuff, subscript[k], st[k]->stuff); */
	if (S->NAME == st[k])
		goto B3;
	else
		goto B5;
}

void test_B(DTE * dt)
{
	DTE * qualified[DATA_TABLE_SIZE];
	const char * B_test_case[] = {
		"",
		"G",
		"G of F of A",
		"D of A",
		"G of C of H",
		"G of H",
		"G of E",
	};

	for (int i = 0; i < sizeof(B_test_case) / sizeof(B_test_case[0]); i++) {
#if 1
		int qualified_nr = my_Algorithm_B(dt, B_test_case[i], qualified);
#else
		int qualified_nr = TAOCP_Algorithm_B(dt, B_test_case[i], qualified);
#endif
		if (qualified_nr == 0) {
			______________________________(dt,
						       "Check a qualified reference: ``%s''\";\n"
						       "\t\"result\" [color=\"yellow\""
						       ", shape=\"plaintext\""
						       ", fontcolor=\"cyan\""
						       ", pos=\"-1,-1!\"];\n"
						       "\t\"0\" [color=\"yellow\""
						       ", shape=\"plaintext\""
						       ", fontcolor=\"cyan\""
						       ", pos=\"-1,-2!\"];\n"
						       "\tedge [color=\"cyan\"];\n"
						       "\t\"result\" -> \"0",
						       B_test_case[i]);
		} else {
			char str[DOT_LABEL_STR_LEN];
			char * ps = str;
			printf("%sfound %d:%s\n", RED, qualified_nr, NOCOLOR);
			for (int j = 0; j < qualified_nr; j++) {
				for (DTE * x = qualified[j]; x; x = x->PARENT)
					printf("%s%c%s ", RED, x->NAME->stuff, NOCOLOR);
				printf("\n");
				if (j == 0)
					ps += snprintf(str, DOT_LABEL_STR_LEN, 
						       "Check a qualified reference: ``%s''\";\n"
						       "\t\"result%s\" [color=\"yellow\""
						       ", shape=\"plaintext\""
						       ", fontcolor=\"cyan\""
						       ", pos=\"-1,-1!\"];\n"
						       "\tedge [color=\"cyan\"];\n"
						       "\t\"result%s\" -> \"%X",
						       B_test_case[i],
						       qualified_nr > 1 ? "s" : "",
						       qualified_nr > 1 ? "s" : "",
						       (unsigned int)qualified[j]);
				else
					ps += snprintf(ps, DOT_LABEL_STR_LEN-(ps-str),
						       "\";\n\t\"result%s\" -> \"%X",
						       qualified_nr > 1 ? "s" : "",
						       (unsigned int)qualified[j]);
			}
			______________________________(dt, str);
		}
	}
}

void my_Algorithm_C(DTE * dt, DTE * P0, DTE * Q0)
{
	DTE * P = P0->CHILD;
	DTE * Q = Q0->CHILD;

	while (1) {
		int r = 0;
		int found = 0;
		while (1) {
			if (P == 0 || Q == 0) {
				found = 1;
				break;
			}
			if (P->NAME == Q->NAME) {
				if (P->CHILD == 0 || Q->CHILD == 0) {
					found = 1;
					break;
				} else {
					P = P->CHILD; /* P goes right */
					Q = Q->CHILD; /* Q goes right */
					r = 1;
					break;
				}
			} else {
				if (Q->SIB) /* Q goes down */
					Q = Q->SIB;
				else
					break;
			}
		}

		if (found) {
			printf("%sfound:%s\n", FOUND_COLOR, NOCOLOR);
			if (P == 0 || Q == 0) {
				assert(P == P0->CHILD && Q == Q0->CHILD);
				printf("%s%c -> %c%s\n", FOUND_COLOR,
				       P0->NAME->stuff,
				       Q0->NAME->stuff,
				       NOCOLOR);
				return;
			} else {
				for (DTE * x = P; x; x = x->PARENT)
					printf("%s%c%s ", FOUND_COLOR, x->NAME->stuff, NOCOLOR);
				printf(" -> ");
				for (DTE * x = Q; x; x = x->PARENT)
					printf("%s%c%s ", FOUND_COLOR, x->NAME->stuff, NOCOLOR);
				printf("\n");
			}
		}

		if (r)
			continue;

		if (P->SIB) {	/* P goes down */
			P = P->SIB;
			Q = Q->PARENT->CHILD;
		} else {	/* P goes left */
			while (1) {
				P = P->PARENT;
				Q = Q->PARENT;

				if (P == P0) {
					assert(Q == Q0);
					return;
				} else {
					if (P->SIB) {
						P = P->SIB;
						Q = Q->PARENT->CHILD;
						break;
					}
				}
			}
		}
	}
}

void TAOCP_Algorithm_C(DTE * dt, DTE * P0, DTE * Q0)
{
	/* C1. [Initialize.] */
	DTE * P = P0;
	DTE * Q = Q0;

C2:
	/* C2. [Elementary?] */
	if (P->CHILD == 0 || Q->CHILD == 0) {
		printf("%sfound:%s\n", FOUND_COLOR, NOCOLOR);
		for (DTE * x = P; x; x = x->PARENT)
			printf("%s%c%s ", FOUND_COLOR, x->NAME->stuff, NOCOLOR);
		printf(" -> ");
		for (DTE * x = Q; x; x = x->PARENT)
			printf("%s%c%s ", FOUND_COLOR, x->NAME->stuff, NOCOLOR);
		printf("\n");
		goto C5;
	} else {
		P = P->CHILD;
		Q = Q->CHILD;
	}

C3:
	/* C3. [Match name.] */
	while (1) {
		if (P->NAME == Q->NAME)
			goto C2;
		else if (Q->SIB)
			Q = Q->SIB;
		else
			break;
	}

C4:
	/* C4. [Move on.] */
	if (P->SIB) {
		P = P->SIB;
		Q = Q->PARENT->CHILD;
		goto C3;
	} else {
		P = P->PARENT;
		Q = Q->PARENT;
	}

C5:
	/* C5. [Done?] */
	if (P == P0)
		return;
	else
		goto C4;
}

void test_C(DTE * dt)
{
#if 1
	my_Algorithm_C(dt, &dt[0], &dt[7]);  /* A1 H1 */
	my_Algorithm_C(dt, &dt[2], &dt[3]);  /* C7 D7 */
	my_Algorithm_C(dt, &dt[1], &dt[10]); /* B3 B5 */
#else
	TAOCP_Algorithm_C(dt, &dt[0], &dt[7]);  /* A1 H1 */
	TAOCP_Algorithm_C(dt, &dt[2], &dt[3]);  /* C7 D7 */
	TAOCP_Algorithm_C(dt, &dt[1], &dt[10]); /* B3 B5 */
#endif
}

int main(void)
{
	DTE * dt = Algorithm_A();
	print_data_table(dt, "Build Data Table: done.");

	printf("\n\n===== Algorithm B =====\n\n");
	test_B(dt);

	printf("\n\n===== Algorithm C =====\n\n");
	test_C(dt);

	return 0;
}
