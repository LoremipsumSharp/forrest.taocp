/*
 * compute P$, $P, P*, *P, P#, #P
 *
 * author: Forrest Y. Yu <forrest.yu@gmail.com>, http://forrestyu.net/
 *
 * build:
 *     $ gcc -Wall -g -o tbintree p.322_threaded.binary.tree.c
 *
 * run:
 *     $ ./tbintree	# a dot file p.322_threaded.binary.tree.dot will be outputted
 *
 * generate a png pic:
 *     $ dot -Tpng p.322_threaded.binary.tree.dot -o test.png
 *
 * generate a svg pic:
 *     $ dot -Tsvg p.322_threaded.binary.tree.dot -o test.svg
 *
 * generate pgf/TikZ format to use in LaTeX file:
 *     $ dot -Txdot p.322_threaded.binary.tree.dot | dot2tex > dot.tex
 *     if the position and font of the nodes in dot.tex is not good enough,
 *     these steps may help improve it:
 *     (1). define new node font:
 *             \def\nodefont{\Large\ttfamily}
 *     (2). replace lines like
 *             \draw (xxxbp,yyybp) node {?};
 *          with
 *             \draw [yshift=5bp] (xxxbp,yyybp) node {\nodefont ?};
 *          in which ``?'' is a node label
 */

#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>

#define POOL_SIZE		128
#define BTNODE_DEFAULT_VAL	'?'
#define BTNODE_HEAD_VAL		'0'

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

/* struct */
struct btnode {
	int val;
	int flags;	/*      |   31 .. 2  |   1  |   0  |
			 *      +------------+------+------+
			 *      |      0     | LTAG | RTAG |
			 *      +------------+------+------+
			 */
	struct btnode * L;
	struct btnode * R;
};

/* Memory Management */
struct btnode * alloc_node(int v)
{
	static struct btnode nodes_pool[POOL_SIZE];
	static struct btnode * p = nodes_pool;
	assert(p < nodes_pool + POOL_SIZE);
	p->val = v ? v : BTNODE_DEFAULT_VAL;
	p->L = 0;
	p->R = 0;
	p->flags = 0;
	return p++;
}

void free_node(struct btnode * p)
{
	printf("binary tree node freed: %d @ %d\n", p->val, (int)p);
	return;
}

/* set LTAG(p) or RTAG(p) */
void set_TAG(struct btnode * p, char LR, int tag)
{
	assert(p);
	assert(LR == 'L' || LR == 'R');

	if (tag)
		p->flags |= (LR == 'L' ? 2 : 1);
	else
		p->flags &= (LR == 'L' ? ~2 : ~1);
}

/* get LTAG(p) or RTAG(p) */
int get_TAG(struct btnode * p, char LR)
{
	assert(p);
	assert(LR == 'L' || LR == 'R');
	int tag = p->flags & (LR == 'L' ? 2 : 1);
	/* printf("%cTAG(%c):%d\n", */
	/*        LR == 'L' ? 'L' : 'R', */
	/*        p->val, */
	/*        tag); */
	return tag;
}

/* routine via which the dot file is written */
void write_dot(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	static FILE * dotf = 0;
	if (!dotf) {
		char dotfilename[FILENAME_MAX];
		char * p;
		strcpy(dotfilename, __FILE__);
		for (p = dotfilename + strlen(dotfilename); *p != '.'; p--);
		strcpy(p, ".dot");
		fprintf(stderr, "dot file: %s\n", dotfilename);

		dotf = fopen(dotfilename, "w");
		assert(dotf);

		fprintf(dotf, "digraph example {\n");

		fprintf(dotf,
			"\tnode [shape=record"
			", fontname=Courier New"
			", penwidth=0.5"
			", height=0.3"
			"];\n");
	}

	assert(dotf);
	if (!fmt[0]) {
		fprintf(dotf, "}\n");
		fclose(dotf);
		dotf = 0;
	} else {
		fprintf(dotf, "\t");
		vfprintf(dotf, fmt, args);
	}

	va_end(args);
}

/* what to do when visiting a node */
void visit(struct btnode * p)
{
	printf("%s%c%s", RED, p->val, NOCOLOR);
}

/* inorder traverse */
void inorder_traverse_recursive(struct btnode * T)
{
	if (!get_TAG(T, 'L'))
		inorder_traverse_recursive(T->L);

	visit(T);

	const char L_ATTR[] = "arrowhead=\"rnormal\", color=\"blue\"";
	const char R_ATTR[] = "arrowhead=\"lnormal\", color=\"red\"";
	const char L_THREAD[] = "arrowhead=\"lopen\", color=\"cyan\", style=\"dashed\"";
	const char R_THREAD[] = "arrowhead=\"ropen\", color=\"magenta\", style=\"dashed\"";
	if (get_TAG(T, 'L'))
		write_dot("%c:l -> %c:m [%s];\n", T->val, T->L->val, L_THREAD);
	else
		write_dot("%c:l -> %c:m [%s];\n", T->val, T->L->val, L_ATTR);

	if (get_TAG(T, 'R'))
		write_dot("%c:r -> %c:m [%s];\n", T->val, T->R->val, R_THREAD);
	else
		write_dot("%c:r -> %c:m [%s];\n", T->val, T->R->val, R_ATTR);

	if (!get_TAG(T, 'R'))
		inorder_traverse_recursive(T->R);
}

/* Algorithm 2.3.1S @ p.323: compute P$ */
struct btnode * compute_Pdollar(struct btnode * p)
{
	struct btnode * q = p->R;
	if (!get_TAG(p, 'R'))
		while (!get_TAG(q, 'L'))
			q = q->L;
	return q;
}

/* compute $P */
struct btnode * compute_dollarP(struct btnode * p)
{
	struct btnode * q = p->L;
	if (!get_TAG(p, 'L'))
		while (!get_TAG(q, 'R'))
			q = q->R;
	return q;
}

/* Algorithm 2.3.1I @ p.327 (slightly modified) */
void insert_to_right(struct btnode * p, struct btnode *q)
{
	/*
	 * these may change no matter if p has right child:
	 *     p->R
	 *     q->L
	 *     q->R
	 *     LTAG(q)
	 *     RTAG(q)
	 * if p has no right child, this may change too
	 *     RTAG(p)
	 *
	 *                  p has right child           p has no right child
	 *     p->R         *                           *
	 *     q->L         *                           *
	 *     q->R         *                           *
	 *     LTAG(q)      *                           *
	 *     RTAG(q)      *                           *
	 *     RTAG(p)      -                           *
	 *     $(p$)        *                           -
	 */
	q->R = p->R;
	set_TAG(q, 'R', get_TAG(p, 'R'));
	set_TAG(q, 'L', 1);
	q->L = p;
	p->R = q;
	if (!get_TAG(p, 'R'))	/* p has right child */
		compute_Pdollar(q)->L = q;
	else			/* p has no right child */
		set_TAG(p, 'R', 0);
	/* if (!get_TAG(p, 'R')) {	/\* p has right child *\/ */
	/* 	assert(!get_TAG(p, 'R')); */
	/* 	struct btnode * s = q->R; */
	/* 	while (!get_TAG(s, 'L')) */
	/* 		s = s->L; */
	/* 	assert(s == compute_Pdollar(q)); */
	/* 	s->L = q; */
	/* } else {		/\* p has no right child *\/ */
	/* 	set_TAG(p, 'R', 0); */
	/* } */
}

/* similar with insert_to_right() */
void insert_to_left(struct btnode * p, struct btnode *q)
{
	/*
	 * these may change no matter if p has left child:
	 *     p->L
	 *     q->R
	 *     q->L
	 *     RTAG(q)
	 *     LTAG(q)
	 * if p has no left child, this may change too
	 *     LTAG(p)
	 *
	 *                  p has left child            p has no left child
	 *     p->L         *                           *
	 *     q->R         *                           *
	 *     q->L         *                           *
	 *     RTAG(q)      *                           *
	 *     LTAG(q)      *                           *
	 *     LTAG(p)      -                           *
	 *     ($p)$        *                           -
	 */
	q->L = p->L;
	set_TAG(q, 'L', get_TAG(p, 'L'));
	set_TAG(q, 'R', 1);
	q->R = p;
	p->L = q;
	if (!get_TAG(p, 'L'))	/* p has left child */
		compute_dollarP(q)->R = q;
	else			/* p has no left child */
		set_TAG(p, 'L', 0);
	/* if (!get_TAG(p, 'L')) {	/\* p has left child *\/ */
	/* 	assert(!get_TAG(p, 'L')); */
	/* 	struct btnode * s = q->L; */
	/* 	while (!get_TAG(s, 'R')) */
	/* 		s = s->R; */
	/* 	s->R = q; */
	/* } else {		/\* p has no left child *\/ */
	/* 	set_TAG(p, 'L', 0); */
	/* } */
}

struct btnode * compute_parent(struct btnode * p)
{
	struct btnode * pl = p;
	while (!get_TAG(pl, 'L'))
		pl = pl->L;	/* goto the leftmost leaf of the left subtree */
	pl = pl->L;
	if (pl->R == p)
		return pl;

	struct btnode * pr = p;
	while (!get_TAG(pr, 'R'))
		pr = pr->R;	/* goto the rightmost leaf of the left subtree */
	pr = pr->R;
	if (pr->L == p)
		return pr;

	/* must be the root, and root's parent is head */
	assert(pl->R == pl && pr->R == pr);
	return pl;
}

/*
 * P*
 */
struct btnode * compute_Pstar(struct btnode * p)
{
	assert(p);
	if (!get_TAG(p, 'L'))	/* p has left child */
		return p->L;

	/* p has no left child */
	struct btnode * q = p;
	while (get_TAG(q, 'R'))	/* go upwards until q has right child */
		q = q->R;
	q = q->R;
	return q;
}

/*
 * *P
 */
struct btnode * compute_starP(struct btnode * p)
{
	assert(p);
	struct btnode * parent = compute_parent(p);
	if (parent->L == p)
		return parent;

	assert(parent->R == p);

	if (get_TAG(parent, 'L'))
		return parent;

	struct btnode * q = parent->L;
	while (get_TAG(q, 'R'))
		if (!get_TAG(q, 'L'))
			q = q->L;
		else
			return q;

	while (!get_TAG(q, 'R'))
		q = q->R;
	while (!get_TAG(q, 'L'))
		q = q->L;
	return q;
}

/* 
 * #P
 */
struct btnode * compute_sharpP(struct btnode * p)
{
	if (!get_TAG(p, 'R'))	/* p has right child */
		return p->R;

	if (p->L->R == p->L)	/* #P is head */
		return p->L;

	/* p has no right child */
	struct btnode * q = p;
	while (get_TAG(q, 'L'))
		q = q->L;
	q = q->L;

	return q;
}

/* 
 * P#
 */
struct btnode * compute_Psharp(struct btnode * p)
{
	assert(p);
	struct btnode * parent = compute_parent(p);

	if (parent->R == parent) /* P# is head */
		return parent;

	if (parent->R == p)
		return parent;

	assert(parent->L == p);

	if (get_TAG(parent, 'R'))
		return parent;

	struct btnode * q = parent->R;
	while (get_TAG(q, 'L'))
		if (!get_TAG(q, 'R'))
			q = q->R;
		else
			return q;

	while (!get_TAG(q, 'L'))
		q = q->L;

	while (!get_TAG(q, 'R'))
		q = q->R;
	return q;
}

/* for test */
void hard_coded(void)
{
	/*      .......> head<---.
	 *      :       / ^ \____|
	 *      :      /  :
	 *      :     A   :
	 *      :....` `...
	 */
	struct btnode * head = alloc_node(BTNODE_HEAD_VAL);

	set_TAG(head, 'L', 1);
	set_TAG(head, 'R', 0);
	head->L = 0;
	head->R = head;

	printf("LTAG(head):%d\n", get_TAG(head, 'L'));
	printf("RTAG(head):%d\n", get_TAG(head, 'R'));

	struct btnode * A = alloc_node('A');
	set_TAG(A, 'L', 1);
	set_TAG(A, 'R', 1);
	A->L = head;
	A->R = head;

	/*              head<---.
	 *              /  \____|
	 *             /
	 *            A
	 *          /   \
	 *        B       C
	 *      /       /   \
	 *    D       E       F
	 *             \     / \
	 *              G   H   J
	 */
	struct btnode * B = alloc_node('B');
	struct btnode * C = alloc_node('C');
	struct btnode * D = alloc_node('D');
	struct btnode * E = alloc_node('E');
	struct btnode * F = alloc_node('F');
	struct btnode * G = alloc_node('G');
	struct btnode * H = alloc_node('H');
	struct btnode * J = alloc_node('J');

	/* insert_to_right(A, F); */
	/* insert_to_right(F, J); */
	/* insert_to_right(A, C); */

	A->L = B; A->R = C;
	set_TAG(A, 'L', 0);
	set_TAG(A, 'R', 0);

	B->L = D; B->R = A;
	set_TAG(B, 'L', 0);
	set_TAG(B, 'R', 1);

	C->L = E; C->R = F;
	set_TAG(C, 'L', 0);
	set_TAG(C, 'R', 0);

	D->L = head; D->R = B;
	set_TAG(D, 'L', 1);
	set_TAG(D, 'R', 1);

	E->L = A; E->R = G;
	set_TAG(E, 'L', 1);
	set_TAG(E, 'R', 0);

	F->L = H; F->R = J;
	set_TAG(F, 'L', 0);
	set_TAG(F, 'R', 0);

	G->L = E; G->R = C;
	set_TAG(G, 'L', 1);
	set_TAG(G, 'R', 1);

	H->L = C; H->R = F;
	set_TAG(H, 'L', 1);
	set_TAG(H, 'R', 1);

	J->L = F; J->R = head;
	set_TAG(J, 'L', 1);
	set_TAG(J, 'R', 1);

	inorder_traverse_recursive(A);
	inorder_traverse_recursive(0);
}

/* if the output graph is not satisfying, use this routine to beautify it */
void beautify_dot_graph(void)
{
	/* write_dot("Y [penwidth=0, fontcolor=\"white\"];\n"); */
	/* write_dot("D -> Y [arrowhead=\"none\", penwidth=0];\n"); */
	/* write_dot("%c -> Y [arrowhead=\"none\", penwidth=0];\n", BTNODE_HEAD_VAL); */

	const char invisible_line[] = "arrowhead=\"none\", penwidth=0";
	const char invisible_node[] = "penwidth=0, fontcolor=\"white\"";
	write_dot("Br [%s];\n", invisible_node);
	write_dot("Dl [%s];\n", invisible_node);
	write_dot("Dr [%s];\n", invisible_node);
	write_dot("El [%s];\n", invisible_node);
	write_dot("Brl [%s];\n", invisible_node);
	write_dot("Brr [%s];\n", invisible_node);
	write_dot("B  -> Br  [%s];\n", invisible_line);
	write_dot("D  -> Dl  [%s];\n", invisible_line);
	write_dot("D  -> Dr  [%s];\n", invisible_line);
	write_dot("E  -> El  [%s];\n", invisible_line);
	write_dot("Br -> Brl [%s];\n", invisible_line);
	write_dot("Br -> Brr [%s];\n", invisible_line);
	}

int main(void)
{
	/*      .......> head<---.
	 *      :       / ^ \____|
	 *      :      /  :
	 *      :     A   :
	 *      :....` `...
	 */
	struct btnode * head = alloc_node(BTNODE_HEAD_VAL);

	set_TAG(head, 'L', 1);
	set_TAG(head, 'R', 0);
	head->L = 0;
	head->R = head;

	printf("LTAG(head):%d\n", get_TAG(head, 'L'));
	printf("RTAG(head):%d\n", get_TAG(head, 'R'));

	struct btnode * A = alloc_node('A');
	set_TAG(A, 'L', 1);
	set_TAG(A, 'R', 1);
	A->L = head;
	A->R = head;
	head->L = A;

	/*              head<---.
	 *              /  \____|
	 *             /
	 *            A
	 *          /   \
	 *        B       C
	 *      /       /   \
	 *    D       E       F
	 *             \     / \
	 *              G   H   J
	 */
	struct btnode * B = alloc_node('B');
	struct btnode * C = alloc_node('C');
	struct btnode * D = alloc_node('D');
	struct btnode * E = alloc_node('E');
	struct btnode * F = alloc_node('F');
	struct btnode * G = alloc_node('G');
	struct btnode * H = alloc_node('H');
	struct btnode * J = alloc_node('J');

	/* use insert_to_right() and insert_to_left() to finish the tree */
	insert_to_right(A, F);
	insert_to_right(F, J);
	insert_to_right(A, C);
	insert_to_left(F, H);
	insert_to_left(C, E);
	insert_to_right(E, G);
	insert_to_left(A, D);
	insert_to_left(A, B);

	/* generate the graph */
	write_dot("%c [label=\"<l>|<m> head|<r>\"];\n", head->val);
	const char node_names[] = "ABCDEFGHJ";
	const char * pn = node_names;
	for (; *pn; pn++)
		write_dot("%c [label=\"<l>|<m> %c|<r>\"];\n", *pn, *pn);
	write_dot("%c:l -> %c:m [arrowhead=\"onormal\", color=\"blue\"];\n",
		  head->val, head->L->val);
	write_dot("%c:r -> %c:m [arrowhead=\"onormal\", color=\"red\"];\n",
		  head->val, head->R->val);

	/* traverse the tree so that all nodes are put into the graph */
	inorder_traverse_recursive(A);

	/* beautify_dot_graph(); */

	/* finish the graph */
	write_dot("");

	printf("\n");

	/*
	 * given threaded tree, compute its P*, *P, P#, #P
	 */
	{
		/* for P* */
		struct btnode * pnodes[] = {A, B, D, C, E, G, F, H, J, 0};
		struct btnode ** pp = pnodes;
		for (; *pp; pp++) {
			struct btnode * Pstar = compute_Pstar(*pp);
			printf("%c*: %c\n", (*pp)->val, Pstar->val);
		}
	}

	{
		/* for parent */
		struct btnode * pnodes[] = {A, B, D, C, E, G, F, H, J, 0};
		struct btnode ** pp = pnodes;
		for (; *pp; pp++) {
			struct btnode * parent = compute_parent(*pp);
			printf("%c's parent: %c\n", (*pp)->val, parent->val);
		}
	}

	{
		/* for *P */
		struct btnode * pnodes[] = {J, H, F, G, E, C, D, B, A, 0};
		struct btnode ** pp = pnodes;
		for (; *pp; pp++) {
			struct btnode * starP = compute_starP(*pp);
			printf("*%c: %c\n", (*pp)->val, starP->val);
		}
	}

	{
		/* for #P */
		struct btnode * pnodes[] = {A, C, F, J, H, E, G, B, D, 0};
		struct btnode ** pp = pnodes;
		for (; *pp; pp++) {
			struct btnode * sharpP = compute_sharpP(*pp);
			printf("#%c: %c\n", (*pp)->val, sharpP->val);
		}
	}

	{
		/* for P# */
		struct btnode * pnodes[] = {D, B, G, E, H, J, F, C, A, 0};
		struct btnode ** pp = pnodes;
		for (; *pp; pp++) {
			struct btnode * Psharp = compute_Psharp(*pp);
			printf("%c#: %c\n", (*pp)->val, Psharp->val);
		}
	}

	return 0;
}
