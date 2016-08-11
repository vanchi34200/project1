/***************************************************************
sample4.pc
This program demonstrates the use of type equivalencing using the
LONG RAW external datatype.  In order to provide a useful example
that is portable across different systems, the program inserts
binary files into and retrieves them from the database.  For
example, suppose you have a file called 'hello' in the current
directory.  You can create this file by compiling the following
source code:

#include <stdio.h>

int
main()
{
  printf("Hello World!\n");
}

When this program is run, we get:

$hello
Hello World!

Here is some sample output from a run of sample4:

$sample4
Connected.
Do you want to create (or recreate) the EXECUTABLES table (y/n)? y
EXECUTABLES table successfully dropped.  Now creating new table...
EXECUTABLES table created.

Sample 4 Menu.  Would you like to:
(I)nsert a new executable into the database
(R)etrieve an executable from the database
(L)ist the executables currently stored in the database
(Q)uit the program

Enter i, r, l, or q: l

Executables currently stored:
----------- --------- ------

Total: 0

Sample 4 Menu.  Would you like to:
(I)nsert a new executable into the database
(R)etrieve an executable from the database
(L)ist the executables currently stored in the database
(Q)uit the program

Enter i, r, l, or q: i
Enter the key under which you will insert this executable: hello
Enter the filename to insert under key 'hello'.
If the file is not in the current directory, enter the full
path: hello
Inserting file 'hello' under key 'hello'...
Inserted.

Sample 4 Menu.  Would you like to:
(I)nsert a new executable into the database
(R)etrieve an executable from the database
(L)ist the executables currently stored in the database
(Q)uit the program

Enter i, r, l, or q: l

Executables currently stored:
----------- --------- ------
hello

Total: 1

Sample 4 Menu.  Would you like to:
(I)nsert a new executable into the database
(R)etrieve an executable from the database
(L)ist the executables currently stored in the database
(Q)uit the program

Enter i, r, l, or q: r
Enter the key for the executable you wish to retrieve: hello
Enter the file to write the executable stored under key hello into.  If you
don't want the file to be in the current directory, enter the
full path: h1
Retrieving executable stored under key 'hello' to file 'h1'...
Retrieved.

Sample 4 Menu.  Would you like to:
(I)nsert a new executable into the database
(R)etrieve an executable from the database
(L)ist the executables currently stored in the database
(Q)uit the program

Enter i, r, l, or q: q

We now have the binary file 'h1' created, and we can run it:

$h1
Hello World!
***************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <fcntl.h>
#include <string.h>
#include <sqlca.h>

/* Oracle error code for 'table or view does not exist'. */
#define NON_EXISTENT -942

/* This is the maximum size (in bytes) of a file that
 * can be inserted and retrieved.
 * If your system cannot allocate this much contiguous
 * memory, this value might have to be lowered.
 */
#define MAX_FILE_SIZE 500000


/* This is the definition of the long varraw structure.
 * Note that the first field, len, is a long instead
 * of a short.  This is becuase the first 4
 * bytes contain the length, not the first 2 bytes.
 */
typedef struct {
    long len;
    char buf[MAX_FILE_SIZE];
} long_varraw;


/* Type Equivalence long_varraw to long varraw.
 * All variables of type long_varraw from this point
 * on in the file will have external type 95 (long varraw)
 * associated with them.
 */
EXEC SQL type long_varraw is long varraw (MAX_FILE_SIZE);


/* This program's functions declared. */
void do_connect();
void create_table();
void sql_error();
void list_executables();
void print_menu();



main()
{
    char reply[20], key[20], filename[100];
    int ok = 1;

    /* Connect to the database. */
    do_connect();

    printf("Do you want to create (or recreate) the EXECUTABLES table (y/n)? ");
    gets(reply);

    if ((reply[0] == 'y') || (reply[0] == 'Y'))
        create_table();

    /* Print the menu, and read in the user's selection. */
    print_menu();
    gets(reply);

    while (ok) {
        switch(reply[0]) {
            case 'I':
            case 'i':
                /* User selected insert - get the key and file name. */
                printf("
                       Enter the key under which you will insert this executable: ");
                gets(key);
                printf(
                    "Enter the filename to insert under key '%s'.\n", key);
                printf(
                    "If the file is not in the current directory, enter the full\n");
                printf("path: ");
                gets(filename);
                insert(key, filename);
                break;
            case 'R':
            case 'r':
                /* User selected retrieve - get the key and file name. */
                printf(
                    "Enter the key for the executable you wish to retrieve: ");
                gets(key);
                printf(
                    "Enter the file to write the executable stored under key ");
                printf("%s into.  If you\n", key);
                printf(
                    "don't want the file to be in the current directory, enter the\n");
                printf("full path: ");
                gets(filename);
                retrieve(key, filename);
                break;
            case 'L':
            case 'l':
                /* User selected list - just call the list routine. */
                list_executables();
                break;
            case 'Q':
            case 'q':
                /* User selected quit - just end the loop. */
                ok = 0;
                break;
            default:
                /* Invalid selection. */
                printf("Invalid selection.\n");
                break;
        }

        if (ok) {
            /* Print the menu again. */
            print_menu();
            gets(reply);
        }
    }

    EXEC SQL commit work release;
}


/* Connect to the database. */
void
do_connect()
{

    /* Note this declaration: uid is a char *
     * pointer, so Oracle will do a strlen() on it
     * at runtime to determine the length.
     */
    char *uid = "scott/tiger";

    EXEC SQL whenever sqlerror do sql_error("Connect");
    EXEC SQL connect :uid;

    printf("Connected.\n");
}


/* Creates the executables table. */
void create_table()
{
    /* We are going to check for errors ourselves
     * for this statement. */
    EXEC SQL whenever sqlerror continue;

    EXEC SQL drop table executables;
    if (sqlca.sqlcode == 0) {
        printf("EXECUTABLES table successfully dropped.  ");
        printf("Now creating new table...\n");
    } else if (sqlca.sqlcode == NON_EXISTENT) {
        printf("EXECUTABLES table does not exist.  ");
        printf("Now creating new table...\n");
    } else
        sql_error("create_table");

    /* Reset error handler. */
    EXEC SQL whenever sqlerror do sql_error("create_table");

    EXEC SQL create table executables
      (name varchar2(20),
       binary long raw);

    printf("EXECUTABLES table created.\n");
}



/* Opens the binary file identified by 'filename' for
 * reading, and copies it into 'buf'.
 * 'bufsize' should contain the maximum size of
 * 'buf'.  Returns the actual length of the file read in,
 * or -1 if there is an error.
 */
int
read_file(filename, buf, bufsize)
char *filename, *buf;
long bufsize;
{

    /* We will read in the file LOCAL_BUFFERSIZE bytes at a time. */
#define LOCAL_BUFFERSIZE 512

    /* Buffer to store each section of the file. */
    char local_buffer[LOCAL_BUFFERSIZE];

    /* Number of bytes read each time. */
    int number_read;

    /* Total number of bytes read (the size of the file). */
    int total_size = 0;

    /* File descriptor for the input file. */
    int in_fd;

    /* Open the file for reading. */
    in_fd = open(filename, O_RDONLY, 0);
    if (in_fd == -1)
        return(-1);

    /* While loop to actually read in the file,
     * LOCAL_BUFFERSIZE bytes at a time.
     */
    while ((number_read = read(in_fd, local_buffer,
                               LOCAL_BUFFERSIZE)) > 0) {
        if (total_size + number_read > bufsize) {
            /* The number of bytes we have read in so far exceeds the buffer
             * size - close the file and return an error. */
            close(in_fd);
            return(-1);
        }

        /* Copy the bytes just read in from the local buffer
           into the output buffer. */
        memcpy(buf+total_size, local_buffer, number_read);

        /* Increment the total number of bytes read by the number
           we just read. */
        total_size += number_read;
    }

    /* Close the file, and return the total file size. */
    close(in_fd);
    return(total_size);
}


/* Generic error handler.  The 'routine' parameter
 * should contain the name of the routine executing when
 * the error occured.  This would be specified in the
 * 'EXEC SQL whenever sqlerror do sql_error()' statement.
 */
void
sql_error(routine)
char *routine;
{
    char message_buffer[512];
    int buffer_size;
    int message_length;

    /* Turn off the call to sql_error() to avoid
     * a possible infinite loop.
     */
    EXEC SQL WHENEVER SQLERROR CONTINUE;

    printf("\nOracle error while executing %s!\n", routine);

    /* Use sqlglm() to get the full text of the error message. */
    buffer_size = sizeof(message_buffer);
    sqlglm(message_buffer, &buffer_size, &message_length);
    printf("%.*s\n", message_length, message_buffer);

    EXEC SQL ROLLBACK WORK RELEASE;
    exit(1);
}


/* Opens the binary file identified by 'filename' for
 * writing, and copies the contents of 'buf' into it.
 * 'bufsize' should contain the size of 'buf'.
 * Returns the number of bytes written (should be == bufsize),
 * or -1 if there is an error.
 */
int write_file(filename, buf, bufsize)
char *filename, *buf;
long bufsize;
{
    int out_fd;        /* File descriptor for the output file. */
    int num_written;   /* Number of bytes written. */

    /* Open the file for writing.  This command replaces
     * any existing version. */
    out_fd = creat(filename, 0755);
    if (out_fd == -1) {
        /* Can't create the output file - return an error. */
        return(-1);
    }

    /* Write the contents of buf to the file. */
    num_written = write(out_fd, buf, bufsize);

    /* Close the file, and return the number of bytes written. */
    close(out_fd);
    return(num_written);
}



/* Inserts the binary file identified by file into the
 * executables table identified by key.
 */
int
insert(key, file)
char *key, *file;
{
    long_varraw lvr;

    printf("Inserting file '%s' under key '%s'...\n", file, key);
    lvr.len = read_file(file, lvr.buf, MAX_FILE_SIZE);
    if (lvr.len == -1) {
        /* File size is too big for the buffer we have -
         * exit with an error.
         */
        fprintf(stderr,
                "\n\nError while reading file '%s':\n", file);
        fprintf(stderr,
                "The file you selected to read is
                too large for the buffer.\n");
        fprintf(stderr,
                "Increase the MAX_FILE_SIZE macro in the source code,\n");
        fprintf(stderr,
                "reprecompile, compile, and link, and try again.\n");
        fprintf(stderr,
                "The current value of MAX_FILE_SIZE is %d bytes.\n",
                MAX_FILE_SIZE);

        EXEC SQL rollback work release;

        exit(1);
    }

    EXEC SQL whenever sqlerror do sql_error("insert");
    EXEC SQL insert into executables (name, binary)
      values (:key, :lvr);

    EXEC SQL commit;
    printf("Inserted.\n");
}


/* Retrieves the executable identified by key into file */
int
retrieve(key, file)
char *key, *file;
{

    /* Type equivalence key to the string external datatype.*/
    EXEC SQL VAR key is string(21);

    long_varraw lvr;
    short ind;
    int num_written;

    printf("Retrieving executable stored under key '%s' to file '%s'...\n",
           key, file);

    EXEC SQL whenever sqlerror do sql_error("retrieve");
    EXEC SQL select binary
      into :lvr :ind
      from executables
      where name = :key;

    num_written = write_file(file, lvr.buf, lvr.len);
    if (num_written != lvr.len) {
        /* Error while writing - exit with an error. */
        fprintf(stderr,
                "\n\nError while writing file '%s':\n", file);
        fprintf(stderr,
                "Can't create the output file.  Check to be sure that you\n");
        fprintf(stderr,
                "have write permissions in the directory into which you\n");
        fprintf(stderr,
                "are writing the file, and that there is enough disk space.\n");

        EXEC SQL rollback work release;

        exit(1);
    }

    printf("Retrieved.\n");
}

void
list_executables()
{
    char key[21];
    /* Type equivalence key to the string external
     * datatype, so we don't have to null-terminate it.
     */
    EXEC SQL VAR key is string(21);

    EXEC SQL whenever sqlerror do sql_error("list_executables");

    EXEC SQL declare key_cursor cursor for
      select name from executables;

    EXEC SQL open key_cursor;

    printf("\nExecutables currently stored:\n");
    printf("----------- --------- ------\n");

    while (1) {
        EXEC SQL whenever not found do break;
        EXEC SQL fetch key_cursor into :key;

        printf("%s\n", key);
    }

    EXEC SQL whenever not found continue;

    EXEC SQL close key_cursor;

    printf("\nTotal: %d\n", sqlca.sqlerrd[2]);
}

/* Prints the menu selections. */
void
print_menu()
{
    printf("\nSample 4 Menu.  Would you like to:\n");
    printf("(I)nsert a new executable into the database\n");
    printf("(R)etrieve an executable from the database\n");
    printf("(L)ist the executables currently stored in the database\n");
    printf("(Q)uit the program\n\n");
    printf("Enter i, r, l, or q: ");
}
