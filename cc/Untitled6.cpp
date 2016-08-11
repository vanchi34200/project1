/*
 *  sample2.pc
 *
 *  This program connects to ORACLE, declares and opens a cursor, 
 *  fetches the names, salaries, and commissions of all
 *  salespeople, displays the results, then closes the cursor. 
 */ 

#include <stdio.h>
#include <sqlca.h>

#define UNAME_LEN      20 
#define PWD_LEN        40 
 
/*
 * Use the precompiler typedef'ing capability to create
 * null-terminated strings for the authentication host
 * variables. (This isn't really necessary--plain char *'s
 * would work as well. This is just for illustration.)
 */
typedef char asciiz[PWD_LEN]; 

EXEC SQL TYPE asciiz IS STRING(PWD_LEN) REFERENCE; 
asciiz     username; 
asciiz     password; 

struct emp_info 
{ 
    asciiz     emp_name; 
    float      salary; 
    float      commission; 
}; 


/* Declare function to handle unrecoverable errors. */ 
void sql_error(); 


main() 
{ 
    struct emp_info *emp_rec_ptr; 

/* Allocate memory for emp_info struct. */ 
    if ((emp_rec_ptr = 
        (struct emp_info *) malloc(sizeof(struct emp_info))) == 0)
    { 
        fprintf(stderr, "Memory allocation error.\n"); 
        exit(1); 
    } 
 
/* Connect to ORACLE. */ 
    strcpy(username, "SCOTT"); 
    strcpy(password, "TIGER"); 
 
    EXEC SQL WHENEVER SQLERROR DO sql_error("ORACLE error--");
 
    EXEC SQL CONNECT :username IDENTIFIED BY :password; 
    printf("\nConnected to ORACLE as user: %s\n", username); 
 
/* Declare the cursor. All static SQL explicit cursors
 * contain SELECT commands. 'salespeople' is a SQL identifier,
 * not a (C) host variable.
 */
    EXEC SQL DECLARE salespeople CURSOR FOR 
        SELECT ENAME, SAL, COMM 
            FROM EMP 
            WHERE JOB LIKE 'SALES%'; 
 
/* Open the cursor. */
    EXEC SQL OPEN salespeople; 
 
/* Get ready to print results. */
    printf("\n\nThe company's salespeople are--\n\n");
    printf("Salesperson   Salary   Commission\n"); 
    printf("-----------   ------   ----------\n"); 
 
/* Loop, fetching all salesperson's statistics.
 * Cause the program to break the loop when no more
 * data can be retrieved on the cursor.
 */
    EXEC SQL WHENEVER NOT FOUND DO break; 

    for (;;) 
    { 
        EXEC SQL FETCH salespeople INTO :emp_rec_ptr; 
        printf("%-11s%9.2f%13.2f\n", emp_rec_ptr->emp_name, 
                emp_rec_ptr->salary, emp_rec_ptr->commission); 
    } 
 
/* Close the cursor. */
    EXEC SQL CLOSE salespeople; 
 
    printf("\nArrivederci.\n\n");

    EXEC SQL COMMIT WORK RELEASE; 
    exit(0); 
} 



void 
sql_error(msg) 
char *msg;
{ 
    char err_msg[512];
    int buf_len, msg_len;

    EXEC SQL WHENEVER SQLERROR CONTINUE;

    printf("\n%s\n", msg);

/* Call sqlglm() to get the complete text of the
 * error message.
 */
    buf_len = sizeof (err_msg);
    sqlglm(err_msg, &buf_len, &msg_len);
    printf("%.*s\n", msg_len, err_msg);

    EXEC SQL ROLLBACK RELEASE;
    exit(1);
} 
