/*
 * Algorithm 2.2.6S @ TAOCP::p.304
 *
 * author: Forrest Y. Yu <forrest.yu@gmail.com>, http://forrestyu.net/
 *
 * Build and Run:
 *        $ gcc -g -o pivot pivot.c
 *        $ ./pivot
 */

/*
 *  From:                             To:
 *            .           .                         .               . 	
 *            .           . 	                    .               . 	
 *            .           . 	                    .               . 	
 *    . . .   a   . . .   b   . . .         . . .  1/a    . . .    b/a  . . .	
 *            .           . 	                    .               . 	
 *            .           . 	      ==>           .               . 	
 *            .           . 	                    .               . 	
 *    . . .   c   . . .   d   . . .         . . . -c/a    . . .  d-bc/a . . .	
 *            .           . 	                    .               . 	
 *            .           . 	                    .               . 	
 *            .           . 	                    .               .         
 *                                                                       (13)
 *
 *  e.g.:
 *
 *     50      0      0      0                     -5      0   -100      0
 *     10      0     20      0        ==>         0.1      0      2      0
 *      0      0      0      0                      0      0      0      0
 *    -30      0    -60      5	                    3      0      0      5
 *
 */

/* 
 * Algorithm S (Pivot step in a sparse matrix). Given a matrix represented as
 * in Fig.14, we perform the pivot operation (13). Assume that PIVOT is a link
 * variable pointing to the pivot element. The algorithm makes use of an auxiliary
 * table of link variables PTR[j], one for each column of the matrix. The variable
 * ALPHA and the VAL field of each node are assumed to be floating point or rational
 * quantities, while everything else in this algorithm has integer values.
 * 
 * S1. [Initialize.] Set ALPHA <- 1.0/VAL(PIVOT), VAL(PIVOT) <- 1.0, and
 *                     I0 <- ROW(PIVOT),  P0 <- LOC(BASEROW[I0]);
 *                     J0 <- COL(PIVOT),  Q0 <- LOC(BASECOL[J0]).
 * 
 * S2. [Process pivot row.] Set P0 <- LEFT(P0), J <- COL(P0). If J < 0, go on
 *     to step S3 (the pivot row has been traversed). Otherwise set
 *     PTR[J] <- LOC(BASECOL[J]) and VAL(P0) <- ALPHA * VAL(P0), and repeat step S2.
 * 
 * S3. [Find new row.] Set Q0 <- UP(Q0). (The remainder of the algorithm deals
 *     successively with each row, from bottom to top, for which there is an entry
 *     in the pivot column.) Set I <- ROW(Q0). If I < 0, the algorithm terminates.
 *     If I = I0, repeat step S3 (we have already done the pivot row). Otherwise
 *     set P <- LOC(BASEROW[I]), P1 <- LEFT(P). (The pointers P and P1 will now
 *     proceed across row I from right to left, as P0 goes in synchronization across
 *     row I0; Algorithm 2.2.4A is analogous. We have P0 = LOC(BASEROW[I0])
 *     at this point.)
 * 
 * S4. [Find new column.] Set P0 <- LEFT(P0), J <- COL(P0). If J < 0, set
 *     VAL(Q0) <- -ALPHA * VAL(Q0) and return to S3. If J = J0, repeat step S4.
 *     (Thus we process the pivot column entry in row I after all other column
 *     entries have been processed; the reason is that VAL(Q0) is needed in step S7.)
 * 
 * S5. [Find I,J element.] If COL(P1) > J, set P <- P1, P1 <- LEFT(P), and repeat
 *     step S5. If COL(P1) = J, go to step S7. Otherwise go to step S6 (we need
 *     to insert a new element in column J of row I).
 * 
 * S6. [Insert I,J element.] If ROW(UP(PTR[J])) > I, set PTR[J] <- UP(PTR[J]),
 *     and repeat step S6. (Othewise, we ill have ROW(UP(PTR[J])) < I; the new
 *     element is to be inserted just above NODE(PTR[J]) in the vertical dimension,
 *     and just left of NODE(p) in the horizontal dimension.) Otherwise set X<= AVAIL,
 *     VAL(X) <- 0, ROW(X) <- I, COL(X) <- J, LEFT(X) <- P1, UP(X) <- UP(PTR[J]),
 *     LEFT(P) <- X, UP(PTR[J]) <- X, P1 <- X.
 * 
 * S7. [Pivot.] Set VAL(P1) <- VAL(P1) - VAL(Q0)xVAL(P0). If now VAL(P1) = 0,
 *     go to S8. (Note: When floating point arithmetic is being used, this test
 *     ``VAL(P!0 = 0'' should be replaced by ``|VAL(P1)| < EPSILON'' or better yet
 *     by the condition ``most of the significant figures of VAL(P1) were lost in the
 *     subtraction.'') Otherwise, set PTR[J] <- P1, P <- P1, P1 <- LEFT(P), and go
 *     back to S4.
 * 
 * S8. [Delete I,J element.] If UP(PTR[J]) != P1 (or, what is essentially the same
 *     thing, if ROW(UP(PTR[J])) > I), set PTR[J] <- UP(PTR[J]) and repeat
 *     step S8; otherwise, set UP(PTR[J]) <- UP(P1), LEFT(P) <- LEFT(P1),
 *     AVAIL <= P1, P1 <- LEFT(P). Go back to S4.
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>

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

#define ROW_NR		  4
#define COL_NR		  4
#define POOL_SIZE	 256
#define VERY_SMALL	 0.000001

/*  +--------+--------+
 *  |  LEFT  |   UP   |
 *  +-----+--+--+-----+
 *  | ROW | COL | VAL |
 *  +-----+-----+-----+
 */
struct node303 {
	struct node303 *	LEFT;
	struct node303 *	UP;
	int			ROW;
	int			COL;
	double			VAL;
};

struct node303 * alloc_node(void)
{
	static int i = 0;
	static struct node303 node_pool[POOL_SIZE];
	return &node_pool[i++];
}

void free_node(struct node303 * p)
{
	return;			/* do nothing */
}

int fzero(double x)
{
	return (fabs(x - 0.0) < VERY_SMALL);
}

void parse_matrix(const double a[],	/* input */
		  struct node303 baserow[], struct node303 basecol[] /* output */ )
{
	int i;
	int j;
	int idx;
	double v;
	struct node303 * p;

	for (i = 0; i < ROW_NR; i++) {
		for (j = 0; j < COL_NR; j++) {
			idx = i * COL_NR + j;
			v = a[idx];
			if (!fzero(v)) {
				p = alloc_node();
				p->VAL = v;
				p->ROW = i;
				p->COL = j;

				printf("adding (%d,%d) %.1f\n", i, j, v);

				p->LEFT = baserow[i].LEFT;
				baserow[i].LEFT = p;

				p->UP = basecol[j].UP;
				basecol[j].UP = p;

				assert(p);
				assert(p->LEFT);
				assert(p->UP);
			}
		}
	}
}

void cleanup(struct node303 baserow[], struct node303 basecol[])
{
}

void print_list(struct node303 baserow[], struct node303 basecol[])
{
	int i;
	struct node303 * p;

	for (i = 0; i < ROW_NR; i++) {
		p = &baserow[i];
		do {
			assert(p);
			printf("%s(%d,%d) %s%.1f%s --L--> ",
			       CYAN, p->ROW, p->COL,
			       RED, p->VAL,
			       NOCOLOR);
			p = p->LEFT;
		} while (p != &baserow[i]);
		printf("CIRCLE\n");
	}

	for (i = 0; i < COL_NR; i++) {
		p = &basecol[i];
		do {
			assert(p);
			printf("%s(%d,%d) %s%.1f%s --U--> ",
			       CYAN, p->ROW, p->COL,
			       RED, p->VAL,
			       NOCOLOR);
			p = p->UP;
		} while (p != &basecol[i]);
		printf("CIRCLE\n");
	}
}

void pivot_me(struct node303 baserow[], struct node303 basecol[],
	      int row, int col,
	      double omatrix[])
{
	int i, j;
	struct node303 * p;
	struct node303 * q;
	double a, b, c, d;
	double v;

	/* find matrix[row][col] */
	p = baserow[row].LEFT;
	while (p->COL > col)
		p = p->LEFT;
	assert(p->COL == col);
	/* a := matrix[row][col] */
	a = p->VAL;
	assert(a);

	for (i = 0; i < ROW_NR; i++) {
		p = baserow[i].LEFT;

		if (p == &baserow[i]) /* nothing in this row */
			continue;

		assert(p->ROW == i);

		for (j = 0; j < COL_NR; j++) {
			q = basecol[j].UP;

			if (q == &basecol[j]) /* nothing in this column */
				continue;

			assert(q->COL == j);

			/* find matrix[i][col] */
			p = baserow[i].LEFT;
			assert(p->ROW == i);
			while (p->COL > col)
				p= p->LEFT;
			/* c := matrix[i][col] */
			if (p->COL == col)
				c = p->VAL;
			else
				c = 0.0;

			/* find matrix[i][j] */
			p = baserow[i].LEFT;
			assert(p->ROW == i);
			while (p->COL > j)
				p = p->LEFT;
			/* d := matrix[i][j] */
			if (p->COL == j)
				d = p->VAL;
			else
				d = 0.0;

			/* find matrix[row][j] */
			while (q->ROW > row)
				q = q->UP;
			/* b := matrix[row][j] */
			if (q->ROW == row)
				b = q->VAL;
			else
				b = 0.0;

			if (row == i && col == j) {
				v = 1.0 / a;
				printf("%s(%d,%d) %s%.1f = 1.0 / %.1f%s\n",
				       YELLOW, i, j,
				       MAGENTA, v, a,
				       NOCOLOR);
			}
			else if (row == i) {
				v = b / a;
				printf("%s(%d,%d) %s%.1f = "
				       "%.1f / %.1f%s\n",
				       YELLOW, i, j,
				       MAGENTA, v, b, a,
				       NOCOLOR);
			}
			else if (col == j) {
				v = -c / a;
				printf("%s(%d,%d) %s%.1f = "
				       "-(%.1f) / %.1f%s\n",
				       YELLOW, i, j,
				       MAGENTA, v, c, a,
				       NOCOLOR);
			}
			else {
				v = d - b * c / a;
				printf("%s(%d,%d) %s%.1f = "
				       "%.1f - %.1f * %.1f / %.1f%s\n",
				       YELLOW, i, j,
				       MAGENTA, v, d, b, c, a,
				       NOCOLOR);
			}

			if (fzero(d) && !fzero(v)) {
				omatrix[i * COL_NR + j] = v;

				/* x = alloc_node(); */
				/* x->VAL = v; */
				/* x->ROW = i; */
				/* x->COL = j; */

				/* p = &baserow[i]; */
				/* while (p->LEFT->COL > j) */
				/* 	p = p->LEFT; */
				/* assert((p == &baserow[i]) || */
				/*        (p->COL > j && p->LEFT->COL < j)); */
				/* x->LEFT = p->LEFT; */
				/* p->LEFT = x; */

				/* q = &basecol[j]; */
				/* while (q->UP->ROW > j) */
				/* 	q = q->UP; */
				/* assert(q->ROW > j && q->UP->ROW < j); */
				/* x->UP = q->UP; */
				/* q->UP = x; */
			}
			else if (!fzero(d) && fzero(v)) {
				;

				/* p = &baserow[i]; */
				/* while (p->LEFT->COL != j) */
				/* 	p = p->LEFT; */
				/* assert(p->LEFT->ROW == i && p->LEFT->COL == j); */
				/* x = p->LEFT; */
				/* p->LEFT = p->LEFT->LEFT; */
				/* free_node(x); */
			}
			else if (!fzero(v)) {
				omatrix[i * COL_NR + j] = v;

				/* p = &baserow[i]; */
				/* while (p->LEFT->COL != j) */
				/* 	p = p->LEFT; */
				/* assert(p->LEFT->ROW == i && p->LEFT->COL == j); */
				/* x = p->LEFT; */
				/* x->VAL = v; */
			}
		}
	}
}

void pivot_taocp(struct node303 baserow[], struct node303 basecol[], struct node303 * PIVOT)
{
	/* 
	 * Algorithm S (Pivot step in a sparse matrix). Given a matrix represented as
	 * in Fig.14, we perform the pivot operation (13). Assume that PIVOT is a link
	 * variable pointing to the pivot element. The algorithm makes use of an auxiliary
	 * table of link variables PTR[J], one for each column of the matrix. The variable
	 * ALPHA and the VAL field of each node are assumed to be floating point or rational
	 * quantities, while everything else in this algorithm has integer values.
	 * 
	 */

	int			I;
	int			J;
	struct node303 *	PTR[COL_NR];
	struct node303 *	P;
	struct node303 *	P1;

	/* S1. [Initialize.] Set ALPHA <- 1.0/VAL(PIVOT), VAL(PIVOT) <- 1.0, and
	 *                     I0 <- ROW(PIVOT),  P0 <- LOC(BASEROW[I0]);
	 *                     J0 <- COL(PIVOT),  Q0 <- LOC(BASECOL[J0]).
	 */
	printf("%s-----S1-----%s\n", GREEN, NOCOLOR);
	const double ALPHA = 1.0 / PIVOT->VAL;
	const int I0 = PIVOT->ROW;
	const int J0 = PIVOT->COL;
	struct node303 * P0 = &baserow[I0];
	struct node303 * Q0 = &basecol[J0];
	PIVOT->VAL = 1.0;

	/* S2. [Process pivot row.] Set P0 <- LEFT(P0), J <- COL(P0). If J < 0, go on
	 *     to step S3 (the pivot row has been traversed). Otherwise set
	 *
	 *     PTR[J] <- LOC(BASECOL[J]) and VAL(P0) <- ALPHA * VAL(P0), and repeat step S2.
	 */
	printf("%s-----S2-----%s\n", GREEN, NOCOLOR);
	while (1) {
		P0 = P0->LEFT;
		J = P0->COL;
		if (J < 0)
			break;
		PTR[J] = &basecol[J];
		P0->VAL *= ALPHA;
	}
	/* printf("After S2:\n"); */
	/* print_list(baserow, basecol); */

	while (1) {
		/* S3. [Find new row.] Set Q0 <- UP(Q0). (The remainder of the algorithm deals
		 *     successively with each row, from bottom to top, for which there is an entry
		 *     in the pivot column.) Set I <- ROW(Q0). If I < 0, the algorithm terminates.
		 *     If I = I0, repeat step S3 (we have already done the pivot row). Otherwise
		 *     set P <- LOC(BASEROW[I]), P1 <- LEFT(P). (The pointers P and P1 will now
		 *     proceed across row I from right to left, as P0 goes in synchronization across
		 *     row I0; Algorithm 2.2.4A is analogous. We have P0 = LOC(BASEROW[I0])
		 *     at this point.)
		 */
		printf("%s-----S3-----%s\n", GREEN, NOCOLOR);
		Q0 = Q0->UP;
		I = Q0->ROW;	/* I: current row */
		if (I < 0) {
			printf("\ndone.\n"); /* <<<<<<<<<<<<<<<<<<<< THE END >>>>>>>>>>>>>>>>>>>> */
			return;
		}
		if (I == I0)
			continue;
		P = &baserow[I]; /* I: P P1 P1 P1 P1 */
		P1 = P->LEFT;

		while (1) {
			/* S4. [Find new column.] Set P0 <- LEFT(P0), J <- COL(P0). If J < 0, set
			 *     VAL(Q0) <- -ALPHA * VAL(Q0) and return to S3. If J = J0, repeat step S4.
			 *     (Thus we process the pivot column entry in row I after all other column
			 *     entries have been processed; the reason is that VAL(Q0) is needed in step S7.)
			 * 
			 */
			printf("%s-----S4-----%s\n", GREEN, NOCOLOR);
			P0 = P0->LEFT;
			J = P0->COL;

			if (J < 0) {
				Q0->VAL *= -ALPHA;
				break;		   /* goto S3 */
			}

			if (J == J0)
				continue;

			/* S5. [Find I,J element.] If COL(P1) > J, set P <- P1, P1 <- LEFT(P), and repeat
			 *     step S5. If COL(P1) = J, go to step S7. Otherwise go to step S6 (we need
			 *     to insert a new element in column J of row I).
			 */
			printf("%s-----S5-----%s\n", GREEN, NOCOLOR);
			while (P1->COL > J) {
				P = P1;
				P1 = P->LEFT;
			}
			if (P1->COL != J) {
				/* S6. [Insert I,J element.] If ROW(UP(PTR[J])) > I, set PTR[J] <- UP(PTR[J]),
				 *     and repeat step S6. (Othewise, we will have ROW(UP(PTR[J])) < I; the new
				 *     element is to be inserted just above NODE(PTR[J]) in the vertical dimension,
				 *     and just left of NODE(p) in the horizontal dimension.) Otherwise set X<= AVAIL,
				 *     VAL(X) <- 0, ROW(X) <- I, COL(X) <- J, LEFT(X) <- P1, UP(X) <- UP(PTR[J]),
				 *     LEFT(P) <- X, UP(PTR[J]) <- X, P1 <- X.
				 * 
				 */
				printf("%s-----S6-----%s\n", GREEN, NOCOLOR);
				while (PTR[J]->UP->ROW > I)
					PTR[J] = PTR[J]->UP;
				struct node303 * X = alloc_node();
				X->VAL = 0.0;
				X->ROW = I;
				X->COL = J;
				X->LEFT = P1;
				X->UP = PTR[J]->UP;
				P->LEFT = X;
				PTR[J]->UP = X;
				P1 = X;
			}
			assert(P1->COL == J);

			/* S7. [Pivot.] Set VAL(P1) <- VAL(P1) - VAL(Q0) * VAL(P0). If now VAL(P1) = 0,
			 *     go to S8. (Note: When floating point arithmetic is being used, this test
			 *     ``VAL(P1) = 0'' should be replaced by ``|VAL(P1)| < EPSILON'' or better yet
			 *     by the condition ``most of the significant figures of VAL(P1) were lost in the
			 *     subtraction.'') Otherwise, set PTR[J] <- P1, P <- P1, P1 <- LEFT(P), and go
			 *     back to S4.
			 */
			printf("%s-----S7-----%s\n", GREEN, NOCOLOR);
			P1->VAL -= Q0->VAL * P0->VAL;
			if (fzero(P1->VAL)) {
				/* S8. [Delete I,J element.] If UP(PTR[J]) != P1 (or, what is essentially the same
				 *     thing, if ROW(UP(PTR[J])) > I), set PTR[J] <- UP(PTR[J]) and repeat
				 *     step S8; otherwise, set UP(PTR[J]) <- UP(P1), LEFT(P) <- LEFT(P1),
				 *     AVAIL <= P1, P1 <- LEFT(P). Go back to S4.
				 */
				printf("%s-----S8-----%s\n", GREEN, NOCOLOR);
				while (PTR[J]->UP != P1) {
					assert(PTR[J]->UP->ROW > I);
					PTR[J] = PTR[J]->UP;
				}
				PTR[J]->UP = P1->UP;
				P->LEFT = P1->LEFT;
				free_node(P1);
				P1 = P->LEFT;
			} else {
				PTR[J] = P1;
				P = P1;
				P1 = P->LEFT;
			} /* S7 ~ S8 */
		} /* S4 ~ S8 */
	} /* S3 ~ S8 */
}

int main(void)
{
	int i;

	struct node303 BASEROW[ROW_NR];
	struct node303 BASECOL[COL_NR];

	const double matrix[ROW_NR * COL_NR] = {
		 50,     0,     0,     0,
		 10,     0,    20,     0,
		  0,     0,     0,     0,
		-30,     0,   -60,     5,
	};

	struct node303 out_BASEROW[ROW_NR];
	struct node303 out_BASECOL[COL_NR];
#ifdef STUPID_ALGORITHM
	double out_matrix[ROW_NR * COL_NR] = {
		 0.0,     0.0,     0.0,     0.0,
		 0.0,     0.0,     0.0,     0.0,
		 0.0,     0.0,     0.0,     0.0,
		 0.0,     0.0,     0.0,     0.0,
	};
#endif

	for (i = 0; i < ROW_NR; i++) {
		BASEROW[i].LEFT = &BASEROW[i];
		BASEROW[i].UP   = 0;
		BASEROW[i].ROW  = 0;
		BASEROW[i].COL  = -1;
		BASEROW[i].VAL  = 0.0;

#ifdef STUPID_ALGORITHM
		out_BASEROW[i].LEFT = &out_BASEROW[i];
		out_BASEROW[i].UP   = 0;
		out_BASEROW[i].ROW  = 0;
		out_BASEROW[i].COL  = -1;
		out_BASEROW[i].VAL  = 0.0;
#endif
	}

	for (i = 0; i < COL_NR; i++) {
		BASECOL[i].LEFT = 0;
		BASECOL[i].UP   = &BASECOL[i];
		BASECOL[i].ROW  = -1;
		BASECOL[i].COL  = 0;
		BASECOL[i].VAL  = 0.0;

#ifdef STUPID_ALGORITHM
		out_BASECOL[i].LEFT = 0;
		out_BASECOL[i].UP   = &out_BASECOL[i];
		out_BASECOL[i].ROW  = -1;
		out_BASECOL[i].COL  = 0;
		out_BASECOL[i].VAL  = 0.0;
#endif
	}

	parse_matrix(matrix, BASEROW, BASECOL);
	print_list(BASEROW, BASECOL);

#ifdef STUPID_ALGORITHM
	pivot_me(BASEROW, BASECOL, 1, 0, out_matrix);
	printf("%s********** pivot **********%s\n", GREEN, NOCOLOR);

	parse_matrix(out_matrix, out_BASEROW, out_BASECOL);
	print_list(out_BASEROW, out_BASECOL);
#else
	int pivot_i = 1;
	int pivot_j = 0;
	struct node303 * p = BASEROW[pivot_i].LEFT;
	for (; p > 0; p = p->LEFT)
		if (p->COL == pivot_j)
			break;
	printf("PIVOT: "
	       "%s(%d,%d) %s%.1f%s\n",
	       CYAN, p->ROW, p->COL,
	       RED, p->VAL,
	       NOCOLOR);
	pivot_taocp(BASEROW, BASECOL, p);
	print_list(BASEROW, BASECOL);
#endif
	
	cleanup(BASEROW, BASECOL);
	cleanup(out_BASEROW, out_BASECOL);

	return 0;
}
