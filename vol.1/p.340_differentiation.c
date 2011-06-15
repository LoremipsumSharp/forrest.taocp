/*
 * Algorithm 2.3.2D @ TAOCP::p.340
 * Algorithm 2.3.1S @ TAOCP::p.323
 *
 * author: Forrest Y. Yu <forrest.yu@gmail.com>, http://forrestyu.net/
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>

/******************************************************************************
 * configuration
 ******************************************************************************/
/*
 * uncomment this line to use a POOL instead of malloc() to allocate memory:
 */
/* #define SAFE_MEMORY_MANAGEMENT */


/******************************************************************************
 * define
 ******************************************************************************/
#ifdef SAFE_MEMORY_MANAGEMENT
#define POOL_SIZE		256
#endif

#define MAX_DOTF_NR		1024
#define NODE_STR_LEN		32
#define CMD_MAX			(FILENAME_MAX + 128)

/* #define test_COPY(xxx)	dot_node_children(id*2+50, NONSENSE_NODE,	\ */
/* 					  headx+xskip(0), heady);	\ */
/* 	preorder_traverse(id*2+50, xxx, dot_node_children, headx, heady, 0); \ */
/* 	write_dot(id*2+50, "");						\ */
/* 	dot_node_children(id*2+51, NONSENSE_NODE, headx+xskip(0), heady); \ */
/* 	preorder_traverse(id*2+51, COPY(xxx), dot_node_children, headx, heady, 0); \ */
/* 	write_dot(id*2+51, "");						\ */
/* 	id++;								\ */
/* 	printf("%s--------------------------------------------------%s\n", \ */
/* 	       RED, NOCOLOR); */

#define	f_eq(f1, f2)	(fabs(f1 - f2) < 0.001)


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
const char R_THREAD[] = "arrowhead=\"ropen\", color=\"magenta\", style=\"dashed\"";

double headx = 5.0;
double heady = 5.0;


/******************************************************************************
 * enum, struct, typedef
 ******************************************************************************/
/* @see diff_func DIFF[] */
enum node_type {
	CON=0, VAR=1,		   /* constant and variable */
	Ln=2,			   /* ln() */
	Neg=3,
	Add=4, Sub=5, Mul=6, Div=7,/* + - * / */
	Power=8			   /* ^ */
};

/* the core data structure */
struct tnode {
	enum node_type	type;
	int		info;
	struct tnode *	L;
	int		RTAG;
	struct tnode *	R;
};

typedef void (*diff_func) (struct tnode  *, struct tnode  *, struct tnode *,
			   struct tnode **, struct tnode **);


/******************************************************************************
 * global var
 ******************************************************************************/
/*
 * 1. NONSENSE_NODE is a special node. It is used when it's impossible to know
 *    who the right child of a node is. It is temporary and will not appear in
 *    the final tree, because eventually each node will have a certain right
 *    child
 * 2. NONSENSE_NODE is initialized in main()
 * 3. NONSENSE_NODE should be freed before this program terminates
 * 4. NONSENSE_NODE is visible in the dot graphs, but one can just ignore it
 *    when looking at the picture(s)
 * 5. @see TREE() and COPY() for more details
 */
struct tnode * NONSENSE_NODE = 0;

/*
 * node_nr is used to make sure there is no memory leak
 */
int node_nr = 0;


/******************************************************************************
 * routines
 ******************************************************************************/
/* xskip & yskip */
#define yskip	-1
double xskip(double y)
{
	double d = fabs((y - heady) / fabs(yskip));

	if (f_eq(d, 2))
		return 10.0;
	else if (f_eq(d, 3))
		return 5.0;
	else if (f_eq(d, 4))
		return 3.0;
	else
		return 1.0;
}

/* visualize a node */
char * node2str(char * s, struct tnode * p)
{
	assert(p != 0 && s != 0);

	if (p->type == CON)
		(void)snprintf(s, NODE_STR_LEN, "%d", p->info);
	else if (p->type == VAR)
		(void)snprintf(s, NODE_STR_LEN, "%c", p->info);
	else if (p->type == Ln)
		(void)snprintf(s, NODE_STR_LEN, "Ln");
	else if (p->type == Neg)
		(void)snprintf(s, NODE_STR_LEN, "_");
	else if (p->type == Add)
		(void)snprintf(s, NODE_STR_LEN, "+");
	else if (p->type == Sub)
		(void)snprintf(s, NODE_STR_LEN, "-");
	else if (p->type == Mul)
		(void)snprintf(s, NODE_STR_LEN, "*");
	else if (p->type == Div)
		(void)snprintf(s, NODE_STR_LEN, "/");
	else if (p->type == Power)
		(void)snprintf(s, NODE_STR_LEN, "^");
	else
		assert(0);

	return s;
}

struct tnode * alloc_node(enum node_type type, int info)
{
	struct tnode * new_node = 0;
	char s[NODE_STR_LEN];

#ifdef SAFE_MEMORY_MANAGEMENT
	static struct tnode node_pool[POOL_SIZE];
	static struct tnode * p = node_pool;
	assert(p < node_pool + POOL_SIZE);
	if (type != CON && type != VAR && info != 0) {
		printf("%sWARNING:"
		       " info ignored (an operator carries no info).%s\n",
		       YELLOW, NOCOLOR);
		info = 0;
	}
	new_node = p++;
#else
	new_node = (struct tnode *)malloc(sizeof(struct tnode));
#endif
	assert(new_node != 0);

	new_node->type = type;
	new_node->info = info;
	new_node->L    = 0;
	new_node->RTAG = 0;
	new_node->R    = 0;

	node_nr++;
	printf("%snode allocated: %X. node_nr:%d        %s%s\n", BLUE,
	       (unsigned int)new_node, node_nr, node2str(s, new_node), NOCOLOR);

	return new_node;
}

void free_node(struct tnode * p)
{
	char s[NODE_STR_LEN];
	node_nr--;
	printf("%snode     freed: %X. node_nr:%d        %s%s\n",
	       BLUE, (unsigned int)p, node_nr, node2str(s, p), NOCOLOR);
#ifndef SAFE_MEMORY_MANAGEMENT
	free(p);
#endif
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
		fprintf(stderr, "dot file: %s\n", dotfilename[id]);

		dotf[id] = fopen(dotfilename[id], "w");
		assert(dotf[id] != 0);

		fprintf(dotf[id], "digraph example {\n");

		fprintf(dotf[id],
			"\tnode [shape=circle"
			", fontname=Courier New"
			", penwidth=0.5"
			", height=0.3"
			"];\n");
	}

	assert(dotf[id] != 0);

	/* if the dot file becomes big, something must be wrong */
	if (ftell(dotf[id]) > 8*1024) {
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
		snprintf(cmd, (size_t)CMD_MAX, "neato -Tpng %s -o \"TAOCP - Algorithm 2.3.2D (%02d).png\"",
			 dotfilename[id], id);
		printf("\n%s$ %s%s%s\n", WHITE, GREEN, cmd, NOCOLOR);
		assert(system(cmd) == 0);
	} else {
		fprintf(dotf[id], "\t");
		vfprintf(dotf[id], fmt, args);
	}

	va_end(args);
}

/* write a node and its children into a dot file */
void dot_node_children(int id, struct tnode * p, double x, double y)
{
	char s[NODE_STR_LEN];

	assert(p != 0);
	(void)node2str(s, p);

	write_dot(id, "\"%X\" [label=\"%s\", pos=\"%.5f,%.5f!\"];\n",
		  (int)p, s, x, y);
	
	if (p->L) {
		write_dot(id, "\"%X\" -> \"%X\" [%s];\n",
			  (int)p, (int)p->L, L_ATTR);
	}
	assert(p->R != 0);

	if (p->RTAG) {
		write_dot(id, "\"%X\" -> \"%X\" [%s];\n",
			  (int)p, (int)p->R, R_THREAD);
	} else {
		write_dot(id, "\"%X\" -> \"%X\" [%s];\n",
			  (int)p, (int)p->R, R_ATTR);
	}
}

void inorder_traverse(int id, struct tnode * T,
		      void (*visit)(int, struct tnode *, double, double),
		      double x, double y, int depth)
{
	if (T->L)
		inorder_traverse(id, T->L, visit, x, y+yskip, depth+1);
	visit(id, T, x, y);
	if (!T->RTAG)
		inorder_traverse(id, T->R, visit, x+xskip(y), y, depth+1);
}

void preorder_traverse(int id, struct tnode * T,
		       void (*visit)(int, struct tnode *, double, double),
		       double x, double y, int depth)
{
	visit(id, T, x, y);
	if (T->L)
		inorder_traverse(id, T->L, visit, x, y+yskip, depth);
	if (!T->RTAG)
		inorder_traverse(id, T->R, visit, x+xskip(y), y, depth);
}

void postorder_traverse(struct tnode * T,
			void (*visit)(struct tnode *))
{
	if (T->L)
		postorder_traverse(T->L, visit);
	if (!T->RTAG)
		postorder_traverse(T->R, visit);
	visit(T);
}

void print_right_threaded_bintree(int id, struct tnode * T)
{
	assert(T && T->L);
	dot_node_children(id, T, headx, heady);
	inorder_traverse(id, T->L, dot_node_children, headx, heady+yskip, 0);
	/* dot_node_children(id, NONSENSE_NODE, headx+xskip(0), heady); */
	write_dot(id, "");
}

void free_tree(struct tnode * T)
{
	assert(T && T->L);
	postorder_traverse(T->L, free_node);
	free_node(T);
}

/* Algorithm 2.3.1S @ p.323: compute P$ */
struct tnode * compute_Pdollar(struct tnode * p)
{
	struct tnode * q = p->R;
	/* char s[NODE_STR_LEN]; */
	/* printf("%s%s* : ", YELLOW, node2str(s, p)); */
	if (p->RTAG == 0)
		while (q->L)
			q = q->L;
	/* printf("%s%s\n", node2str(s, q), NOCOLOR); */
	return q;
}

struct tnode * compute_Pstar(struct tnode * p)
{
	struct tnode * q;

	assert(p != 0);
	if (p->L)	/* p has left child */
		return p->L;

	/* p has no left child */
	q = p;
	while (q->RTAG)	/* go upwards until q has right child */
		q = q->R;
	q = q->R;
	return q;
}

struct tnode * TREE(enum node_type t, int x, struct tnode * U, struct tnode * V)
{
	struct tnode * w;

	/* manipulate special cases: */
	int i, j;
	int ok = 0;
	/* case 1: both U and V are constants */
	if (U && V) {
		if (U->type == CON && V->type == CON) {
			switch (t) {
			case Add:
				U->info += V->info;
				ok = 1;
				break;
			case Sub:
				U->info -= V->info;
				ok = 1;
				break;
			case Mul:
				U->info *= V->info;
				ok = 1;
				break;
			case Div:
				assert(V->info != 0);
				if (U->info % V->info == 0) {
					U->info /= V->info;
					ok = 1;
				}
				break;
			case Power:
				i = U->info;
				j = V->info;
				while (--j)
					i *= U->info;
				ok = 1;
				break;
			default:
				break;
			}
		}
	}
	if (ok) {
		free_node(V);
		return U;
	}
	/* case 2: either U or V is a special constant: 1 */
	if (t == Mul) {
		assert(U && V);
		assert(!(U->type == CON && U->info == 0));
		assert(!(V->type == CON && V->info == 0));
		if (V->type == CON && V->info == 1) {
			free_node(V);
			printf("%sU*1=U%s\n", MAGENTA, NOCOLOR);
			return U; /* U*1=U */
		}
		if (U->type == CON && U->info == 1) {
			free_node(U);
			printf("%s1*V=V%s\n", MAGENTA, NOCOLOR);
			return V; /* 1*V=V */
		}
	} else if (t == Div) {
		assert(U && V);
		assert(!(U->type == CON && U->info == 0));
		assert(!(V->type == CON && V->info == 0));
		if (V->type == CON && V->info == 1) {
			free_node(V);
			printf("%sU/1=U%s\n", MAGENTA, NOCOLOR);
			return U; /* U/1=U */
		}
	} else if (t == Power) {
		assert(U && V);
		assert(!(U->type == CON && U->info == 0));
		assert(!(V->type == CON && V->info == 0));
		if (V->type == CON && V->info == 1) {
			free_node(V);
			printf("%sU^1=U%s\n", MAGENTA, NOCOLOR);
			return U; /* U^1=U */
		}
		if (U->type == CON && U->info == 1) {
			free_node(V);
			printf("%s1^V=1%s\n", MAGENTA, NOCOLOR);
			return U; /* 1^V=1 */
		}
	} else if (t == Ln) {
		assert(U && !V);
		assert(!(U->type == CON && U->info == 0));
		if (U->type == CON && U->info == 1) {
			U->info = 0;
			printf("%sLn(1)=0%s\n", MAGENTA, NOCOLOR);
			return U; /* Ln(1)=0 */
		}
	} /* end of special cases */

	w = alloc_node(t, x);
	w->L    = U;
	w->RTAG = 1;
	w->R    = NONSENSE_NODE;
	if (U) {
		if (V) {
			U->RTAG = 0;
			U->R    = V;

			V->RTAG = 1;
			V->R    = w;
		} else {
			U->RTAG = 1;
			U->R    = w;
		}
	} else {
		assert(!V);
	}
	return w;
}

/*
 * makes a copy of the tree pointed to by U and returns (has as its value)
 * a pointer to the tree thereby created
 */
struct tnode * COPY(struct tnode * U)
{
	/* This routine is tricky. Read (and remember) the NOTEs below:
	 *
	 * NOTE1: Both P$ and P* are used in this routine.
	 *        P$:
	 *                when P->RTAG == 1, P->R points to P$
	 *        P*:
	 *                when traversing the tree in this routine,
	 *                the code always goes to P* when P is done.
	 * NOTE2: This routine does not copy the whole tree leading by U.
	 *        It copies U (step 1) and its left subtree (step 2), leaving
	 *        the right subtree ignored, because the U->R is U's sibling,
	 *        not its child. We want U and its child only.
	 */

	struct tnode * p = U;
	struct tnode * V = alloc_node(CON, 0);
	struct tnode * q = V;

	assert(U != 0);

	/* step 1: copy U */
	*q = *p;
	q->L    = 0;
	q->RTAG = 1;
	q->R    = NONSENSE_NODE;

	/* step 2: copy U's left subtree */
	while (1) {
		if (p->L) {	/* left subtree is not empty */
			struct tnode * new_node = alloc_node(CON, 0);
			new_node->L    = 0;
			new_node->RTAG = 1;
			new_node->R    = q; /* Q$ */
			q->L = new_node;
		}

		/*
		 * move on to p* and q*
		 */
		p = compute_Pstar(p);
		q = compute_Pstar(q);

		if (q == NONSENSE_NODE)
			break;

		if (p->RTAG == 0) { /* right subtree is not empty */
			struct tnode * new_node = alloc_node(CON, 0);
			new_node->L    = 0;
			new_node->RTAG = 1;
			new_node->R    = q->R; /* new_node$ */
			q->RTAG = 0;
			q->R    = new_node; /* Q$ */
		}

		q->type = p->type;
		q->info = p->info;
	}

	return V;
}

void diff_con(struct tnode * P, struct tnode * P1, struct tnode * P2,
	      struct tnode **pQ1, struct tnode **pQ)
{
	printf("CON\n");
	assert(P->type == CON);
	*pQ = TREE(CON, 0, 0, 0);
}

void diff_var(struct tnode * P, struct tnode * P1, struct tnode * P2,
	      struct tnode **pQ1, struct tnode **pQ)
{
	printf("VAR\n");
	assert(P->type == VAR);
	if (P->info == 'x')
		*pQ = TREE(CON, 1, 0, 0);
	else
		*pQ = TREE(CON, 0, 0, 0);
}

void diff_ln(struct tnode * P, struct tnode * P1, struct tnode * P2,
	     struct tnode **pQ1, struct tnode **pQ)
{
	printf("Ln\n");
	assert(P->type == Ln);
	if (!((*pQ)->type == CON && (*pQ)->info == 0)) /* Q is not zero */
		*pQ = TREE(Div, 0, *pQ, COPY(P1));
}

void diff_neg(struct tnode * P, struct tnode * P1, struct tnode * P2,
	      struct tnode **pQ1, struct tnode **pQ)
{
	printf("Neg\n");
	assert(P->type == Neg);
	if (!((*pQ)->type == CON && (*pQ)->info == 0)) /* Q is not zero */
		*pQ = TREE(Neg, 0, *pQ, 0);
}

void diff_add(struct tnode * P, struct tnode * P1, struct tnode * P2,
	      struct tnode **pQ1, struct tnode **pQ)
{
	printf("Add\n");
	/* assert(P->type == Add); */
	if ((*pQ1)->type == CON && (*pQ)->type == CON) { /* constants */
		(*pQ)->info += (*pQ1)->info;
		free_node(*pQ1);
		return;
	}

	if ((*pQ1)->type == CON && (*pQ1)->info == 0) { /* Q1 == 0 */
		free_node(*pQ1);
	} else if ((*pQ)->type == CON && (*pQ)->info == 0) { /* Q == 0 */
		free_node(*pQ);
		*pQ = *pQ1;
	} else {
		*pQ = TREE(Add, 0, *pQ1, *pQ);
	}
}

void diff_sub(struct tnode * P, struct tnode * P1, struct tnode * P2,
	      struct tnode **pQ1, struct tnode **pQ)
{
	printf("Sub\n");
	/* assert(P->type == Sub); */
	if ((*pQ1)->type == CON && (*pQ)->type == CON) { /* constants */
		(*pQ)->info = (*pQ1)->info - (*pQ)->info;
		free_node(*pQ1);
		return;
	}

	if ((*pQ)->type == CON && (*pQ)->info == 0) { /* Q == 0 */
		free_node(*pQ);
		*pQ = *pQ1;
	} else if ((*pQ1)->type == CON && (*pQ1)->info == 0) { /* Q1 == 0 */
		free_node(*pQ1);
		*pQ = TREE(Neg, 0, *pQ, 0);
	} else {
		*pQ = TREE(Sub, 0, *pQ1, *pQ);
	}
}

/* MULT(U,V) is a function that constructs a tree for UxV but also
 * makes a test to see if U or V is equal to 1
 */
struct tnode * MULT(struct tnode *U, struct tnode *V)
{
	if (U->info == 1 && U->type == CON) {
		free_node(U);
		return V;
	}
	if (V->info == 1 && V->type == CON) {
		free_node(V);
		return U;
	} else {
		return TREE(Mul, 0, U, V);
	}
}

void diff_mul(struct tnode * P, struct tnode * P1, struct tnode * P2,
	      struct tnode **pQ1, struct tnode **pQ)
{
	printf("Mul\n");
	assert(P->type == Mul);
	if (!((*pQ1)->type == CON && (*pQ1)->info == 0)) /* Q1 != 0 */
		*pQ1 = MULT(*pQ1, COPY(P2));
	if (!((*pQ)->type == CON && (*pQ)->info == 0)) /* Q != 0 */
		*pQ = MULT(COPY(P1), *pQ);

	if ((*pQ1)->type == CON && (*pQ1)->info == 1) { /* Q1 == 1 */
		free_node(*pQ1);
	} else if ((*pQ)->type == CON && (*pQ)->info == 1) { /* Q == 1 */
		free_node(*pQ);
		*pQ = *pQ1;
	} else {
		diff_add(P, P1, P2, pQ1, pQ);
	}
}

void diff_div(struct tnode * P, struct tnode * P1, struct tnode * P2,
	      struct tnode **pQ1, struct tnode **pQ)
{
	printf("Div\n");
	assert(P->type == Div);
	if (!((*pQ1)->type == CON && (*pQ1)->info == 0)) /* Q1 != 0 */
		*pQ1 = TREE(Div, 0, *pQ1, COPY(P2));
	if (!((*pQ)->type == CON && (*pQ)->info == 0)) /* Q != 0 */
		*pQ = TREE(Div, 0,
			   MULT(COPY(P1), *pQ),
			   TREE(Power, 0, COPY(P2), TREE(CON, 2, 0, 0)));
	diff_sub(P, P1, P2, pQ1, pQ);
}

void diff_power(struct tnode * P, struct tnode * P1, struct tnode * P2,
		struct tnode **pQ1, struct tnode **pQ)
{
	printf("Power\n");
	assert(P->type == Power);
	if (!((*pQ1)->type == CON && (*pQ1)->info == 0)) /* Q1 != 0 */
		*pQ1 = MULT(*pQ1, /* Q1 * */
			    MULT(COPY(P2), /* P2 * */
				 TREE(Power, 0,
				      COPY(P1), /* P1 ^ */
				      TREE(Sub, 0,     /* P2 - 1 */
					   COPY(P2),
					   TREE(CON, 1, 0, 0)
					      ))));

	if (!((*pQ)->type == CON && (*pQ)->info == 0)) /* Q != 0 */
		*pQ = MULT(MULT(TREE(Ln, 0, COPY(P1), 0), /* Ln(P1) * */
				*pQ),			             /* Q */
			   TREE(Power, 0, /* P1^P2 */
				COPY(P1),
				COPY(P2)));

	diff_add(P, P1, P2, pQ1, pQ);
}

void differentiate(struct tnode * Y, struct tnode * DY)
{
	/* Suppose we have
	 *             Y
	 *             :
	 *             V
	 *             P <.
	 *             |   `.
	 *             V     `.
	 *             P1 --> P2
	 *             |      |
	 *             V      V
	 *             ...    ...
	 * It's good to know that we have had P1, P1', P2, P2' when we reach P.
	 * Therefore we can calculate P' via 2.3.2-(11)~(19) @ p.338~p.339:
	 *     x'      = 1                                           (11)
	 *     a'      = 0                                           (12)
	 *     (ln(u))'= u'/u                                        (13)
	 *     (-u)'   = -u'                                         (14)
	 *     (u+v)'  = u'+v'                                       (15)
	 *     (u-v)'  = u'-v'                                       (16)
	 *     (uv)'   = u'v+uv'                                     (17)
	 *     (u/v)'  = (u'v-uv')/(v^2) = u'/v-(uv')/(v^2)          (18)
	 *     (u^v)'  = u'v(u^(v-1))+ln(u)v'(u^v)                   (19)
	 */
	struct tnode * P  = Y;
	struct tnode * P1 = 0;
	struct tnode * P2 = 0;
	struct tnode * Q  = 0;
	struct tnode * Q1 = 0;

	/* @see enum node_type */
	diff_func DIFF[] = {diff_con, diff_var,
			    diff_ln,
			    diff_neg,
			    diff_add, diff_sub, diff_mul, diff_div,
			    diff_power};

	assert(Y && Y->L);
	assert(DY->L == DY && DY->R == DY && DY->RTAG == 0);

	/* D1 */
	while (P->L)
		P = P->L;	/* goto the leftmost node */

	/* static int id = 30; */
	do {
		/* D2 */
		P1 = P->L;
		if (P1)
			Q1 = P1->R; /* this is the ``future use'' */
		/* The routines DIFF[0], DIFF[1], etc., will form the derivative
		 * of the tree with root P, and will set pointer variable Q to
		 * the address of the root of the derivative. The variables P1
		 * and Q1 are set up first, in order to simplify the specification
		 * of the DIFF routines.
		 */
		/* if P is a  unary operator (from Possibility α in last loop):
		 *         P
		 *         |
		 *         V
		 *         P1
		 *
		 *         Q=P1'
		 *
		 * therefore,
		 * DIFF[Ln] = P1'/P1 = Q/P1
		 *
		 * if P is a binary operator (from Possibility γ in last loop):
		 *         P
		 *         |
		 *         V
		 *         P1     P2
		 *          \
		 *           `-->Q1 (=P1')
		 *
		 *                Q=P2'
		 *
		 * therefore,
		 * DIFF[P->type]
		 *               = Q1 + Q                                    if ``+''
		 *               = Q1 - Q                                    if ``-''
		 *               = Q1*P2 + Q*P1                              if ``*''
		 *               = Q1/P2 - (P1*Q)/(P2^2)                     if ``/''
		 *               = Q1*(P2*(P1^(P2-1))) + (Ln(P1)*Q)*(P1^P2)  if ``^''
		 *
		 */
		DIFF[P->type](P, P1, P2, &Q1, &Q);
		/* preorder_traverse(++id, Q, dot_node_children); write_dot(id, ""); */

		/* D3 */
		if (P->type == Add || P->type == Sub || P->type == Mul ||
		    P->type == Div || P->type == Power) { /* binary operators */
			P1->R = P2;
		}

		/* D4 */
		/* Possibility α:
		 *         P$             P
		 *         |              |
		 *         V         ==>  V
		 *         P              P2
		 *
		 *         Q=P'           Q=P2'
		 *
		 * Possibility β:
		 *         Y              Y
		 *         :              :
		 *         :              :
		 *         V         ==>  V
		 *         P --> P$       P2     P
		 *                          \
		 *         Q=P'              `-->Q (P2')
		 *
		 *         in this case, ``We temporarily destroy the structure
		 *         of tree Y, so that a link to the derivative of P2 is
		 *         saved for future use.'', as Knuth said.
		 *
		 * Possibility γ:
		 *         P$             P
		 *         |              |
		 *         V         ==>  V
		 *         P2    P        X     P2
		 *          \              \
		 *           `-->P2'        `-->X'
		 *
		 *               Q=P'           Q=P2'
		 */
		P2 = P;
		P = compute_Pdollar(P);
		if (P2->RTAG == 0)
			P2->R = Q;
		/* This is the tricky part of the algorithm: We temporarily destroy
		 * the structure of tree Y, so that a link to the derivative of P2
		 * is saved for future use. The missing link will be restored later
		 * in step D3.
		 *
		 * ``a link to the derivative of P2 is saved for future use'':
		 */

		/* D5 (1/2) */
	} while (P != Y);

	/* D5 (2/2) */
	DY->L = Q;
	Q->R = DY;
	Q->RTAG = 1;
}

struct tnode * sub_formula(struct tnode * op, struct tnode * l, struct tnode * r)
{
	struct tnode * q;
	assert(op && l);
	l->R = r;
	op->L = l;
	q = r ? r : l;
	q->RTAG = 1;
	q->R = op;
	return op;
}

struct tnode * get_formula(int id)
{
	/*  (y)
	 *   |
	 *   V
	 *  (3)
	 */
	struct tnode * p;
	struct tnode * q;

	if (id == -1) {
		/* struct tnode * p0; */
		/* struct tnode * p1; */
		/* struct tnode * p2; */
		/* struct tnode * p3; */
		/* struct tnode * q1; */
		/* struct tnode * q2; */
		/* struct tnode * q3; */

		/* /\* x+1 *\/ */
		/* p1 = sub_formula(alloc_node(Add, 0), */
		/* 		 alloc_node(VAR, 'x'), */
		/* 		 alloc_node(CON, 1)); */
		/* /\* ln(x+1) *\/ */
		/* p2 = sub_formula(alloc_node(Ln, 0), */
		/* 		 p1, */
		/* 		 0); */
		/* /\* 3ln(x+1) *\/ */
		/* p3 = sub_formula(alloc_node(Mul, 0), */
		/* 		 alloc_node(CON, 3), */
		/* 		 p2); */
		/* /\* x^2 *\/ */
		/* q1 = sub_formula(alloc_node(Power, 0), */
		/* 		 alloc_node(VAR, 'x'), */
		/* 		 alloc_node(CON, 2)); */
		/* /\* a/x^2 *\/ */
		/* q2 = sub_formula(alloc_node(Div, 0), */
		/* 		 alloc_node(VAR, 'a'), */
		/* 		 q1); */
		/* /\* 3ln(x+1)-a/x^2 *\/ */
		/* q3 = sub_formula(alloc_node(Sub, 0), */
		/* 		 p3, */
		/* 		 q2); */

		/* p0 = alloc_node(VAR, 'y'); */
		/* p0->L = q3; */
		/* p0->R = p0; */
		/* q3->RTAG = 1; */
		/* q3->R = p0; */

		/* /\*  (y) p0 */
		/*  *   | */
		/*  *   V */
		/*  *  (-) q3 */
		/*  *   | */
		/*  *   V  p3 */
		/*  *  (*)--------------->(/) q2 */
		/*  *   |                  | */
		/*  *   V                  V */
		/*  *  (3)--->(ln) p2     (a)--->(^) q1 */
		/*  *  p3->L   |          q2->L   | */
		/*  *          V                  V */
		/*  *         (+) p1             (x)--->(2) */
		/*  *          |                 q1->L  q1->L->R */
		/*  *          V */
		/*  *         (x)--->(1) */
		/*  *         p1->L  p1->L->R */
		/*  * */
		/*  *  inorder (postorder in the original tree): */
		/*  *       3  x  1  +  ln  *  a  x  2  ^  /  - */
		/*  *\/ */

		/* test_COPY(p1->L->R); */
		/* test_COPY(p1->L); */
		/* test_COPY(p1); */
		/* test_COPY(p2); */
		/* test_COPY(p3->L); */
		/* test_COPY(p3); */
		/* test_COPY(q1->L->R); */
		/* test_COPY(q1->L); */
		/* test_COPY(q1); */
		/* test_COPY(q2->L); */
		/* test_COPY(q2); */
		/* test_COPY(q3); */

		return 0;
	} else if (id == 0) {
		/* 3 */
		q = alloc_node(CON, 3);
	} else if (id == 1) {
		/* x */
		q = alloc_node(VAR, 'x');
	} else if (id == 2) {
		/* Ln(x) */
		q = sub_formula(alloc_node(Ln, 0),
				alloc_node(VAR, 'x'),
				0);
	} else if (id == 3) {
		/* 3 + x */
		q = sub_formula(alloc_node(Add, 0),
				alloc_node(CON, 3),
				alloc_node(VAR, 'x'));
	} else if (id == 4) {
		/* x + x */
		q = sub_formula(alloc_node(Add, 0),
				alloc_node(VAR, 'x'),
				alloc_node(VAR, 'x'));
	} else if (id == 5) {
		/* 3 * x */
		q = sub_formula(alloc_node(Mul, 0),
				alloc_node(CON, 3),
				alloc_node(VAR, 'x'));
	} else if (id == 6) {
		/* x ^ 3 */
		q = sub_formula(alloc_node(Power, 0),
				alloc_node(VAR, 'x'),
				alloc_node(CON, 3));
	} else if (id == 7) {
		/* 3 / x */
		q = sub_formula(alloc_node(Div, 0),
				alloc_node(CON, 3),
				alloc_node(VAR, 'x'));
	} else if (id == 8) {
		/* ln(x+3) */
		q = sub_formula(alloc_node(Ln, 0),
				sub_formula(alloc_node(Add, 0),
					    alloc_node(VAR, 'x'),
					    alloc_node(CON, 3)),
				0);
	} else if (id == 9) {
		/* 3ln(x+1) */
		q = sub_formula(alloc_node(Mul, 0),
				alloc_node(CON, 3),
				sub_formula(alloc_node(Ln, 0),
					    sub_formula(alloc_node(Add, 0),
							alloc_node(VAR, 'x'),
							alloc_node(CON, 1)),
					    0));
	} else if (id == 10) {
		/* a / x^2 */
		q = sub_formula(alloc_node(Div, 0),
				alloc_node(VAR, 'a'),
				sub_formula(alloc_node(Power, 0),
					    alloc_node(VAR, 'x'),
					    alloc_node(CON, 2)));
	} else if (id == 11) {
		/* 2.3.2-(7) @ p.337:
		 *
		 *  (y)
		 *   |
		 *   V
		 *  (-)
		 *   |
		 *   V
		 *  (*)--------------->(/)
		 *   |                  |
		 *   V                  V
		 *  (3)--->(ln)        (a)--->(^)
		 *          |                  |
		 *          V                  V
		 *         (+)                (x)--->(2)
		 *          |
		 *          V
		 *         (x)--->(1)
		 *
		 *  inorder (postorder in the original tree):
		 *       3  x  1  +  ln  *  a  x  2  ^  /  -
		 */
		q = sub_formula(alloc_node(Sub, 0),
				sub_formula(alloc_node(Mul, 0),
					    alloc_node(CON, 3),
					    sub_formula(alloc_node(Ln, 0),
							sub_formula(alloc_node(Add, 0),
								    alloc_node(VAR, 'x'),
								    alloc_node(CON, 1)),
							0)),
				sub_formula(alloc_node(Div, 0),
					    alloc_node(VAR, 'a'),
					    sub_formula(alloc_node(Power, 0),
							alloc_node(VAR, 'x'),
							alloc_node(CON, 2))));
	} else {
		return 0;
	}
		
	p = alloc_node(VAR, 'y');
	p->L = q;
	p->R = p;
	q->RTAG = 1;
	q->R = p;

	return p;
}

int main(void)
{
	struct tnode * formula;
	struct tnode * d_formula;
	int i = 0;

	NONSENSE_NODE = alloc_node(VAR, '~');
	NONSENSE_NODE->L = 0;
	NONSENSE_NODE->RTAG = 0;
	NONSENSE_NODE->R = NONSENSE_NODE;

	/* if (0) { */
	/* 	(void)get_formula(-1); */
	/* 	return 100; */
	/* } */

	while (1) {
		formula = get_formula(i);
		if (!formula)
			break;
		printf("%stree%d, tree%d%s\n", RED, i*2, i*2+1, NOCOLOR);
		print_right_threaded_bintree(i*2, formula);

		d_formula = alloc_node(VAR, 'Y');
		d_formula->L    = d_formula;
		d_formula->RTAG = 0;
		d_formula->R    = d_formula;

		differentiate(formula, d_formula);
		print_right_threaded_bintree(i*2+1, d_formula);
		free_tree(formula);
		free_tree(d_formula);

		i++;
	}

	free_node(NONSENSE_NODE);

	return 0;
}
