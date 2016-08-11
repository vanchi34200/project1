/*-- PL/SQL source for a package that declares and
-- opens a ref cursor
CONNECT SCOTT/TIGER
CREATE OR REPLACE PACKAGE emp_demo_pkg as
   TYPE emp_cur_type IS REF CURSOR RETURN emp%ROWTYPE;
     PROCEDURE open_cur(curs IN OUT emp_cur_type, dno IN NUMBER);
END emp_demo_pkg;
 
 
CREATE OR REPLACE PACKAGE BODY emp_demo_pkg AS
    PROCEDURE open_cur(curs IN OUT emp_cur_type, dno IN NUMBER) IS
    BEGIN 
        OPEN curs FOR SELECT *
            FROM emp WHERE deptno = dno
            ORDER BY ename ASC;
    END;
END emp_demo_pkg;

sample11.pc


 *  Fetch from the EMP table, using a cursor variable.
 *  The cursor is opened in the stored PL/SQL procedure
 *  open_cur, in the EMP_DEMO_PKG package.
 *
 *  This package is available on-line in the file
 *  sample11.sql, in the demo directory.
 *
 */
 
#include <stdio.h>
#include <sqlca.h>
 
/* Error handling function. */
void sql_error();
 
main()
{
    char temp[32];
 
    EXEC SQL BEGIN DECLARE SECTION;
        char *uid = "scott/tiger";
        SQL_CURSOR emp_cursor;
        int dept_num;
        struct
        {
            int   emp_num;
            char  emp_name[11];
            char  job[10];
            int   manager;
            char  hire_date[10];
            float salary;
            float commission;
            int   dept_num;
        } emp_info;

        struct
        {
            short emp_num_ind;
            short emp_name_ind;
            short job_ind;
            short manager_ind;
            short hire_date_ind;
            short salary_ind;
            short commission_ind;
            short dept_num_ind;
        } emp_info_ind;
         EXEC SQL END DECLARE SECTION;
    
    EXEC SQL WHENEVER SQLERROR do sql_error("Oracle error");
    
/* Connect to Oracle. */
    EXEC SQL CONNECT :uid;
 
/* Allocate the cursor variable. */
    EXEC SQL ALLOCATE :emp_cursor;
 
/* Exit the inner for (;;) loop when NO DATA FOUND. */
    EXEC SQL WHENEVER NOT FOUND DO break;
 
    for (;;)
    {
        printf("\nEnter department number  (0 to exit): ");
        gets(temp);
        dept_num = atoi(temp);
        if (dept_num <= 0)
            break;
 
        EXEC SQL EXECUTE
            begin
             emp_demo_pkg.open_cur(:emp_cursor, :dept_num);
            end;
        END-EXEC;
 
        printf("\nFor department %d--\n", dept_num);
        printf("ENAME\t             SAL\t            COMM\n");
        printf("-----\t             ---\t            ----\n");
 

/* Fetch each row in the EMP table into the data struct.
   Note the use of a parallel indicator struct. */
        for (;;)
        {
             EXEC SQL FETCH :emp_cursor 
                 INTO :emp_info INDICATOR :emp_info_ind;
 
             printf("%s\t", emp_info.emp_name);
             printf("%8.2f\t\t", emp_info.salary);
             if (emp_info_ind.commission_ind != 0)
                 printf("    NULL\n");
             else
                 printf("%8.2f\n", emp_info.commission);
        }
 
    }
 
 
/* Close the cursor. */
    EXEC SQL CLOSE :emp_cursor;
    exit(0);
}
 
 
void
sql_error(msg)
char *msg;
{
    long clen, fc;
    char cbuf[128];
 
    clen = (long) sizeof (cbuf);
    sqlgls(cbuf, &clen, &fc);
 
    printf("\n%s\n", msg);
    printf("Statement is--\n%s\n", cbuf);
    printf("Function code is %ld\n\n", fc);
 
    sqlglm(cbuf, (int *) &clen, (int *) &clen);
    printf ("\n%.*s\n", clen, cbuf);
  
    EXEC SQL WHENEVER SQLERROR CONTINUE;
    EXEC SQL ROLLBACK WORK;
    exit(-1);
}
