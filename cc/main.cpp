
/*
NAME
  sample
FUNCTION
  Simple 32-bit Pro*C sample program from Pro*C User's Guide
NOTES

   sample  is a simple example program which adds new employee
           records to the personnel data base.  Checking
           is done to insure the integrity of the data base.
           The employee numbers are automatically selected using
           the current maximum employee number + 10.

           The program queries the user for data as follows:

           Enter employee name:
           Enter employee job:
           Enter employee salary:
           Enter employee dept:

           The program terminates if control Z (end of file) or null
           string (<return> key) is entered when the employee name
           is requested.

           If the record is successfully inserted, the following
           is printed:

           ename added to department dname as employee # nnnnnn


OWNER
  Clare
DATE
  05/01/84
MODIFIED
  dhood       12/23/09 - Use parse_args()
  dhood       12/23/09 - Changed askn() and asks() to use fgets()
  dhood       01/27/09 - Prompt for password
  kakiyama    05/05/97 - replaced sqlproto.h with sqlcpr.h
  rahmed      08/10/95 - No need yo define WIN_NT
  rahmed      07/05/95 - Removed all the warnings for NT.
  syau        03/07/95 - WIN_NT: add prototype files
  Hartenstine 04/27/93 - WIN_NT: Port to Windows NT
  dcriswel    04/11/92 - IBMC: Add .H to EXEC SQL INCLUDE
  bfotedar    01/07/92 - Included stdlib.h, modified compiling instructions
  bfotedar    11/22/91 - OS2_2: Port to OS/2 2.0
  Criswell    11/08/90 - Declared main() void
  Criswell    11/01/90 - Microsoft C 6.0 changes
  Okamura     07/05/89 - Update documentation for V6 on OS/2
  RPatterson  03/26/87 - Updated error handling.
  RPatterson  02/20/86 - Port: support Microsoft 'C' compiler and sqlmsc.lib.
  Clare       09/19/84 - Port: remove VMSisms.
  Clare       12/26/84 - Port IBM: fflush() prompts.
*/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sqlda.h>
#include <sqlcpr.h>

int errrpt(void);
int asks(char *,char *, int);
int askn(char *,int *);

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

#define MAX_USERNAME     31
#define MAX_SERVICENAME 128

extern char  username[];
extern char  password[];
extern char  service[ ];

EXEC SQL INCLUDE SQLCA.H;

EXEC SQL BEGIN DECLARE SECTION;
	VARCHAR  user[31];
	VARCHAR  pass[31];
	VARCHAR  svc[128];             /* Connect String                      */


	int     empno;                     /* employee number                     */
	VARCHAR ename[10 + 1];
	/* employee name                       */
	int     deptno;                    /* department number                   */
	VARCHAR dname[14 + 1];
	/* department name                     */

	VARCHAR job[9 + 1];
	/* employee job                        */
	int     sal;                       /* employee salary                     */
EXEC SQL END DECLARE SECTION;

int main(int argc, char** argv) {

	/* --------------------------------------------------------------------------
	 logon to ORACLE, and open the cursors. The program exits if any errors occur.
	-------------------------------------------------------------------------- */

	EXEC SQL WHENEVER SQLERROR GOTO errexit;

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

	/* Connect to TimesTen or Oracle DB. */
	EXEC SQL CONNECT :user IDENTIFIED BY :pass USING :svc;

	/* hide password */
	memset(pass.arr, 0, MAX_USERNAME);
	pass.len = 0;

	printf("Connected\n\n");


	/* --------------------------------------------------------------------------
	   Retrieve the current maximum employee number
	-------------------------------------------------------------------------- */

	EXEC SQL SELECT NVL(MAX(EMPNO),0) + 10
	           INTO :empno
	           FROM EMP;

	/* --------------------------------------------------------------------------
	   read the user's input from STDIN.  If the employee name is
	   not entered, exit.
	   Verify that the entered department number is valid and echo the
	   department's name
	-------------------------------------------------------------------------- */

	for( ; ; empno+=10 ) {
		int l;

		/* Get employee name to be inserted. */
		l = asks("Enter employee name  : ", (char *)ename.arr, sizeof(ename.arr));

		if ( l <= 0 )
			break;

		ename.len = (short) l;

		job.len = (short) asks("Enter employee job   : ", (char *)job.arr, sizeof(job.arr));
		askn("Enter employee salary: ",&sal);

		for ( ; ; ) {
			if ( askn("Enter employee dept  :   ",&deptno) < 0 )
				break;

			EXEC SQL WHENEVER NOT FOUND GOTO nodept;
			EXEC SQL SELECT DNAME
			           INTO :dname
			           FROM DEPT
			           WHERE DEPTNO = :deptno;

			dname.arr[dname.len] = '\0';

			EXEC SQL WHENEVER NOT FOUND STOP;

			/* Here if deptno was found in dbs. Insert new employee into dbs. */

			EXEC SQL INSERT INTO EMP(EMPNO,ENAME,JOB,SAL,DEPTNO)
			           VALUES (:empno,:ename,:job,:sal,:deptno);

			printf("\n%s added to the %s department as employee number %d\n",
			       ename.arr,dname.arr,empno);
			break;

			/* Here if deptno NOT found in dbs */
nodept:
			printf("\nNo such department\n");
			continue;
		}
	}

	/* --------------------------------------------------------------------------
	   close the cursors and log off from TimesTen or Oracle DB
	-------------------------------------------------------------------------- */

	EXEC SQL COMMIT WORK RELEASE;
	printf ("\nEnd of the Pro*C Sample example program.\n");
	return 0;

errexit:
	errrpt();
	EXEC SQL WHENEVER SQLERROR CONTINUE;
	EXEC SQL ROLLBACK WORK RELEASE;
	return 1;
}

/*---------------------------------------------------------------------------
COUNT askn(text,variable)

   print the 'text' on STDOUT and read an integer variable from
   SDTIN.

   text points to the null terminated string to be printed
   variable points to an integer variable

   askn returns a 1 if the variable was read successfully or a
       -1 if -eof- was encountered
-------------------------------------------------------------------------- */

int askn(text,variable)
char text[];
int  *variable;
{
	char s[20];

	printf(text);
	fflush(stdout);
	if ( fgets((char *) s, sizeof(s), stdin) == (char *) 0 )
		return(EOF);

	*variable = atoi(s);
	return(1);
}

/* --------------------------------------------------------------------------
COUNT asks(text,variable)

   Printf the 'text' to STDOUT and read up to the size of the
   input buffer.

   Return either the length of the input string or 0 otherwise
----------------------------------------------------------------------- */

int asks(char * text, char * variable, int len) {
	char * temp = NULL;
	char * cp   = NULL;
	char buf[50];

	printf(text);
	fflush(stdout);

	temp = fgets((char *) variable, len, stdin);
	if (NULL == temp) {
		printf("Bad input, exiting ...\n\n");
		return 0;
	}

	cp = (char *) strchr((char *) variable, '\n');

	if (cp == variable) {
		return 0;
	}

	if (cp)
		*cp = '\0';
	else {
		printf("Data may be truncated.\n");
		fgets((char *) buf, 50, stdin);
	}

	return strlen(variable);
}

/* --------------------------------------------------------------------------
int errrpt()

   errrpt prints the ORACLE error msg and number.
-------------------------------------------------------------------------- */

int errrpt( void ) {
	printf("%.70s (%d)\n", sqlca.sqlerrm.sqlerrmc, -sqlca.sqlcode);
	return(0);
}

