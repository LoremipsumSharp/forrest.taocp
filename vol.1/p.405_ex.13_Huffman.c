/*
 * Exercise 2.3.4.5-13 @ TAOCP::p.405
 * Answer to Exercise 2.3.4.5-13 @ TAOCP::p.596
 *
 * author: Forrest Y. Yu <forrest.yu@gmail.com>, http://forrestyu.net/
 *
 * Build:
 *     $ gcc -std=c99 -g -Wall -o foo p.405_ex.13_Huffman.c
 * Run:
 *     $ ./foo
 */

#include <stdio.h>
#include <assert.h>

/******************************************************************************
 * configuration
 ******************************************************************************/
#define ANSWER_P596
#define k_COLOR		WHITE
#define A_COLOR		CYAN
#define L_COLOR		CYAN
#define R_COLOR		CYAN

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

/******************************************************************************
 * define
 ******************************************************************************/
#define INFINITY	99999
/* input */
#define m 13
/* const int weights[m+1] = {0, 2, 3, 4, 11}; */
/* const int weights[m+1] = {-INFINITY, -11, -4, -3, -2}; */
const int weights[] = {0, 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41};

void print_all(const int A[], const int L[], const int R[])
{
	const char format_str[] = "%4d";
	int k;

	printf("  k :%s", k_COLOR);
	for (k = 0; k < 2*m; k++)
		printf(format_str, k);
	printf("%s\n", NOCOLOR);

	printf("A[k]:%s", A_COLOR);
	for (k = 0; k < 2*m; k++)
		printf(format_str, A[k]);
	printf("%s\n", NOCOLOR);

	printf("L[k]:%s", L_COLOR);
	for (k = 0; k < m; k++)
		printf(format_str, L[k]);
	printf("%s\n", NOCOLOR);

	printf("R[k]:%s", R_COLOR);
	for (k = 0; k < m; k++)
		printf(format_str, R[k]);
	printf("%s\n\n", NOCOLOR);
}

void prepare(const int w[], int A[], int L[], int R[])
{
	int i;

	for (i = 0; i < 2*m; i++)
		A[i] = 0;

	for (i = 1; i <= m; i++) {
		if (i)
			assert(w[i] >= w[i-1]);
		A[m+i-1] = w[i]; /* A[m..2m-1] ← w[1..m] */
	}

	for (i = 0; i < m; i++) {
		L[i] = 0;
		R[i] = 0;
	}

	/* print_all(A, L, R); */
}

/* from TAOCP p.596 */
void the_answer(void)
{
	int A[2*m+1];
	int L[m];
	int R[m];
	int i, j, k, x, y;
	const int * w = weights;
	prepare(w, A, L, R);

	/* [See J. van Leeuwen, Proc. 3rd International Colloq. Automata, Languages and
	 * Programming (Edinburgh University Press, 1976), 382-410.]
	 */
	/* H1. [Initialize.] Set A[m-1+i] ← w[i] for 1 <= i <= m. Then set A[2m] ← ∞,
	 *     x ← m, i ← m+1, j ← m-1, k ← m. (During this algorithm A[i] <= ... <= A[2m-1]
	 *     is the queue of unused external weights; A[k] >= ... >= A[j] is the
	 *     queue of unused internal weights, empty if j < k; the current left and right
	 *     pointers are x and y.)
	 */
	/* index:        0       1     ...     m-1       m     m+1     ...    2m-1      2m
	 *
	 *                                       j       x       i
	 *                                               k
	 *
	 * external node weights:                       |m--------------------2m-1|
	 * queue of unused external weights:                    |i------------2m-1|
	 * queue of unused internal weights:    |j-------k|
	 */
	for (i = 1; i <= m; i++)
		A[m-1+i] = w[i]; /* A[m..2m-1] ← w[1..m] */
	A[2*m] = INFINITY;
	x = m;
	i = m + 1;
	j = m - 1;
	k = m;

H2:
	/* H2. [Find right pointer.] If j < k or A[i] <= A[j], set y ← i and i ← i+1;
	 *     otherwise set y ← j and j ← j-1
	 */
	if (j < k ||		/* internal_weights_queue is empty */
	    A[i] <= A[j]) {	/* external_weights_queue[head] is smaller */
		y = i;		/* y ⇐ external_weights_queue */
		i++;
	} else {
		y = j;		/* y ⇐ internal_weights_queue */
		j--;
	}

	/* H3. [Create internal node.] Set k ← k-1, L[k] ← x, R[k] ← y, A[k] ← A[x] + A[y].
	 */
	k--;
	L[k] = x;
	R[k] = y;
	A[k] = A[x] + A[y];

	/* H4. [Done?] Terminate the algorithm if k = 1.
	 */
	if (k == 1)
		goto END;

	/* H5. [Find left pointer.] (At this point j >= k and the queues contain a total of k
	 *     unused weights. If A[y] < 0 we have j = k, i = y+1, and A[i] > A[j].) If
	 *     A[i] <= A[j], set x ← i and i ← i+1; otherwise set x ← j and j ← j-1.
	 *     Return to step H2.
	 */
	assert(j >= k);
	if (A[y] < 0) {
		assert(j == k);
		assert(i == y + 1);
		assert(A[i] > A[j]);
	}
	if (A[i] <= A[j]) {	/* external_weights_queue[head] is smaller */
		x = i;		/* x ⇐ external_weights_queue */
		i++;
	} else {
		x = j;		/* x ⇐ internal_weights_queue */
		j--;
	}
	goto H2;
END:
	print_all(A, L, R);
	printf("%s[END of ANSWER]%s\n\n", RED, NOCOLOR);
}

void my_algorithm(void)
{
	/* output */
	int A[2*m];
	int L[m];
	int R[m];

	/* queue */
	int Q[m];
	int Qhead = 0;
	int Qtail = 0;

	/* prepare */
	assert(sizeof(weights) / sizeof(weights[0]) == m + 1);
	prepare(weights, A, L, R);
	for(int j = 0; j < m; j++)
		Q[j] = 0;

	/* go */
	int i = m;		/* i: m..2m-1 */
	int a = m - 1;		/* a: m-1..1 */
	int u[2];
	while (a) {
		int k;
		for (k = 0; k < 2; k++) {
			/* printf("i:%d, A[i]:%d, Qtail:%d, Qhead:%d, Q[Qhead]:%d\n", */
			/*        i, A[i], Qtail, Qhead, Q[Qhead]); */
			assert(Qhead <= Qtail);
			if (Qhead == Qtail) { /* internal_w_queue is empty */
				assert(i < 2*m);
				u[k] = i++;
			} else if (i > 2*m-1) { /* external_w_queue is empty */
				assert(Qhead != Qtail);
				u[k] = Q[Qhead++];
			} else { /* neither of the queues is empty */
				if (A[i] < A[Q[Qhead]])
					u[k] = i++;
				else
					u[k] = Q[Qhead++];
			}
			assert(Qhead < m && Qtail < m);
			printf("A[u[%d]]:A[%d]:%d\n", k, u[k], A[u[k]]);
		}
		A[a] = A[u[0]] + A[u[1]];
		L[a] = u[0];
		R[a] = u[1];
		if (a > 1)
			Q[Qtail++] = a;
		assert(Qhead < m && Qtail < m);
		a--;
	}
	print_all(A, L, R);
	assert(Qhead == Qtail);
}

int main(void)
{
	the_answer();
	my_algorithm();
	return 0;
}
