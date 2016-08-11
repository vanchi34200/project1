/********************************************************************************
 *******  Copyright(C), 2000, for Klarity-ACE(Yield Enhancemant Solution)  ******
 ********************************************************************************

 Title         : NanYa2 Wafer History EDA LOADER
 Date          : 06/23/2000
 Vesrion       : 1.0
 Initial Author: Felix Lai
 Modified Log  :

 **Formate:
 **MM/DD/RR <name>
 **  <content>
 ******************************************************************************/
   
/******************************************************************************
                            definitions section
*******************************************************************************/
/* set run-time environment */
#define _UNIX_
#define _nTEST_
#define _nCHECK_
#define _nDEBUG_


/*****************************************************************************/
/*                              include section                              */
/*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include <errno.h>

/* In PC environment, the following item need be marked */

EXEC SQL INCLUDE "../nanya2.h";

/* In Unix environment, the following two items need be marked */
/*
EXEC SQL INCLUDE "..\\NanYa2.h";
EXEC SQL INCLUDE sqlproto;
*/
EXEC SQL INCLUDE sqlca;

/***** set run-time enviroment *****/
#ifdef _UNIX_
    #define     SEPS        "/"
    #define     INI_DIR     "../fab2wafer"
    #define     COPY        " cp "
    #define     MOVE        " mv -f "
    #define     REMOVE      " rm "
    #define     LIST        " ls "
    #define     INIFILE     "nanya2WaferHist.ini"
    #define     FILELIST    "filelist.whs"
    #define     LOG         ".whs.log"  
    #define     FAB         '2'
#else
    #define     SEPS        "\\"
    #define     INI_DIR     "..\\fab2wafer"
    #define     COPY        " copy "
    #define     MOVE        " move "
    #define     REMOVE      " del "
    #define     LIST        " dir/b "
    #define     INIFILE     "nanya2WaferHist.ini"
    #define     FILELIST    "filelist.whs"
    #define     LOG         ".whs.log"
    #define     FAB         '2'
#endif

/***** Client *****/
int  ReadIniFile();
int  OpenLogFile();
void CloseLogFile();
void RemoveDataFile();
void RemoveErrorFile();
int  OpenDataDir();
int  GetNextFilename();
int  RTrim();
void covChangeDateTimeFormat();

/***** Server *****/
int  Connection();
void CheckSqlError();

/* for WAT */
int  CheckRouteID();
int  CheckStepID();
int  SelectRouteStepID();
int  InsertRouteStepID();

/* for Lot History */
int  InsertLotHistData();
int  GetRecipeID();
int  GetReticleID();
int  GetTechID();
int  GetProdID();
int  SelectTechProdID();
int  InsertTechProdID();
int  GetLotID();
int  GetEquipID();
int  GetOperatorID();
int  ProcTechProdMap();
int  ProcTechRouteStepMap();
int  ProcProdRouteStepMap();
int  ProcRouteStepEquip();
int  ProcLotHistData();
int  InsertLotID();
int  UpdateLotID(); 

/*** library ***/
void TrimComa();   /* Trim the right one and left one " */
int CheckLotHistColumn();  
int goParseLine();
/*****************************************************************************/
/*                          galoble variable section                         */
/*****************************************************************************/

EXEC SQL BEGIN DECLARE SECTION ;

    struct wafer_hist
    {
     VARCHAR   wafer_id[24+1];
     VARCHAR   lot_name[20+1];
     int       lot_id;
     VARCHAR   wafer_no[10+1];
     VARCHAR   ope_cat[28+1];
     VARCHAR   product_name[20+1];
     int       product_id;
     char      s_time[30+1];
    } grWafer_HIST;

EXEC SQL END DECLARE SECTION;

    char        gsSource[50],         /* path of source code directory */
                gsDataDir[50],        /* path of data directory which used to store raw data file */
                gsBackupDir[50],      /* path of backup directory which used to backup raw data file */
                gsLogDir[50],         /* path of log directory which used to store log file */
                gsErrorDir[50],       /* path of error directory which used to store error log file */
                gsUserName[15],       /* string of User Name */
                gsPassword[15],       /* string of Password */
                gsHostString[30],     /* string of ORACLE DATABASE */
                gsDataFileName[80],   /* loadfile name */
                gsDataLine[LENGTH+1]; /* using to store a line of data which read from data file */


    FILE        *gpLogFp,             /* file pointer which points to log file */
                *gpErrFp,             /* file pointer which points to error file */
                *gpDataFileFp;         /* file pointer which points to raw data file */

    int         giCurLine;            /* no. of line which will be processed */

/*****************************************************************************/
/*                            functions section
*****************************************************************************/

/*****************************************************************************/
/* TITLE     : Truncate Redundant Blank Field
* DESC.     : To truncate redundanct leading and ending blank character(s) from
*             a specific string.
* PARAMETER : sStr -- the string, which will be cleaned
* RETURN    : a pointer, which point to the cleaned string
* EXAMPLE   : char str="  DAVID     Chang   ";// will be manipulated string
*             RTrim(str);// call RTrim()
* RESULT    : str="DAVID     Chang"
*****************************************************************************/
int RTrim(char *sStr)
{
   char    *sTemp = "";    /* store truncated string temporarily */
   int     iLen;           /* store the length of string */

   sTemp = sStr;
   iLen = strlen(sTemp);

   for (; iLen > 0; iLen--,sTemp++)
       if (isalnum(*sStr))
          break;

   iLen = strlen(sTemp);

   for (; iLen > 0; iLen--)
      if (!isprint(sTemp[iLen - 1]) || sTemp[iLen - 1] == ' ')
          sTemp[iLen - 1] = '\0';
      else
          break;

    sStr = sTemp;

    return (strlen(sTemp));
}/* End RTrim() */

/*****************************************************************************/
/* TITLE     : Change Date Format
* DESC.     : Transfer the format of date from YYYYMMDDHH24.MI.SS to YYYY/MM/DD HH24:MI:SS.
*             For example, transfer "1997071013.22.11" to "1997/07/10 13:22:11".
* PARAMETER : char *fromDate -- the date string which will be transfer
*             char *toDate   -- the date string which has been transfered
* RETURN    : return a transfered date-string which formate is "YYYY/MM/DD HH24:MI:DD"
* EXAMPLE   : char date1,         // destination string
*                  date2="1997071010.03.55 ";// source string
*             covChangeDateTimeFormat(date1,date2);// call covChangeDateTimeFormat()
* RESULT    : date1 = "1997/07/10 10:03:55"
*****************************************************************************/
void covChangeDateTimeFormat(char *toDate, char *fromDate)
{
  char   sYY[4+1] = "",   /* year */
         sMM[2+1] = "",   /* month */
         sDD[2+1] = "",   /* day */
         sHH[2+1] = "",   /* hour */
         sMI[2+1] = "",   /* minute */
         sSS[2+1] = "";   /* second */

  /* Decomposite the date string (ie. fromDate) to 6 sub_string */
  sscanf(fromDate,"%4c-%2c-%2c-%2c.%2c.%2c", sYY, sMM, sDD, sHH, sMI, sSS);

  /* Change date format from YYYYMMDD to YYYY/MM/DD HH24:MI:SS*/
  sprintf((char *)toDate,"%s/%s/%s %s:%s:%s", sYY, sMM, sDD, sHH, sMI, sSS);

}/* End of covChangeDateTimeFormat() */

/*****************************************************************************/
/* TITLE     : Get Product ID
* DESC.     : To get product_id from PRODUCT table
* PARAMETER : None
* RETURN    : if an error occurs, then return FAIL, else return SUCCEED
* EXAMPLE   : GetProductID()//just call it
*****************************************************************************/
int GetProdID()
{
   /* Get product_id */
   EXEC SQL
      SELECT   PRODUCT_ID
      INTO    :grWafer_HIST.product_id
      FROM    PRODUCT
      WHERE   NAME = :grWafer_HIST.product_name;

   /* Check whether product's name exists in PRODUCT table or not */
   if (sqlca.sqlcode == 100 || sqlca.sqlcode == 1403)
   {
      EXEC SQL
         INSERT
         INTO    PRODUCT(
                 NAME,
                 PRODUCT_ID)
         VALUES   ( :grWafer_HIST.product_name,
                    product_id.NEXTVAL);

      /* Check whether occurs an error condition */
      if (sqlca.sqlcode)
      {
         CheckSqlError("GetProdID() INSERT PRODUCT");
         return FAIL;
      }


      /* Get product_id */
      EXEC SQL
         SELECT  PRODUCT_ID
         INTO    :grWafer_HIST.product_id
         FROM    PRODUCT
         WHERE   NAME=:grWafer_HIST.product_name;

      /* Check whether occurs an error condition */
      if (sqlca.sqlcode)
      {
         CheckSqlError("GetProdID() SELECT PRODUCT_ID after INSERT PRODUCT");
         return FAIL;
      }

      /* No error occurs */
      return SUCCEED;

   }

   /* Check whether occurs an error condition */
   else if (sqlca.sqlcode)
   {
      CheckSqlError("GetProdID() SELECT PRODUCT_ID");
      return FAIL;
   }

   /* No error occurs ( sqlca.sqlcode == 0 ) */
   return SUCCEED;
}/* End GetProdID() */

/*****************************************************************************/
/* TITLE     : Get Lot's ID
* DESC.     : To get lot_id from LOT table
* PARAMETER : None
* RETURN    :
* EXAMPLE   :
*****************************************************************************/
int GetLotID()
{
    /* Select lot_id */
    EXEC SQL
        SELECT  LOT_ID
        INTO    :grWafer_HIST.lot_id
        FROM    LOT
        WHERE   LOTNO = :grWafer_HIST.lot_name;

    /* Check if no data exists */
    if (sqlca.sqlcode == 100 || sqlca.sqlcode == 1403)
    {
        /* Insert data of lot's number into LOT table, and get lot_id */
        if (InsertLotID() == FAIL)
            return FAIL;
        else /* If no error condition occurs */
            return SUCCEED;
    }

    /* Check if an error condition occurs */
    else if (sqlca.sqlcode)
    {
        CheckSqlError("GetLotID() SELECT LOT_ID");
        return FAIL;
    }

    return SUCCEED;

}/* End of SelectLotID() */

/*
******************************************************************************
TITLE     :
DESC.     :
PARAMETER :
RETURN    :
EXAMPLE   :
******************************************************************************
*/
int InsertLotID()
{

/* Insert data of lot's number and type */
    EXEC SQL
        INSERT
        INTO    LOT(
                  LOTNO,
                  LOT_ID,
                  PARENTLOT_ID,
                  FAB)
        VALUES  (
                  :grWafer_HIST.lot_name,
                  lot_id.NEXTVAL,
                  lot_id.NEXTVAL,
                  '2'
                );

    /* Check if an error condition occurs */
    if (sqlca.sqlcode)
    {
        CheckSqlError("InsertLotID() INSERT LOT");
        return FAIL;
    }

    /* Select lot_id */
    EXEC SQL
        SELECT  LOT_ID
        INTO    :grWafer_HIST.lot_id
        FROM    LOT
        WHERE   LOTNO = :grWafer_HIST.lot_name;

    if (sqlca.sqlcode)
    {
        CheckSqlError("InsertLotID() SELECT LOT_ID");
        return FAIL;
    }

    /* If no error occurs */
    return SUCCEED;
}/* end InsertLotID() */

/*****************************************************************************/
/* TITLE     : Get Next Datafile Name
* DESC.     : To get the name of next datafile which will be processed
* PARAMETER : fp -- file pointer which point to the file that store the file-
*                   name of datafile
* RETURN    : if error occur then put error message and return FAIL
*             if is end of file then return "EOF"
*             if found next filename then return SUCCEED
* EXAMPLE   : FILE *datafile;
*             datafile = open("datafile.dat","r");// open the file which store
*                :                                // the name of datafile
*                :
*             GetNextFilename(datafile);
*****************************************************************************/
int GetNextFilename(FILE *fp)
{
    int iStatus;

    iStatus = fscanf(fp, "%s", gsDataFileName);
    if (!iStatus)
    {
       fprintf(gpLogFp, "Fatal Error: Can't read data from datafile\n");
       return FAIL;
    }
    else if (iStatus == EOF)
       strcpy((char *)gsDataFileName, "EOF");
    return SUCCEED;
}/* end of GetNextFilename() */


/*****************************************************************************/
/* TITLE     : Read Initialization File
* DESC.     : To open and read the initial data from .INI file.
* PARAMETER : None
* RETURN    : if error occur then put error message and return FAIL
*             else return SOURCE path, DATA path, BACKUP path, ERROR path, LOG
*             path, USERNAME, PASSWORD and HOST_STRING.
* EXAMPLE   : if (ReadIniFile() == FAIL)
*                exit(0);
*****************************************************************************/
int ReadIniFile()
{
    FILE *pIniFileFp; /* use to store the pointer which points to initial file */
    char sBuffer[LENGTH];
    char tmpCommand[80];
    int  i;
    char *pPtr;

    sprintf(tmpCommand,"%s%s%s",INI_DIR,SEPS,INIFILE);
    if ((pIniFileFp = fopen(tmpCommand,"r")) == NULL)
    {
        printf("Error EDA-XXXXX: Couldn't open nanya2WaferHist.ini file\n");
        printf("   >> %s", strerror(errno));
        return FAIL;
    }

    while (!feof(pIniFileFp))
    {
        fgets(sBuffer, LENGTH, pIniFileFp);
        pPtr = sBuffer;

        if (strchr(sBuffer,'#')!=NULL)
            continue;
        pPtr=strchr(pPtr,'=');
        pPtr++;
        if (strstr(sBuffer,"SOURCE")!=NULL)
        {
            for(i=0;*pPtr!='\n';i++)
                gsSource[i]=*pPtr++;
            gsSource[i]='\0';
            continue;
        }

        if (strstr(sBuffer,"DATADIR")!=NULL)
        {
            for(i=0;*pPtr!='\n';i++)
                gsDataDir[i]=*pPtr++;
            gsDataDir[i]='\0';
            continue;
        }

        if (strstr(sBuffer,"BACKUPDIR")!=NULL)
        {
            for(i=0;*pPtr!='\n';i++)
                gsBackupDir[i]=*pPtr++;
            gsBackupDir[i]='\0';
            continue;
        }

        if (strstr(sBuffer,"ERRORDIR")!=NULL)
        {
            for(i=0;*pPtr!='\n';i++)
                gsErrorDir[i]=*pPtr++;
            gsErrorDir[i]='\0';
            continue;
        }

        if (strstr(sBuffer,"LOG")!=NULL)
        {
            for(i=0;*pPtr!='\n';i++)
               gsLogDir[i]=*pPtr++;
            gsLogDir[i]='\0';
            continue;
        }

        if (strstr(sBuffer,"USERNAME")!=NULL)
        {
            for(i=0;*pPtr!='\n';i++)
                gsUserName[i]=*pPtr++;
            gsUserName[i]='\0';
            continue;
        }

        if (strstr(sBuffer,"PASSWORD")!=NULL)
        {
            for(i=0;*pPtr!='\n';i++)
                gsPassword[i]=*pPtr++;
            gsPassword[i]='\0';
            continue;
        }

        if (strstr(sBuffer,"HOST_STRING")!=NULL)
        {
            for(i=0;*pPtr!='\n';i++)
               gsHostString[i]=*pPtr++;
            gsHostString[i]='\0';
            continue;
        }
    }/* end while */
    fclose(pIniFileFp);

    /* Put an message to screen */
    printf("Reading initialization file ...... OK\n\n");

    /* No error occurs */
    return SUCCEED;
}

/*****************************************************************************/
/* TITLE     : Connection
* DESC.     : To connect ACME database.
* PARAMETER : None
* RETURN    : if no error occurs, then return SUCCEED, else return FAIL
* EXAMPLE   : Connection(); //just call it
*****************************************************************************/
int Connection()
{
    EXEC SQL BEGIN DECLARE SECTION;
        char username[15];
        char password[10];
        char db_string[30];
    EXEC SQL END DECLARE SECTION;

    strcpy((char *)username,gsUserName);
    strcpy((char *)password,gsPassword);
    strcpy((char *)db_string,gsHostString);

    EXEC SQL
        CONNECT         :username
        IDENTIFIED BY   :password
        USING           :db_string;

    if (sqlca.sqlcode)
    {
        CheckSqlError("Connection() CONNECT");
        printf("Error: Couldn't connect Sever[UserName: %s][DB Name: %s]\n", gsUserName,db_string);
        fprintf(gpLogFp,"Error EDA-XXXXX: Couldn't connect Sever[UserName: %s][DB Name: %s]\n\n", gsUserName,db_string);
        fflush(gpLogFp);
        return FAIL;
    }
    else
    {
        printf("Connecting to Sever[UserName: %s][DB Name: %s].....OK\n\n", gsUserName,db_string);
        /*fprintf(gpLogFp,"Connecting to Sever[UserName: %s][DB Name: %s].....OK\n", UserName,db_string);*/
        return SUCCEED;
    }

}/* End Connection() */

/*****************************************************************************/
/* TITLE     :
* DESC.     :
* PARAMETER :
* RETURN    :
* EXAMPLE   :
*****************************************************************************/
void CheckSqlError(str)
char *str;
{
    char   msg[200];
    size_t buf_len, msg_len;

    if (sqlca.sqlcode != 0)
    {
        buf_len = sizeof(msg);
        sqlglm(msg, &buf_len, &msg_len);
        msg[msg_len] = '\0';
        fprintf(gpLogFp,"SQL Error(%05d): %s\n", giCurLine, str);
        fprintf(gpLogFp,"   [x] %s\n", msg);
    }
    fflush(gpLogFp);
}/* End CheckSqlError() */

/*****************************************************************************/
/* TITLE     : Open Log File
* DESC.     : To open log file for recording the log messages
* PARAMETER : None
* RETURN    : None
* EXAMPLE   : OpenLogFile()//just call it
*****************************************************************************/
int OpenLogFile()
{
    char      s_time[30],
              *pPtr,
              temp[30],
              buff[10], 
              *date = s_time;
    time_t    nseconds;
    char      logfile[100]="",
              errfile[100]="";

    nseconds = (long)time(NULL);
    strcpy(s_time,(char *)ctime(&nseconds));
    s_time[24] = '\0';
    buff[0]='\0';
    pPtr = strtok(date, " ");
    strcpy(temp,strtok(NULL, " "));
    strcpy(buff,strtok(NULL, " "));
    if(strlen((char*)buff)==1)
       strcat((char*)temp,"0"); /* if day is between 1 to 9,let the day to 01 ~ 09 */
    strcat((char*)temp,(char*)buff);   

    /* open log file */
    sprintf(logfile,"%s%s%s%s",gsLogDir,SEPS,temp,LOG);

    printf("Opening logfile: %10s",logfile);
    if((gpLogFp=fopen(logfile,"a")) == NULL)
    {
       printf(".....FAIL\n");
       printf("   >> %s\n",strerror(errno));
       return FAIL;
    }
    printf(".....OK\n\n");

    /* put message which begining time into log file */
    nseconds=(long)time(NULL);
    strcpy(s_time,(char *)ctime(&nseconds));
    s_time[24] = '\0';
    fprintf(gpLogFp,"\n%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% B E G I N  [%s] %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n\n", s_time);
    fflush(gpLogFp);
    return SUCCEED;
}/* End OpenLogFile() */

/*****************************************************************************/
/* TITLE     : Close Log File
* DESC.     : To close file pointer which point to log file.
* PARAMETER : None
* RETURN    : None
* EXAMPLE   : CloseLogFile();//just call it
*****************************************************************************/
void CloseLogFile()
{
    char      s_time[30];
    time_t    nseconds;

    nseconds=(long)time(NULL);
    strcpy(s_time,(char *)ctime(&nseconds));
    s_time[24] = '\0';
    fprintf(gpLogFp,"\n\n%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%   E N D  [%s]   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n", s_time);
    printf("\n%%%%%%%%%%%%%%%%%%  END OF PROCESS LOADFIEL(S)  [%s]  %%%%%%%%%%%%%%%%%%%\n\n", s_time);
    /*fclose(errfp);*/
    fclose(gpLogFp);
    return ;
}/* End CloseLogFile() */

/*****************************************************************************/
/* TITLE     : Remove Data File
* DESC.     : To remove the file which had processed successfully to backup
*             directory
* PARAMETER : filename -- name which will be remove (with the path which file located)
* RETURN    : None
* EXAMPLE   : RemoveDataFile("edbr1:[acme.incoming]N19998723.LD");//just call it
*****************************************************************************/
void RemoveDataFile(sFileName)
char *sFileName;
{
    char    sCommand[512];
    sprintf(sCommand, "%s %s%s%s %s", MOVE, gsDataDir, SEPS, sFileName, gsBackupDir);
    system(sCommand);
}/* End RemoveDataFile() */

/*****************************************************************************/
/* TITLE     : Remove Error File
* DESC.     : To remove the file which had processed failure to errordata directory
* PARAMETER : filename -- name which will be remove (with the path which file located)
* RETURN    : None
* EXAMPLE   : RemoveErrorFile("edbr1:[acme.incoming]N19998723.LD");//just call it
*****************************************************************************/
void RemoveErrorFile(sFileName)
char *sFileName;
{
    char    sCommand[512];
    sprintf(sCommand,"%s %s%s%s %s",MOVE,gsDataDir, SEPS, sFileName, gsErrorDir);
    system(sCommand);
}/* End RemoveErrorFile() */

/*****************************************************************************/
/* TITLE     : Open Data Diretory
* DESC.     : To get datafile name list from data-directory.
* PARAMETER : None
* RETURN    : a file(named DATAFILE.DAT) which store the datafile name list
*             will be processed
* EXAMPLE   : OpenDataDir()//just call it
*****************************************************************************/
int OpenDataDir()
{
    char  sCommand[LENGTH];
    sprintf(sCommand,"%s %s > %s",LIST,gsDataDir,FILELIST);
    system(sCommand);

    printf("Getting the list of name of data file(s).....OK\n\n");

    return SUCCEED;
}/* End OpenDataDir() */

/*****************************************************************************/
/* TITLE     : Insert Wafer History Data
* DESC.     : Used to insert processed data of lot history into DB.
* PARAMETER :
* RETURN    :
* EXAMPLE   :
*****************************************************************************/
int InsertWaferHistData()
{
    /* Insert the newest one into DB */
    EXEC SQL
        INSERT
        INTO    DEFECT_WAFER_NO_MAPPING(
                  WAFER_ID_BACK,
                  LOT_ID,
                  WAFER_NO,
                  OPE_CAT,
                  PRODUCT_ID,
                  STORE_TIME)
        VALUES  ( :grWafer_HIST.wafer_id,
                  :grWafer_HIST.lot_id,
                  :grWafer_HIST.wafer_no,
                  :grWafer_HIST.ope_cat,
                  :grWafer_HIST.product_id,
                  TO_DATE(:grWafer_HIST.s_time, 'YYYY/MM/DD HH24:MI:SS'));

    /* Check if an error condition occurs */
    if(sqlca.sqlcode)
       {
        CheckSqlError("InsertWaferHistData() Insert into Defect_Wafer_No_Mapping"); 
        return FAIL;
       }

    /* If no error condition occurs */
    return SUCCEED;
}/* End InsertLotHistData() */


/****************************************************************************
  TITLE      : Process Lot History Data
  DESC.      : To process data, which  are  extracted from WAT spec. files. The
               loader process relevant data for maintaining tables, which are
               correlate to WAT. The tables consist of LOT, PRODUCT, TECHNOLOGY,
               TECH_PROD_MAPPING, ..., etc.
  PARAMETERS : None
  RETURN     : if there is no error encuntered then return SUCCEED, else return
               FAIL
  EXAMPLE    : ProcLotHistData(); //just call it
*****************************************************************************/
int ProcWaferHistData()
{
 EXEC SQL BEGIN DECLARE SECTION ;
     VARCHAR   old_wafer_id[24+1];
 EXEC SQL END DECLARE SECTION; 

    int     i;
    char    wafer_id[24+1] = "";
    char    lot_id[20+1] = "";
    char    wafer_no[10+1] = "";
    char    ope_cat[30+1] = "";
    char    prod_id[40+1] = "";
    char    s_time[30+1] = "";
    char    sBuff[80+1]="";


    /*to get WAFER_ID*/
    strcpy(sBuff,strtok(gsDataLine,","));
    TrimComa(sBuff,16);
    strcpy(wafer_id,sBuff);


    /*to get LOT_ID*/
    strcpy(sBuff,strtok(NULL,","));
    TrimComa(sBuff,12);
    strcpy(lot_id,sBuff);

    /*to get WAFER_NO*/
    strcpy(sBuff,strtok(NULL,","));
    strcpy(wafer_no,sBuff);

    /*to get OPE_CAT*/
    strcpy(sBuff,strtok(NULL,","));
    TrimComa(sBuff,20);
    strcpy(ope_cat,sBuff);
      
    /*to get PRODUCT_ID*/
    strcpy(sBuff,strtok(NULL,","));
    TrimComa(sBuff,32);
    strcpy(prod_id,sBuff);

    /*to get Store_TIME*/
    strcpy(sBuff,strtok(NULL,","));
    TrimComa(sBuff,26);
    strcpy(s_time,sBuff);

    /* >>>>> Get product_id from PRODUCT table <<<<<*/
    /* prepare name of product */
    if (RTrim((char*)prod_id) == 0)
    {
        fprintf(gpLogFp, "Error EDA-XXXXX(%05d): Found a blank string in prod_id field\n", giCurLine);
        return FAIL;
    }
    prod_id[strlen(prod_id)-3]='\0';
    strcpy((char*)grWafer_HIST.product_name.arr, (char*)prod_id);
    grWafer_HIST.product_name.len = strlen((char*)grWafer_HIST.product_name.arr);

    /* select product_id */
    if (GetProdID() == FAIL)
       return FAIL;

    /* >>>>> Get lot_id from LOT table <<<<<*/
    /* prepare data of lot's number and type */
    if (RTrim((char*)lot_id) == 0)
    {
        fprintf(gpLogFp, "Error EDA-XXXXX(%05d): Found a blank string in lot_id field\n", giCurLine);
        return FAIL;
    }
    strcpy((char*)grWafer_HIST.lot_name.arr, (char*)lot_id);
    grWafer_HIST.lot_name.len = strlen((char*)grWafer_HIST.lot_name.arr);

    /* select lot_id */
    if (GetLotID() == FAIL)
        return FAIL;

    /***** get store_time *****/
    strcpy(grWafer_HIST.s_time, s_time);

    for(i = 0; i < 26; i++)
    {
      if (!isdigit(grWafer_HIST.s_time[i]) && grWafer_HIST.s_time[i] != '.' && grWafer_HIST.s_time[i] != '-' && grWafer_HIST.s_time[i] == '\0')
      {
         fprintf(gpLogFp, "Error(%d): Found a illegal date/time format(s_time=%s)\n", giCurLine, s_time);
         return FAIL;
      }
    }
    covChangeDateTimeFormat((char*)grWafer_HIST.s_time, (char*)grWafer_HIST.s_time);

    /* set wafer_id */
    strcpy(grWafer_HIST.wafer_id.arr,wafer_id);
    grWafer_HIST.wafer_id.len = strlen((char*)grWafer_HIST.wafer_id.arr);

    /* set ope_cat */
    strcpy(grWafer_HIST.ope_cat.arr,ope_cat);
    grWafer_HIST.ope_cat.len = strlen((char*)grWafer_HIST.ope_cat.arr);

    /* set wafer_id */
    if(strlen(wafer_no)==1)
       {
        strcpy(grWafer_HIST.wafer_no.arr,"0");
        strcat(grWafer_HIST.wafer_no.arr,wafer_no);
       }
    else   
       strcpy(grWafer_HIST.wafer_no.arr,wafer_no);
    grWafer_HIST.wafer_no.len = strlen((char*)grWafer_HIST.wafer_no.arr);
                                      
    /*###################################################################*/
    /*# The following portion begin to process the data of LOT HISTORY  #*/
    /*###################################################################*/

    EXEC SQL
        SELECT  WAFER_ID_BACK
        INTO    :old_wafer_id
        FROM    DEFECT_WAFER_NO_MAPPING
        WHERE   WAFER_ID_BACK = :grWafer_HIST.wafer_id;

    if(sqlca.sqlcode == 100 || sqlca.sqlcode == 1403)
       {
        if(InsertWaferHistData()==FAIL)
           return FAIL;
       }
    else if (sqlca.sqlcode)
       {
        CheckSqlError("SelectWaferHistData() Check Old Data");
        return FAIL;
       }
    /* If no error condition occurs (sqlca.sqlcode == 0) */
    return SUCCEED;
}/* End ProcLotHistData() */

/*****************************************************************************/
/************************  M A I N     P R O G R A M  ************************/
/*****************************************************************************/
int main(int argc, char *argv)
{
    int     iReturnCode;    /* to restore the return code */
    FILE    *pFptr;         /* Temporary store the file pointer */
    char    sPath[80],      /* the path where the data file is loacated */
            sLogTime[30];   /* log time */
    time_t  nseconds;

/***********************************************************/
/* The following portion begin to set running environment. */
/***********************************************************/

    /* Open and read the informantion of initialization */
    if (ReadIniFile() == FAIL)
        exit(1);

    /* Open the log file which store the message */
    if (OpenLogFile() == FAIL)
        exit(1);

    /* Get the filename of loadfile(s) which will be processed */
    if (OpenDataDir() == FAIL)
    {
        CloseLogFile();
        exit(1);
    }

    /* Open DATAFILE.DAT file which store the filename of
       loadfile(s) that will be processed */

    if ((pFptr = fopen(FILELIST,"r")) == NULL)
    {
        /* put an error message into log file */
        fprintf(gpLogFp,"Error EDA-XXXXX: Couldn't open filelist.whs\n");
        fprintf(gpLogFp,"   >> %s\n", strerror(errno));

        printf("Opening filelist.whs file.....FAIL\n\n");
        perror("\nCouldn't to open filelist.whs ");
        CloseLogFile();
        exit(1);
    }
    printf("Opening FILELIST.WHS file.....OK\n\n");

    /* Connect database */
    if (Connection() == FAIL)
    {
        fclose(pFptr);
        CloseLogFile();
        exit(1);
    }

/**********************************************************************/
/* The following portion to extract data from data file process them. */
/**********************************************************************/
    while (TRUE)
    {
        /* Get next load filename which will be process */
        iReturnCode = fscanf(pFptr, "%s", gsDataFileName);
        if (!iReturnCode)
        {
            fprintf(gpLogFp,"Error EDA-XXXXX: Can't read data from FILELIST.WIP file\n");
            fprintf(gpLogFp,"   >> %s\n", strerror(errno));
            break;
        }
        else if (iReturnCode == EOF)
        {
            fclose(pFptr);
            break;
        }

        /* draw a line, which is used to split defferent log message for each data file,
           in log file */
        fprintf(gpLogFp,"\n-------------------------------------------------\n");

        /* Reset the counter of processed line */
        giCurLine = 0;

        {
            /* Get date and time when the data file be processed */
            nseconds = (long)time(NULL);
            strcpy((char*)sLogTime, (char*)ctime(&nseconds));
            sLogTime[16] = '\0';

            /* Get path where is used to store data file. */
            sprintf(sPath,"%s%s%s",gsDataDir,SEPS,gsDataFileName);

            /* Open data file for read */
            gpDataFileFp = fopen(sPath, "r");

            /* Check whether exists an error condition. */
            if (gpDataFileFp == NULL)
            {
                fprintf(gpLogFp,"Error EDA-XXXXX: Couldn't open data file %s   [%s]\n", gsDataFileName, sLogTime);
                fprintf(gpLogFp,"   >> %s\n", strerror(errno));
                return FAIL;
            }

            /* Put a message into log file. */
            fprintf(gpLogFp, "\nOpened data file: %20s  [%s]\n", gsDataFileName, sLogTime);
#ifdef _TEST_ 
   printf("\nOpened data file: %20s  [%s]\n", gsDataFileName, sLogTime);
#endif
        }

        /* Reset flag */
        iReturnCode = SUCCEED;

        while (TRUE)
        {
            /* Get the first line of data from data file. It is the header information */
            if (fgets(gsDataLine, LENGTH, gpDataFileFp) == NULL)
            {

                /* Check if exists an 00 condition */
                if (feof(gpDataFileFp))
                {
                    if (giCurLine == 0)
                        fprintf(gpLogFp, "Error EDA-XXXXX: Found an empty data file\n");

                    /* If end of file */
                    break;
                }

                /* Check if exists an error condition */
                else if (ferror(gpDataFileFp))
                {
                    fprintf(gpLogFp, "Error EDA-XXXXX: Couldn't access data file\n");
                    fprintf(gpLogFp,"   >> %s\n", strerror(errno));
                    return FAIL;
                }
            }

            /* Set the no. of line, which will be processed */
            giCurLine++;
#ifdef _TEST_
   printf("Process data file: %20s line: %5d", gsDataFileName, giCurLine);
#endif
            /* Process data of lot history */
            if (ProcWaferHistData() == FAIL)
                iReturnCode = FAIL;
#ifdef _TEST_
   printf(" ===> PASSED \n");
#endif
            /* Flush stream */
            fflush(gpLogFp);
        }/* while (TRUE) ==> process one raw data line at one pass */

        fclose(gpDataFileFp);   
        /* Check the status after extract data, and do some suited process */
        switch(iReturnCode)
        {
            case FAIL:      /* If an error condition occurs */
                /* Remove this error file to Error Directory */
                RemoveErrorFile(gsDataFileName);
                /* Commit work */
                EXEC SQL ROLLBACK WORK;
                fprintf(gpLogFp,"...Fail\n");
                break;
            case SUCCEED:   /* If no error occurs */
                /* Remove this data file to Backup Directory */
                RemoveDataFile(gsDataFileName);

                /* Commit work */
                EXEC SQL COMMIT WORK;
                fprintf(gpLogFp,"...OK\n");
        }/* End switch(iReturnCode) */
    }/* End while(TRUE) ==>process one rawdata file at one pass */

    fclose(pFptr);     /* Close temporary file point */
    CloseLogFile();    /* Close log file */
    /* Remove temporary file */
    sprintf(sPath,"%s %s%s%s",REMOVE,gsSource,SEPS,FILELIST);
    system(sPath);
    return 0;
} /* end of main program */

/*****************************************************************************
  TITLE     : TrimComa
  DESC.     : Trim the right and the left one coma
  PARAMETER :
  RETURN    :
  EXAMPLE   :
******************************************************************************/
void TrimComa(sTemp,sleng)
char *sTemp;
unsigned sleng;
{  
   char    *sStr = ""; 
   sStr = sTemp;            
   if(sStr[0]=='"')
      sStr++;
   if(sStr[strlen(sStr)-1]=='"')
      sStr[strlen(sStr)-1]='\0';
   else
      sStr[strlen(sStr)]='\0'; 
   if(strlen(sStr)>sleng)
      strncpy(sTemp,sStr,sleng);
   else
      strcpy(sTemp,sStr);
}/* End TrimComa() */

