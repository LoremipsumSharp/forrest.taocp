/*
 * Algorithm 2.2.4A @ TAOCP::p.276
 * Algorithm 2.2.4M @ TAOCP::p.277
 *
 * author: Forrest Y. Yu <forrest.yu@gmail.com>, http://forrestyu.net/
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

/*
 * Macroes
 */
#define POOL_SIZE	32
#define ABC(t)		((t)->sign * ((t)->A << 8 | (t)->B << 4 | (t)->C))
#define ENABLE_COLOR(x)	{putchar(033); printf("[%sm", x);}
#define DISABLE_COLOR()	{putchar(033); printf("[0m");}

/*
 * Data Structure
 */
struct PTerm {			/* polynomial term */
	int		coef;	/* coefficient */
	int		sign;	/* 1, -1 */
	int		A;	/* exponent of x */
	int		B;	/* exponent of y */
	int		C;	/* exponent of z */
	struct PTerm *	LINK;
};

/*
 * Memory Management
 */
struct PTerm * Alloc_PTerm(int coef,
			   int sign, int A, int B, int C,
			   struct PTerm * LINK)
{
	static struct PTerm PT_Pool[POOL_SIZE];
	static int pos = 0;
	struct PTerm * t =  &PT_Pool[pos++];
	t->coef = coef;
	t->sign = sign;
	t->A = A;
	t->B = B;
	t->C = C;
	t->LINK = LINK;
	return t;
}

void Free_PTerm(struct PTerm * t)
{
	return;			/* temporarily do nothing */
}

/*
 * print a PTerm struct literally
 */
void print_PTerm(struct PTerm *t)
{
	assert(t);
	printf("-------------------- %ph\n", t);
	printf("%19d\n",  t->coef);
	printf("%2d",     t->sign);
	printf("%3d",     t->A);
	printf("%2d",     t->B);
	printf("%2d",     t->C);
	printf("%9ph\n",  t->LINK);
}

/*
 * print a circular list, in which every node is a PTerm struct
 */
void print_polynomial(struct PTerm *PTR)
{
	struct PTerm *t;
	print_PTerm(PTR);
	for (t = PTR->LINK; t != PTR; t = t->LINK)
		print_PTerm(t);
	printf("==================================================\n");
}

/*
 * check the validity of the circular list
 */
void assert_polynomial_valid(struct PTerm *PTR)
{
	struct PTerm *t;
	assert(PTR->coef == 0   &&
	       PTR->sign == -1  &&
	       PTR->A    == 0   &&
	       PTR->B    == 0   &&
	       PTR->C    == 1);
	for (t = PTR->LINK; t != PTR; t = t->LINK) {
		assert(t->sign == 1 || t->sign == -1);
		assert(t->coef != 0);
		assert(t->LINK);

		if (!(ABC(t) > ABC(t->LINK))) {
			printf("ABC(t): %d, ABC(t->LINK): %d\n",
			       ABC(t), ABC(t->LINK));
			print_PTerm(t);
			print_PTerm(t->LINK);
		}
		assert(ABC(t) > ABC(t->LINK));
	}
}

/*
 * given a polynomial string, returns a circular list
 * input example:
 *     x^4+2x^3y+3x^2y^2+4xy^3+5y^4+7
 */
struct PTerm * str2polynomial(const char *s)
{
	int n;
	int * e;
	enum NUM_TYPE {COEFFICIENT, EXPONENT} nt;

	const char * p = s;
	struct PTerm *PTR = Alloc_PTerm(0, -1, 0, 0, 1, 0);
	struct PTerm *t = PTR;
	
	while (*p) {
		if (*p != '-') { /* for the invisible leading '+' */
			t->LINK = Alloc_PTerm(0, 1, 0, 0, 0, 0);
			t = t->LINK;
			t->coef = 1;
		}

		e = 0;
		nt = COEFFICIENT;

		while (*p) {
			switch (*p) {
			case 'x':
				if (p[1] == '^')
					e = &t->A;
				else
					t->A = 1;
				break;
			case 'y':
				if (p[1] == '^')
					e = &t->B;
				else
					t->B = 1;
				break;
			case 'z':
				if (p[1] == '^')
					e = &t->C;
				else
					t->C = 1;
				break;
			case '+':
			case '-':
				/* '+' or '-' is the beginning of a new term */
				t->LINK = Alloc_PTerm(0, 1, 0, 0, 0, 0);
				t = t->LINK;
				nt = COEFFICIENT;
				t->coef = (*p == '+' ? 1 : -1);
				break;
			case '^':
				nt = EXPONENT;
				break;
			default:
				assert(*p >= '0' && *p <= '9');

				n = 0;
				while (*p && *p >= '0' && *p <= '9')
					n = n * 10 + (*p++ - '0');
				p--; /* this is important */

				if (nt == COEFFICIENT) {
					assert(t->coef == 1 || t->coef == -1);
					t->coef *= n;
				}
				else {
					assert(nt == EXPONENT);
					assert(e);
					*e = n;
				}

				break;
			}

			assert(p);
			p++;
		}
	}
	t->LINK = PTR;

	assert_polynomial_valid(PTR);

	return PTR;
}

/*
 * given a circular list, returns a polynomial string
 * output example:
 *     x⁴+2x³y+3x²y²+4xy³+5y⁴+7
 */
char * polynomial2str(char * ps, struct PTerm *PTR)
{
	int i;
	char * s = ps;
	char * e[] = {"⁰", "¹", "²", "³", "⁴", "⁵", "⁶", "⁷", "⁸", "⁹"};
	struct PTerm *t;
	for (t = PTR->LINK; t != PTR; t = t->LINK) {
		assert(t->coef != 0);

		if ((t->coef > 0) && (s != ps))
			*s++ = '+';

		if (t->coef == -1)
			*s++ = '-';
		else if (t->coef != 1)
			s += sprintf(s, "%d", t->coef);

		int xyz[3];
		xyz[0] = t->A;
		xyz[1] = t->B;
		xyz[2] = t->C;
		for (i = 0; i < 3; i++) {
			if (xyz[i] != 0) {
				*s++ = 'x' + i;
				if (xyz[i] > 1) {
					int l = strlen(e[xyz[i]]);
					memcpy(s, e[xyz[i]], l);
					s += l;
				}
			}
		}
	}
	*s = 0;
	return ps;
}

/*
 * Algorithm 2.2.4A @ p.276
 */
void add_polynomials(struct PTerm *P, struct PTerm *Q)
{
	struct PTerm *Q1;

	/* A1 */
	P  = P->LINK;
	Q1 = Q;
	Q  = Q->LINK;

	while (1) {
		if (ABC(P) < ABC(Q)) { /* A2 */
			printf("(%x < %x) ", ABC(P), ABC(Q));

			Q1 = Q;
			Q  = Q->LINK;
		} else if (ABC(P) == ABC(Q)) { /* A3 */
			printf("(%x == %x) ", ABC(P), ABC(Q));

			if (ABC(P) < 0)
				break;

			Q->coef += P->coef;

			if (Q->coef == 0) {
				/* A4.[Delete zero term.] */
				struct PTerm * Q2 = Q;
				Q = Q->LINK;
				Q1->LINK = Q;
				Free_PTerm(Q2);
				P = P->LINK;
			} else {
				P = P->LINK;
				Q1 = Q;
				Q = Q->LINK;
			}

		} else {/* ABC(P) > ABC(Q) */ /* A5 */
			printf("(%x > %x) ", ABC(P), ABC(Q));

			struct PTerm * Q2 = Alloc_PTerm(P->coef,
							P->sign,
							P->A, P->B, P->C,
							Q);
			Q1->LINK = Q2;
			Q1 = Q2;
			P = P->LINK;
		}
	}
	printf("\n");
}

/*
 * Algorithm 2.2.4M @ p.277
 */
void mul_polynomials(struct PTerm *P, struct PTerm *M, struct PTerm *Q)
{
	struct PTerm *Q1;

	while (1) {
		/* M1 */
		M = M->LINK;
		if (ABC(M) < 0)
			break;

		assert(ABC(P) < 0 && ABC(Q) < 0);

		/* M2 (a slightly modified add_polynomials()) */

		/* A1 */
		P  = P->LINK;
		Q1 = Q;
		Q  = Q->LINK;

		while (1) {
			int ABC_P = ABC(P) < 0 ? -1 : ABC(P) + ABC(M);
			int ABC_Q = ABC(Q);

			if (ABC_P < ABC_Q) { /* A2 */
				printf("(%x < %x) ", ABC_P, ABC_Q);

				Q1 = Q;
				Q  = Q->LINK;
			} else if (ABC_P == ABC_Q) { /* A3 */
				printf("(%x == %x) ", ABC_P, ABC_Q);

				if (ABC_P < 0)
					break;

				Q->coef += P->coef * M->coef;

				if (Q->coef == 0) {
					/* A4.[Delete zero term.] */
					struct PTerm * Q2 = Q;
					Q = Q->LINK;
					Q1->LINK = Q;
					Free_PTerm(Q2);
					P = P->LINK;
				} else {
					P = P->LINK;
					Q1 = Q;
					Q = Q->LINK;
				}

			} else {/* ABC_P > ABC_Q */ /* A5 */
				printf("(%x > %x) ", ABC_P, ABC_Q);

				assert(P->sign > 0 && M->sign >0);
				struct PTerm * Q2;
				Q2 = Alloc_PTerm(P->coef * M->coef,
						 P->sign,
						 P->A + M->A,
						 P->B + M->B,
						 P->C + M->C,
						 Q);
				Q1->LINK = Q2;
				Q1 = Q2;
				P = P->LINK;
			}
		}
	}
	printf("\n");
}

void test_add_1()
{
	char s1[128] = "";
	char s2[128] = "";

	/* add
	 *     (x⁴ + 2x³y + 3x²y² + 4xy³ + 5y⁴ + 7)
	 * to
	 *     (-x⁴ + x² - 2xy + y²)
	 */
	const char sP[] = "x^4+2x^3y+3x^2y^2+4xy^3+3xy+5y^4+7";
	const char sQ[] = "-x^4+x^2-2xy+y^2";

	struct PTerm * P = str2polynomial(sP);
	printf("%s <%s>\n", sP, polynomial2str(s1, P));
	print_polynomial(P);

	struct PTerm * Q = str2polynomial(sQ);
	printf("%s <%s>\n", sQ, polynomial2str(s2, Q));
	print_polynomial(Q);

	printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	printf("(%s) + (%s)\n", polynomial2str(s1, P), polynomial2str(s2, Q));
	add_polynomials(P, Q);

	/* printf("%s\n", sP); */
	/* print_polynomial(P); */
	printf("(%s)\n", polynomial2str(s2, Q));
	print_polynomial(Q);
}

void test_add_2()
{
	char s1[128] = "";
	char s2[128] = "";

	/* add
	 *     (x + y + z)
	 * to
	 *     (x² - 2y - z)
	 */
	const char sP[] = "x+y+z";
	const char sQ[] = "x^2-2y-z";

	struct PTerm * P = str2polynomial(sP);
	printf("%s <%s>\n", sP, polynomial2str(s1, P));
	print_polynomial(P);

	struct PTerm * Q = str2polynomial(sQ);
	printf("%s <%s>\n", sQ, polynomial2str(s2, Q));
	print_polynomial(Q);

	printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	printf("(%s) + (%s)\n", polynomial2str(s1, P), polynomial2str(s2, Q));
	add_polynomials(P, Q);

	/* printf("%s\n", sP); */
	/* print_polynomial(P); */
	printf("(%s)\n", polynomial2str(s2, Q));
	print_polynomial(Q);
}

void test_mul_1()
{
	char s1[128] = "";
	char s2[128] = "";

	const char sP[] = "x+y+z";
	const char sM[] = "";
	const char sQ[] = "";

	struct PTerm * P = str2polynomial(sP);
	printf("%s <%s>\n", sP, polynomial2str(s1, P));
	print_polynomial(P);

	struct PTerm * M = str2polynomial(sM);
	printf("%s <%s>\n", sM, polynomial2str(s2, M));
	print_polynomial(M);

	struct PTerm * Q = str2polynomial(sQ);
	printf("%s <%s>\n", sQ, polynomial2str(s2, Q));
	print_polynomial(Q);

	printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	printf("(%s) * (%s)\n", polynomial2str(s1, P), polynomial2str(s2, M));
	mul_polynomials(P, M, Q);

	printf("(%s)\n", polynomial2str(s2, Q));
	print_polynomial(Q);
}

void test_mul_2()
{
	char s1[128] = "";
	char s2[128] = "";
	char s3[128] = "";

	const char sP[] = "x+y+z";
	const char sM[] = "x^2-2y-z";
	const char sQ[] = "";

	struct PTerm * P = str2polynomial(sP);
	printf("%s <%s>\n", sP, polynomial2str(s1, P));
	print_polynomial(P);

	struct PTerm * M = str2polynomial(sM);
	printf("%s <%s>\n", sM, polynomial2str(s2, M));
	print_polynomial(M);

	struct PTerm * Q = str2polynomial(sQ);
	printf("%s <%s>\n", sQ, polynomial2str(s2, Q));
	print_polynomial(Q);

	printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	printf("(%s) + (%s) * (%s)\n",
	       polynomial2str(s2, Q),
	       polynomial2str(s1, P),
	       polynomial2str(s3, M));
	mul_polynomials(P, M, Q);

	printf("(%s)\n", polynomial2str(s2, Q));
	print_polynomial(Q);
}

void test_mul_3()
{
	char s1[128] = "";
	char s2[128] = "";
	char s3[128] = "";

	const char sP[] = "x+y+z";
	const char sM[] = "x^2-2y-z";
	const char sQ[] = "x^4+3xy+xz";

	struct PTerm * P = str2polynomial(sP);
	printf("%s <%s>\n", sP, polynomial2str(s1, P));
	print_polynomial(P);

	struct PTerm * M = str2polynomial(sM);
	printf("%s <%s>\n", sM, polynomial2str(s2, M));
	print_polynomial(M);

	struct PTerm * Q = str2polynomial(sQ);
	printf("%s <%s>\n", sQ, polynomial2str(s2, Q));
	print_polynomial(Q);

	printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	printf("(%s) + (%s) * (%s)\n",
	       polynomial2str(s2, Q),
	       polynomial2str(s1, P),
	       polynomial2str(s3, M));
	mul_polynomials(P, M, Q);

	printf("(%s)\n", polynomial2str(s2, Q));
	print_polynomial(Q);
}

int main()
{
	ENABLE_COLOR("31")
	printf("##################################################\n");

	test_add_1();

	ENABLE_COLOR("32")
	printf("##################################################\n");

	test_add_2();

	ENABLE_COLOR("31")
	printf("##################################################\n");

	test_mul_1();

	ENABLE_COLOR("32")
	printf("##################################################\n");

	test_mul_2();

	ENABLE_COLOR("31")
	printf("##################################################\n");

	test_mul_3();

	DISABLE_COLOR()

	return 0;
}
