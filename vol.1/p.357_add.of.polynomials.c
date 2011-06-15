/*
 * Algorithm 2.3.3A @ TAOCP::p.357
 *
 * author: Forrest Y. Yu <forrest.yu@gmail.com>, http://forrestyu.net/
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

/*****
 *  0                                                       3+x²+xyz+z³-3xz³
 *  z
 *  :......................................................
 *  0                          1                          3
 *  x                          y                          x
 *  :.........                 :.........                 :.........
 *  0        2                 0        1                 0        1
 *  3        1                 0        x                 1       -3
 *                                      :.........
 *                                      0        1
 *                                      0        1
 *****
 * 
 *  0                                                       xy-x²-xyz-z³+3xz³
 *  z
 *  :...............................................................
 *  0                                  1                           3
 *  y                                  y                           x
 *  :..................                :.........                  :.........
 *  0                 1                0        1                  0        1
 *  x                 x                0        x                 -1        3
 *  :.........        :.........                :.........
 *  0        2        0        1                0        1
 *  0       -1        0        1                0       -1
 *
 *****/


/******************************************************************************
 * configuration
 ******************************************************************************/
#define ERR_COLOR	RED
#define POLY_COLOR	BLUE
#define MM_COLOR	MAGENTA
#define CAUTION_COLOR	MAGENTA
#define INFO_COLOR	WHITE
#define A_COLOR		YELLOW
#define GRAPHVIZ_CMD	"neato"
#define DBG_LEVEL	0
#define LOWEST_DOT_ID	0
#define MM_DBG		0

/******************************************************************************
 * define
 ******************************************************************************/
#define POOL_SIZE		256
#define STR_POOL_SIZE		8
#define NODE_STR_LEN		32
#define TERM_STR_LEN		1024
#define DOT_LABEL_STR_LEN	1024
#define MAX_DOTF_NR		1024
#define CMD_MAX			(FILENAME_MAX + 128)
#define POLYNOMIAL_MAX		256
#define IS_ROOT(x)		(x->e == 0 && x->U == 0 && x->L == x && x->R == x)
#define IS_ONLY_CHILD(x)	(x->L == x                     &&	\
				 x->R == x                     &&	\
				 x == (x->U ? x->U->D : x)     &&	\
				 x->e == 0)
#define IS_LEFTMOST(x)		(x == (x->U ? x->U->D : x) && x->e == 0)
#define IS_CON(x)		(x->D == 0)
#define IS_VAR(x)		(x->D != 0)
#define ______________________________	if (DBG_LEVEL >= 2) {poly2dot
#define ______________________________1	if (DBG_LEVEL >= 1) {poly2dot
#define ______________________________D	{poly2dot
#define __________ ;}

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
const char U_ATTR[] = "arrowhead=\"lopen\", color=\"yellow\"";
const char D_ATTR[] = "arrowhead=\"ropen\", color=\"green\"";
const char L_ATTR[] = "arrowhead=\"rnormal\", color=\"blue\"";
const char R_ATTR[] = "arrowhead=\"lnormal\", color=\"red\"";
const char _Q_ATTR[] = "arrowhead=\"onormal\", color=\"cyan\"";
const char _P_ATTR[] = "arrowhead=\"onormal\", color=\"cyan\"";
const char _R_ATTR[] = "arrowhead=\"onormal\", color=\"gray\"";
const char _S_ATTR[] = "arrowhead=\"onormal\", color=\"grey\"";

/* for set_node_links() */
struct polynode NONSENSE_NODE;
struct polynode * KEEP = &NONSENSE_NODE;

/* for output */
const char * superscript[] = {"⁰", "¹", "²", "³", "⁴", "⁵", "⁶", "⁷", "⁸", "⁹"};
/* const char * subscript[]   = {"₀", "₁", "₂", "₃", "₄", "₅", "₆", "₇", "₈", "₉"}; */

/******************************************************************************
 * enum, struct
 ******************************************************************************/
/*
 * +--------+-------+---------+
 * |   UP   |  EXP  |  RIGHT  |
 * +--------+-------+---------+
 * |  LEFT  |  CV   |  DOWN   |
 * +--------+-------+---------+
 *
 * ROOT:
 * +--------+-------+---------+
 * |   0    |   0   |  SELF   |
 * +--------+-------+---------+
 * |  SELF  |  CV   |  DOWN   |
 * +--------+-------+---------+
 */
struct polynode {
	struct polynode * U;	/* U == 0  <=>  node is root */
	struct polynode * D;	/* D == 0  <=>  node is a constant
				 * D != 0  <=>  node is a variable
				 */
	struct polynode * L;	/* @see free_node::{what defines a free slot} */
	struct polynode * R;	/* @see free_node::{what defines a free slot} */
	int               e;	/* e == 0  <=>  node is the leftmost child,
				 *              maybe the only child
				 * e != 0  <=>  node is not the leftmost child,
				 *              it must have siblings / a sibling
				 */
	int               cv;
};

/******************************************************************************
 * global var
 ******************************************************************************/
int id = 0;
int node_nr = 0;

/******************************************************************************
 * declaration
 ******************************************************************************/
struct polynode * poly_add(struct polynode * Q, const struct polynode * P);

/******************************************************************************
 * test cases
 ******************************************************************************/
const char * poly4test[] = {
	/* "3", "3", */
	/* "3+2", "5", */
	/* "3+x", "3+x", */
	/* "3-5x^2", "3-5x^2", */
	/* "3x + 5xz", "3x+5xz", */
	/* "z^3-3xz^3", "z^3-3xz^3", */
	/* "x^2+xyz", "x^2+xyz", */
	/* "3+x^2+xyz+z^3-3xz^3", "3+x^2+xyz+z^3-3xz^3", */
	/* "3x^2", "3x^2", */
	/* "3xyz", "3xyz", */
	/* "-3", "-3", */
	/* "-3x^5", "-3x^5", */
	/* "-3xyz", "-3xyz", */
	/* "3+2xy", "3+2xy", */
	/* "3x+2xy", "3x+2xy", */
	/* "3x^2+2xy", "3x^2+2xy", */
	/* "3xyz+2xy", "2xy+3xyz", */
	/* "-3+2xy", "-3+2xy", */
	/* "-3x^5+2xy", "-3x^5+2xy", */
	/* "-3xyz+2xy", "2xy-3xyz", */
	/* "3x-3x^2", "3x-3x^2", */
	/* "-3x^2+xyz", "-3x^2+xyz", */
 
	/* "3x-3x^2+xyz", "3x-3x^2+xyz", */
	/* "3x^2+2x+5xz", "2x+3x^2+5xz", */
	/* "3x^2+5yz", "3x^2+5yz", */
	/* "3x^2+3x+5yz", "3x+3x^2+5yz", */
	/* "7+3x+5yz", "7+3x+5yz", */
	/* "7+3xy+5xyz", "7+3xy+5xyz", */
	/* "2z^3-5xz^3", "2z^3-5xz^3", */
	/* "7+3xy+5xyz+2wy+w^2x^3y^5z^7", "7+2wy+3xy+5xyz+w^2x^3y^5z^7", */
	/* "7+3xy+5xyz+2wy+wxyz", "7+2wy+3xy+5xyz+wxyz", */
	/* "-5wx-3yz+5+xy+5xyz+x^3y^5z^7", "5-5wx+xy-3yz+5xyz+x^3y^5z^7", */
	/* "3yz+7wx+2wy", "7wx+2wy+3yz", */
};

const char * polypair4test[] = {
	/* "3", "-3", "0", */
	/* "3+x", "-3", "x", */
	/* "3+x", "-x", "3", */
	/* "-3", "3+x", "x", */
	/* "3x+5xz", "-3x", "5xz", */
	/* "3x+5xz", "-3x-5xz", "0", */
	/* "3x+5xz", "-3x-5xz+2x^2z", "2x^2z", */
	/* "3x+5x^2z", "-3x+7xz-5x^2z", "7xz", */

	/* "3x+5xz", "7-3x-5xz", "7", */
	/* "7+3xy+5xyz+2wy+wxyz", "23xz-2xy", "7+2wy+xy+23xz+5xyz+wxyz", */
	/* "7+3xy+5xyz+2wy+wxyz", "23xz-3xy", "7+2wy+23xz+5xyz+wxyz", */
	/* "7+3x+5yz", "15x+2z^3-5xz^3", "7+18x+5yz+2z^3-5xz^3", */
	/* "2+2xy+5xyz+2wy", "5+xy+x^3y^5z^7", "7+2wy+3xy+5xyz+x^3y^5z^7", */
	/* "7wy+2wz", "5x", "5x+7wy+2wz", */
	/* "5x", "7wy+2wz", "5x+7wy+2wz", */
	/* "7wy+2wz", "5wx", "5wx+7wy+2wz", */
	/* "5wx", "7wy+2wz", "5wx+7wy+2wz", */
	/* "2z+5w^3z+7y^11z+13y^17z^2", "3z+2u^4z+6x^8z+10y^12z^2", "5z+2u^4z+5w^3z+6x^8z+7y^11z+10y^12z^2+13y^17z^2", */
	/* "2z+5w^3z+7y^11z+13y^17z^2", "-2z-5w^3z-7y^11z", "13y^17z^2", */
	/* "5z+2u^4z+5w^3z+6x^8z+7y^11z+10y^12z^2+13y^17z^2", "-5w^3z", "5z+2u^4z+6x^8z+7y^11z+10y^12z^2+13y^17z^2", */
	/* "2+2xy+3yz+7wx+2wy", "-5wx-3yz+5+xy+5xyz+x^3y^5z^7", "7+2wx+2wy+3xy+5xyz+x^3y^5z^7", */

	"3+x^2+xyz+z^3-3xz^3", "xy-x^2-xyz-z^3+3xz^3", "3+xy",

	/* "-x^2+xy-xyz-z^3+3xz^3", "x^2+x^3+2xy-xyz-z^3+2xz^3", "x^3+3xy-2xyz-2z^3+5xz^3", */
	/* "7-x^2+xy-xyz-z^3+3xz^3", "x^2+x^3+2xy-xyz-z^3+2xz^3", "7+x^3+3xy-2xyz-2z^3+5xz^3", */
	/* "-x^2+xy-xyz-z^3+3xz^3", "-xy+x^2y+x^3y", "-x^2+x^2y+x^3y-xyz-z^3+3xz^3", */
	/* "-x^2+xy-xyz-z^3+3xz^3", "x^2+x^3-xy+x^2y+x^3y-xyz-z^3+2xz^3", "x^3+x^2y+x^3y-2xyz-2z^3+5xz^3", */
	/* "x^2+x^3-xy+x^2y+x^3y-xyz-z^3+2xz^3", "-x^2-x^3+xy-x^2y-x^3y", "-xyz-z^3+2xz^3", */
	/* "x^2+x^3-xy+x^2y+x^3y-xyz-z^3+2xz^3", "xyz", "x^2+x^3-xy+x^2y+x^3y-z^3+2xz^3", */
	/* "x^2+x^3-xy+x^2y+x^3y-xyz-z^3+2xz^3", "z^3-2xz^3", "x^2+x^3-xy+x^2y+x^3y-xyz", */
	/* "x^2+x^3-xy+x^2y+x^3y-xyz-z^3+2xz^3", "-x^2-x^3+xy-x^2y-x^3y+xyz+z^3-2xz^3", "0", */
	/* "5-5wx+xy-3yz+5xyz+x^3y^5z^7", "-5+5wx", "xy-3yz+5xyz+x^3y^5z^7", */
	/* "5-5wx+xy-3yz+5xyz+x^3y^5z^7", "-xy", "5-5wx-3yz+5xyz+x^3y^5z^7", */
};


/******************************************************************************
 * functions
 ******************************************************************************/
char * node2str(const struct polynode * p, const char * m)
{
	static char str_pool[STR_POOL_SIZE*NODE_STR_LEN];
	static char * s = str_pool + (STR_POOL_SIZE * NODE_STR_LEN);

	s += NODE_STR_LEN;
	if (s >= str_pool + STR_POOL_SIZE * NODE_STR_LEN)
		s = str_pool;

	int c = '~';

	if (p->U) {
		if (IS_VAR(p->U))
			c = p->U->cv;
		else
			c = '?'; /* parent has been changed */
	}

	char * formats[] = {
		"%d * %c^%d",	/* 0*2+0 */
		"%c * %c^%d",	/* 0*2+1 */
		"%d",		/* 1*2+0 */
		"%c",		/* 1*2+1 */
		"%d",		/* 2*2+0 */
		"%d",		/* 2*2+1 */
		"%d·%c%s",	/* 3*2+0 */
		"%c·%c%s",	/* 3*2+1 */
	};
	int f_idx = (IS_CON(p) ? 0 : 1);

	const char * q;
	int has_cv = 0;
	int has_e  = 0;
	for (q = m; *q; q++) {
		switch (*q) {
		case 'c':
			has_cv = 1;
			break;
		case 'e':
			has_e = 1;
			break;
		default:
			assert(0);
			break;
		}
	}

	if (has_cv && has_e) {
		snprintf(s, NODE_STR_LEN, formats[6+f_idx], p->cv, c,
			 superscript[p->e]);
	} else if (has_e) {
		assert(!has_cv);
		snprintf(s, NODE_STR_LEN, formats[4+f_idx], p->e);
	} else if (has_cv) {
		assert(!has_e);
		snprintf(s, NODE_STR_LEN, formats[2+f_idx], p->cv);
	} else {
		assert(!has_cv && !has_e);
		s[0] = 0;
	}

	return s;
}

struct polynode * alloc_node(int cv)
{
	static struct polynode node_pool[POOL_SIZE];
	static struct polynode * ptop = node_pool;

	if (ptop == node_pool) {
		for (; ptop < node_pool + POOL_SIZE; ptop++) {
			ptop->L = 0;
			ptop->R = 0;
		}
	}

	if (ptop >= node_pool + POOL_SIZE) {
		assert(ptop == node_pool + POOL_SIZE);
		ptop = node_pool + 1;
	}

	assert(ptop->L == 0 && ptop->R == 0); /* @see free_node::{what defines a free slot} */

	/* suppose the new allocated node is a ROOT */
	ptop->U = 0;
	ptop->D = 0; /* an just-allocated node is supposed to be a constant */
	ptop->L = ptop;
	ptop->R = ptop;
	ptop->e = 0;
	ptop->cv= cv;

	node_nr++;
	if (MM_DBG)
		printf("%s[node alloc] %X. node_nr:%d        %s%s\n", MM_COLOR,
		       (unsigned int)ptop, node_nr, node2str(ptop, "ce"), NOCOLOR);

	return ptop++;
}

void free_node(struct polynode * p)
{
	node_nr--;

	if(MM_DBG)
		printf("%s[node freed] %X. node_nr:%d        %s%s\n", MM_COLOR,
		       (unsigned int)p, node_nr, node2str(p, "ce"), NOCOLOR);

	p->U = 0;	p->D = 0;
	p->L = 0;	p->R = 0; /* {what defines a free slot}:
				   * L and R links of a node are never NULL,
				   * so (L == 0 && R == 0) implies this node
				   * is a free slot
				   */
	p->e = 0;
	p->cv= 0;
}

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
			"\tnode [shape=record"
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
			 "\"TAOCP - Algorithm 2.3.3A (%02d).png\"",
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

void node2dot(const struct polynode * p, double x, double y)
{
	/* printf("[node2dot] %s\n", node2str(p, "ce")); */

	write_dot(id, "\"%X\" [label=\"{<u>|<l>}|{<e>%d|<c>%s}|{<r>|<d>}\""
		  ", pos=\"%.5f,%.5f!\"];\n",
		  (int)p, p->e, node2str(p, "c"), x, y);
}

void line2dot(const struct polynode * p1, const struct polynode * p2,
	      char anchor1, char anchor2, const char * attr/* , double x, double y */)
{
	assert(p1);
	if (p2) {
		write_dot(id,
			  "\"%X\":%c -> \"%X\":%c [%s];\n",
			  (int)p1, anchor1, (int)p2, anchor2, attr);
		/* write_dot(id, */
		/* 	  "\"%X\":%c -> \"%X\":%c [%s, pos=\"%.5f,%.5f!\"];\n", */
		/* 	  (int)p1, anchor1, (int)p2, anchor2, attr, x+2, y-2); */
	}
}

void visit_polynode(const struct polynode * p, const char * lines, double x, double y)
{
	const char * q = lines;
	node2dot(p, x, y);
	while (*q) {
		/* printf("%c\n", *q); */
		switch (*q) {
		case 'u':
			line2dot(p, p->U, 'u', 'l', U_ATTR/* , x, y */);
			break;
		case 'd':
			line2dot(p, p->D, 'd', 'r', D_ATTR/* , x, y */);
			break;
		case 'l':
			line2dot(p, p->L, 'l', 'd', L_ATTR/* , x, y */);
			break;
		case 'r':
			line2dot(p, p->R, 'r', 'u', R_ATTR/* , x, y */);
			break;
		default:
			printf("invalid: %s\n", lines);
			assert(0);
			break;
		}
		q++;
	}
}

void preorder_traverse(const struct polynode * P,
		       void (*visit)(const struct polynode *, const char *,
				     double, double),
		       double x, double y,
		       int depth)
{
	double xskip =  7.0;
	const double xskip_min = 1.5;
	const double yskip = -1.5;
	int i;
	for (i = 0; i < depth; i++) {
		xskip /= 2.5;
		xskip = xskip > xskip_min ? xskip : xskip_min;
	}

	if (P == 0)
		return;

	if (P->e == 0 && P->cv == 0)
		visit(P, "udr""l", x, y);
	else if (P->R->e == 0 && P->R->cv == 0)
		visit(P, "udl""r", x, y);
	else
		visit(P, "udlr", x, y);

	struct polynode * q = P->D;
	y += yskip;		/* coordinate changed */
	if (q) {
		assert(q->e == 0);
		do {
			assert(q->R->e == 0 || q->R->e > q->e);
			preorder_traverse(q, visit, x, y, depth+1);
			q = q->R;
			assert(q);
			x += xskip; /* coordinate changed */
		} while (q != P->D);
	}
}

void postorder_traverse(struct polynode * P, void (*visit)(struct polynode *))
{
	assert(P);
	if (P->D) {
		struct polynode * q = P->D;
		do {
			struct polynode * qr = q->R;
			postorder_traverse(q, visit);
			q = qr;
		} while (q != P->D);
	}
	visit(P);
}

void free_poly(struct polynode * P)
{
	postorder_traverse(P, free_node);
}

void set_node_links(struct polynode * p,
		    struct polynode * up,   struct polynode * down,
		    struct polynode * left, struct polynode * right)
{
	if (up != KEEP)
		p->U = up;
	if (down != KEEP)
		p->D = down;
	if (left != KEEP)
		p->L = left;
	if (right != KEEP)
		p->R = right;
}

struct polynode * str2term(const char * s)
{
	const char * p = s;
	int v = 0;
	int n = 0;
	int sign = 1;
	struct polynode * q = alloc_node(0);

	do {
		/* printf("*p: '%c' %d\n", *p, *p); */
		if (*p == ' ')
			continue; /* ignore spaces */

		if (*p == '-') {
			assert(sign == 1);
			sign = -1;
			continue;
		}
		/* coefficient */
		if (*p >= '0' && *p <= '9') {
			n = n * 10 + (*p - '0');
			continue;
		}

		if (*p == '^') {
			assert(*(p-1) >= 'a' && *(p-1) <= 'z');
			assert(*(p+1) >  '0' && *(p+1) <= '9');
			continue; /* do nothing */
		}

		if (q->cv == 0) {
			assert(*p == 0 || (*p >= 'a' && *p <= 'z'));
			q->cv = (n ? n : 1) * sign;
		} else {
			q->e = n ? n : 1;
			struct polynode * subpoly = alloc_node(v);
			struct polynode * znode = alloc_node(0);
			set_node_links(subpoly, 0, znode, subpoly, subpoly);
			set_node_links(znode, subpoly, 0, q, q);
			set_node_links(q, subpoly, KEEP, znode, znode);
			assert(IS_VAR(subpoly));
			assert(IS_CON(znode));
			q = subpoly;
		}

		/* a-z */
		if (*p >= 'a' && *p <= 'z') {
			if ((v >= 'a' && v <= 'z') &&
			    (v >= *p)) {
				printf("%sInvalid term: %s", INFO_COLOR, NOCOLOR);
				const char * u1 = s;
				const char * u2 = p;
				while (*u2 != v)
					u2--;
				for (; *u1; u1++) {
					if ((u1 < u2) ||
					    (u1 > u2 && u1 < p) ||
					    (u1 > p))
						printf("%s%c%s", WHITE,
						       *u1, NOCOLOR);
					else
						printf("%s%c%s", ERR_COLOR,
						       *u1, NOCOLOR);
				}
				printf("%s  (Term a₀^e₀a₁^e₁...aᵢ^eᵢ should satisfy:"
				       " aᵤ < aᵥ for any u < v.)%s\n",
				       INFO_COLOR, NOCOLOR);
				assert(v < *p);
			}
			n = 0;
			v = *p;
		}
	} while (*p++);

	return q;
}

void parse_node(const struct polynode * p, char ** ps)
{
	if (IS_CON(p)) {
		int coeff = p->cv;

		if (coeff == 0)
			return;

		if (**ps != '!') /* not the beginning */
			*ps += sprintf(*ps, "+");

		if (coeff != 1) {
			if ((coeff < 0) && (**ps != '!')) {
				assert(*(*ps-1) == '+');
				(*ps)--;
			}
			if (coeff == -1)
				*ps += sprintf(*ps, "-");
			else
				*ps += sprintf(*ps, "%d", coeff);
		}

		const struct polynode * q = p;
		while (q->U) {
			assert(IS_VAR(q->U));
			if (q->e == 1)
				*ps += sprintf(*ps, "%c", q->U->cv);
			else if (q->e != 0)
				*ps += sprintf(*ps, "%c^%d", q->U->cv, q->e);
			q = q->U;
		}
	}
}

void postorder_traverse_2(const struct polynode * P,
			  void (*visit)(const struct polynode *, char **),
			  char ** ps)
{
	if (!P) {
		strcpy(*ps, "--");
		return;
	}

	if (P->D) {
		struct polynode * q = P->D;
		do {
			struct polynode * qr = q->R;
			postorder_traverse_2(q, visit, ps);
			q = qr;
		} while (q != P->D);
	}
	visit(P, ps);
}

char * polynomial2str(const struct polynode * p)
{
	static char termstr_pool[2][TERM_STR_LEN];
	static int idx = 0;

	idx = 1 - idx;
	char * termstr = termstr_pool[idx];

	memset(termstr, 0, TERM_STR_LEN);
	char * s = termstr;
	*s = '!';		/* mark the beginning of a termstr */
	postorder_traverse_2(p, parse_node, &s);
	if (termstr[0] == '!')
		termstr[0] = '0';
	return termstr;
}

void poly2dot(const struct polynode * P, const char * label_fmt, ...)
{
	va_list args;
	va_start(args, label_fmt);

	char label[DOT_LABEL_STR_LEN];

	vsnprintf(label, DOT_LABEL_STR_LEN, label_fmt, args);

	printf("%sdot %02d : %s%s\n", INFO_COLOR, id, label, NOCOLOR);
	write_dot(id, "label = \"%s\\n%s\";\n", label, polynomial2str(P));
	preorder_traverse(P, visit_polynode, 5.0, 5.0, 0);
	write_dot(id, "");
	id++;

	va_end(args);
}

/******************************************************************************
 * My Algorithm of Addition of polynomials using tree structure shown in Fig.28
 * @ p.356 (written before I read Algorithm 2.3.3A @ p.357                    
 ******************************************************************************/

int cmp_term(const struct polynode * p, const struct polynode * q)
{
	if (IS_VAR(p) && IS_CON(q))
		return 1;
	else if (IS_CON(p) && IS_VAR(q))
		return -1;
	else if (IS_VAR(p) && IS_VAR(q))
		return (p->cv - q->cv);
	else /* (IS_CON(p) && IS_CON(q)) */
		return 0;
}

struct polynode * COPY(const struct polynode * P)
{
	assert(P);
	struct polynode * head = alloc_node(P->cv);
	assert(head->D == 0);/* an just-allocated node is supposed to be a constant */
	head->e = P->e;
	
	if (P->D != 0) {
		const struct polynode * p = P->D;
		struct polynode * leftmost = COPY(p);
		struct polynode * q0 = leftmost;
		struct polynode * q;
		for (p = p->R; p != P->D; p = p->R, q0 = q) {
			q = COPY(p);
			q0->R = q;
			q->L = q0;
			q->U = head;
		}
		q->R = leftmost;
		leftmost->L = q;
		head->D = leftmost;
		leftmost->U = head;
	}
	return head;
}

int delete_on_demand(struct polynode * Q)
{
	assert(Q);

	if (IS_ROOT(Q))	/* ROOT is not erasable */
		return 0;

	if (IS_ONLY_CHILD(Q))
		return 0;

	assert(!IS_ONLY_CHILD(Q));
	assert(IS_CON(Q));
	assert(Q->cv == 0);

	if (IS_LEFTMOST(Q))
		return 0;

	Q->L->R = Q->R;
	Q->R->L = Q->L;

	free_node(Q);

	return 1;
}

void merge_upwards(struct polynode * Q)
{
	assert(IS_ONLY_CHILD(Q));

	Q->U->cv = Q->cv;
	Q->U->D  = Q->D;
	if (Q->D) {
		struct polynode * p = Q->D;
		assert(IS_LEFTMOST(p));
		do {
			assert(p->U == Q);
			p->U = Q->U;
			p = p->R;
			assert(p);
		} while (p != Q->D);
		assert(IS_LEFTMOST(p));
	}

	/* Q->U must have at least one sibling */
	assert(IS_ROOT(Q->U) || !IS_ONLY_CHILD(Q->U));

	if (IS_CON(Q->U))
		if (Q->U->cv == 0)
			delete_on_demand(Q->U);

	free_node(Q);
}

void insert_copy_to_left(struct polynode * Q, const struct polynode * P)
{
	struct polynode * P_cp = COPY(P);
	set_node_links(P_cp, Q->U, KEEP, Q->L, Q);
	Q->L->R = P_cp;
	Q->L = P_cp;
}

struct polynode * combine(struct polynode * Q, const struct polynode * P)
{
	assert(Q && P);
	______________________________(Q, "[combine] Q");__________;
	______________________________(P, "[combine] P");__________;
	assert(cmp_term(P, Q) == 0 && P->e == Q->e);

	struct polynode * t = Q->D;
	const struct polynode * s = P->D;

	if (s == 0 && t == 0) {
		assert(IS_CON(P) && IS_CON(Q));
		Q->cv += P->cv;
		______________________________(Q->U, "before delete_on_demand")__________;
		if (Q->cv == 0)
			return (delete_on_demand(Q) ? 0 : Q);
		else
			return Q;
	} else {
		do {
			if (t->e == s->e) {
				assert(!IS_ONLY_CHILD(t));
				struct polynode * tr = t->R;
				(void)poly_add(t, s);
				t = tr;
			} else if (t->e < s->e) {
				t = t->R;
				assert(t);
				if (IS_LEFTMOST(t)) {
					do {
						assert(Q == t->U);
						insert_copy_to_left(t, s);
						s = s->R;
					} while (!IS_LEFTMOST(s));
					break;
				} else {
					continue;
				}
			} else { /* t->e > s->e */
				assert(Q == t->U);
				insert_copy_to_left(t, s);
			}
			s = s->R;
		} while (s != P->D);
	}
	if (IS_ONLY_CHILD(t))
		merge_upwards(t);
	______________________________(Q, "[after combine] Q");__________;

	return Q;
}

/* replace q with p */
void replace(struct polynode * q, struct polynode * p)
{
	/*    ::     \
	 * :: q  ::   |
	 *    ::      |       ::
	 *            |=>  :: p  :: ,  q
	 *    ||      |       ||       ::
	 *  = p  =    |
	 *    ||     /
	 */
	if(IS_ONLY_CHILD(q))
		set_node_links(p, q->U, KEEP, p, p);
	else
		set_node_links(p, q->U, KEEP, q->L, q->R);

	q->L->R = p;
	q->R->L = p;
	if (q->U)
		if (q->U->D == q)
			q->U->D = p;

	set_node_links(q, 0, KEEP, q, q);
}

/*****
 *  0                           2wz      \  
 *  z                                     | 
 *  :.........                            |
 *  :       |1|                           |
 *          |w|                           |  0                           
 *           :.........                   |  z                          
 *           0        1                   |  :.................................... 
 *           0        2                   |  :        1                          2
 *+                                       +=          x                          7
 *  0                            5x³z+7z² |           :..................
 *  z                                     |           0                 3
 *  :...........................          |           w                 5 
 *  :       |1|                2          |           :.........
 *          |x|                7          |           0        1
 *           :.........                   |           0        2
 *           0        3                   |
 *           0        5                  /
 *****/
struct polynode * poly_add(struct polynode * Q, const struct polynode * P)
{
	______________________________(Q, "[before poly_add] Q")__________;
	______________________________(P, "[before poly_add] P")__________;

	/* this is the only restriction: */
	assert(Q->e == P->e);

	int cmp_result = cmp_term(Q, P);
	struct polynode * sum = Q;

	if (cmp_result == 0) {
		sum = combine(Q, P);			/* combine */
	} else if (cmp_result < 0) {
		struct polynode * p = COPY(P);
		replace(Q, p); /* this line is necessary when Q is not ROOT */
		if (IS_ROOT(Q))
			printf("%sQ is changed.%s\n", INFO_COLOR, NOCOLOR);
		sum = poly_add(p, Q);
		free_poly(Q);
	} else { /* cmp_result > 0 */
		struct polynode * q = Q;
		while (cmp_term(q, P) > 0) {
			q = q->D;
			assert(q->e == 0);
		}

		assert(cmp_term(q, P) <= 0);
		struct polynode * p = COPY(P);
		p->e = 0;	/* ready to hang p under Q */
		replace(q, p);
		p = poly_add(p, q);
		free_poly(q);
		sum = Q;	/* should not change Q */
	}

	if (sum == 0)
		printf("%sQ and P neutralized each other.%s\n", INFO_COLOR, NOCOLOR);

	______________________________(sum, "[after poly_add] sum")__________;

	return sum;
}

struct polynode * str2polynomial(const char * s)
{
	const char * r = s;
	char l[POLYNOMIAL_MAX];
	char * u = l;
	do {
		*u++ = *r++;
	} while (*r && *r != '+' && *r != '-');
	*u = 0;
	if (*r == '+')
		r++;

	printf("\t%s -> %s, %s\n", s, l, r);

	if (*r == 0) {
		return str2term(l);
	} else {
		/* printf("[before add] l:%s, r:%s\n", l, r); */
		struct polynode * q = str2term(l);
		struct polynode * q1 = COPY(q);
		struct polynode * p = str2polynomial(r);
		q = poly_add(q, p);
		______________________________(q1, "[before add] q: %s",
						polynomial2str(q1));__________;
		______________________________(p, "[before add] p: %s",
						polynomial2str(p));__________;
		______________________________(q, "[ after add] q: %s",
						polynomial2str(q));__________;
		free_poly(q1);
		free_poly(p);
		return q;
	}
}
/******************************************************************************/
/* End of My Algorithm of Addition of polynomials                             */
/******************************************************************************/
void test_my_algorithm(void)
{
	int i;
	struct polynode * p;
	if (0) {
		const char * s[] = {"-3", "-3x^2", "-xyz"};
		char label[] = "term 0";

		for (i = 0; i < sizeof(s) / sizeof(s[0]); i++) {
			p = str2term(s[i]);
			______________________________(p, label);__________;
			label[strlen(label)-1]++;
		}
	}

	for (i = 0; i < sizeof(poly4test) / sizeof(poly4test[0]); i += 2) {
		const char * poly_str0;
		const char * poly_str1 = poly4test[i + 1];
		int k = 0;
		for (; k < 2; k++) {
			if (k == 0)
				poly_str0 = poly4test[i];
			else
				poly_str0 = poly4test[i + 1];

			printf("%s input: %s%s\n", POLY_COLOR, poly_str0, NOCOLOR);
			p = str2polynomial(poly_str0);
			char * o_str = polynomial2str(p);
			printf("%soutput: %s%s\n", POLY_COLOR, o_str, NOCOLOR);
			poly2dot(p, "%s -> %s", poly_str0, o_str);
			printf("%sMUST: %s == %s%s\n", CAUTION_COLOR,
			       o_str, poly_str1, NOCOLOR);
			assert(strcmp(o_str, poly_str1) == 0);
			free_poly(p);
			assert(node_nr == 0);
			printf("----------------------------------------"
			       "----------------------------------------\n");
		}
	}

	for (i = 0; i < sizeof(polypair4test) / sizeof(polypair4test[0]); i += 3) {
		const char * s1 = polypair4test[i];
		const char * s2 = polypair4test[i+1];
		const char * s3 = polypair4test[i+2];

		struct polynode * p1 = str2polynomial(s1);
		struct polynode * p2 = str2polynomial(s2);
		poly2dot(p1, "p1: %s", s1);
		poly2dot(p2, "p2: %s", s2);

		p1 = poly_add(p1, p2);

		char * ssum = polynomial2str(p1);
		poly2dot(p1, "p1+p2: %s (REF: %s)", ssum, s3);
		assert(strcmp(ssum, s3) == 0);

		free_poly(p1);
		free_poly(p2);

		assert(node_nr == 0);
		printf("----------------------------------------"
		       "----------------------------------------\n");
	}
}

/******************************************************************************/
/* Knuth's Algorithm 2.3.3A @ p.357 below                                     */
/******************************************************************************/

struct polynode * _Q_ = 0;
struct polynode * _P_ = 0;
struct polynode * _R_ = 0;
struct polynode * _S_ = 0;
void A1(void);
void A2(void);
void A3(void);
void A4(void);
void A5(void);
void A6(void);
void A7(void);
void A8(void);
void A9(void);
void A10(void);
void A11(void);

int tab_nr = 0;
char * summary[] = {"[START]",
		    "[Test type of polynomial.]",
		    "[Downward insertion.]",
		    "[Match found.]",
		    "[Advance to left.]",
		    "[Insert to right.]",
		    "[Return upward.]",
		    "[Move Q up to right level.]",
		    "[Delete zero term.]",
		    "[Delete constant polynomial.]",
		    "[Zero detected?]",
		    "[Terminate.]"};
int last_step = 0;

#if 1
#define A_BEGIN(x) do							\
	{								\
		int i;							\
		printf("%s", A_COLOR);				\
		for (i = 0; i < tab_nr * 2; i++) printf(". ");		\
		printf("(A%d\n", x);					\
		printf("%s\n", summary[x]);				\
		printf("%s", NOCOLOR);					\
		tab_nr++;						\
		PQ2dot("From: A%d. %s        Next: A%d. %s",		\
		       last_step, summary[last_step],			\
		       x, summary[x]);					\
		last_step = x;						\
	} while (0);
#define A_END(x) do							\
	{								\
		int i;							\
		tab_nr--;						\
		printf("%s", A_COLOR);					\
		for (i = 0; i < tab_nr * 2; i++) printf(". ");		\
		printf(" A%d)\n", x);					\
		printf("%s", NOCOLOR);					\
		if (x == 11) {						\
			PQ2dot("A%d. %s", x, summary[x]);		\
			last_step = 0;					\
		}							\
	} while (0);
#else
#define A_BEGIN(x)
#define A_END(x)
#endif

void PQ2dot(const char * label_fmt, ...)
{
	va_list args;
	va_start(args, label_fmt);

	char label[DOT_LABEL_STR_LEN];
	vsnprintf(label, DOT_LABEL_STR_LEN, label_fmt, args);
	printf("%sdot %02d : %s%s\n", INFO_COLOR, id, label, NOCOLOR);

	const struct polynode * p = _P_;
	const struct polynode * q = _Q_;
	while (p->U) p = p->U;
	while (q->U) q = q->U;
	double px = 5.0;
	double py = 5.0;
	double qx = 5.0;
	double qy = 12.5;

	if (label[0] == 0)
		write_dot(id, "label = \"\\n\\nQ:%s\\nP:%s\";\n",
			  polynomial2str(q), polynomial2str(p));
	else
		write_dot(id, "label = \"\\n%s\\n\\n\";\n", label);

	preorder_traverse(q, visit_polynode, qx, qy, 0);
	preorder_traverse(p, visit_polynode, px, py, 0);

	write_dot(id, "\"0\" [shape=circle, pos=\"%.5f,%.5f!\"];\n", qx+4.9, qy+1);
	write_dot(id, "\"Qptr\" [label=\"Q\", pos=\"%.5f,%.5f!\"];\n", qx+2.2, qy);
	write_dot(id, "\"Qptr\" -> \"%X\" [%s];\n", (unsigned int)_Q_, _Q_ATTR);
	write_dot(id, "\"Pptr\" [label=\"P\", pos=\"%.5f,%.5f!\"];\n", px+2.2, py);
	write_dot(id, "\"Pptr\" -> \"%X\" [%s];\n", (unsigned int)_P_, _P_ATTR);
	write_dot(id, "\"Rptr\" [label=\"R\", pos=\"%.5f,%.5f!\"];\n", qx+4, qy);
	write_dot(id, "\"Rptr\" -> \"%X\" [%s];\n", (unsigned int)_R_, _R_ATTR);
	write_dot(id, "\"Sptr\" [label=\"S\", pos=\"%.5f,%.5f!\"];\n", qx+5.8, qy);
	write_dot(id, "\"Sptr\" -> \"%X\" [%s];\n", (unsigned int)_S_, _S_ATTR);

	write_dot(id, "");
	id++;

	va_end(args);
}

void A0(void)
{
	PQ2dot("");
	A1();
}

/* A1. [Test type of polynomial.] If DOWN(P) = ^ (that is, if P points to a
 * constant), then set Q <- DOWN(Q) zero or more times until DOWN(Q) = ^
 * and go to A3. If DOWN(P) != ^, then if DOWN(Q) = ^ or if CV(Q) < CV(P),
 * go to A2. Otherwise if CV(Q) = CV(P), set P <- DOWN(P), Q <- DOWN(Q)
 * and repeat this step; if CV(Q) > CV(P), set Q <- DOWN(Q) and repeat this
 * step. (Step A1 either finds two matching terms of the polynomials or
 * else determines that an insertion of a new variable must be made into the
 * current part of polynomial(Q).)
 */
void A1(void)
{
	A_BEGIN(1);

	if (IS_CON(_P_)) {
		while (_Q_->D != 0)
			_Q_ = _Q_->D;
		assert(IS_CON(_Q_));
		A3();		/* exit ----------> A3 */
	} else {
		assert(IS_VAR(_P_));
		if (IS_CON(_Q_) || _Q_->cv < _P_->cv) {
			A2();	/* exit ----------> A2 */
		} else if (_Q_->cv == _P_->cv) {
			_P_ = _P_->D;
			_Q_ = _Q_->D;
			A1();	/* exit ----------> A1 */
		} else {
			assert(_Q_->cv > _P_->cv);
			_Q_ = _Q_->D;
			A1();	/* exit ----------> A1 */
		}
	}

	A_END(1);
}
/* A2. [Downward insertion.] Set R <= AVAIL, S <- DOWN(Q). If S != ^, set
 * UP(S) <- R, S <- RIGHT(S) and if EXP(S) != 0, repeat this operation
 * until ultimately EXP(S) = 0. Set UP(R) <- Q, DOWN(R) <- DOWN(Q),
 * LEFT(R) <- R, RIGHT(R) <- R, CV(R) <- CV(Q) and EXP(R) <- 0. Finally,
 * set CV(Q) <- CV(P) and DOWN(Q) <- R, and return to A1. (We have inserted
 * a ``dummy'' zero polynomial just below NODE(Q), to obtain a match with
 * a corresponding polynomial found within P's tree. The link manipulations
 * done in this step are straightforward and may be derived easily using
 * ``before-and-after'' diagrams, as explained in Section 2.2.3.)
 */
void A2(void)
{
	A_BEGIN(2);

	assert(_P_->D != 0);
	assert(cmp_term(_Q_, _P_) < 0);

	_R_ = alloc_node(_Q_->cv);
	_S_ = _Q_->D;

	if (_S_ != 0) {
		/* From
		 *    :
		 * -- Q --
		 *    :    :       :
		 * -- S -- S' ...  S" --
		 *    :    :       :
		 *
		 * To
		 *    :
		 * -- Q --
		 *    :
		 *    R
		 *    :    :       :
		 * -- S -- S' ...  S" --
		 *    :    :       :
		 *
		 * in which
		 *         *R = *Q
		 *         *Q = *P
		 */
		do {
			_S_->U = _R_;
			_S_ = _S_->R;
		} while (_S_->e != 0);
	}
	set_node_links(_R_, _Q_, _Q_->D, _R_, _R_);
	_R_->cv = _Q_->cv;
	_R_->e = 0;
	_Q_->cv = _P_->cv;
	_Q_->D  = _R_;

	A1();			/* exit ----------> A1 */

	A_END(2);
}

/* A3. [Match found.] (At this point, P and Q point to corresponding terms of
 * the given polynomials, so addition is ready to proceed.) Set CV(Q) <- CV(Q) + CV(P).
 * If this sum is zero and if EXP(Q) != 0, go to step A8. If EXP(Q) = 0, go to A7.
 */
void A3(void)
{
	A_BEGIN(3);

	assert(IS_CON(_P_) && IS_CON(_Q_));
	_Q_->cv += _P_->cv;

	if (_Q_->cv == 0 && _Q_->e != 0) { /* not leftmost (cv==0) */
		A8();					/* exit ----------> A8 */
	} else if (_Q_->e == 0) { /* leftmost */
		A7();					/* exit ----------> A7 */
	} else {		/* not leftmost (cv!=0) */
		assert(_Q_->cv != 0 && _Q_->e != 0);
		A4();					/* exit ----------> A4 */
	}

	A_END(3);
}

/* A4. [Advance to left.] (After successfully adding a term, we look for the next
 * term to add.) Set P <- LEFT(P). If EXP(P) = 0, go to A6. Otherwise
 * set Q <- LEFT(Q) one or more times until EXP(Q) <= EXP(P). If then
 * EXP(Q) = EXP(P), return to step A1.
 */
void A4(void)
{
	A_BEGIN(4);

	_P_ = _P_->L;
	if (_P_->e == 0) {
		A6();		/* exit ----------> A6 */
	} else {
		do {
			_Q_ = _Q_->L;
		} while (_Q_->e > _P_->e);

		if (_Q_->e == _P_->e) {
			A1();	/* exit ----------> A1 */
		} else {
			A5();	/* exit ----------> A5 */
		}
	}

	A_END(4);
}

/* A5. [Insert to right.] Set R <= AVAIL. Set UP(R) <- UP(Q), DOWN(R) <- ^,
 * CV(R) <- 0, LEFT(R) <- Q, RIGHT(R) <- RIGHT(Q), LEFT(RIGHT(R)) <- R,
 * RIGHT(Q) <- R, EXP(R) <- EXP(P), and Q <- R. Return to step A1. (We
 * needed to insert a new term in the current row, just to the right of NODE(Q),
 * in order to match a corresponding exponent in polynomial(P). As in
 * step A2, a ``before-and-after'' diagram makes the operations clear.)
 */
void A5(void)
{
	A_BEGIN(5);

	_R_ = alloc_node(0);

	set_node_links(_R_, _Q_->U, 0, _Q_, _Q_->R);
	_R_->R->L = _R_;
	_Q_->R = _R_;
	_R_->e = _P_->e;
	_Q_ = _R_;
	A1();			/* exit ----------> A1 */

	A_END(5);
}

/* A6. [Return upward.] (A row of polynomial(P) has now been completely traversed.)
 * Set P <- UP(P).
 */
void A6(void)
{
	A_BEGIN(6);

	_P_ = _P_->U;
	A7();			/* exit ----------> A7 */

	A_END(6);
}

/* A7. [Move Q up to right level.] If UP(P) = ^, go to A11; otherwise set
 * Q <- UP(Q) zero or more times until CV(UP(Q)) = CV(UP(P)). Return
 * to step A4.
 */
void A7(void)
{
	A_BEGIN(7);

	if (_P_->U == 0) {
		A11();		/* exit ----------> A11 */
	} else {
		while (_Q_->U->cv != _P_->U->cv)
			_Q_ = _Q_->U;

		A4();		/* exit ----------> A4 */
	}

	A_END(7);
}

/* A8. [Delete zero term.] Set R <- Q, Q <- RIGHT(R), S <- LEFT(R), LEFT(Q) <- S,
 * RIGHT(S) <- Q, and AVAIL <= R. (Cancellation occurred, so a row element
 * of polynomial(Q) is deleted.) If now EXP(LEFT(P)) = 0 and Q = S, go
 * to A9; otherwise return to A4.
 */
void A8(void)
{
	A_BEGIN(8);

	assert((_Q_->cv == 0) && (_Q_->e != 0));

	_R_ = _Q_;
	_Q_ = _R_->R;
	_S_ = _R_->L;
	_Q_->L = _S_;
	_S_->R = _Q_;
	free_node(_R_); _R_ = 0;

	if (_P_->L->e == 0 && _Q_ == _S_) {
		A9();		/* exit ----------> A9 */
	} else {
		A4();		/* exit ----------> A4 */
	}

	A_END(8);
}

/* A9. [Delete constant polynomial.] (Cancellation has caused a polynomial to
 * reduce to a constant, so a row of polynomial(Q) is deleted.) Set R <- Q,
 * Q <- UP(Q), DOWN(Q) <- DOWN(R), CV(Q) <- CV(R), and AVAIL <= R. Set
 * S <- DOWN(Q); if S != ^, set UP(S) <- Q, S <- RIGHT(S), and if EXP(S) != 0,
 * repeat this operation until ultimately EXP(S) = 0.
 */
void A9(void)
{
	A_BEGIN(9);

	_R_ = _Q_;
	_Q_ = _Q_->U;
	_Q_->D = _R_->D;
	_Q_->cv = _R_->cv;
	free_node(_R_); _R_ = 0;
	_S_ = _Q_->D;
	if (_S_ != 0) {
		do {
			_S_->U = _Q_;
			_S_ = _S_->R;
		} while (_S_->e != 0);
	}

	A10();			/* exit ----------> A10 */

	A_END(9);
}

/* A10. [Zero detected?] If DOWN(Q) = ^, CV(Q) = 0, and EXP(Q) != 0, set P <- UP(P)
 * and go to A8; otherwise go to A6.
 */
void A10(void)
{
	A_BEGIN(10);

	if (_Q_->D == 0 && _Q_->cv == 0 && _Q_->e != 0) {
		_P_ = _P_->U;
		A8();		/* exit ----------> A8 */
	} else {
		A6();		/* exit ----------> A6 */
	}

	A_END(10);
}

/* A11. [Terminate.] Set Q <- UP(Q) zero or more times until UP(Q) = ^ (thus
 * bringing Q to the root of the tree).
 */
void A11(void)
{
	A_BEGIN(11);

	while (_Q_->U)
		_Q_ = _Q_->U;

	/* while (_P_->U) */
	/* 	_P_ = _P_->U; */

	A_END(11);
}


/******************************************************************************/
/* End of Knuth's Algorithm 2.3.3A */
/******************************************************************************/
void test_knuth_algorithm(void)
{
	int i;

	for (i = 0; i < sizeof(polypair4test) / sizeof(polypair4test[0]); i += 3) {
		const char * s1 = polypair4test[i];
		const char * s2 = polypair4test[i+1];
		const char * s3 = polypair4test[i+2];

		_Q_ = str2polynomial(s1);
		_P_ = str2polynomial(s2);
		/* poly2dot(_Q_, "Q: %s", s1); */
		/* poly2dot(_P_, "P: %s", s2); */

		printf("----------------------------------------\n");
		A0();
		printf("----------------------------------------\n");

		char * ssum = polynomial2str(_Q_);
		printf("ssum: %s\n", ssum);
		poly2dot(_Q_, "p1+p2: %s (Expected: %s)", ssum, s3);
		assert(strcmp(ssum, s3) == 0);

		free_poly(_Q_);
		free_poly(_P_);

		assert(node_nr == 0);
		printf("----------------------------------------"
		       "----------------------------------------\n");
	}
}


int main(void)
{
	if (0)
		test_my_algorithm();
	else
		test_knuth_algorithm();

	return 0;
}
