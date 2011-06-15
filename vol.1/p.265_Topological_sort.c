/*
 * Algorithm 2.2.3T @ TAOCP::p.265
 *
 * author: Forrest Y. Yu <forrest.yu@gmail.com>, http://forrestyu.net/
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* if no more X pairs are acceptable, then LIMIT=(X+2)*2 */
#define LIMIT		24	/* X=10 */
#define NODE_POOL_SIZE	16

struct Node {
	unsigned int	SUC;
	struct Node *	NEXT;
};

union CQ {
	unsigned int	COUNT;
	unsigned int	QLINK;
};

struct Node * AllocNode()
{
	static struct Node NodePool[NODE_POOL_SIZE];
	static int pos = 0;
	assert(pos < NODE_POOL_SIZE);
	return &NodePool[pos++];
}

/* TERMINOLOGY
 *
 * TAOCP	this C code	comments
 * --------	-----------	--------
 * COUNT[k]	cq[k].COUNT	k has how many precedes
 * QLINK[k]	cq[k].QLINK
 * TOP[k]	TOP[k]		succeeds of k
 * SUC(P)	P->SUC
 * NEXT(P)	P->NEXT
 */
void Algorithm_T(const unsigned int topo_input[])
{
	int i;
	int n, N;
	int j,k;
	union CQ * cq = 0;
	struct Node ** TOP = 0;

	assert(topo_input[0] == 0);
	n = topo_input[1];

	/* position 0 is reserved, so (n + 1) elements are allocated */
	cq  = (union CQ *)malloc(sizeof(union CQ) * (n + 1));
	TOP = (struct Node **)malloc(sizeof(struct Node *) * (n + 1));

	/***** T1 *****/
	for (i = 0; i <= n; i++) {
		cq[i].COUNT = 0;
		assert(cq[i].QLINK == 0);
		TOP[i] = 0;
	}
	N = n;

	printf("Input:  ");
	for (i = 1; i < LIMIT; i++) {
		/***** T2 *****/
		/* j ≺ k */
		j = topo_input[i*2];
		k = topo_input[i*2+1];

		/* (0,0): THE END */
		if (j == 0) {
			assert(k == 0);
			break;
		}

		/* output the (j,k) pair */
		if (i > 1)
			printf(", ");
		printf("%d ≺ %d", j, k);

		/***** T3 *****/
		/* k has one more precede */
		cq[k].COUNT++;

		/* j has one more succeed */
		struct Node * P = AllocNode();
		P->SUC = k;
		P->NEXT = TOP[j];
		TOP[j] = P;
	}
	printf(".  (TAOCP p.264 (18))\n");

	/***** T4 *****/
	int R = 0;
	cq[0].QLINK = 0;
	for (k = 1; k <= n; k++) {
		if (cq[k].COUNT == 0) {
			cq[R].QLINK = k;
			R = k;
		}
	}
	int F = cq[0].QLINK;

	printf("Output: ");
	while (F) {
		/***** T5 *****/
		if (N != n)
			printf(", ");
		printf("%d", F);
		N--;

		struct Node * P = TOP[F];
		/***** T6 *****/
		while (P) {
			if (--cq[P->SUC].COUNT == 0) {
				cq[R].QLINK = P->SUC;
				R = P->SUC;
			}
			P = P->NEXT;
		}

		/***** T7 *****/
		F = cq[F].QLINK;
	}
	printf(".  (≡ p.263 Fig.7)\n");

	/***** T8 *****/
	assert(N == 0);

	free(cq);
	free(TOP);

	return;
}

int main()
{
	const unsigned int input[LIMIT] = {0, 9, /* (0,n) */
					   9, 2,
					   3, 7,
					   7, 5,
					   5, 8,
					   8, 6,
					   4, 6,
					   1, 3,
					   7, 4,
					   9, 5,
					   2, 8,
					   0, 0}; /* (0,0) terminates the input */

	Algorithm_T(input);

	return 0;
}

