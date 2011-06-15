/* elevator simulator -- a discrete simulation example
 *
 * Algorithm 2.2.5U, 2.2.5E, 2.2.5D @ TAOCP::p.282~p.296
 *
 * author: Forrest Y. Yu <forrest.yu@gmail.com>, http://forrestyu.net/
 * date:   Jan.24,2011
 *
 * compile and run:
 *         $ gcc -g -Wall -o elevator p.283_elevator.c
 *         $ ./elevator
 *
 * profiling:
 *         $ gcc -g -Wall -pg -o elevator p.283_elevator.c
 *         $ ./elevator
 *         $ gprof ./elevator
 *
 * part of gprof output:
 *         calls    name    
 *          3224    xlog
 *          1152    get_func_info
 *           300    get_STATE_str
 *           288    alloc_WAIT_node
 *           288    in_WAIT
 *           248    xlog_state
 *            67    out_WAIT
 *            56    E4
 *            42    D
 *            32    alloc_USER_node
 *            28    E5
 *            28    E6
 *            25    E3
 *            23    E2
 *            18    U1
 *            18    U2
 *            17    U3
 *            17    in_QUEUE
 *            17    out_QUEUE
 *            16    E7
 *            16    E7A
 *            16    E8
 *            16    E8A
 *            15    U5
 *            15    U6
 *            15    in_ELEVATOR
 *            15    out_ELEVATOR
 *             6    E1
 *             5    E9
 *             3    U4
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <pthread.h>

/*
 * TODO:
 *     1. rewrite alloc_XXXX_node() so that freed node can be reused.
 *     2. write ascii-art log to show an elevator system animation
 */

/****************************************************************************************************
 * Coroutine U (Users). Everyone who enters the system begins to perform the
 * actions specified below, starting at step U1.
 ****************************************************************************************************
 *
 * U1. [Enter, prepare for successor.] The following quantities are determined in
 *     some manner that will not be specified here:
 *
 *     IN, the floor on which the new user has entered the system;
 *     OUT, the floor to which this user wants to go (OUT != IN);
 *     GIVEUPTIME, the amount of time this user will wait for the elevator before
 *         running out of patience and deciding to walk;
 *     INTERTIME, the amount of time before another user will enter the system.
 *
 *     After these quantities have been computed, the simulation program sets
 *     things up so that another user enters the system at TIME+INTERTIME
 *
 * U2. [Signal and wait.] (The purpose of this step is to call for the elevator; some
 *     special cases arise if the elevator is already on the right floor.) if FLOOR == IN
 *     and if the elevator's next action is step E6 below (that is, if the elevator doors
 *     are now closing), send the elevator immediately to its step E3 and cancel its
 *     activity E6. (This means that the doors will open again before the elevator
 *     moves.) If FLOOR == IN and if D3 != 0, set D3 <-- 0, set D1 to a nonzero value,
 *     and start up the elevator's activity E4 again. (This means that the elevator
 *     doors are open on this floor, but everyone else has already gotten on or
 *     off. Elevator step E4 is a sequencing step that grants people permission to
 *     enter the elevator according to normal laws of courtesy; therefore, restarting
 *     E4 gives this user a chance to get in before the doors close.) In all other
 *     cases, the user sets CALLUP[IN] <-- 1 or CALLDOWN[IN] <-- 1, according as
 *     OUT > IN or OUT < IN; and if D2 == 0 or the elevator is in its ``dormant''
 *     position E1, the DECISION subroutine specified below is performed. (The
 *     DECISION subroutine is used to take the elevator out of NEUTRAL state at
 *     certain critical times.)
 *
 * U3. [Enter queue.] Insert this user at the rear of QUEUE[IN], which is a linear
 *     list representing the people waiting on this floor. Now the user waits
 *     patiently for GIVEUPTIME units of time, unless the elevator arrives first --
 *     more precisely, unless step E4 of the elevator routine below sends this user
 *     to U5 and cancels the scheduled activity U4.
 *
 * U4. [Give up.] If FLOOR != IN or D1 == 0, delete this user from QUEUE[IN]
 *     and from the simulated system. (The user has decided that the elevator is
 *     too slow, or that a bit of exercise will be better than an elevator ride.) If
 *     FLOOR == IN and D1!= 0, the user stays and waits (knowing that the wait
 *     won't be long).
 *
 * U5. [Get in.] This user now leaves QUEUE[IN] and enters ELEVATOR, which is
 *     a stack-like list representing the people now on board the elevator. Set
 *     CALLCAR[OUT] <-- 1.
 *         Now if STATE == NEUTRAL, set STATE <-- GOINGUP or GOINGDOWN as
 *     appropriate, and set the elevator's activity E5 to be executed after 25 units
 *     of time. (This is a special feature of the elevator, allowing the doors to close
 *     faster than usual if the elevator is in NEUTRAL state when the user selects a
 *     destination floor. The 25-unit time interval gives step E4 the opportunity
 *     to make sure that D1 is properly set up by the time step E5, the door-closing
 *     action, occurs.)
 *         Now the user waits until being sent to step U6 by step E4 below, when
 *     the elevator has reached the desired floor.
 *
 * U6. [Get out.] Delete this user from the ELEVATOR list and from the simulated
 *     system.  ▍
 *
 ****************************************************************************************************
 * Coroutine E (Elevator). This coroutine represents the actions of the elevator;
 * step E4 also handles the control of when people get in and out.
 ****************************************************************************************************
 *
 * E1. [Wait for call.] (At this point the elevator is sitting at floor 2 with the doors
 *     closed, waiting for something to happen.) If someone presses a button, the
 *     DECISION subroutine will take us to step E3 or E6. Meanwhile, wait.
 *
 * E2. [Change of state?] If STATE == GOINGUP and CALLUP[j] == CALLDOWN[j] == CALLCAR[j] == 0
 *     for all j > FLOOR, then set STATE <-- NEUTRAL or STATE <-- GOINGDOWN,
 *     according as CALLCAR[j] == 0 for all j < FLOOR or not, and set
 *     all CALL variables for the current floor to zero. If STATE == GOINGDOWN, do
 *     similar actions with directions reversed.
 *
 * E3. [Open doors.] Set D1 and D2 to any nonzero values. Set elevator activity
 *     E9 to start up independently after 300 units of time. (This activity may be
 *     canceled in step E6 below before it occurs. If it has already been scheduled
 *     and not canceled, we cancel it and reschedule it.) Also set elevator activity
 *     E5 to start up independently after 76 units of time. Then wait 20 units of
 *     time (to simulate opening of the doors) and go to E4.
 *
 * E4. [Let people out, in.] If anyone in the ELEVATOR list has OUT == FLOOR, send
 *     the user of this type who has most recently entered immediately to step U6,
 *     wait 25 units, and repeat step E4. If no such users exist, but QUEUE[FLOOR]
 *     i snot empty, send the front person of that queue immediately to step U5
 *     instead of U4, wait 25 units, and repeat step E4. But if QUEUE[FLOOR]
 *     is empty, set D1 <-- 0, make D3 nonzero, and wait for some other activity
 *     to initiate further action. (Step E5 will send us to E6, or step U2 will
 *     restart E4.)
 *
 * E5. [Close doors.] If D1 != 0, wait 40 units and repeat this step (the doors flutter
 *     a little, but they spring open again, since someone is still getting out or in).
 *     Otherwise set D3 <-- 0 and set the elevator to start at step E6 after 20 units
 *     of time. (This simulates closing the doors after people have finished getting
 *     in or out; but if a new user enters on this floor while the doors are closing,
 *     they will open again as stated in step U2.)
 *
 * E6. [Prepare to move.] Set CALLCAR[FLOOR] to zero; also set CALLUP[FLOOR]
 *     to zero if STATE != GOINGDOWN, and also set CALLDOWN[FLOOR] to zero if
 *     STATE != GOINGUP. (Note: If STATE == GOINGUP, the elevator does not clear
 *     out CALLDOWN, since it assumes that people who are going down will not
 *     have entered; but see exercise 6.) Now perform the DECISION subroutine.
 *         If STATE == NEUTRAL even after the DECISION subroutine has acted, go
 *     to E1. Otherwise, if D2 != 0, cancel the elevator activity E9. Finally, if
 *     STATE == GOINGUP, wait 15 units of time (for the elevator to build up speed)
 *     and go to E7; if STATE == GOINGDOWN, wait 15 units and go to E8.
 *
 * E7. [Go up a floor.] Set FLOOR <-- FLOOR + 1 and wait 51 units of time. If
 *     now CALLCAR[FLOOR] == 1 or CALLUP[FLOOR] == 1, or
 *     if ((FLOOR == 2 or CALLDOWN[FLOOR] == 1) and CALLUP[j] == CALLDOWN[j] == CALLCAR[j] == 0 for all j > FLOOR),
 *     wait 14 units (for deceleration) and go to E2. Otherwise,
 *     repeat this step.
 *
 * E8. [Go down a floor.] This step is like E7 with directions reversed, and also
 *     the times 51 and 14 are changed to 61 and 23, respectively. (It takes the
 *     elevator longer to go down than up.)
 *
 * E9. [Set inaction indicator.] Set D2 <-- 0 and perform the DECISION subroutine.
 *     ( This independent action is initiated in step E3 but it is almost always
 *     canceled in step E6. See exercise 4.)  ▍
 *
 ****************************************************************************************************
 * Subroutine D (DECISION subroutine) This subroutine is performed at certain
 * critical times, as specified in the coroutines above, when a decision about the
 * elevator's next direction is to be made.
 ****************************************************************************************************
 *
 * D1. [Decision necessary?] If STATE != NEUTRAL, exit from this subroutine.
 *
 * D2. [Should doors open?] If the elevator is positioned at E1 and if CALLUP[2],
 *     CALLCAR[2], and CALLDOWN[2] are not all zero, cause the elevator to start
 *     its activity E3 after 20 units of time, and exit from this subroutine. (if
 *     the DECISION subroutine is currently being invoked by the independent
 *     activity E9, it is possible for the elevator coroutine to be positioned at E1.)
 *
 * D3. [Any calls?] Find the smallest j != FLOOR for which CALLUP[j], CALLCAR[j],
 *     or CALLDOWN[j] is nonzero, and go on to step D4. But if no such j exists,
 *     then set j <- 2 if the DECISION subroutine is currently being invoked by
 *     step E6; otherwise exit from this subroutine.
 *
 * D4. [Set STATE.] If FLOOR > j, set STATE <-- GOINGDOWN; if FLOOR < j, set
 *     STATE <-- GOINGUP.
 *
 * D5. [Elevator dormant?] If the elevator coroutine is positioned at step E1, and
 *     if j != 2, set the elevator to perform step E6 after 20 units of time. Exit
 *     from the subroutine.  ▍
 ****************************************************************************************************/

/*
 *     I realized in the train (Beijing->Taishan) that the Book Code is
 *     not implemented with threads, but a WAIT list.
 *     
 *     Everything that will happen should be inserted into a double linked
 *     list (WAIT list). Later the nodes in the WAIT list will be executed
 *     one by one.
 *
 *     The data structure in TAOCP p.288 is like:
 *             +-----+-----+-----+-----+-----+
 *             | IN  |   LLINK1  |   RLINK1  |
 *             +-----+-----+-----+-----+-----+
 *             |          ent_time           |
 *             +-----+-----+-----+-----+-----+
 *             |   INST    |     |     |     |
 *             +-----+-----+-----+-----+-----+
 *             | OUT |   LLINK2  |   RLINK2  |
 *             +-----+-----+-----+-----+-----+
 *     This data structure is shared by three types of nodes:
 *             1. nodes in WAIT list
 *             2. nodes in QUEUE[0..4]
 *             1. nodes in ELEVATOR list
 *     In the code below, three different structs are used instead for the
 *     sake of readibility.
 */

/*
	=======================+===============+=================================
                               |               |
                               |               |
                               |               |
                               |               |
                               |               |
                               |               |
                               |               | QUEUE[4]
                               |               | CALLUP[4] (never be used)
        floor 4 (third floor)  |               | CALLDOWN[4]
	=======================+---------------+=================================
                               |               |
                               |               |
                               |               |
                               |               |
                               |               |
                               |               |
                               |               | QUEUE[3]
                               |               | CALLUP[3]
        floor 3 (second floor) |               | CALLDOWN[3]
	=======================+---------------+=================================
                               |+-------------+|
                               ||             ||
                               || CALLCAR[4]  || CALLCAR[destination_floor] = 1
                               ||        [3]  ||
                               ||        [2]  ||
                               ||        [1]  ||
                               ||        [0]  || QUEUE[2]
                (home floor)   ||             || CALLUP[2]
        floor 2 (first floor)  |+-------------+| CALLDOWN[2]
	=======================+---------------+=================================
                               |               |
                               |               |
                               |               |
                               |               |
                               |               |
                               |               |
                               |               | QUEUE[1]
                               |               | CALLUP[1]
        floor 1 (basement)     |               | CALLDOWN[1]
	=======================+---------------+=================================
                               |               |
                               |               |
                               |               |
                               |               |
                               |               |
                               |               |
                               |               | QUEUE[0]
                               |               | CALLUP[0]
        floor 0 (sub-basement) |               | CALLDOWN[0] (never be used)
	=======================+===============+=================================
 */

#define NO_ARG			0
#define ENABLE_COLOR(x)		{putchar(033); printf("[%sm", x);}
#define DISABLE_COLOR()		{putchar(033); printf("[0m");}
#define DEFAULT_GIVEUPTIME	1000
#define POOL_SIZE		8192
#define FLOOR_NR		5
#define MIN_FLOOR		0
#define MAX_FLOOR		(FLOOR_NR - 1)
#define QL			LLINK2
#define QR			RLINK2
#define WL			LLINK1
#define WR			RLINK1
#define EL			LLINK2
#define ER			RLINK2
#define TIME			(current_step->ent_time)

/* this enum must correspond with ``state_str[]'' in get_STATE_str(): */
enum elevator_state	{NEUTRAL = 0, GOINGUP = 1, GOINGDOWN = 2};

/* see xlog() */
enum log_type		{LOG_CLOSE,
			 LOG_PLAIN, LOG_U, LOG_E, LOG_FUNC_NAME,
			 LOG_TIME, LOG_STATE, LOG_FLOOR, LOG_FLAG0, LOG_FLAG1};

/* see get_func_info() */
enum func_info_action	{GET_FUNC_NAME, GET_FUNC_DESC};

/* structs and typedefs */
struct user_info {
	int	IN;
	int	OUT;
	int	ENTERTIME;
	int	GIVEUPTIME;
	char	name[32];
};

/** conceptually: waiting_func = funcU | funcE
 *    - waiting_func, as the name suggests, is used in the WAIT list, or in
 *      any cases where funcU and funcE are not distinguished.
 *    - funcU and funcE are only used when being called.
 */
typedef void *		waiting_func;
typedef void (*funcU)  (struct user_info *);
typedef void (*funcE)  (void);

/* node in the WAIT list */
typedef struct wait_node {
	struct wait_node *	LLINK1;
	struct wait_node *	RLINK1;
	int			ent_time;
	waiting_func		inst;
	struct user_info *	arg;
} WAIT_NODE;

/* node in QUEUE[0..4] and ELEVATOR lists */
typedef struct user_node {
	struct user_node *	LLINK2;
	struct user_node *	RLINK2;
	struct user_info *	uinfo;
} USER_NODE;

/* prototype */
void D(void);
void U1(struct user_info * p);
void U2(struct user_info * p);
void U3(struct user_info * p);
void U4(struct user_info * p);
void U5(struct user_info * p);
void U6(struct user_info * p);
void U7(struct user_info * p);
void U8(struct user_info * p);
void U9(struct user_info * p);
void E1(void);
void E2(void);
void E3(void);
void E4(void);
void E5(void);
void E6(void);
void E7(void);
void E7A(void);
void E8(void);
void E8A(void);
void E9(void);
void dummy_func(void);

/* globals */

/** This is the only copy of the info of users.
 *  Whenever the user info is needed, a pointer to this array will be used.
 */
struct user_info users[] = {
	/* IN, OU, ENTERTIME,         GIVEUPTIME,    name */
	{   0,  2,         0,                152, "User 1"},
	{   4,  1,        38, DEFAULT_GIVEUPTIME, "User 2"},
	{   2,  1,       136, DEFAULT_GIVEUPTIME, "User 3"},
	{   2,  1,       141, DEFAULT_GIVEUPTIME, "User 4"},
	{   3,  1,       291, DEFAULT_GIVEUPTIME, "User 5"},
	{   2,  1,       364,            540-364, "User 6"},
	{   1,  2,       602, DEFAULT_GIVEUPTIME, "User 7"},
	{   1,  0,       827, DEFAULT_GIVEUPTIME, "User 8"},
	{   1,  3,       876, DEFAULT_GIVEUPTIME, "User 9"},
	{   0,  4,      1048, DEFAULT_GIVEUPTIME, "User 10"},
	{   2,  2,      4384, DEFAULT_GIVEUPTIME, "User 16"},
	{   2,  3,      4384, DEFAULT_GIVEUPTIME, "User 17"},
	{   3,  2,      5100, DEFAULT_GIVEUPTIME, "User 18"},
	{   3,  1,      5700, DEFAULT_GIVEUPTIME, "User 19"},
	{   2,  0,      5710, DEFAULT_GIVEUPTIME, "User 20"},
	{   3,  1,      7000, DEFAULT_GIVEUPTIME, "User 21"},
	{   1,  3,      7000,                350, "User 22"},
	{   3,  2,      7620, DEFAULT_GIVEUPTIME, "User 23"},
	{   0,  0,      9999, DEFAULT_GIVEUPTIME, "DUMMY"}
};

/** THE double linked lists.
 *  These three lists are what the book wants to teach in this section.
 */
WAIT_NODE	WAIT;
USER_NODE	QUEUE[FLOOR_NR];
USER_NODE	ELEVATOR;

enum elevator_state STATE = NEUTRAL;
int FLOOR =  2;	/* the current position of the elevator */
int D1	  =  0; /* is zero except during the time people are getting
		 * in or out of the elevator
		 */
int D2	  =  0; /* becomes zero if the elevator has sat on one floor
		 * without moving for 30s or more
		 */
int D3	  =  0; /* is zero except when the doors are open but nobody is getting
		 * in or out of the elevator
		 */
int CALLUP[FLOOR_NR];
int CALLDOWN[FLOOR_NR];
int CALLCAR[FLOOR_NR];

WAIT_NODE *	current_step = 0;
int		current_elevator_step = 0;	/* 0xE1~0xE9 means E1~E9 respectively */
int		dormant = 1;

#define DECLARE_ALLOC_NODE(x) x##_NODE * alloc_##x##_node() \
	{					    \
	static x##_NODE node_pool[POOL_SIZE];	    \
	static int i = 0;			    \
	assert(i < POOL_SIZE);	/** if this assert fails, either the POOL_SIZE \
				 *  should be enlarged, or it is time to improve \
				 *  the node allocation mechanism to make nodes	\
				 *  reusable.				\
				 */					\
	return &node_pool[i++];						\
	}

/* node allocation routines */
DECLARE_ALLOC_NODE(WAIT)
DECLARE_ALLOC_NODE(USER)
	
/* logging routine */
void xlog(enum log_type lt, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	static FILE * logf = 0;
	if (!logf) {
		char logfilename[FILENAME_MAX];
		char * p;
		strcpy(logfilename, __FILE__);
		for (p = logfilename + strlen(logfilename); *p != '.'; p--);
		strcpy(p, ".log");
		fprintf(stderr, "logfile: %s\n", logfilename);

		logf = fopen(logfilename, "w");
		assert(logf);
		fprintf(logf, "%%%% %s - %s\n%%%% %s (%s)\n\n",
			__DATE__, __TIME__, logfilename, __FILE__);
	}
	else {
		vfprintf(logf, fmt, args);
	}

	if (lt == LOG_CLOSE) {
		assert(logf);
		fclose(logf);
		fprintf(stderr, "log file closed.\n");
		logf = 0;
	}
	else if (lt == LOG_PLAIN) {
		DISABLE_COLOR();
	}
	else if (lt == LOG_U) {
		ENABLE_COLOR("1;35"); /* cyan (underscored) */
	}
	else if (lt == LOG_E) {
		ENABLE_COLOR("36"); /* cyan */
	}
	else if (lt == LOG_FUNC_NAME) {
		ENABLE_COLOR("31"); /* red */
	}
	else if (lt == LOG_TIME) {
		ENABLE_COLOR("43"); /* /yellow */
	}
	else if (lt == LOG_STATE) {
		ENABLE_COLOR("47"); /* /gray */
	}
	else if (lt == LOG_FLOOR) {
		ENABLE_COLOR("43"); /* /yellow */
	}
	else if (lt == LOG_FLAG0) {
		ENABLE_COLOR("1;37;47"); /* white/gray */
	}
	else if (lt == LOG_FLAG1) {
		ENABLE_COLOR("47"); /* /gray */
	}
	else {
		assert(0);
	}
        vprintf(fmt, args);
	DISABLE_COLOR();

	va_end(args);
}

const char * get_func_info(waiting_func fp, enum func_info_action a)
{
	int i;
	static const char * names[] = {
		"",
		"U1()", "U2()", "U3()", "U4()", "U5()", "U6()",
		"E1()", "E2()", "E3()", "E4()", "E5()", "E6()",
		"E7()", "E7A()", "E8()", "E8A()", "E9()",
		"dummy_func()"
	};
	static const char * desc[] = {
		"",
		"U1. [Enter, prepare for successor.]",
		"U2. [Signal and wait.]",
		"U3. [Enter queue.]",
		"U4. [Give up.]",
		"U5. [Get in.]",
		"U6. [Get out.]",
		"E1. [Wait for call.]",
		"E2. [Change of state?]",
		"E3. [Open doors.]",
		"E4. [Let people out, in.]",
		"E5. [Close doors.]",
		"E6. [Prepare to move.]",
		"E7. [Go up a floor.]",
		"E7A. [Go up a floor.]",
		"E8. [Go down a floor.]",
		"E8A. [Go down a floor.]",
		"E9. [Set inaction indicator.]",
		"dummy_func. [DUMMY.]"
	};
	static const waiting_func fps[] = {0,
				     U1, U2, U3, U4, U5, U6,
				     E1, E2, E3, E4, E5, E6, E7, E7A, E8, E8A, E9,
				     dummy_func};

	assert((sizeof(names)/sizeof(names[0])) == (sizeof(fps)/sizeof(fps[0])));
	assert((sizeof(desc)/sizeof(desc[0]))	== (sizeof(fps)/sizeof(fps[0])));

	for (i = 0; i < sizeof(fps)/sizeof(fps[0]); i++) {
		if (fp == fps[i]) {
			if (a == GET_FUNC_NAME)
				return names[i];
			else if (a == GET_FUNC_DESC)
				return desc[i];
			else
				assert(0);
		}
	}

	/* should not reach here */
	/* for (i = 0; i < sizeof(fps)/sizeof(fps[0]); i++) */
	/* 	xlog(LOG_PLAIN, "%s: 0x%X. ", names[i], (int)fps[i]); */
	/* xlog(LOG_PLAIN, "\nunknown func: 0x%X.\n", (int)fp); */
	assert(0);
	return 0;
}

/* void xlog_node(struct node288 *x, char struct_type) */
/* { */
/* 	const char * pending[] = {"", */
/* 				  "\t\t\t\t\t\t", */
/* 				  "\t\t\t\t\t\t\t\t\t\t\t\t" */
/* 	}; */
/* 	const char * p; */
/* 	switch (struct_type) { */
/* 	case 'W':	/\* WAIT *\/ */
/* 		p = pending[0]; */
/* 		break; */
/* 	case 'Q':	/\* QUEUE[0..4] *\/ */
/* 		p = pending[1]; */
/* 		break; */
/* 	case 'E':	/\* ELEVATOR *\/ */
/* 		p = pending[2]; */
/* 		break; */
/* 	default: */
/* 		assert(0); */
/* 		break; */
/* 	} */

/* 	if (!x) { */
/* 		xlog(LOG_PLAIN, "%s=============================================\n", p); */
/* 		return; */
/* 	} */

/* 	xlog(LOG_PLAIN, "%s---------------------------------------------\n", p); */
/* 	xlog(LOG_PLAIN, "%s %8X", p, (int)x); */
/* 	xlog(LOG_PLAIN, "%12d    %8X    %8X\n", x->IN, (int)x->LLINK1, (int)x->RLINK1); */
/* 	xlog(LOG_TIME, "%s%45d\n", p, x->ent_time); */
/* 	xlog(LOG_FUNC_NAME, "%s%21s%24s\n", p, "", get_func_info(x->inst, GET_FUNC_NAME)); */
/* 	xlog(LOG_PLAIN, "%s%21d    %8X    %8X\n", p, x->OUT, (int)x->LLINK2, (int)x->RLINK2); */
/* 	xlog(LOG_PLAIN, "%s%21d            \"%10s\"\n", p, x->giveup_time, x->name); */
/* } */

void in_QUEUE(struct user_info *p)
{
	int in = p->IN;
	USER_NODE * q = alloc_USER_node();

	assert((in >= MIN_FLOOR) && (in <= MAX_FLOOR));

	q->uinfo = p;

	q->QL = QUEUE[in].QL;
	q->QR = &QUEUE[in];
	QUEUE[in].QL->QR = q;
	QUEUE[in].QL = q;

	xlog(LOG_U, "      %s (%d->%d) is inserted into QUEUE[%d].\n",
	     p->name, p->IN, p->OUT, p->IN);
}

USER_NODE * out_QUEUE(int in, struct user_info * p)
{
	USER_NODE * q = 0;
	if (p == 0) {
		q = QUEUE[in].QR;
		QUEUE[in].QR = QUEUE[in].QR->QR;
		QUEUE[in].QR->QL = &QUEUE[in];
	}
	else {
		assert(in == p->IN);
		for (q = QUEUE[in].QR; q != &QUEUE[in]; q = q->QR) {
			if (q->uinfo == p) {
				q->QL->QR = q->QR;
				q->QR->QL = q->QL;
			}
		}
	}

	xlog(LOG_U, "      %s (%d->%d) is removed from QUEUE[%d].\n",
	     p->name, p->IN, p->OUT, p->IN);

	return q;
}

void in_WAIT(int ent_time, waiting_func inst, struct user_info * arg)
{
	WAIT_NODE * w = alloc_WAIT_node();
	w->ent_time = ent_time;
	w->inst = inst;
	w->arg  = arg;

	xlog(LOG_PLAIN, "%s will be scheduled at TIME %d\n",
	     get_func_info(inst, GET_FUNC_NAME), ent_time);

	WAIT_NODE * p = WAIT.WL;
	while (ent_time < p->ent_time)
		p = p->WL;

	/* insert to the right of p */
	w->WL = p;
	w->WR = p->WR;
	p->WR->WL = w;
	p->WR = w;
}

void out_WAIT(waiting_func inst, struct user_info * arg)
{
	WAIT_NODE * p = WAIT.WR;
	assert(p != &WAIT);	/* WAIT list should not be empty */
	int cnt = 0;
	for (; p != &WAIT; p = p->WR) {
		if (p->ent_time < TIME)
			continue;

		if ((p->inst == inst) && (p->arg == arg)) {
			assert(cnt == 0);
			cnt++;
			p->WL->WR = p->WR;
			p->WR->WL = p->WL;
			/* xlog(LOG_PLAIN, "%s (#%d) cancelled (TIME %d)\n", */
			/*      get_func_info(inst, GET_FUNC_NAME), cnt, p->ent_time); */
			xlog(LOG_PLAIN, "%s is cancelled (TIME %d)\n",
			     get_func_info(inst, GET_FUNC_NAME), p->ent_time);
		}
	}
	if (cnt == 0) {
		xlog(LOG_PLAIN, "%s is not in the WAIT list.\n",
		     get_func_info(inst, GET_FUNC_NAME));
	}
}

void in_ELEVATOR(struct user_info * p)
{
	USER_NODE * e = alloc_USER_node();

	e->uinfo = p;

	e->EL = ELEVATOR.EL;
	e->ER = &ELEVATOR;
	ELEVATOR.EL->ER = e;
	ELEVATOR.EL = e;
}

void out_ELEVATOR(struct user_info * p)
{
	USER_NODE * e = ELEVATOR.ER;
	assert(e != &ELEVATOR);	/* ELEVATOR list should not be empty */
	for (; e != &ELEVATOR; e = e->ER) {
		if (e->uinfo == p) {
			e->EL->ER = e->ER;
			e->ER->EL = e->EL;
			return;
		}
	}
}

/* /\* log double-linked list *\/ */
/* void xlog_DLL(struct node288 *p, char struct_type) */
/* { */
/* 	struct node288 * x = p; */
/* 	do { */
/* 		xlog_node(x, struct_type); */

/* 		switch (struct_type) { */
/* 		case 'W':	/\* WAIT *\/ */
/* 			x = x->WR; */
/* 			break; */
/* 		case 'Q':	/\* QUEUE[0..4] *\/ */
/* 			x = x->QR; */
/* 			break; */
/* 		case 'E':	/\* ELEVATOR *\/ */
/* 			x = x->ER; */
/* 			break; */
/* 		default: */
/* 			assert(0); */
/* 			break; */
/* 		} */
/* 	} while (x != p); */
/* 	xlog_node(0, struct_type); */
/* } */

const char * get_STATE_str(void)
{
	/* this array must correspond with ``enum elevator_state'': */
	static const char * state_str[] = {"NEUTRAL", "GOINGUP", "GOINGDOWN"};
	assert(STATE < sizeof(state_str)/sizeof(state_str[0]));
	return state_str[STATE];
}

void xlog_state(const char * s, int level)
{
	if (level > 1)
		xlog(LOG_PLAIN, "\n%s\n", s);

	if (level > 0) {
		xlog(LOG_TIME, "[TIME:%d]", TIME);
		xlog(LOG_STATE, "[STATE:%s]", get_STATE_str());
		xlog(LOG_FLOOR, "[FLOOR:%d]", FLOOR);
		xlog(LOG_FLAG1, "[");
		if (D1)
			xlog(LOG_FLAG1, "[D1(peopleIO):%d ", D1);
		else
			xlog(LOG_FLAG0, "[D1(peopleIO):%d ", D1);
		if (D2)
			xlog(LOG_FLAG1, "D2(stopped30s):%d ", D2);
		else
			xlog(LOG_FLAG0, "D2(stopped30s):%d ", D2);
		if (D3)
			xlog(LOG_FLAG1, "D3(nobody):%d", D3);
		else
			xlog(LOG_FLAG0, "D3(nobody):%d", D3);
		xlog(LOG_FLAG1, "]");
		/* xlog(LOG_PLAIN, "\n"); */
	}

	/* if (level > 1) { */
	/* 	xlog(LOG_PLAIN, "\nWAIT:\n"); */
	/* 	xlog_DLL(&WAIT, 'W'); */
	/* 	xlog(LOG_PLAIN, "\nQUEUE[0..4]:\n"); */
	/* 	for (i = MIN_FLOOR; i <= MAX_FLOOR; i++) */
	/* 		xlog_DLL(&QUEUE[i], 'Q'); */
	/* 	xlog(LOG_PLAIN, "\nELEVATOR:\n"); */
	/* 	xlog_DLL(&ELEVATOR, 'E'); */
	/* } */
}


/****************************************************************************************************
 * Subroutine D (DECISION subroutine) This subroutine is performed at certain
 * critical times, as specified in the coroutines above, when a decision about the
 * elevator's next direction is to be made.
 ****************************************************************************************************/
void D()
{
	/*
	 * D1. [Decision necessary?] If STATE != NEUTRAL, exit from this subroutine.
	 */
	xlog(LOG_PLAIN, "--------D1\n");
	if (STATE != NEUTRAL) {
		xlog(LOG_PLAIN, "--------STATE != NEUTRAL. Do nothing.\n");
		return;
	}

	/*
	 * D2. [Should doors open?] If the elevator is positioned at E1 and if CALLUP[2],
	 *     CALLCAR[2], and CALLDOWN[2] are not all zero, cause the elevator to start
	 *     its activity E3 after 20 units of time, and exit from this subroutine. (if
	 *     the DECISION subroutine is currently being invoked by the independent
	 *     activity E9, it is possible for the elevator coroutine to be positioned at E1.)
	 */
	/** this happens when:
	 *      1. elevator is dormant
	 *      2. new user arrives at floor 2
	 */
	xlog(LOG_PLAIN, "--------D2\n");
	if ((current_elevator_step == 0xE1) && (CALLUP[2] || CALLCAR[2] || CALLDOWN[2])) {
		in_WAIT(TIME + 20, E3, NO_ARG);
		return;
	}

	/*
	 * D3. [Any calls?] Find the smallest j != FLOOR for which CALLUP[j], CALLCAR[j],
	 *     or CALLDOWN[j] is nonzero, and go on to step D4. But if no such j exists,
	 *     then set j <- 2 if the DECISION subroutine is currently being invoked by
	 *     step E6; otherwise exit from this subroutine.
	 */
	xlog(LOG_PLAIN, "--------D3\n");
	int j;
	int flag = 0;
	for (j = MIN_FLOOR; j <= MAX_FLOOR; j++) {
		if (CALLUP[j] || CALLCAR[j] || CALLDOWN[j]) {
			flag = 1;
			break;
		}
	}
	if (!flag) {		/* no such j exists */
		if (current_elevator_step == 0xE6)
			j = 2;
		else
			return;
	}

	/*
	 * D4. [Set STATE.] If FLOOR > j, set STATE <-- GOINGDOWN; if FLOOR < j, set
	 *     STATE <-- GOINGUP.
	 */
	xlog(LOG_PLAIN, "--------D4. FLOOR:%d, j:%d\n", FLOOR, j);
	if (FLOOR > j)
		STATE = GOINGDOWN;
	else if (FLOOR < j)
		STATE = GOINGUP;
	else
		;		/* do nothing */

	/*
	 * D5. [Elevator dormant?] If the elevator coroutine is positioned at step E1, and
	 *     if j != 2, set the elevator to perform step E6 after 20 units of time. Exit
	 *     from the subroutine.  ▍
	 */
	/** this happens when:
	 *      1. elevator is dormant
	 *      2. new user arrives at floor X (X!=2)
	 */
	xlog(LOG_PLAIN, "--------D5\n");
	if ((current_elevator_step == 0xE1) && (j != 2)) {
		in_WAIT(TIME + 20, E6, NO_ARG);
	}
}

/****************************************************************************************************
 * Coroutine U (Users). Everyone who enters the system begins to perform the
 * actions specified below, starting at step U1.
 ****************************************************************************************************/
/*
 * U1. [Enter, prepare for successor.] The following quantities are determined in
 *     some manner that will not be specified here:
 *
 *     IN, the floor on which the new user has entered the system;
 *     OUT, the floor to which this user wants to go (OUT != IN);
 *     GIVEUPTIME, the amount of time this user will wait for the elevator before
 *         running out of patience and deciding to walk;
 *     INTERTIME, the amount of time before another user will enter the system.
 *
 *     After these quantities have been computed, the simulation program sets
 *     things up so that another user enters the system at TIME+INTERTIME
 */
/** ptr: &uinfo[] */
void U1(struct user_info * p)
{
	xlog(LOG_U, "U1(). %s arrives at floor %d, destination is %d.\n",
	     p->name, p->IN, p->OUT);

	U2(p);
}

/*
 * U2. [Signal and wait.] (The purpose of this step is to call for the elevator; some
 *     special cases arise if the elevator is already on the right floor.)
 */
void U2(struct user_info * p)
{
	/* xlog_node(p, 'Q'); */
	xlog(LOG_U, "U2(). %s (%d->%d).\n", p->name, p->IN, p->OUT);

	/* If FLOOR == IN and if the elevator's next action is step E6 below
	 *    (that is, if the elevator doors are now closing),
	 * send the elevator immediately to its step E3 and cancel its activity E6.
	 * (This means that the doors will open again before the elevator moves.)
	 */
	int next_is_E6 = 0;
	WAIT_NODE * w = current_step->WR;
	for (; w != &WAIT; w = w->WR) {
		const char * p = get_func_info(w->inst, GET_FUNC_NAME);
		xlog(LOG_U, "U2(). traversing - %s.\n", p);
		if (p[0] == 'E') {
			if (p[1] == '6') { /* E6 */
				next_is_E6 = 1;
				break;
			}
			else {	         /* E[1-5,7-9] */
				assert(!next_is_E6);
				break;
			}
		}
		else {		/* not the elevator's action */
			continue;
		}
	}
	if ((p->IN == FLOOR) && next_is_E6) {
		in_WAIT(TIME, E3, NO_ARG);

		/** cancel E6. these two lines will work:
		 *		w->WL->WR = w->WR;
		 *		w->WR->WL = w->WL;
		 *  however, to keep uniformity, out_WAIT() is used instead:
		 */
		out_WAIT(E6, 0);

		xlog(LOG_U, "U2(). Doors will open again.\n");
	}
	/* If FLOOR == IN and if D3 != 0, set D3 <-- 0, set D1 to a nonzero value,
	 * and start up the elevator's activity E4 again. (This means that the elevator
	 * doors are open on this floor, but everyone else has already gotten on or
	 * off. Elevator step E4 is a sequencing step that grants people permission to
	 * enter the elevator according to normal laws of courtesy; therefore, restarting
	 * E4 gives this user a chance to get in before the doors close.)
	 */
	else if ((p->IN == FLOOR) && (D3 != 0)) {
		D3 = 0;
		D1 = 1;
		in_WAIT(TIME, E4, NO_ARG);
	}
	/* In all other
	 * cases, the user sets CALLUP[IN] <-- 1 or CALLDOWN[IN] <-- 1, according as
	 * OUT > IN or OUT < IN; and if D2 == 0 or the elevator is in its ``dormant''
	 * position E1, the DECISION subroutine specified below is performed. (The
	 * DECISION subroutine is used to take the elevator out of NEUTRAL state at
	 * certain critical times.)
	 */
	else {
		if (p->OUT > p->IN) {
			CALLUP[p->IN] = 1;
			xlog(LOG_U, "U2(). %s pressed button UP.\n", p->name);
		}
		else if (p->OUT < p->IN) {
			CALLDOWN[p->IN] = 1;
			xlog(LOG_U, "U2(). %s pressed button Down.\n", p->name);
		}
		else {
			xlog(LOG_U, "U2(). No elevator is needed since %d==%d. "
			     "%s is stupid.\n",
			     p->IN, p->OUT, p->name);
			return;
		}

		if ((D2 == 0) || (current_elevator_step == 0xE1)) {
			D();
		}
	}
	U3(p);
}

/*
 * U3. [Enter queue.] Insert this user at the rear of QUEUE[IN], which is a linear
 *     list representing the people waiting on this floor. Now the user waits
 *     patiently for GIVEUPTIME units of time, unless the elevator arrives first --
 *     more precisely, unless step E4 of the elevator routine below sends this user
 *     to U5 and cancels the scheduled activity U4.
 */
void U3(struct user_info * p)
{
	in_WAIT(p->ENTERTIME + p->GIVEUPTIME, U4, p);
	in_QUEUE(p);
}

/*
 * U4. [Give up.] 
 */
void U4(struct user_info * p)
{
	xlog(LOG_U, "U4(). %s (%d->%d) gives up.\n", p->name, p->IN, p->OUT);

	/* If FLOOR != IN or D1 == 0, delete this user from QUEUE[IN]
	 * and from the simulated system. (The user has decided that the elevator is
	 * too slow, or that a bit of exercise will be better than an elevator ride.)
	 */
	if ((FLOOR != p->IN) || (D1 == 0)) {
		out_QUEUE(p->IN, p);
	}
	/* If FLOOR == IN and D1 != 0, the user stays and waits (knowing that the wait
	 * won't be long).
	 */
	else {
		assert((FLOOR == p->IN) && (D1 != 0));
		xlog(LOG_U, "U4(). %s (%d->%d) changes his/her mind to stay and wait.\n",
		     p->name, p->IN, p->OUT);
	}
}

/*
 * U5. [Get in.] 
 */
void U5(struct user_info * p)
{
	xlog(LOG_U, "U5(). %s (%d->%d) gets in the elevator. (FLOOR:%d)\n",
	     p->name, p->IN, p->OUT, FLOOR);

	/* This user now leaves QUEUE[IN] and enters ELEVATOR, which is
	 * a stack-like list representing the people now on board the elevator. Set
	 * CALLCAR[OUT] <-- 1.
	 */
	out_QUEUE(p->IN, p);
	in_ELEVATOR(p);

	CALLCAR[p->OUT] = 1;

	/*     Now if STATE == NEUTRAL, set STATE <-- GOINGUP or GOINGDOWN as
	 * appropriate, and set the elevator's activity E5 to be executed after 25 units
	 * of time. (This is a special feature of the elevator, allowing the doors to close
	 * faster than usual if the elevator is in NEUTRAL state when the user selects a
	 * destination floor. The 25-unit time interval gives step E4 the opportunity
	 * to make sure that D1 is properly set up by the time step E5, the door-closing
	 * action, occurs.)
	 */
	if (STATE == NEUTRAL) {
		if (p->OUT > p->IN)
			STATE = GOINGUP;
		else if (p->OUT < p->IN)
			STATE = GOINGDOWN;
		else
			assert(0);

		xlog(LOG_U, "U5(). STATE: NEUTRAL --> %s\n", get_STATE_str());

		out_WAIT(E5, 0);
		in_WAIT(TIME + 25, E5, NO_ARG); /** when this E5 is excuted, U5's caller E4
						 *  must have set D1 = 0, so that the door
						 *  will be closed.
						 */
	}

	/*     Now the user waits until being sent to step U6 by step E4 below, when
	 * the elevator has reached the desired floor.
	 */
	/** ``waits'' means no source code */
}

/*
 * U6. [Get out.] Delete this user from the ELEVATOR list and from the simulated
 *     system.  ▍
 */
void U6(struct user_info * p)
{
	xlog(LOG_U, "U6(). %s (%d->%d) gets out of the elevator. (FLOOR:%d)\n",
	     p->name, p->IN, p->OUT, FLOOR);

	out_ELEVATOR(p);

	/** if ``*p'' is allocated dynamically, it's the right time and place
	 *  to free the memory.
	 */
}

/****************************************************************************************************
 * Coroutine E (Elevator). This coroutine represents the actions of the elevator;
 * step E4 also handles the control of when people get in and out.
 ****************************************************************************************************/
/*
 * E1. [Wait for call.] (At this point the elevator is sitting at floor 2 with the doors
 *     closed, waiting for something to happen.) If someone presses a button, the
 *     DECISION subroutine will take us to step E3 or E6. Meanwhile, wait.
 */
void E1(void)
{
	int i;
	int flag = 0;

	xlog(LOG_E, "E1(). Elevator is waiting for call. (FLOOR:%d)\n", FLOOR);

	/* If someone presses a button, the
	 * DECISION subroutine will take us to step E3 or E6. Meanwhile, wait. */
	for (i = 0; i < FLOOR_NR; i++) {
		flag |= CALLUP[i];
		flag |= CALLDOWN[i];
	}

	if (flag) {
		D();
	}
	else {
		dormant = 1;
		xlog(LOG_E, "E1(). Elevator dormant.\n");
	}
}

/*
 * E2. [Change of state?] 
 */
void E2(void)
{
	xlog(LOG_E, "E2(). Elevator stops.\n");
	xlog(LOG_E, "E2(). STATE: %s --> ", get_STATE_str());

	int j;
	/* If STATE == GOINGUP and
	 * CALLUP[j] == CALLDOWN[j] == CALLCAR[j] == 0 for all j > FLOOR
	 */
	if (STATE == GOINGUP) {
		int flag1 = 0;
		for (j = MAX_FLOOR; j > FLOOR; j--)
			if (CALLUP[j] || CALLDOWN[j] || CALLCAR[j])
				flag1 = 1;
		if (!flag1) {
			/** !flag1 (i.e. flag1 == 0) means
			 *         CALLUP[FLOOR..4]   ==
			 *         CALLDOWN[FLOOR..4] ==
			 *         CALLCAR[FLOOR..4]  == 0
			 */
			/** there's nobody upstairs */
			int flag2 = 0; /* to avoid confusion, don't reuse flag1 */
			for (j = MIN_FLOOR; j < FLOOR; j++)
				if (CALLCAR[j])
					flag2 = 1;
			/** !flag2 (i.e. flag2 == 0) means
			 *         CALLCAR[0..FLOOR] == 0
			 */
			/* then set STATE <-- NEUTRAL or STATE <-- GOINGDOWN,
			 * according as CALLCAR[j] == 0 for all j < FLOOR or not,
			 */
			if (!flag2)
				/** there's no person going down in the car */
				STATE = NEUTRAL;
			else
				STATE = GOINGDOWN;

			/* and set all CALL variables for the current floor to zero. */
			CALLUP[FLOOR]  = 0;
			CALLDOWN[FLOOR]= 0;
			CALLCAR[FLOOR] = 0;
		}
	}

	/* If STATE == GOINGDOWN, do similar actions with directions reversed. */
	if (STATE == GOINGDOWN) {
		int flag1 = 0;
		for (j = MIN_FLOOR; j < FLOOR; j++)
			if (CALLUP[j] || CALLDOWN[j] || CALLCAR[j])
				flag1 = 1;
		/* xlog(LOG_E, "(%s)", flag1 ? "flag1" : ""); */
		if (!flag1) {
			/** !flag1 (i.e. flag1 == 0) means
			 *         CALLUP[0..FLOOR]   ==
			 *         CALLDOWN[0..FLOOR] ==
			 *         CALLCAR[0..FLOOR]  == 0
			 */
			/** there's nobody downstairs */
			int flag2 = 0; /* to avoid confusion, don't reuse flag1 */
			for (j = MAX_FLOOR; j > FLOOR; j--)
				if (CALLCAR[j])
					flag2 = 1;
			/* xlog(LOG_E, "(%s)", flag2 ? "flag2" : ""); */
			/** !flag2 (i.e. flag2 == 0) means
			 *         CALLCAR[FLOOR..4] == 0
			 */
			/* then set STATE <-- NEUTRAL or STATE <-- GOINGUP,
			 * according as CALLCAR[j] == 0 for all j > FLOOR or not,
			 */
			if (!flag2)
				/** there's no person going down in the car */
				STATE = NEUTRAL;
			else
				STATE = GOINGUP;

			/* and set all CALL variables for the current floor to zero. */
			CALLUP[FLOOR]  = 0;
			CALLDOWN[FLOOR]= 0;
			CALLCAR[FLOOR] = 0;
		}
	}

	xlog(LOG_E, "%s.\n", get_STATE_str());

	in_WAIT(TIME, E3, NO_ARG);			/** open doors */
}

/*
 * E3. [Open doors.] Set D1 and D2 to any nonzero values. Set elevator activity
 *     E9 to start up independently after 300 units of time. (This activity may be
 *     canceled in step E6 below before it occurs. If it has already been scheduled
 *     and not canceled, we cancel it and reschedule it.) Also set elevator activity
 *     E5 to start up independently after 76 units of time. Then wait 20 units of
 *     time (to simulate opening of the doors) and go to E4.
 */
void E3(void)
{
	xlog(LOG_E, "E3(). Open doors.\n");

	D1 = 1;
	D2 = 1;
	if (dormant) {
		dormant = 0;
		xlog(LOG_E, "E3(). Elevator no more dormant.\n");
	}

	in_WAIT(TIME + 20, E4, NO_ARG); /** let people out, in */
	in_WAIT(TIME + 76, E5, NO_ARG); /** close doors */
	out_WAIT(E9, 0);		/** cancel E9 if it has been scheduled */
	in_WAIT(TIME + 300, E9, NO_ARG); /** set inaction indicator */
}

/*
 * E4. [Let people out, in.] If anyone in the ELEVATOR list has OUT == FLOOR, send
 *     the user of this type who has most recently entered immediately to step U6,
 *     wait 25 units, and repeat step E4. If no such users exist, but QUEUE[FLOOR]
 *     is not empty, send the front person of that queue immediately to step U5
 *     instead of U4, wait 25 units, and repeat step E4. But if QUEUE[FLOOR]
 *     is empty, set D1 <-- 0, make D3 nonzero, and wait for some other activity
 *     to initiate further action. (Step E5 will send us to E6, or step U2 will
 *     restart E4.)
 */
void E4(void)
{
	USER_NODE * p = ELEVATOR.ER;
	for (; p != &ELEVATOR; p = p->ER) {
		int out = p->uinfo->OUT;
		if (out == FLOOR) {
			xlog(LOG_E, "E4(). %s is sent to U6().\n",
			     p->uinfo->name);
			U6(p->uinfo);
			in_WAIT(TIME + 25, E4, NO_ARG); /** empty the elevator */
			return;
		}
	}

	p = QUEUE[FLOOR].ER;
	if (p != &QUEUE[FLOOR]) { /** QUEUE[FLOOR] is not empty */
		xlog(LOG_E, "E4(). %s is sent to U5().\n",
		     p->uinfo->name);
		out_WAIT(U4, p->uinfo);	     /** fortunately the elevator comes */

		/** the sequence of these two lines cannot be exchanged, because
		 *  it should be guarranteed that ``E5'' in this line (called in U5):
		 *      in_WAIT(TIME + 25, E5, NO_ARG);
		 *  should be scheduled after ``E4'' of in the following in_WAIT():
		 */
		in_WAIT(TIME + 25, E4, NO_ARG); /** fill the elevator */
		U5(p->uinfo);
	}
	else {
		D1 = 0;
		D3 = 1;
		xlog(LOG_E, "E4(). Nobody is here. "
		     "wait for some other activity to initiate further action\n");
		return;
	}
}

/*
 * E5. [Close doors.] If D1 != 0, wait 40 units and repeat this step (the doors flutter
 *     a little, but they spring open again, since someone is still getting out or in).
 *     Otherwise set D3 <-- 0 and set the elevator to start at step E6 after 20 units
 *     of time. (This simulates closing the doors after people have finished getting
 *     in or out; but if a new user enters on this floor while the doors are closing,
 *     they will open again as stated in step U2.)
 */
void E5(void)
{
	if (D1) {
		xlog(LOG_E, "E5(). Doors flutter.\n");
		in_WAIT(TIME + 40, E5, NO_ARG); /** close the door later */
		return;
	}
	D3 = 0;
	in_WAIT(TIME + 20, E6, NO_ARG); /** prepare to move */

	xlog(LOG_E, "E5(). Doors closed.\n");
}

/*
 * E6. [Prepare to move.] Set CALLCAR[FLOOR] to zero; also set CALLUP[FLOOR]
 *     to zero if STATE != GOINGDOWN, and also set CALLDOWN[FLOOR] to zero if
 *     STATE != GOINGUP. (Note: If STATE == GOINGUP, the elevator does not clear
 *     out CALLDOWN, since it assumes that people who are going down will not
 *     have entered; but see exercise 6.) Now perform the DECISION subroutine.
 *         If STATE == NEUTRAL even after the DECISION subroutine has acted, go
 *     to E1. Otherwise, if D2 != 0, cancel the elevator activity E9. Finally, if
 *     STATE == GOINGUP, wait 15 units of time (for the elevator to build up speed)
 *     and go to E7; if STATE == GOINGDOWN, wait 15 units and go to E8.
 */
void E6(void)
{
	xlog(LOG_E, "E6(). Prepare to move.\n");

	if (dormant) {
		dormant = 0;
		xlog(LOG_E, "E6(). Elevator no more dormant.\n");
	}

	CALLCAR[FLOOR] = 0;
	if (STATE != GOINGDOWN)
		CALLUP[FLOOR] = 0;
	if (STATE != GOINGUP)
		CALLDOWN[FLOOR] = 0;

	D();

	if (STATE == NEUTRAL) {
		in_WAIT(TIME, E1, NO_ARG); /** wait for call */
		return;
	}
	else {
		if (D2) {
			out_WAIT(E9, 0); /** cancel E9 */
			xlog(LOG_E, "E6(). cancelled E9()\n");
		}
	}

	if (STATE == GOINGUP) {
		in_WAIT(TIME + 15, E7, NO_ARG); /** go up a floor */
	} else if (STATE == GOINGDOWN) {
		in_WAIT(TIME + 15, E8, NO_ARG); /** go down a floor */
	} else {
		assert(0);
	}
}

/*
 * E7. [Go up a floor.] Set FLOOR <-- FLOOR + 1 and wait 51 units of time. If
 *     now CALLCAR[FLOOR] == 1 or CALLUP[FLOOR] == 1, or
 *     if ((FLOOR == 2 or CALLDOWN[FLOOR] == 1) and
 *         CALLUP[j] == CALLDOWN[j] == CALLCAR[j] == 0 for all j > FLOOR),
 *     wait 14 units (for deceleration) and go to E2. Otherwise,
 *     repeat this step.
 */
void E7A(void)
{
	int j;
	int all_zero = 1;
	for (j = MAX_FLOOR; j > FLOOR; j--) {
		if (CALLUP[j] || CALLDOWN[j] || CALLCAR[j]) {
			all_zero = 0;
			break;
		}
	}
	if (CALLCAR[FLOOR] || CALLUP[FLOOR] ||
	    ((FLOOR == 2 || CALLDOWN[FLOOR]) && all_zero)) {
		xlog(LOG_E,
		     "E7A(). "
		     "FLOOR:%d. CALLCAR[%d]:%d. "
		     "CALLUP[%d]:%d. CALLDOWN[%d]:%d. "
		     "CALLUP[0..4]:%d,%d,%d,%d,%d. "
		     "CALLDOWN[0..4]:%d,%d,%d,%d,%d."
		     "CALLCAR[0..4]:%d,%d,%d,%d,%d.\n",
		     FLOOR, FLOOR, CALLCAR[FLOOR],
		     FLOOR, CALLUP[FLOOR], FLOOR, CALLDOWN[FLOOR],
		     CALLUP[0], CALLUP[1], CALLUP[2], CALLUP[3], CALLUP[4],
		     CALLDOWN[0], CALLDOWN[1], CALLDOWN[2], CALLDOWN[3], CALLDOWN[4],
		     CALLCAR[0], CALLCAR[1], CALLCAR[2], CALLCAR[3], CALLCAR[4]);

		in_WAIT(TIME + 14, E2, NO_ARG); /** change state */
		return;
	}
	else {
		E7();		/** repeat E7 */
	}
}
void E7(void)
{
	FLOOR++;
	assert(FLOOR <= MAX_FLOOR);

	xlog(LOG_E, "E7(). Elevator goes up a floor. (%d->%d)\n", FLOOR-1, FLOOR);

	in_WAIT(TIME + 51, E7A, NO_ARG); /** wait 51 units of time */
}

/*
 * E8. [Go down a floor.] This step is like E7 with directions reversed, and also
 *     the times 51 and 14 are changed to 61 and 23, respectively. (It takes the
 *     elevator longer to go down than up.)
 */
void E8A(void)
{
	int j;
	int all_zero = 1;
	for (j = MIN_FLOOR; j < FLOOR; j++) {
		if (CALLUP[j] || CALLDOWN[j] || CALLCAR[j]) {
			/** the car need not go downstairs anymore
			 *  because nobody's there and nobody's getting there
			 */
			all_zero = 0;
			break;
		}
	}
	xlog(LOG_E,
	     "E8A(). "
	     "FLOOR:%d. CALLCAR[%d]:%d. "
	     "CALLUP[%d]:%d. CALLDOWN[%d]:%d. "
	     "CALLUP[0..4]:%d,%d,%d,%d,%d. "
	     "CALLDOWN[0..4]:%d,%d,%d,%d,%d."
	     "CALLCAR[0..4]:%d,%d,%d,%d,%d.\n",
	     FLOOR, FLOOR, CALLCAR[FLOOR],
	     FLOOR, CALLUP[FLOOR], FLOOR, CALLDOWN[FLOOR],
	     CALLUP[0], CALLUP[1], CALLUP[2], CALLUP[3], CALLUP[4],
	     CALLDOWN[0], CALLDOWN[1], CALLDOWN[2], CALLDOWN[3], CALLDOWN[4],
	     CALLCAR[0], CALLCAR[1], CALLCAR[2], CALLCAR[3], CALLCAR[4]);
	if (CALLCAR[FLOOR] || CALLDOWN[FLOOR] ||
	    ((FLOOR == 2 || CALLUP[FLOOR]) && all_zero)) {
		in_WAIT(TIME + 23, E2, NO_ARG); /** change state */
		return;
	}
	else {
		E8();		/** repeat E8 */
	}
}
void E8(void)
{
	FLOOR--;
	assert(FLOOR >= MIN_FLOOR);

	xlog(LOG_E, "E8(). Elevator goes down a floor. (%d->%d)\n", FLOOR+1, FLOOR);

	in_WAIT(TIME + 61, E8A, NO_ARG); /** wait 61 units of time */
}

/*
 * E9. [Set inaction indicator.] Set D2 <-- 0 and perform the DECISION subroutine.
 *     ( This independent action is initiated in step E3 but it is almost always
 *     canceled in step E6. See exercise 4.)  ▍
 */
void E9(void)
{
	xlog(LOG_E, "E9(). Set inaction indicator.\n");

	/** if E9 is being excuted, the elevator must be
	 *  in ``dormant'' position E1
	 */
	assert(dormant);

	/** E9 is not considered as an elevator step, so
	 *  if the elevator is in this "step", its real
	 *  step should be set.
	 *  Because E9 is executed only when the elevator
	 *  is in ``dormant'' position E1, current_elevator_step
	 *  is set to E1:
	 */
	current_elevator_step = 0xE1;

	D2 = 0;
	D();
}

void dummy_func(void)
{
	assert(0);
}

/*
 * +      +      +      -+-   +    +
 * |\    /|     / \      |    |\   |
 * | \  / |    /___\     |    | \  |
 * |  \/  |   /     \    |    |  \ |
 * |      |  /       \  -+-   |   \|
 */
int main()
{
	int i;
	for (i = MIN_FLOOR; i <= MAX_FLOOR; i++) {
		QUEUE[i].QL = &QUEUE[i];
		QUEUE[i].QR = &QUEUE[i];
	}
	WAIT.WL = &WAIT;
	WAIT.WR = &WAIT;
	WAIT.ent_time = 0;
	WAIT.inst = dummy_func;
	ELEVATOR.EL = &ELEVATOR;
	ELEVATOR.ER = &ELEVATOR;

	/* xlog_state("***** data structures *****"); */

	in_WAIT(0, E1, NO_ARG);

	for(i = 0; i < sizeof(users) / sizeof(users[0]) - 1; i++) {
		xlog(LOG_PLAIN, "IN:%d; OUT:%d; GIVEUPTIME:%d; INTERTIME: %d; %s\n",
		     users[i].IN,
		     users[i].OUT,
		     users[i].GIVEUPTIME,
		     users[i+1].ENTERTIME - users[i].ENTERTIME,
		     users[i].name);
		in_WAIT(users[i].ENTERTIME, U1, &users[i]);
	}

	/* xlog_state("***** data structures *****"); */

	xlog(LOG_PLAIN, "\n"
	     "=================================================="
	     "execute the WAIT list"
	     "=================================================="
	     "\n");
	current_step = WAIT.WR;
	for (; current_step != &WAIT; current_step = current_step->WR) {
		const char * p = get_func_info(current_step->inst, GET_FUNC_NAME);
		if (p[0] == 'E') {
			current_elevator_step = p[1] - '0' + 0xE0;

			assert(current_elevator_step >= 0xE1);
			assert(current_elevator_step <= 0xE9);
		}
		xlog_state("", 1);
		xlog(LOG_FUNC_NAME, " %s - %s\n",
		     get_func_info(current_step->inst, GET_FUNC_NAME),
		     get_func_info(current_step->inst, GET_FUNC_DESC));

		/* execute this node */
		if (p[0] == 'E') {
			funcE fe = (funcE)current_step->inst;
			assert(current_step->arg == 0);
			fe();
		}
		else if (p[0] == 'U') {
			funcU fu = (funcU)current_step->inst;
			fu(current_step->arg);
		}

		assert(current_elevator_step != 0xE9); /* see E9() */
	}
	xlog_state("", 1);

	/* done */
	xlog(LOG_CLOSE, "\nDONE.\n");
	exit(EXIT_SUCCESS);
}
