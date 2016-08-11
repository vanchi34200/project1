
/*
 *  sample1.pc
 *
 *  Prompts the user for an employee number,
 *  then queries the emp table for the employee's
 *  name, salary and commission.  Uses indicator
 *  variables (in an indicator struct) to determine
 *  if the commission is NULL.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlda.h>
#include <sqlcpr.h>

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

#define UNAME_LEN     (10 + 1)
#define MAX_USERNAME     31
#define MAX_SERVICENAME 128

extern char  username[];
extern char  password[];
extern char  service[ ];


/* Declare variables.  No declare section is
   needed if MODE=ORACLE. */
VARCHAR     user[ MAX_USERNAME ];    /* VARCHAR is an Oracle-supplied struct */
varchar     pass[ MAX_USERNAME ];      /* varchar can be in lower case also. */
VARCHAR     svc[  MAX_SERVICENAME ];


/* Define a host structure for the output values of
   a SELECT statement.  */
struct
{
    VARCHAR   emp_name[UNAME_LEN];
    float     salary;
    float     commission;
} emprec;

/* Define an indicator struct to correspond
   to the host output struct. */
struct
{
    short     emp_name_ind;
    short     sal_ind;
    short     comm_ind;
} emprec_ind;

/*  Input host variable. */
int         emp_number;

int         total_queried;

/* Include the SQL Communications Area.
   You can use #include or EXEC SQL INCLUDE. */
#include <sqlca.h>

/* Declare error handling function. */
void sql_error(msg)
    char *msg;
{
    char err_msg[128];
    size_t buf_len, msg_len;

    EXEC SQL WHENEVER SQLERROR CONTINUE;

    printf("\n%s\n", msg);
    buf_len = sizeof (err_msg);
    sqlglm((unsigned char *) err_msg, &buf_len, &msg_len);
    printf("%.*s\n", (int) msg_len, err_msg);

    EXEC SQL ROLLBACK RELEASE;
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv)
{
    char temp_char[32];

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

  /* Register sql_error() as the error handler. */
  EXEC SQL WHENEVER SQLERROR DO sql_error("ORACLE error--\n");

  /* Connect to Oracle. */
  EXEC SQL CONNECT :user IDENTIFIED BY :pass USING :svc;

  /* hide password */
  memset(pass.arr, 0, MAX_USERNAME);
  pass.len = 0;

  printf("Connected\n\n");

/* Loop, selecting individual employee's results */

    total_queried = 0;

    for (;;)
    {
        emp_number = 0;
        printf("\nEnter employee number (0 to quit): ");
        fgets(temp_char, sizeof(temp_char), stdin);
        emp_number = atoi(temp_char);
        if (emp_number == 0)
            break;

/* Branch to the notfound label when the
 * 1403 ("No data found") condition occurs.
 */
        EXEC SQL WHENEVER NOT FOUND GOTO notfound;

        EXEC SQL SELECT ename, sal, comm
            INTO :emprec INDICATOR :emprec_ind
            FROM EMP
            WHERE EMPNO = :emp_number;

/* Print data. */
        printf("\n\nEmployee   Salary    Commission\n");
        printf("--------   -------   ----------\n");

/* Null-terminate the output string data. */
        emprec.emp_name.arr[emprec.emp_name.len] = '\0';
        printf("%s      %7.2f      ",
            emprec.emp_name.arr, emprec.salary);

        if (emprec_ind.comm_ind == -1)
            printf("NULL\n");
        else
            printf("%7.2f\n", emprec.commission);

        total_queried++;
        continue;

notfound:
        printf("\nNot a valid employee number - try again.\n");

    } /* end for(;;) */

    printf("\n\nTotal rows returned was %d.\n", total_queried); 
    printf("\nG'day.\n\n\n");

/* Disconnect from ORACLE. */
    EXEC SQL ROLLBACK WORK RELEASE;
    exit(EXIT_SUCCESS);
}

