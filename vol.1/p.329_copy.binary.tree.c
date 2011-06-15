/*
 * Algorithm 2.3.1C @ TAOCP::p.329
 * Algorithm 2.3.1S @ TAOCP::p.323
 * Algorithm 2.3.1I @ TAOCP::p.327
 *
 * author: Forrest Y. Yu <forrest.yu@gmail.com>, http://forrestyu.net/
 *
 * Build:
 *     $ gcc -g -Wall -o cpbintree p.329_copy.binary.tree.c
 * Run:
 *     ./cpbintree
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>

#define POOL_SIZE		128
#define MAX_DOTF_NR		128
#define BTNODE_DEFAULT_VAL	'?'
#define BTNODE_HEAD_VAL		'0'

int pic_id = 0;

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
}

/* similar with insert_to_right() */
void insert_to_left(struct btnode * p, struct btnode *q)
{
	assert(p && q);

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
}

/* routine via which the dot file is written */
void write_dot(int id, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	assert(id >= 0 && id < MAX_DOTF_NR);

	static int flag = 0;
	static FILE * dotf[MAX_DOTF_NR];
	static char dotfilename[MAX_DOTF_NR][FILENAME_MAX];
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
		sprintf(p, "%02d.dot", id);
		fprintf(stderr, "\ndot file: %s\n", dotfilename[id]);

		dotf[id] = fopen(dotfilename[id], "w");
		assert(dotf[id]);

		fprintf(dotf[id], "digraph example {\n");

		fprintf(dotf[id],
			"\tnode [shape=record"
			", fontname=Courier New"
			", penwidth=0.5"
			", height=0.3"
			"];\n");
	}

	assert(dotf[id]);
	if (!fmt[0]) {
		fprintf(dotf[id], "}\n");
		fclose(dotf[id]);
		dotf[id] = 0;
		char cmd[FILENAME_MAX + 128];
		sprintf(cmd, "dot -Tpng %s -o \"TAOCP - Algorithm 2.3.1 C (%02d).png\"", dotfilename[id], id);
		printf("\n%s$ %s%s%s\n", WHITE, GREEN, cmd, NOCOLOR);
		assert(system(cmd) == 0);
	} else {
		fprintf(dotf[id], "\t");
		vfprintf(dotf[id], fmt, args);
	}

	va_end(args);
}

/* what to do when visiting a node */
void visit(int id, struct btnode * p)
{
	printf("%s%c%s", RED, p->val, NOCOLOR);
	if (p->R == p)
		write_dot(id, "\"%X\" [label=\"<l>|<m> head|<r>\"];\n", (int)p);
	else
		write_dot(id, "\"%X\" [label=\"<l>|<m> %c|<r>\"];\n", (int)p, p->val);
}

/* inorder traverse */
void inorder_traverse_recursive(int id, struct btnode * T)
{
	if (!get_TAG(T, 'L') && T->L != T)
		inorder_traverse_recursive(id, T->L);

	if (T->R != T)
		visit(id, T);

	const char L_ATTR[] = "arrowhead=\"rnormal\", color=\"blue\"";
	const char R_ATTR[] = "arrowhead=\"lnormal\", color=\"red\"";
	const char L_THREAD[] = "arrowhead=\"lopen\", color=\"cyan\", style=\"dashed\"";
	const char R_THREAD[] = "arrowhead=\"ropen\", color=\"magenta\", style=\"dashed\"";
	if (get_TAG(T, 'L'))
		write_dot(id, "\"%X\":l -> \"%X\":m [%s];\n", (int)T, (int)T->L, L_THREAD);
	else
		write_dot(id, "\"%X\":l -> \"%X\":m [%s];\n", (int)T, (int)T->L, L_ATTR);

	if (get_TAG(T, 'R'))
		write_dot(id, "\"%X\":r -> \"%X\":m [%s];\n", (int)T, (int)T->R, R_THREAD);
	else
		write_dot(id, "\"%X\":r -> \"%X\":m [%s];\n", (int)T, (int)T->R, R_ATTR);

	if (!get_TAG(T, 'R') && T->R != T)
		inorder_traverse_recursive(id, T->R);
}

void print_tree(int id, struct btnode * head)
{
	assert(head && head->L);

	/* generate the graph */
	visit(id, head);
	inorder_traverse_recursive(id, head);

	/* finish the graph */
	write_dot(id, "");
	printf("tree %d outputted.\n", id);
}

void copy_binary_tree(struct btnode * head, struct btnode * u)
{
	/* C1 */
	struct btnode * P = head;
	struct btnode * Q = u;

	assert(P->R == P && Q->R == Q);
	assert(Q->L == Q);

	while (1) {
		/* C4 */
		if (P->R == P || !get_TAG(P, 'L')) /* P is head or P has a left subtree */
			insert_to_left(Q, alloc_node(BTNODE_DEFAULT_VAL));

		/* C5 */
		P = compute_Pstar(P);
		Q = compute_Pstar(Q);

		/* C6 */
		if (Q == u->R)
			break;

		/* C2 */
		if (!get_TAG(P, 'R')) /* P has a right subtree */
			insert_to_right(Q, alloc_node(BTNODE_DEFAULT_VAL));

		/* C3 */
		Q->val = P->val;

		print_tree(pic_id++, u);
	}
}

void test0(void)
{
	printf("test0\n");

	/*      .......> head<---.
	 *      :       / ^ \____|
	 *      :      /  :
	 *      :     A   :
	 *      :....` `...
	 */
	struct btnode * head = alloc_node(BTNODE_HEAD_VAL);

	set_TAG(head, 'L', 1);
	set_TAG(head, 'R', 0);
	head->L = head;
	head->R = head;

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
	struct btnode * A = alloc_node('A');
	struct btnode * B = alloc_node('B');
	struct btnode * C = alloc_node('C');
	struct btnode * D = alloc_node('D');
	struct btnode * E = alloc_node('E');
	struct btnode * F = alloc_node('F');
	struct btnode * G = alloc_node('G');
	struct btnode * H = alloc_node('H');
	struct btnode * J = alloc_node('J');

	/* use insert_to_right() and insert_to_left() to finish the tree */
	insert_to_left(head, A);
	insert_to_right(A, F);
	insert_to_right(F, J);
	insert_to_right(A, C);
	insert_to_left(F, H);
	insert_to_left(C, E);
	insert_to_right(E, G);
	insert_to_left(A, D);
	insert_to_left(A, B);

	/* struct btnode * pnodes[] = {head, A, B, D, C, E, G, F, H, J, 0}; */
	/* struct btnode ** pp = pnodes; */
	/* for (; *pp; pp++) { */
	/* 	struct btnode * t = *pp; */
	/* 	printf("%X %c, LTAG:%d, RTAG:%d, L:%X, R:%X\n", */
	/* 	       (int)t, t->val, get_TAG(t, 'L'), get_TAG(t, 'R'), */
	/* 	       (int)t->L, (int)t->R); */
	/* } */

	/*       u<----.
	 *       |\____|
	 *       |
	 *       V
	 *       0
	 */
	struct btnode * u = alloc_node(BTNODE_HEAD_VAL);
	set_TAG(u, 'L', 1);
	set_TAG(u, 'R', 0);
	u->L = u;
	u->R = u;

	print_tree(pic_id++, u);

	copy_binary_tree(head, u);

	printf("\n");
}

void test1(void)
{
	printf("test1\n");

	/*      .......> head<---.
	 *      :       / ^ \____|
	 *      :      /  :
	 *      :     A   :
	 *      :....` `...
	 */
	struct btnode * head = alloc_node(BTNODE_HEAD_VAL);

	set_TAG(head, 'L', 1);
	set_TAG(head, 'R', 0);
	head->L = head;
	head->R = head;

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
	struct btnode * A = alloc_node('-');
	struct btnode * B = alloc_node('*');
	struct btnode * C = alloc_node('3');
	struct btnode * D = alloc_node('L');
	struct btnode * E = alloc_node('+');
	struct btnode * F = alloc_node('x');
	struct btnode * G = alloc_node('1');
	struct btnode * H = alloc_node('/');
	struct btnode * J = alloc_node('a');
	struct btnode * K = alloc_node('^');
	struct btnode * M = alloc_node('x');
	struct btnode * N = alloc_node('2');

	/* use insert_to_right() and insert_to_left() to finish the tree */
	insert_to_left(head, A);
	insert_to_left(A, B);
	insert_to_left(B, C);
	insert_to_right(C, D);
	insert_to_left(D, E);
	insert_to_left(E, F);
	insert_to_right(F, G);

	insert_to_right(B, H);
	insert_to_left(H, J);
	insert_to_right(J, K);
	insert_to_left(K, M);
	insert_to_right(M, N);

	/*       u<----.
	 *       |\____|
	 *       |
	 *       V
	 *       0
	 */
	struct btnode * u = alloc_node(BTNODE_HEAD_VAL);
	set_TAG(u, 'L', 1);
	set_TAG(u, 'R', 0);
	u->L = u;
	u->R = u;

	print_tree(pic_id++, u);

	copy_binary_tree(head, u);

	printf("\n");
}

int main(void)
{
	test0();
	test1();

	return 0;
}

