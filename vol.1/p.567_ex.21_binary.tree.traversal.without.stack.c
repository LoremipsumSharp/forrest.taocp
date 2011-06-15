/*
 * Answer to Exercise 2.3.1-21 @ TAOCP::p.567
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
#define OUTPUT_BUF_SIZE		1024
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
const char P_ATTR[] = "fontcolor=\"cyan\", shape=\"none\"";
const char Q_ATTR[] = "fontcolor=\"cyan\", shape=\"none\"";
const char T_ATTR[] = "fontcolor=\"cyan\", shape=\"none\"";

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
 * nodes
 ******************************************************************************/
/* 
 * Test Case 1
 */
#if 1
/*
 * node val[k] has two children: left[k] and right[k]
 */
const char val[]   = "abcdefghi";
const char left[]  = "bde--h---";
const char right[] = "c-f-gi---";
/* 
 * graphviz coordinates of node val[k]: (x[k], y[k])
 */
/*               a   b   c   d   e   f   g   h   i */
const int x[] = {6,  3,  9,  1,  7, 11,  8, 10, 12};
const int y[] = {7,  5,  5,  3,  3,  3,  1,  1,  1};
const int corner[] = {0, 0, 13, 8};
struct node sentinel = {0, 0, 'S', 9, 7};
#endif

/* 
 * Test Case 2
 */
#if 0
/*
 * node val[k] has two children: left[k] and right[k]
 */
const char val[]   = "abc";
const char left[]  = "b--";
const char right[] = "c--";
/* 
 * graphviz coordinates of node val[k]: (x[k], y[k])
 */
/*               a   b   c */
const int x[] = {6,  3,  9};
const int y[] = {7,  5,  5};
const int corner[] = {2, 4, 10, 8};
struct node sentinel = {0, 0, 'S', 9, 7};
#endif

/* 
 * Test Case 3
 */
#if 0
/*
 * node val[k] has two children: left[k] and right[k]
 */
const char val[]   = "ab";
const char left[]  = "b-";
const char right[] = "--";
/* 
 * graphviz coordinates of node val[k]: (x[k], y[k])
 */
/*               a   b */
const int x[] = {6,  3};
const int y[] = {7,  5};
const int corner[] = {2, 4, 10, 8};
struct node sentinel = {0, 0, 'S', 9, 7};
#endif

/******************************************************************************
 * global var
 ******************************************************************************/
int id = 0;
struct node * P;
struct node * Q;
struct node * R;

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
			 "\"TAOCP - EX 2.3.1-21 (%02d).png\"",
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
	double ratio = .4;
	double xshiftP = 0.3;
	double yshiftP = .25;
	double xshiftQ = xshiftP + 0;
	double yshiftQ = -.25;
	double xshiftR = -xshiftQ;
	double yshiftR = yshiftQ;

	va_start(args, label_fmt);
	vsnprintf(label, DOT_LABEL_STR_LEN, label_fmt, args);

	printf("%s\n", label);

	write_dot(id, "label = \"%s\";\n", label);

	write_dot(id, "\"corner0\" [shape=\"none\", label=\"⌞\""
		  ", pos=\"%.5f,%.5f!\", fontcolor=\"white\"];\n", corner[0] * ratio, corner[1] * ratio);
	write_dot(id, "\"corner1\" [shape=\"none\", label=\"⌟\""
		  ", pos=\"%.5f,%.5f!\", fontcolor=\"white\"];\n", corner[2] * ratio, corner[1] * ratio);
	write_dot(id, "\"corner2\" [shape=\"none\", label=\"⌝\""
		  ", pos=\"%.5f,%.5f!\", fontcolor=\"white\"];\n", corner[2] * ratio, corner[3] * ratio);
	write_dot(id, "\"corner3\" [shape=\"none\", label=\"⌜\""
		  ", pos=\"%.5f,%.5f!\", fontcolor=\"white\"];\n", corner[0] * ratio, corner[3] * ratio);

	if (label_fmt[0] == 'W') {
		write_dot(id, "\"%X\" [shape=\"square\", label=\"%c\""
			  ", pos=\"%.5f,%.5f!\"];\n",
			  (unsigned int)(&sentinel), 'S',
			  sentinel.x * ratio, sentinel.y * ratio);
		if (P == &sentinel)
			write_dot(id, "\"%c\" [label=\"%c\", pos=\"%.5f,%.5f!\""
				  "%s];\n", 'P', 'P',
				  (sentinel.x + xshiftP) * ratio,
				  (sentinel.y + yshiftP) * ratio,
				  P_ATTR);
		if (Q == &sentinel)
			write_dot(id, "\"%c\" [label=\"%c\", pos=\"%.5f,%.5f!\""
				  "%s];\n", 'Q', 'Q',
				  (sentinel.x + xshiftQ) * ratio,
				  (sentinel.y + yshiftQ) * ratio,
				  P_ATTR);
		if (R == &sentinel)
			write_dot(id, "\"%c\" [label=\"%c\", pos=\"%.5f,%.5f!\""
				  "%s];\n", 'R', 'R',
				  (sentinel.x + xshiftR) * ratio,
				  (sentinel.y + yshiftR) * ratio,
				  T_ATTR);
	}


	for (p = P0; p->val != 0; p++) {
		write_dot(id, "\"%X\" [label=\"%c\", pos=\"%.5f,%.5f!\"];\n",
			  (unsigned int)p, p->val, p->x * ratio, p->y * ratio);
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
		if (R == p)
			write_dot(id, "\"%c\" [label=\"%c\", pos=\"%.5f,%.5f!\""
				  "%s];\n", 'R', 'R',
				  (p->x + xshiftR) * ratio,
				  (p->y + yshiftR) * ratio,
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

enum method {preorder=0, inorder=1, NO_IDEA=2};
void visit(struct node * p, enum method m)
{
	static char output[OUTPUT_BUF_SIZE];
	static char * s = output;
	char * sub[] = {"₁", "₂"};
	if (m == NO_IDEA) {
		s += snprintf(s, OUTPUT_BUF_SIZE-(s-output), "%s%c%s ",
			      MAGENTA, p->val, NOCOLOR);
	} else {
		assert(m == inorder || m == preorder);
		s += snprintf(s, OUTPUT_BUF_SIZE-(s-output), "%s%c%s%s ",
			      m == preorder ? RED : BLUE, p->val, sub[m], NOCOLOR);
	}
	printf("%s\n", output);
}

/*
 * traversal of binary tree without auxiliary stack
 * @see TAOCP - answer of exercise 2.3.1-21 @ p.567
 *
 * Joseph M. Morris Algorithm
 */
void Joseph_M_Morris(struct node * T)
{
	______________________________(T, "U1. [Initialize.]\\n%02d", id);
	/* U1. [Initialize.] */
	P = T;
	R = 0;

	while (P) { /* U2. [Done?] */
		while (1) {
			______________________________(T, "U3. [Look left.]\\n%02d", id);
			/* U3. [Look left.] */
			Q = P->L;
			if (Q == 0) {
				visit(P, preorder);
				break; /* goto U6; */
			}
			______________________________(T, "U4. [Search for thread.]\\n%02d", id);
			/* U4. [Search for thread.] */
			while (Q != R && Q->R != 0)
				Q = Q->R;
			assert(Q == R || Q->R == 0);
			______________________________(T, "U5. [Insert or remove thread.]\\n%02d", id);
			/* U5. [Insert or remove thread.] */
			if (Q != R) {
				Q->R = P;
			} else {
				Q->R = 0;
				break;
			}
			______________________________(T, "U8. [Preorder visit.]\\n%02d", id);
			/* U8. [Preorder visit.] */
			visit(P, preorder);
			______________________________(T, "U9. [Go to left.]\\n%02d", id);
			/* U9. [Go to left.] */
			P = P->L;
			/* goto U3 */
		}
		______________________________(T, "U6. [Inorder visit.]\\n%02d", id);
		/* U6. [Inorder visit.] */
		visit(P, inorder);
		______________________________(T, "U7. [Go to right or up.]\\n%02d", id);
		/* U7. [Go to right or up.] */
		R = P;
		P = P->R;
		/* goto U2 */
	}
}

/*
 * traversal of binary tree without auxiliary stack
 * the extraction of the inorder part of answer of exercise 2.3.1-21 @ p.567
 */
void inorder_Joseph_M_Morris(struct node * T)
{
	______________________________(T, "U1. [Initialize.]\\n%02d", id);
	/* U1. [Initialize.] */
	P = T;
	R = 0;

	while (P) { /* U2. [Done?] */
		while (P->L) {
			/* U3. [Look left.] */
			______________________________(T, "U3. [Look left.]\\n%02d", id);
			Q = P->L;

			/* U4. [Search for thread.] */
			______________________________(T, "U4. [Search for thread.]\\n%02d", id);
			while (Q != R && Q->R != 0)
				Q = Q->R;
			assert(Q == R || Q->R == 0);

			/* U5. [Insert or remove thread.] */
			______________________________(T, "U5. [Insert or remove thread.]\\n%02d", id);
			if (Q == R) {
				Q->R = 0;
				break;
			} else {
				Q->R = P;
			}

			/* U9. [Go to left.] */
			______________________________(T, "U9. [Go to left.]\\n%02d", id);
			P = P->L;
			/* goto U3 */
		}
		/* U6. [Inorder visit.] */
		______________________________(T, "U6. [Inorder visit.]\\n%02d", id);
		visit(P, inorder);

		/* U7. [Go to right or up.] */
		______________________________(T, "U7. [Go to right or up.]\\n%02d", id);
		R = P;
		P = P->R;
		/* goto U2 */
	}
}

/*
 * traversal of binary tree without auxiliary stack
 * the extraction of the preorder part of answer of exercise 2.3.1-21 @ p.567
 */
void preorder_Joseph_M_Morris(struct node * T)
{
	______________________________(T, "U1. [Initialize.]\\n%02d", id);
	/* U1. [Initialize.] */
	P = T;
	R = 0;

	while (P) { /* U2. [Done?] */
		while (1) {
			______________________________(T, "U3. [Look left.]\\n%02d", id);
			/* U3. [Look left.] */
			Q = P->L;
			if (Q == 0) {
				visit(P, preorder);
				break; /* goto U6,U7 */
			}
			______________________________(T, "U4. [Search for thread.]\\n%02d", id);
			/* U4. [Search for thread.] */
			while (Q != R && Q->R != 0)
				Q = Q->R;
			assert(Q == R || Q->R == 0);
			______________________________(T, "U5. [Insert or remove thread.]\\n%02d", id);
			/* U5. [Insert or remove thread.] */
			if (Q == R) {
				Q->R = 0;
				break;
			} else {
				Q->R = P;
			}
			______________________________(T, "U8. [Preorder visit.]\\n%02d", id);
			/* U8. [Preorder visit.] */
			visit(P, preorder);
			______________________________(T, "U9. [Go to left.]\\n%02d", id);
			/* U9. [Go to left.] */
			P = P->L;
			/* goto U3 */
		}
		______________________________(T, "U7. [Go to right or up.]\\n%02d", id);
		/* U7. [Go to right or up.] */
		R = P;
		P = P->R;
		/* goto U2 */
	}
}

void G_Lindstrom__B_Dwyer(struct node * T)
{
	struct node * S = &sentinel;

	______________________________(T, "W1. [Initialize.]\\n%02d", id);
	/* W1. [Initialize.] */
	P = T;
	Q = S;

	while (1) {
		______________________________(T, "W2. [Bypass null.]\\n%02d", id);
		/* W2. [Bypass null.] */
		if (P == 0) {
			P = Q;
			Q = 0;
		}

		/* W3. [Done?] */
		if (P == S) {
			assert(Q == T);
			break;;
		}

		/* W4. [Visit.] */
		visit(P, NO_IDEA);

		______________________________(T, "W5. [Rotate.]\\n%02d", id);
		/* W5. [Rotate.] */
		/* (R, P->L, P->R, Q, P) <-- (P->L, P->R, Q, P, R) */
		R = P->L;
		P->L = P->R;
		P->R = Q;
		Q = P;
		P = R;
		/* goto W2 */
	}
}

void print_usage(char * bin_name)
{
	const char * usage_color = MAGENTA;
	const char * cmd_color   = CYAN;
	printf("%s"
	       "------\n"
	       "USAGE:\n"
	       "------\n"
	       "To run Joseph M. Morris Algorithm:\n"
	       "        both inorder and preorder traversal:\n"
	       "%s        $ %s -M%s\n"
	       "        inorder only:\n"
	       "%s        $ %s -M --inorder%s\n"
	       "        preorder only:\n"
	       "%s        $ %s -M --preorder%s\n"
	       "To run G. Lindstrom - B. Dwyer Algorithm:\n"
	       "%s        $ %s --GB%s"
	       "\n",
	       usage_color,
	       cmd_color, bin_name, usage_color,
	       cmd_color, bin_name, usage_color,
	       cmd_color, bin_name, usage_color,
	       cmd_color, bin_name, NOCOLOR);
}

int main(int argc, char * argv[])
{
	struct node * head = init_nodes();
	______________________________(head, "start\\n%02d", id);
	if (argc == 2) {
		if (strcmp(argv[1], "-M") == 0)
			Joseph_M_Morris(head);
		else if (strcmp(argv[1], "--GB") == 0)
			G_Lindstrom__B_Dwyer(head);
		else
			print_usage(argv[0]);
	} else if (argc == 3) {
		if (strcmp(argv[2], "--inorder") == 0)
			inorder_Joseph_M_Morris(head);
		else if(strcmp(argv[2], "--preorder") == 0)
			preorder_Joseph_M_Morris(head);
		else
			print_usage(argv[0]);
	} else {
		print_usage(argv[0]);
	}
	______________________________(head, "end\\n%02d", id);
	return 0;
}

