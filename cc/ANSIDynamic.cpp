/*******************************************************************
ANSI Dynamic Demo 1:  ANSI Dynamic SQL with value semantics,
                                   literal descriptor names
                                   and ANSI type codes

This program demonstates using ANSI Dynamic SQL to process SQL
statements which are not known until runtime.  It is intended to
demonstrate the simplest (though not the most efficient) approach
to using ANSI Dynamic SQL.  It uses ANSI compatible value semantics
and ANSI type codes. ANSI Sqlstate is used for error numbers.
Descriptor names are literals. All input and output is via ANSI the
varying character type.

The program connects you to TimesTen using your username, password,
and connect string, then prompts you for a SQL statement.
Enter legal SQL or PL/SQL statements using regular, not embedded,
SQL syntax and terminate each statement with a seimcolon.
Your statement will be processed.  If it is a query, the fetched
rows are displayed.

You can enter multi-line statements.  The limit is 1023 characters.
There is a limit on the size of the variables, MAX_VAR_LEN, defined as 255.
This program processes up to 40 bind variables and 40 select-list items.
DML returning statments and user defined types are not supported with
value semantics.

Precompile the program with mode=ansi, i.e:

proc mode=ansi ansidyn1

Using mode=ansi will set dynamic and type_code to ansi.

*******************************************************************/

#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <stdlib.h>
#include <sqlcpr.h>

#include "tt_version.h"

#define MAX_OCCURENCES 40
#define MAX_VAR_LEN    255
#define MAX_NAME_LEN   31

#define SERVICE_SIZE   256

#ifndef NULL
#define NULL  0
#endif


/* Prototypes */
#if defined(__STDC__)
void sql_error(void);
int dbConnect( int argc, char *argv[] );
int get_dyn_statement(void);
int process_input(void);
int process_output(void);
void help(void);
void usage(char *prog);
void parse_args(int argc, char **argv);
extern  int chg_echo(int echo_on);
#else
void sql_error(/*_ void _*/);
int dbConnect( int, char ** );
int get_dyn_statement(/* void _*/);
int process_input(/*_ void _*/);
int process_output(/*_ void _*/);
void help(/*_ void _*/);
void usage(char *);
void parse_args(int, char **);
extern  int chg_echo(int echo_on);
#endif

#define MAX_USERNAME     31
#define MAX_SERVICENAME 128

extern char  username[];
extern char  password[];
extern char  service[ ];

EXEC SQL INCLUDE sqlca;

char SQLSTATE[6];

/* global variables */
EXEC SQL BEGIN DECLARE SECTION;
	char    dyn_statement[1024];
	char SQLSTATE[6];
	char  user[MAX_USERNAME];
	char  pass[MAX_USERNAME];
	char  svc[MAX_SERVICENAME];
EXEC SQL END DECLARE SECTION;


/* Define a buffer to hold longjmp state info. */
jmp_buf jmp_continue;

/* A global flag for the error routine. */
int parse_flag = 0;
/* A global flag to indicate statement is a select */
int select_found;

int main(int argc, char *argv[] ) {

	/* Connect to the database. */
	if (dbConnect(argc, argv) != 0)
		exit(1);

	EXEC SQL WHENEVER SQLERROR DO sql_error();

	/* Allocate the input and output descriptors. */
	EXEC SQL ALLOCATE DESCRIPTOR 'input_descriptor';
	EXEC SQL ALLOCATE DESCRIPTOR 'output_descriptor';

	/* Process SQL statements. */
	for (;;) {
		(void) setjmp(jmp_continue);

		/* Get the statement.  Break on "exit". */
		if (get_dyn_statement() != 0)
			break;

		/* Prepare the statement and declare a cursor. */
		parse_flag = 1;     /* Set a flag for sql_error(). */
		EXEC SQL PREPARE S FROM :dyn_statement;
		parse_flag = 0;     /* Unset the flag. */

		EXEC SQL DECLARE C CURSOR FOR S;

		/* Call the function that processes the input. */
		if (process_input())
			exit(1);

		/* Open the cursor and execute the statement. */
		EXEC SQL OPEN C USING DESCRIPTOR 'input_descriptor';

		/* Call the function that processes the output. */
		if (process_output())
			exit(1);

		/* Close the cursor. */
		EXEC SQL CLOSE C;

	}   /* end of for(;;) statement-processing loop */


	/* Deallocate the descriptors */
	EXEC SQL DEALLOCATE DESCRIPTOR 'input_descriptor';
	EXEC SQL DEALLOCATE DESCRIPTOR 'output_descriptor';

	EXEC SQL WHENEVER SQLERROR CONTINUE;
	EXEC SQL COMMIT WORK RELEASE;
	puts("\nHave a good day!\n");

	EXEC SQL WHENEVER SQLERROR DO sql_error();
	return 0;
}



int get_dyn_statement() {
	char *cp, linebuf[256];
	int iter, plsql;

	for (plsql = 0, iter = 1; ;) {
		if (iter == 1) {
			printf("\nSQL> ");
			dyn_statement[0] = '\0';
			select_found = 0;
		}

		fgets(linebuf, sizeof linebuf, stdin);

		cp = strrchr(linebuf, '\n');
		if (cp && cp != linebuf)
			*cp = ' ';
		else if (cp == linebuf)
			continue;

		if ((strncmp(linebuf, "SELECT", 6) == 0) ||
		        (strncmp(linebuf, "select", 6) == 0)) {
			select_found=1;;
		}

		if ((strncmp(linebuf, "EXIT", 4) == 0) ||
		        (strncmp(linebuf, "exit", 4) == 0)) {
			return -1;
		}

		else if (linebuf[0] == '?' ||
		         (strncmp(linebuf, "HELP", 4) == 0) ||
		         (strncmp(linebuf, "help", 4) == 0)) {
			help();
			iter = 1;
			continue;
		}

		if (strstr(linebuf, "BEGIN") ||
		        (strstr(linebuf, "begin"))) {
			plsql = 1;
		}

		strcat(dyn_statement, linebuf);

		if ((plsql && (cp = strrchr(dyn_statement, '/'))) ||
		        (!plsql && (cp = strrchr(dyn_statement, ';')))) {
			*cp = '\0';
			break;
		} else {
			iter++;
			printf("%3d  ", iter);
		}
	}
	return 0;
}


int process_input() {
	int i;
	EXEC SQL BEGIN DECLARE SECTION;
		char name[31];
		int  input_count, input_len, occurs, ANSI_varchar_type;
		char input_buf[MAX_VAR_LEN];
	EXEC SQL END DECLARE SECTION;

	EXEC SQL DESCRIBE INPUT S USING DESCRIPTOR 'input_descriptor';
	EXEC SQL GET DESCRIPTOR 'input_descriptor' :input_count = COUNT;

	ANSI_varchar_type=12;
	for (i=0; i < input_count; i++) {
		occurs = i +1;                       /* occurence is 1 based */
		EXEC SQL GET DESCRIPTOR 'input_descriptor'
		         VALUE :occurs :name = NAME;
		printf ("\nEnter value for input variable %*.*s:  ", 10,31, name);
		fgets(input_buf, sizeof(input_buf), stdin);
		input_len = strlen(input_buf) - 1;  /* get rid of new line */
		input_buf[input_len] = '\0';        /* null terminate */
		EXEC SQL SET DESCRIPTOR 'input_descriptor'
		         VALUE :occurs TYPE = :ANSI_varchar_type,
		                       LENGTH = :input_len,
		                       DATA = :input_buf;
	}
	return(sqlca.sqlcode);
}


int process_output() {
	int i;
	EXEC SQL BEGIN DECLARE SECTION;
		int output_count, occurs, type, len;
		short indi;
		char data[MAX_VAR_LEN], name[MAX_NAME_LEN];
	EXEC SQL END DECLARE SECTION;
	if (!select_found)
		return(0);

	EXEC SQL DESCRIBE OUTPUT S USING DESCRIPTOR 'output_descriptor';

	EXEC SQL GET DESCRIPTOR 'output_descriptor' :output_count = COUNT;


	printf ("\n");
	type = 12;            /* ANSI VARYING character type */
	len = MAX_VAR_LEN;    /* use the max allocated length */
	for (i = 0; i < output_count; i++) {
		occurs = i + 1;
		EXEC SQL GET DESCRIPTOR 'output_descriptor' VALUE :occurs
		         :name = NAME;
		printf("%-*.*s ", 9,9, name);
		EXEC SQL SET DESCRIPTOR 'output_descriptor' VALUE :occurs
		         TYPE = :type, LENGTH = :len;
	}
	printf("\n");

	/* FETCH each row selected and print the column values. */
	EXEC SQL WHENEVER NOT FOUND GOTO end_select_loop;

	for (;;) {
		EXEC SQL FETCH C INTO DESCRIPTOR 'output_descriptor';
		for (i=0; i < output_count; i++) {
			occurs = i + 1;
			EXEC SQL GET DESCRIPTOR 'output_descriptor' VALUE :occurs
			     :data = DATA, :indi = INDICATOR;
			if (indi == -1)
				printf("%-*.*s ", 9,9, "NULL");
			else
				printf("%-*.*s ", 9,9, data);  /* simplified output formatting */
			/* truncation will occur, but columns will line up */
		}
		printf ("\n");
	}
end_select_loop:
	return(0);
}



void help() {
	puts("\n\nEnter a SQL statement or a PL/SQL block at the SQL> prompt.");
	puts("Statements can be continued over several lines, except");
	puts("within string literals.");
	puts("Terminate a SQL statement with a semicolon.");
	puts("Terminate a PL/SQL block (which can contain embedded semicolons)");
	puts("with a slash (/).");
	puts("Typing \"exit\" (no semicolon needed) exits the program.");
	puts("You typed \"?\" or \"help\" to get this message.\n\n");
}


void sql_error() {
	/* error handler */
	printf("\n\nANSI sqlstate: %s: ", SQLSTATE);
	printf ("\n\n%.70s\n",sqlca.sqlerrm.sqlerrmc);
	if (parse_flag)
		printf
		("Parse error at character offset %d in SQL statement.\n",
		 sqlca.sqlerrd[4]);

	EXEC SQL WHENEVER SQLERROR CONTINUE;
	EXEC SQL ROLLBACK WORK;
	longjmp(jmp_continue, 1);
}


int dbConnect( int argc, char *argv[] ) {
	char err_msg[512];
	size_t buf_len, msg_len;

	/* parse the command line arguments */
	parse_args(argc,argv);

	/* Assign the char array components */
	strncpy((char *) user, (const char *) username, MAX_USERNAME);
	strncpy((char *) svc,  (const char *) service,  MAX_SERVICENAME);
	strncpy((char *) pass, (const char *) password, MAX_USERNAME);

	/* hide password */
	memset(password, 0, MAX_USERNAME);

	printf("\nConnecting as %s@%s\n", user, svc);

	EXEC SQL WHENEVER SQLERROR GOTO connect_error;

	/* Connect to Oracle. */
	EXEC SQL CONNECT :user IDENTIFIED BY :pass USING :svc;

	/* hide password */
	memset(pass, 0, MAX_USERNAME);

	printf("Connected\n\n");


	printf("\nYou are now in a SQL intrepreter shell using ANSI Dynamic SQL.\n");
	printf("Please enter SQL DDL and DML statements, eg create table, select, commit etc\n\n");

	return 0;

connect_error:
	if (0 != service[0])
		printf("\nCannot connect as %s@%s\n\n", username, service);
	else
		printf("\nCannot connect as user %s\n\n", username);

	buf_len = sizeof (err_msg);
	sqlglm((unsigned char *) err_msg, &buf_len, &msg_len);
	printf("%.*s\n\n", (int) msg_len, err_msg);

	return 1;
}

