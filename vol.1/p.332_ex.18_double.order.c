/*
 * Exercise 2.3.1-18 @ TAOCP::p.332
 *
 * author: Forrest Y. Yu <forrest.yu@gmail.com>, http://forrestyu.net/
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

/******************************************************************************
 * struct
 ******************************************************************************/
struct node {
	struct node * L;
	struct node * R;
	int val;

	/* extra fields for graphviz */
	int x;
	int y;
};

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
#define STACK_SIZE		256
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

/******************************************************************************
 * nodes
 ******************************************************************************/
struct node node_pool[POOL_SIZE];

const char val[]   = "ABCDEFGHI";
const char left[]  = "BDE--H---";
const char right[] = "C-F-GI---";
/*               a   b   c   d   e   f   g   h   i */
const int x[] = {6,  3,  9,  1,  7, 11,  8, 10, 12};
const int y[] = {7,  5,  5,  3,  3,  3,  1,  1,  1};
const int corner[] = {0, 0, 13, 8};


/******************************************************************************
 * global var
 ******************************************************************************/
int id = 0;

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
			 "\"TAOCP - EX 2.3.1-18 (%02d).png\"",
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
		write_dot(id, "\"%X\" [label=\"%c\", pos=\"%.5f,%.5f!\"];\n",
			  (unsigned int)p, p->val, p->x * ratio, p->y * ratio);
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

void visit(struct node * p, int d)
{
	char * sub[] = {"₁", "₂"};
	assert(d == 1 || d == 2);
	printf("%s%c%s%s ", d == 1 ? RED : BLUE, p->val, sub[d-1], NOCOLOR);
}

void double_order_recursive(struct node * P0)
{
	static int depth = 0;
	struct node * p = P0;
	if (!p)
		return;
	if (depth == 0)
		printf("%straversal of 2.3.1-(1) @ p.318 in double order "
		       "(recursive method):%s\n", GREEN, NOCOLOR);
	depth++;

	visit(p, 1);
	double_order_recursive(p->L);
	visit(p, 2);
	double_order_recursive(p->R);

	if (--depth == 0)
		printf("\n");
}

/* a simple stack implementation */
enum stack_action {STACK_PUSH, STACK_POP};
struct node * node_stack(enum stack_action a, struct node * p)
{
	static struct node * STACK[STACK_SIZE];
	static int top = 0;
	if (a == STACK_PUSH) {
		assert(top < STACK_SIZE);
		STACK[top++] = p;
	}
	else {
		assert(a == STACK_POP);
		if (top)
			return STACK[--top];
		else
			return 0;
	}

	return 0;
}
#define push(x) node_stack(STACK_PUSH,x);
#define pop()   node_stack(STACK_POP, 0);

/*
 * double order traversal
 */
void double_order(struct node * P0)
{
	struct node * p = P0;
	printf("%straversal of 2.3.1-(1) @ p.318 in double order:%s\n",
	       GREEN, NOCOLOR);
	while (1) {
		while (p) {
			visit(p, 1);
			push(p);
			p = p->L;
		}
		p = pop();
		if (!p)
			break;
		visit(p, 2);
		p = p->R;
	}
	printf("\n");
}

/*              Δ
 * compute (P,d)
 */
void compute_P_d_delta(struct node * P, int d)
{
	struct node * Q;
	int e;

	if (d == 1) {
		if (P->L) {
			Q = P->L;
			e = 1;
		} else {
			Q = P;
			e = 2;
		}
	} else {
		assert(d == 2);
		if (P->R) {
			Q = P->R;
			e = 1;
		} else {
			Q = 0;	/* Q should be P$ */
			e = 2;
		}
	}
	if (Q)
		printf("(%c,%d)^Δ is (%c, %d)\n",
		       P->val, d, Q->val, e);
	else
		printf("(%c,%d)^Δ is (%c$, %d)\n",
		       P->val, d, P->val, e);
}

int main(void)
{
	struct node * head = init_nodes();

	______________________________(head, "2.3.1-(1) @ p.318");

	double_order_recursive(head);
	double_order(head);

	printf("%scompute (P,d)^Δ:%s\n", GREEN, NOCOLOR);
	int i, j;
	for (i = 0; i < strlen(val); i++)
		for (j = 1; j <= 2; j++)
			compute_P_d_delta(node_pool+i, j);

	return 0;
}



