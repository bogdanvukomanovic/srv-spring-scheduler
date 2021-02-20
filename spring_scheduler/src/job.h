#ifndef JOB_H_
#define JOB_H_

/**
 * Configuration file example:
 * ---------------------------------------------------------------
 * RESOURCES 	5	3 5 17 2 0
 *
 * JOB_NUMBER 	6
 *
 * JOB 			0		 0   1   2 		4	1 2 3 4
 * JOB 			1		 3   5   6 		3	0 3 4
 * JOB 			2		12   1  15 		3	2 3 4
 * JOB 			3		 2   5  20 		3	0 3 4
 * JOB 			4		 6   1  16 		2	1 3
 * JOB			5		 0   3  30		0
 *
 * HEURISTIC 		d+W*T_est|E		1
 *
 * CONNECT			0			1
 * CONNECT			0			2
 * CONNECT			1			3
 * CONNECT			1			4
 * CONNECT			2			5
 * ---------------------------------------------------------------
 *
 * KEYWORD "RESOURCES":
 * 		5 - Number of shared resources.
 * 		3 5 17 2 0 - Values of Earlies Availabe Time of each Resource.
 *
 * 	KEYWORD "JOB_NUMBER":
 * 		6 - Number of Jobs in System - you need to specify the exact number of "JOB" keywords.
 *
 * 	KEYWORD "JOB":   	( JOB 		2		12   1  15 		3	2 3 4 )
 * 		2 - Parameter of function that given Job is going to execute. Needs to be 'int'. Should change to float...
 *
 * 		12 - Start time of Job.
 * 		1  - Execution time of Job.
 * 		15 - Deadline of Job.
 *
 * 		3 - Number of Resources that this Job uses.
 * 		2 3 4 - Indexes of these Resources.
 *
 * 	KEYWORD "HEURISTIC":
 * 		Example: d+W*T_est|E		1
 * 		Example: d|E
 * 		Example: d
 * 		Example: d+W*C|E	 		5
 * 		...
 * 		(!) Note: When heuristic function uses 'W', its value needs to be specified.
 *
 * 	KEYWORD "CONNECT": 	( CONNECT		0			1 )
 * 		0 - Index of predecessor.
 * 		1 - Index of successor.
 **/


// Macros:
#define JOB_COUNT 					6
#define MAX_PREDECESORS				10
#define MAX_SUCCESSORS				10
#define HEURISTIC_FUNCTION_COUNT 	8


// Enums:
enum JobState {NEW, RUNNING, FINISHED};


// Structures:
typedef struct Job
{

	TickType_t xStartTime;
	TickType_t xDeadline;
	TickType_t xExecutionTime;

	// Function and its parameters of the Job.
	void ( *func )( void *pvParams );
	void *pvParams;

	// Resources ( Indexes of the resources that Job use ):
	int *piUsingResourcesIndexes;
	int iUsingResourcesCount;

	// Predecessors and successors.
	struct Job *pxPredecessors[MAX_PREDECESORS];
	int iPredecessorCount;
	struct Job *pxSuccessors[MAX_SUCCESSORS];
	int iSuccessorCount;

	int E;			// Eligibility.
	int iChecked; 	// iChecked is 0 if Job is not scheduled in the order array.
	int iJobID;		// Job ID.

	enum JobState xJobState;
	TaskHandle_t xTaskHandle;

} Job;


typedef struct Heuristic
{

	int E;				// Eligibility.
	int FCFS;			// H = a ( First Come First Served )
	int SJF;			// H = C ( Shortest Job First )
	int EDF;			// H = d ( Earliest Deadline First )
	int ESTF;			// H = T_est ( Earliest Start Time First )
	int EDF_SJF;		// H = d + W * C
	int EDF_ESTF;		// H = d + W * T_est

	int W;		// TODO: Change to float...

} Heuristic;


typedef struct JobBatch
{

	Job *xJobArray;
	int iJobCount;

	int *piResources;
	int iResourcesCount;

	Heuristic xHeuristic;

	int *piSpringOrderResult;

} JobBatch;


// Functions:
void vMakeJob( Job *, void ( * )( void * ), void *, TickType_t, TickType_t, TickType_t, int, int );
void vUserJobFunction(void *);
void vPrintJobs( JobBatch * );
void vPrintBatch( JobBatch * );



#endif /* JOB_H_ */
