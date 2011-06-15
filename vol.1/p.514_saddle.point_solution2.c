/*
 * Answer to Exercise 1.3.2-10 @ TAOCP::p.514
 *
 * author: Forrest Y. Yu <forrest.yu@gmail.com>, http://forrestyu.net/
 */

#include <stdio.h>
#include <stdlib.h>

/*
 * SOLUTION 2
 *
 *        CMAX[1] CMAX[2] CMAX[3] CMAX[4]  CMAX[5] CMAX[6] CMAX[7] CMAX[8]
 *    a₀₀      a₀₁      a₀₂      a₀₃      a₀₄      a₀₅      a₀₆      a₀₇      a₀₈(a₁₀)
 *            a₁₁      a₁₂      a₁₃      a₁₄      a₁₅      a₁₆      a₁₇      a₁₈
 *            a₂₁      a₂₂      a₂₃      a₂₄      a₂₅      a₂₆      a₂₇      a₂₈
 *            a₃₁      a₃₂      a₃₃      a₃₄      a₃₅      a₃₆      a₃₇      a₃₈
 *            a₄₁      a₄₂      a₄₃      a₄₄      a₄₅      a₄₆      a₄₇      a₄₈
 *            a₅₁      a₅₂      a₅₃      a₅₄      a₅₅      a₅₆      a₅₇      a₅₈
 *            a₆₁      a₆₂      a₆₃      a₆₄      a₆₅      a₆₆      a₆₇      a₆₈
 *            a₇₁      a₇₂      a₇₃      a₇₄      a₇₅      a₇₆      a₇₇      a₇₈
 *            a₈₁      a₈₂      a₈₃      a₈₄      a₈₅      a₈₆      a₈₇      a₈₈
 *            a₉₁      a₉₂      a₉₃      a₉₄      a₉₅      a₉₆      a₉₇      a₉₈
 *
 */

#define ROW_NR 9
#define COL_NR 8

void init_matrix(int * A)
{
	int i;
	A[0] = 1016;
 	A[1] = 1010;
	for (i = 2; i <= ROW_NR*COL_NR; i++) {
		srand(A[i-1] + A[i-2]);
		A[i] = rand() % 100 + 1000;
	}
}

int main(void)
{
	int coli;
	int answer = 0;
	int idx;
	int rowi;
	int colj;
	int rA, rX;

	int CMAX[ROW_NR + ROW_NR * COL_NR];
	int * A10 = CMAX + COL_NR;
	int i, j;

	init_matrix(A10);

	printf("==================================================\n");
	for (i = 0; i < ROW_NR; i++) {
		for (j = 0; j < COL_NR; j++) {
			printf("%4d  ", A10[i*COL_NR+j]);
		}
		printf("\n");
	}
	printf("==================================================\n");

	coli = COL_NR;
	for (; coli > 0; coli--) {		/* traverse the columns */
		idx = ROW_NR * COL_NR - COL_NR + coli - 1;
		rX = 0;
		for (; idx > 0; idx -= COL_NR) {
			if (A10[idx] > rX)
				rX = A10[idx];	/* rX : max in column */
		}
		CMAX[idx+COL_NR] = rX;
		if ((idx == -1) ||		/* -COL_NR <= idx <= -1 */
		    (CMAX[idx+COL_NR] < rA)) {
			rA = CMAX[idx+COL_NR];	/* rA : min of CMAX[1..COL_NR] */
			printf("* CMAX[%d]: %d\n", coli - 1, rX);
		}
		else {
			printf("  CMAX[%d]: %d\n", coli - 1, rX);
		}
	}
					
	rowi = ROW_NR * COL_NR - COL_NR;	/* rowi: the 1st element of every column */
	for (; rowi > 0; rowi -= COL_NR) {
		printf("traversing row %d\n", rowi / COL_NR);
		idx = rowi + COL_NR - 1;
		colj = COL_NR;
		for(; colj > 0; colj--,idx--) {	/* traverse elements in a row */
			printf("\t\ttraversing column %d\n", colj);
			printf("\t\t\t\t%d    A10[%d] (%d)\n", rA, idx, A10[idx]);
			if (rA > A10[idx]) {	/* rA is supposed to be the min of current row*/
				printf("\t\t\t\t     >      NO\n");
				break;
			}
			else if (rA < A10[idx]) {
				printf("\t\t\t\t     <      \n");
				continue;
			}
			else {  /* rA == A10[idx] */
				printf("\t\t\t\t     ==     \n");
				printf("\t\t\t\t%d    CMAX[%d] (%d)\n",
				       rA, colj - 1, CMAX[colj - 1]);
				if (rA == CMAX[colj - 1]) {
					answer = A10[idx];
					printf("\t\t\t\tpossible answer: %d (%d,%d)\n",
					       answer, rowi / COL_NR, colj - 1);
				}
			}
		}
		if (colj == 0) {
			printf("saddle point found: %d\n", answer);
			return 0;
		}
	}

	answer = 0;
	printf("no saddle point found (%d).\n", answer);

	return 0;
}
