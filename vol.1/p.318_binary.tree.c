/*
 * traverse a binary tree in inorder, preorder and postorder;
 * Answer to Exercise 2.3.1-13 @ TAOCP::p.565
 *
 * author: Forrest Y. Yu <forrest.yu@gmail.com>, http://forrestyu.net/
 */

#include <stdio.h>
#include <assert.h>

#define POOL_SIZE		128
#define STACK_LIMIT		128
#define BTNODE_DEFAULT_VAL	'?'

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
	return p++;
}

void free_node(struct btnode * p)
{
	printf("binary tree node freed: %d @ %d\n", p->val, (int)p);
	return;
}

/*
 * simple stack
 */
enum stack_action {STACK_PUSH, STACK_POP};
struct btnode * bt_stack(enum stack_action a, struct btnode * p)
{
	/*
	 * available: STACK[1..STACK_LIMIT-1]
	 * @note: STACK[0] is considered as the base (reserved) and STACK[0] = 0.
	 */
	static struct btnode * STACK[STACK_LIMIT] = {0,};
	static int top = 0;
	if (a == STACK_PUSH) {
		assert(top < STACK_LIMIT);
		STACK[++top] = p;
	} else {
		assert(a == STACK_POP);
		if (top >= 0)
			return STACK[top--];
		else
			return 0;
	}

	return 0;
}

#define PUSH(x)	bt_stack(STACK_PUSH, x) /* usage: PUSH(p); */
#define POP()	bt_stack(STACK_POP, 0)  /* usage: POP(); */

struct btnode * STACK_TOP(void)
{
	struct btnode * q = POP();
	PUSH(q);
	return q;
}

/* what to do when visiting a node */
void visit(struct btnode * p)
{
	printf("%s%c%s", RED, p->val, NOCOLOR);
}

/* traverse */
int inorder_traverse_recursive(struct btnode * root)
{
	if (!root)
		return 0;
	int cl = inorder_traverse_recursive(root->L);
	visit(root);
	int cr = inorder_traverse_recursive(root->R);
	return cl + 1 + cr;
}

int inorder_traverse(struct btnode * root)
{
	/* EXAMPLE:
	 *            A
	 *          /   \
	 *        B       C
	 *      /       /   \         ==>     D B A E G C H F J
	 *    D       E       F
	 *             \     / \
	 *              G   H   J
	 */
	int cnt = 0;
	struct btnode * p;

	/* stack should be empty */
	assert(!POP());

	p = root;
	while (1) {
		/* this algorithm is essentially symmetric (just like the recursive version):
		 *                             +--------+
		 *       .-------------------> | p!=0 ? | <-------------------.
		 *       |                     +--------+                     |
		 *       |                       |    |                       |
		 *       |                   Yes |    | No                    |
		 *       |                 .-----`    `-----.                 |
		 *       |                 |                |                 |
		 *       |                 |                |                 |
		 *       |                 V                V                 |
		 *     p=p->L <--------- push p           pop p    -------> p=p->R
		 *                                        visit(p)
		 */
		while (p) {
			PUSH(p);
			p = p->L;
		}

		if (!(p = POP()))
			break;

		visit(p);
		cnt++;

		p = p->R;
	}
	return cnt;
}

int inorder_traverse_variant(struct btnode * root)
{
	/* EXAMPLE:
	 *            A
	 *          /   \
	 *        B       C
	 *      /       /   \         ==>     D B A E G C H F J
	 *    D       E       F
	 *             \     / \
	 *              G   H   J
	 */
	int cnt = 0;
	struct btnode * p;

	/* stack should be empty */
	assert(!POP());

	p = root;
	while (p) {
		while (p->L) {
			if (p != STACK_TOP()) {
				PUSH(p);
			} else {
				p = POP();
				break;
			}
			p = p->L;
		}
		visit(p);
		cnt++;

		if (p->R)
			p = p->R;
		else
			p = STACK_TOP(); /* @note: STACK[BASE] == 0 */
	}
	return cnt;
}

int preorder_traverse_recursive(struct btnode * root)
{
	if (!root)
		return 0;
	visit(root);
	int nl = preorder_traverse_recursive(root->L);
	int nr = preorder_traverse_recursive(root->R);
	return nl + nr + 1;
}

int preorder_traverse(struct btnode * root)
{
	/* EXAMPLE:
	 *            A
	 *          /   \
	 *        B       C
	 *      /       /   \         ==>     A B D C E G F H J
	 *    D       E       F
	 *             \     / \
	 *              G   H   J
	 */
	int cnt = 0;
	struct btnode * p;

	/* stack should be empty */
	assert(!POP());

	p = root;
	while (1) {
		/* this algorithm is essentially symmetric (just like the recursive version):
		 *                             +--------+
		 *       .-------------------> | p!=0 ? | <-------------------.
		 *       |                     +--------+                     |
		 *       |                       |    |                       |
		 *       |                   Yes |    | No                    |
		 *       |                 .-----`    `-----.                 |
		 *       |                 |                |                 |
		 *       |                 |                |                 |
		 *       |                 V                V                 |
		 *     p=p->L <--------- visit(p)         pop p ----------> p=p->R
		 *                       push p
		 */
		while (p) {
			visit(p); cnt++;
			PUSH(p);
			p = p->L;
		}

		if (!(p = POP()))
			break;

		p = p->R;
	}

	return cnt;
}

int postorder_traverse_recursive(struct btnode * root)
{
	if (!root)
		return 0;
	int nl = postorder_traverse_recursive(root->L);
	int nr = postorder_traverse_recursive(root->R);
	visit(root);
	return nl + nr + 1;
}

int postorder_traverse(struct btnode * root)
{
	/* EXAMPLE:
	 *            A
	 *          /   \
	 *        B       C
	 *      /       /   \         ==>     D B G E H J F C A
	 *    D       E       F
	 *             \     / \
	 *              G   H   J
	 */
	int cnt = 0;
	struct btnode dummy_node;
	struct btnode * dummy = &dummy_node;
	struct btnode * p;

	/* stack should be empty */
	assert(!POP());

	p = root;
	while (1) {
		while (p) {
			PUSH(p);
			PUSH(dummy);
			if (p->R)
				PUSH(p->R);
			p = p->L;
		}

		if(!(p = POP()))
			break;

		if (p == dummy) {
			assert(p = POP());
			visit(p); cnt++;
			p = 0;
		}
	}

	return cnt;
}

int postorder_traverse_improved(struct btnode * root)
{
	/* EXAMPLE:
	 *            A
	 *          /   \
	 *        B       C
	 *      /       /   \         ==>     D B G E H J F C A
	 *    D       E       F
	 *             \     / \
	 *              G   H   J
	 */
	int cnt = 0;
	struct btnode * p;
	struct btnode * q = 0;

	/* stack should be empty */
	assert(!POP());

	p = root;
	while (1) {
		while (p) {
			PUSH(p);
			p = p->L;
		}

		if (!(p = POP()))
			break;

		if (p->R == q) {
			visit(p); cnt++;
			q = p;
			p = 0;
			continue;
		}

		PUSH(p);
		p = p->R;
		q = p;
	}

	return cnt;
}

int postorder_traverse_taocp(struct btnode * root)
{
	/* EXAMPLE:
	 *            A
	 *          /   \
	 *        B       C
	 *      /       /   \         ==>     D B G E H J F C A
	 *    D       E       F
	 *             \     / \
	 *              G   H   J
	 */
	int cnt = 0;

	/* T1 */
	struct btnode * p;
	struct btnode * q = 0;

	/* stack should be empty */
	assert(!POP());

	p = root;
	while (1) {
		/* this algorithm is not symmetric, but one thing must be noticed:
		 * ``p=p->L'' and ``p=p->R'' must be followed by going to the beginning of the while loop,
		 * so that the subtree is manipulated just as the tree.
		 *                        +--------+
		 *       .--------------> | p!=0 ? | <------------------------------------.
		 *       |                +--------+                                      |
		 *       |                  |    |                                        |
		 *       |              Yes |    | No                                     |
		 *       |            .-----`    `-----.                                  |
		 *       |            |                |                                  |
		 *       |            |                |        +-------------+           |
		 *       |            V                V        | p->R == 0   |  No       |
		 *     p=p->L <---- push p           pop p ---->|     or      |-------> push p
		 *                                     ^        | p->R == q ? |         p=p->R
		 *                                     |        +-------------+
		 *                                     |               | Yes
		 *                                     |               |
		 *                                     |               V
		 *                                     |            visit(p)
		 *                                     `----------- q = p
		 */
		while (p) {	/* T2 */
			/* T3 */
			PUSH(p);
			p = p->L;
		}

		/* T4 */
		if (!(p = POP()))
			break;

		/* T5 */
		if ((p->R == 0) || (p->R == q)) {
			/* T6 */
			visit(p); cnt++;
			q = p;
			p = 0; continue; /* goto T4 */
		}

		PUSH(p);
		p = p->R;
	}

	return cnt;
}

/* printing service */
void print_bintree(struct btnode * root)
{
	printf("\n%s********** inorder traversal (recursive) **********\n%s",
	       GREEN, NOCOLOR);
	printf("  %d nodes visited.\n", inorder_traverse_recursive(root));
	printf("\n%s********** inorder traversal **********\n%s",
	       GREEN, NOCOLOR);
	printf("  %d nodes visited.\n", inorder_traverse(root));
	printf("\n%s********** inorder traversal variant **********\n%s",
	       GREEN, NOCOLOR);
	printf("  %d nodes visited.\n", inorder_traverse_variant(root));

	printf("\n%s********** preorder traversal (recursive) **********\n%s",
	       GREEN, NOCOLOR);
	printf("  %d nodes visited.\n", preorder_traverse_recursive(root));
	printf("\n%s********** preorder traversal **********\n%s",
	       GREEN, NOCOLOR);
	printf("  %d nodes visited.\n", preorder_traverse(root));

	printf("\n%s********** postorder traversal (recursive) **********\n%s",
	       GREEN, NOCOLOR);
	printf("  %d nodes visited.\n", postorder_traverse_recursive(root));
	printf("\n%s********** postorder traversal **********\n%s",
	       GREEN, NOCOLOR);
	printf("  %d nodes visited.\n", postorder_traverse(root));
	printf("\n%s********** postorder traversal improved"
	       " **********\n%s", GREEN, NOCOLOR);
	printf("  %d nodes visited.\n", postorder_traverse_improved(root));
	printf("\n%s********** postorder traversal (answer of 2.3.1.13 @ p.565)"
	       " **********\n%s", GREEN, NOCOLOR);
	printf("  %d nodes visited.\n", postorder_traverse_taocp(root));
}

/* MAIN */
int main(void)
{
	/*            A
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
	A->L = B; A->R = C;
	B->L = D;
	C->L = E; C->R = F;
	E->R = G;
	F->L = H; F->R = J;

	print_bintree(A);

	return 0;
}
