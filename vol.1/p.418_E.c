/*
 * Algorithm 2.3.5E @ TAOCP::p.418
 *
 * author: Forrest Y. Yu <forrest.yu@gmail.com>, http://forrestyu.net/
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

/******************************************************************************
 * configuration
 ******************************************************************************/
#define INFO_COLOR	WHITE
#define GRAPHVIZ_CMD	"neato"
#define LOWEST_DOT_ID	0

/******************************************************************************
 * define
 ******************************************************************************/
#define POOL_SIZE		256
#define MAX_DOTF_NR		1024
#define DOT_LABEL_STR_LEN	1024
#define CMD_MAX			(FILENAME_MAX + 128)
#define ______________________________	print_nodes

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
const char L_ATTR[] = "arrowhead=\"rnormal\", color=\"blue\"";
const char R_ATTR[] = "arrowhead=\"lnormal\", color=\"red\"";
const char P_ATTR[] = "fontcolor=\"gray\", shape=\"none\"";
const char Q_ATTR[] = "fontcolor=\"gray\", shape=\"none\"";
const char T_ATTR[] = "fontcolor=\"gray\", shape=\"none\"";
const char ATOM_ATTR[] = ", style=\"filled\", fillcolor=\"gray\"";
const char MARK_ATTR[] = ", penwidth=\"2\", color=\"green3\"";

/******************************************************************************
 * nodes
 ******************************************************************************/
/*
 * Test Case 1
 */
#if 1
const char val[]   = "abcde";
const char left[]  = "b-be-";
const char right[] = "c-ddc";
/*         a  b  c  d  e */
const int x[] = {1, 0, 3, 5, 4};
const int y[] = {6, 3, 4, 2, 0};
const int corner[] = {-1, -1, 7, 7};
#endif

/*
 * Test Case 2
 */
#if 0
const char val[]   = "abcdefghi";
const char left[]  = "bde--h---";
const char right[] = "c-f-gi---";
/*               a   b   c   d   e   f   g   h   i */
const int x[] = {9,  6, 12,  3, 10, 15, 11, 14, 16};
const int y[] = {7,  5,  5,  3,  3,  3,  1,  1,  1};
const int corner[] = {2, 0, 17, 8};
#endif

/*
 * Test Case 3
 */
#if 0
const char val[]   = "abcdefghi";
const char left[]  = "bde--h---";
const char right[] = "cef-gi---";
/*               a   b   c   d   e   f   g   h   i */
const int x[] = {9,  6, 12,  3, 10, 15, 11, 14, 16};
const int y[] = {7,  5,  5,  3,  3,  3,  1,  1,  1};
const int corner[] = {2, 0, 17, 8};
#endif

/******************************************************************************
 * global var
 ******************************************************************************/
int id = 0;
struct node * P;
struct node * Q;
struct node * T;

/******************************************************************************
 * struct
 ******************************************************************************/
struct node {
	int mark;
	int atom;
	struct node * L;
	struct node * R;
	int val;

	/* extra fields for graphviz */
	int x;
	int y;
};

/******************************************************************************
 * functions
 ******************************************************************************/
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
			", splines=\"compound\""
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
		snprintf(cmd, (size_t)CMD_MAX, "%s -Tpng \"%s\" -o "
			 "\"TAOCP - Algorithm 2.3.5E (%02d).png\"",
			 GRAPHVIZ_CMD, dotfilename[id], id);
		printf("%s$ %s%s%s\n", WHITE, WHITE, cmd, NOCOLOR);
		/* printf("%s$ %s%s%s\n", WHITE, GREEN, cmd, NOCOLOR); */
		assert(system(cmd) == 0);
	} else {
		fprintf(dotf[id], "\t");
		vfprintf(dotf[id], fmt, args);
	}

	va_end(args);
}

struct node * init_nodes(void)
{
	static struct node node_pool[POOL_SIZE];

	int node_nr = strlen(val);

	assert(node_nr == strlen(left));
	assert(node_nr == strlen(right));
	assert(node_nr == sizeof(x)/sizeof(x[0]));
	assert(node_nr == sizeof(y)/sizeof(y[0]));
	assert(node_nr < POOL_SIZE);

	int i;
	for (i = 0; i < POOL_SIZE; i++) {
		if (i >= node_nr) {
			node_pool[i].val = 0;
			continue;
		}

		assert(i == 0 || val[i] - val[i-1] == 1);
		node_pool[i].mark = 0;
		node_pool[i].atom = (left[i] == '-' && right[i] == '-') ? 1 : 0;
		node_pool[i].L = (left[i]  == '-' ? 0 : &node_pool[left[i]-val[0]]);
		node_pool[i].R = (right[i] == '-' ? 0 : &node_pool[right[i]-val[0]]);
		node_pool[i].val = val[i];
		node_pool[i].x = x[i];
		node_pool[i].y = y[i];
	}

	return node_pool;
}


void print_nodes(struct node * P0, const char * label_fmt, ...)
{
	va_list args;
	char label[DOT_LABEL_STR_LEN];
	struct node * p;
	double ratio = .7;
	double xshiftP = .55;
	double yshiftP = -.2;
	double xshiftQ = xshiftP + 0;
	double yshiftQ = yshiftP - .3;
	double xshiftT = .5;
	double yshiftT = .5;

	va_start(args, label_fmt);
	vsnprintf(label, DOT_LABEL_STR_LEN, label_fmt, args);

	printf("%s\n", label);

	write_dot(id, "label = \"%s\";\n", label);

	write_dot(id, "\"corner0\" [shape=\"none\", label=\"⌞\""
		  ", pos=\"%.5f,%.5f!\"];\n", corner[0] * ratio, corner[1] * ratio);
	write_dot(id, "\"corner1\" [shape=\"none\", label=\"⌟\""
		  ", pos=\"%.5f,%.5f!\"];\n", corner[2] * ratio, corner[1] * ratio);
	write_dot(id, "\"corner2\" [shape=\"none\", label=\"⌝\""
		  ", pos=\"%.5f,%.5f!\"];\n", corner[2] * ratio, corner[3] * ratio);
	write_dot(id, "\"corner3\" [shape=\"none\", label=\"⌜\""
		  ", pos=\"%.5f,%.5f!\"];\n", corner[0] * ratio, corner[3] * ratio);

	for (p = P0; p->val != 0; p++) {
		write_dot(id, "\"%X\" [label=\"%c\", pos=\"%.5f,%.5f!\"%s%s];\n",
			  (unsigned int)p, p->val, p->x * ratio, p->y * ratio,
			  p->atom ? ATOM_ATTR : "",
			  p->mark ? MARK_ATTR : "");
		if (P == p)
			write_dot(id, "\"%c\" [label=\"%c\", pos=\"%.5f,%.5f!\""
				  "%s];\n", 'P', 'P',
				  (p->x + xshiftP) * ratio,
				  (p->y + yshiftP) * ratio,
				  P_ATTR);
		if (Q == p)
			write_dot(id, "\"%c\" [label=\"%c\", pos=\"%.5f,%.5f!\""
				  "%s];\n", 'Q', 'Q',
				  (p->x + xshiftQ) * ratio,
				  (p->y + yshiftQ) * ratio,
				  P_ATTR);
		if (T == p)
			write_dot(id, "\"%c\" [label=\"%c\", pos=\"%.5f,%.5f!\""
				  "%s];\n", 'T', 'T',
				  (p->x + xshiftT) * ratio,
				  (p->y + yshiftT) * ratio,
				  T_ATTR);
		if (p->L)
			write_dot(id, "\"%X\" -> \"%X\" [%s];\n",
				  (unsigned int)p, (unsigned int)p->L, L_ATTR);
		if (p->R)
			write_dot(id, "\"%X\" -> \"%X\" [%s];\n",
				  (unsigned int)p, (unsigned int)p->R, R_ATTR);
	}
	write_dot(id, "");
	id++;

	va_end(args);
}

void mark(struct node * p)
{
	/* assert(p->mark == 0); */
	p->mark = 1;
}

void push_L(void)
{
	assert(P->atom == 0);
	P->atom = 1;
	Q = P->L;
	P->L = T;
	T = P;
}

void push_R(void)
{
	assert(P->atom == 0);
	Q = P->R;
	P->R = T;
	T = P;
}

struct node * pop_L(void)
{
	Q = T;
	Q->atom = 0;
	T = Q->L;
	Q->L = P;
	return Q;
}

struct node * pop_R(void)
{
	Q = T;
	assert(Q->atom == 0);
	T = Q->R;
	Q->R = P;
	return Q;
}

struct node * pop(void)
{
	if (!T)
		return 0;
	else
		return T->atom ? pop_L() : pop_R();
}

/* 
 * How did I write this function: I
 *     1. read and understood Algorithm 2.3.5E @ p.418~p.419, then
 *     2. closed the book, then
 *     3. wrote this function
 * so:
 *     1. this is not my original work, but
 *     2. it is not the implementation of Algorithm 2.3.5E either
 */
void E_parody(struct node * P0)
{
	P = P0;

	int i = 0;
	while (1) {
		assert(++i < 50); /* avoid forever loop */
		______________________________(P0, "%02d A", id);

		while (P) {
			if (P->mark)
				break;
			if (P->atom) {
				mark(P);
				break;
			}
			push_L(); P = Q;
			______________________________(P0, "%02d A1", id);
		}
		______________________________(P0, "%02d B", id);
		P = pop();
		if (!P)
			break;
		______________________________(P0, "%02d C", id);
		if (P->mark)
			continue;
		else
			mark(P);
		if (P->atom)
			continue;
		push_R(); P = Q;
		______________________________(P0, "%02d D", id);
	}
}

/*
 * Algorithm 2.3.5E
 */
void E(struct node * P0)
{
	______________________________(P0, "E1. [Initialize.]\\n%02d", id);
	/* E1. [Initialize.] */
	T = 0;
	P = P0;

	while (1) {
		while (1) {
			______________________________(P0, "E2. [Mark.]\\n%02d", id);
			/* E2. [Mark.] */
			P->mark = 1;
			printf("%s%c%s\n", RED, P->val, NOCOLOR);

			/* ______________________________(P0, "E3. [Atom?]\\n%02d", id); */
			/* E3. [Atom?] */
			if (P->atom == 1) {
				goto E6;
			} else {
				/* ______________________________(P0, "E4. [Down ALINK.]\\n%02d", id); */
				/* E4. [Down ALINK.] */
				Q = P->L;
				if (Q != 0) {
					if (Q->mark == 0) {
						P->atom = 1;
						P->L = T;
						T = P;
						P = Q;
						continue; /* goto E2; */
					}
				}
				break; /* goto E5; */
			}
		}
	E5:
		______________________________(P0, "E5. [Down BLINK.]\\n%02d", id);
		/* E5. [Down BLINK.] */
		Q = P->R;
		if (Q != 0) {
			if (Q->mark == 0) {
				P->R = T;
				T = P;
				P = Q;
				continue; /* goto E2; */
			}
		}

	E6:
		while (1) {
			______________________________(P0, "E6. [Up.]\\n%02d", id);
			/* E6. [Up.] */
			if (T == 0) {
				return; /* terminate */
			} else {
				Q = T;
				if (Q->atom == 1) {
					Q->atom = 0;
					T = Q->L;
					Q->L = P;
					P = Q;
					goto E5;
				} else {
					assert(Q->atom == 0);
					T = Q->R;
					Q->R = P;
					P = Q;
					continue; /* goto E6; */
				}
			}
		}
	}
}

int main(void)
{
	struct node * head = init_nodes();
	______________________________(head, "start\\n%02d", id);
	E(head);
	______________________________(head, "end\\n%02d", id);
	return 0;
}

