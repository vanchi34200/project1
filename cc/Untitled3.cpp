/*
 *  sample3.pc
 *  Host Arrays
 *
 *  This program connects to ORACLE, declares and opens a cursor,
 *  fetches in batches using arrays, and prints the results using
 *  the function print_rows().
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlca.h>

#define NAME_LENGTH        8
#define ARRAY_LENGTH       5
#define MAX_USERNAME     31
#define MAX_SERVICENAME 128

/* Prototypes */
#if defined(__STDC__)
void usage(char *prog);
void parse_args(int argc, char **argv);
extern  int chg_echo(int echo_on);
#else
void usage(char *);
void parse_args(int, char **);
extern  int chg_echo(int echo_on);
#endif


extern char  username[];
extern char  password[];
extern char  service[ ];


/* Declare a host structure tag. */
struct {
	int    emp_number[ARRAY_LENGTH];
	char   emp_name[ARRAY_LENGTH][NAME_LENGTH];
	float  salary[ARRAY_LENGTH];
} emp_rec;

void print_rows(n)
int n;
{
	int i;

	printf("\nNumber  Employee  Salary");
	printf("\n------  --------  -------\n");

	for (i = 0; i < n; i++)
		printf("%d    %s  %8.2f\n", emp_rec.emp_number[i],
		       emp_rec.emp_name[i], emp_rec.salary[i]);

}

void sql_error(msg)
char *msg;
{
	EXEC SQL WHENEVER SQLERROR CONTINUE;

	printf("\n%s", msg);
	printf("\n %.70s \n", sqlca.sqlerrm.sqlerrmc);

	EXEC SQL ROLLBACK WORK RELEASE;
	exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
	int  num_ret;               /* number of rows returned */

	VARCHAR     user[ MAX_USERNAME ];      /* VARCHAR is an Oracle-supplied struct */
	varchar     pass[ MAX_USERNAME ];      /* varchar can be in lower case also. */
	VARCHAR     svc[  MAX_SERVICENAME];

	/* Connect to ORACLE. */
	EXEC SQL WHENEVER SQLERROR DO sql_error("Connect error:");

	/* parse the command line arguments */
	parse_args(argc, argv);

	/* Assign the VARCHAR char array components */
	strncpy((char *) user.arr, (const char *) username, MAX_USERNAME);
	strncpy((char *) svc.arr,  (const char *) service,  MAX_SERVICENAME);
	strncpy((char *) pass.arr, (const char *) password, MAX_USERNAME);

	/* hide password */
	memset(password, 0, MAX_USERNAME);

	/* Assign the VARCHAR length components */
	user.len =  (unsigned short) strlen((char *) user.arr);
	pass.len =  (unsigned short) strlen((char *) pass.arr);
	svc.len  =  (unsigned short) strlen((char *)  svc.arr);

	printf("\nConnecting as %s@%s\n", user.arr, svc.arr);

	/* Connect to Oracle. */
	EXEC SQL CONNECT :user IDENTIFIED BY :pass USING :svc;

	/* hide password */
	memset(pass.arr, 0, MAX_USERNAME);
	pass.len = 0;

	printf("Connected\n\n");



	EXEC SQL WHENEVER SQLERROR DO sql_error("Oracle error:");
	/* Declare a cursor for the FETCH. */
	EXEC SQL DECLARE c1 CURSOR FOR
	    SELECT empno, ename, sal FROM emp;

	EXEC SQL OPEN c1;

	/* Initialize the number of rows. */
	num_ret = 0;

	/* Array fetch loop - ends when NOT FOUND becomes true. */
	EXEC SQL WHENEVER NOT FOUND DO break;

	for (;;) {
		EXEC SQL FETCH c1 INTO :emp_rec;

		/* Print however many rows were returned. */
		print_rows(sqlca.sqlerrd[2] - num_ret);
		num_ret = sqlca.sqlerrd[2];        /* Reset the number. */
	}

	/* Print remaining rows from last fetch, if any. */
	if ((sqlca.sqlerrd[2] - num_ret) > 0)
		print_rows(sqlca.sqlerrd[2] - num_ret);

	EXEC SQL CLOSE c1;
	printf("\nAu revoir.\n\n\n");

	/* Disconnect from the database. */
	EXEC SQL COMMIT WORK RELEASE;
	exit(EXIT_SUCCESS);
}
