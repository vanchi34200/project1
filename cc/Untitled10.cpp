/******************************************************************************
        Copyright(C), 2000, for Klarity-ACE(Yield Enhancemant Solution)
 ******************************************************************************

 Title         : NANYA2 EDC & RTC EDA LOADER
 Date          : 2000/05/22
 Vesrion       : 1.1
 Initial Author: Felix Lai
 Modified Log  :
     2000/06/21 Felix Lai
        1.Add RTC by LOT
        2.If item_value == NULL,ignore this record.

     2000/06/30
        1.Add:insert data both 9-sites and List(modify for 9-site version)


 ******************************************************************************/


/*****************************************************************************/
/*                            definitions section                            */
/*****************************************************************************/

/* set run-time environment */
#define _UNIX_
#define _nTEST_
#define _nCHECK_
#define _nDEBUG_
#define _nALLSITE_   /* _ALLSITE_ for Full_list or _nALLSITE_ for 9-site and all*/
#define _nSHOWTIME_  /* set _SHOWTIME_ to print time to log file */

/*****************************************************************************/
/*                            call macros section                            */
/*****************************************************************************/
#define sqr(a) ((a)*(a)) /* square an number */

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


/* In UNIX environment, the following two items need be marked */
/*
EXEC SQL INCLUDE "d:\project\nanya2\NanYa2.h";
EXEC SQL INCLUDE sqlproto;
*/

EXEC SQL INCLUDE sqlca;

/*****************************************************************************/
/*                            definitions section                            */
/*****************************************************************************/

/* set run-time environment */
#ifdef _UNIX_
#define     SEPS        "/"
#define     SEP         '/'
#define     COPY        " cp "
#define     MOVE        " mv -f "
#define     REMOVE      " rm "
#define     LIST        " ls "
#define     INIFILE     "nanya2EdcRtcnew.ini"
#define     DOT         '.'
#define     FILELIST    "filelist.edc"
#else
#define     SEPS        "\\"
#define     SEP         '\\'
#define     COPY        " copy "
#define     MOVE        " move "
#define     REMOVE      " del "
#define     LIST        " dir/b "
#define     DOT         '.'
#define     INIFILE     "nanya2EdcRtcnew.ini"
#define     FILELIST    "filelist.edc"
#endif
/***** Client *****/
int  ReadIniFile();
int  OpenLogFile();
void CloseLogFile();
void RemoveDataFile();
void RemoveErrorFile();
int  OpenDataDir();
int  GetNextFilename();
int  TruncateRedundantField();
void covChangeDateTimeFormat();

/***** Server *****/
int  Connection();
void CheckSqlError();

/* for WAT */
int  InsertRouteStepID();

/* for EDC */
int  GetEdcParaID();
int  InsertEdcParaID();
int  CheckEdcSpec();
int  InsertEdcSpec();
int  UpdateEdcSpec();

/* for RTC */
int  InsertRtcParaID();

/* Library */
void   Sort();
double Find_max();
double Find_min();
double Find_average();
double Find_stddev();
double Find_medium();
double Find_q1();
double Find_q3();
void   CleanBlankField();

/********************/
int  GetTechID();
int  GetProdID();
int  SelectTechProdID();
int  InsertTechProdID();
int  SelectParProdID();
int  InsertParProdID();
int  CheckParLotID();
int  SelectParLotID();
int  InsertParLotID();
int  GetLotID();
int  GetEquipID();
int  GetOperatorID();
int  ProcTechProdMap();
int  ParseRawDataRec();
int  ExtractData();
int  ExtractSpecInfo();
int  ProcSpecData();
int  CheckLotID();
int  SelectLotID();
int  InsertLotID();
int  SelectEquipID();
int  InsertEquipID();
int  SelectWatSpecID();
int  InsertWatSpecID();
int  GetParaID();
int  SelectParaID();
int  InsertParaID();
int  GetWaferID();
int  InsertWaferID();
int  InsertSiteID();
int  GetTestProgID();
int  InsertTestProgID();
int  SelectOperatorID();
int  InsertOperatorID();

int  GetRouteStepID();
int  ProcProdEdcParaMap();
int  GetRtcParaID();
int  ProcEquipRtcParaMap();
int  ExtractMeasHistData();
int  ProcMeasHistData();
int  TrimComa();
int  gotToken();
void CleanOldData();
void StoreOldData();
void StoreMeasHistData();
int  CheckEdcLotInfo();
int  CheckEdcLotSummary();
int  CheckEdcWaferInfo();
int  CheckEdcWaferSummary();
int  CheckRtcSummary();
int  UpdateRtcSummary();
int  InsertEdcLotInfo();
int  InsertEdcLotSummary();
int  InsertEdcWaferInfo();
int  InsertEdcWaferSummary();
int  InsertRtcSummaryByWafer();
int  InsertRtcSummaryBySite();
int  ProcessSummaryBySite();
int  InsertEdcWaferSummaryBySite();
void ProcessLotAvgBySite();
int  InsertEdcLotSummaryBySite();
void CheckEdcSpecValue();
void CheckRtcSpecValue();
int  CheckExistRawData();
int  InsertWaferID();

/* 9 site*/
int  InsertEdcWaferSummaryBy9Site();
int  InsertRtcSummaryBy9Site();
int  InsertEdcLotSummaryBy9Site();
void ProcessLotAvgBy9Site();
int  ProcessSummaryBy9Site();

/* for TEST */
void ShowTime();
void ShowStep();
/*****************************************************************************/
/*                          galoble variable section                         */
/*****************************************************************************/

EXEC SQL BEGIN DECLARE SECTION ;

    WAT_WAFER_SUMMARY   grEdcWfrSum,
                        grRtcSum,
                        grPreSum,
                        grSum;
    WAT_LOT_SUMMARY     grEdcLotSum;
    LOT_HISTORY         grLotHist;
    ROUTE               grRoute;
    STEP                grStep;
    ROUTE_STEP          grRouteStep,
                        grProcRouteStep,
                        grMeasRouteStep,
                        grPreRouteStep;
    RETICLE             grReticle;
    RECIPE              grRecipe,
                        grMeasRecipe,
                        grProcRecipe;
    MEAS_FIELD_INFO     grMeasFieldInfo;
    PARAMETER           grParameter,
                        grCurParameter,
                        grPreParameter;

    SPEC                grSpec;

    PRODUCT             grProduct,
                        grNullProd;
    TECHNOLOGY          grTechnology,
                        grMeasTech,
                        grProcTech;
    LOT                 grLot;
    WAFER               grWafer;
    EQUIPMENT           grEquipment,
                        grMeasEquip,
                        grProcEquip;
    OPERATOR            grOperator;
    HEADER              grHeader;
    VARCHAR             FAB[2];
EXEC SQL END DECLARE SECTION;
WAFER_STAT          grWaferStat;  /* store wafer's statistical data */
char        gsSource[50],         /* path of source code directory */
            gsDataDir[50],        /* path of data directory which used to store raw data file */
            gsBackupDir[50],      /* path of backup directory which used to backup raw data file */
            gsLogDir[50],         /* path of log directory which used to store log file */
            gsErrorDir[50],       /* path of error directory which used to store error log file */
            gsSpecDir[50],        /* Path of spec directory which used to store spec data file */
            gsUserName[15],       /* string of User Name */
            gsPassword[15],       /* string of Password */
            gsHostString[30],     /* string of ORACLE DATABASE */
            gsDataFileName[80],   /* loadfile name */
            gsDataLine[LENGTH+1]; /* using to store a line of data which read from data file */
char        gcDataType;           /* a flag use to denote if is a EDC, RTC or both record */
int         iReturnCode;    /* to restore the return code when process data*/
/* Temporary store extracted data     */
struct  meas_hist {
    char    p_lot_id[12+1],
            p_lot_type[20+1],
            reprt_time[26+1],
            m_lot_id[12+1],
            m_lot_type[20+1],
            m_route_id[11+1],
            m_oper_no[25+1],
            mfld_id[15+1],
            item_fld[12+1],
            meas_type[12+1],
            p_prod_id[32+1],
            p_tech_id[12+1],
            p_route_id[11+1],
            p_oper_no[25+1],
            p_pd_id[11+1],
            p_pd_name[32+1],
            p_tool_id[32+1],
            p_tool_name[32+1],
            p_lc_recipe[25+1],
            p_recipe[64+1],
            p_ph_recipe[25+1],
            empl_id[32+1],
            p_wfhist_time[26+1],
            p_customer_id[32+1],
            p_time[26+1],
            m_prod_id[32+1],
            m_tech_id[12+1],
            m_pd_id[11+1],
            m_pd_name[32+1],
            m_tool_id[32+1],
            m_tool_name[32+1],
            m_lc_recipe[25+1],
            m_recipe[64+1],
            m_ph_recipe[25+1],
            report_time_2[26+1],
            empl_id_2[32+1],
            m_wfhist_time[26+1],
            wafer_id[16+1],
            prcss_grp[64+1];
    int     wafer_pos,
            site_pos;
    double  item_value;
} grMeasHist,gtMeasHist;

struct  meas_spec {
    char    prod_id[29+1],
            route_id[8+1],
            oper_no[6+1],
            p_route_id[8+1],
            p_oper_no[6+1],
            delta_chk_flg,
            delta_oper_no[6+1];
    int     spec_cnt;
} grMeasSpec;

struct  spec_data {
    char    item_fld[12+1],
            pre_item_fld[12+1],
            spec_hi[12+1],
            spec_lo[12+1],
            ctrl_hi[12+1],
            ctrl_lo[12+1],
            valid_hi[12+1],
            valid_lo[12+1],
            target[12+1],
            coefficient[12+1];
} grSpecData[75];

struct {
    int     wfr_flag;           /* used to denote if exist a wafer of data */
    int     site_cnt;           /* quantity of site */
    int     site_flag[MAX_SITE];/* used to denote if exist a site of data */
    double  value[MAX_SITE];    /* measuremented value */
    char    wafer_id_back[MAX_SITE][30];
} grRawData[MAX_WAFER];         /* [0] for average */

struct {
    char    wafer_id_back[20];
} grWaferIdBack[MAX_WAFER];         /* [0] for average */


struct {
    char    p_lot_id[12+1],
            m_route_id[11+1],
            m_oper_no[25+1],
            item_fld[12+1],
            meas_type[12+1];
} stOldData;

/*-------------------------------------*/

FILE        *gpLogFp,             /* file pointer which points to log file */
            *gpErrFp,             /* file pointer which points to error file */
            *gpDataFileFp;         /* file pointer which points to raw data file */

long int    giCurLine;            /* no. of line which will be processed */

unsigned long     ticks;          /* used to store system date and time */

/*****************************************************************************/
/************************  M A I N     P R O G R A M  ************************/
/*****************************************************************************/
int main(int argc, char *argv)
{
    int     gReturnCode;    /* to restore the return code */
    FILE    *pFptr;         /* Temporary store the file pointer */
    char    sPath[80],      /* the path where the data file is loacated */
            sLogTime[30],   /* log time */
            sCommand[80];
    time_t  nseconds;
    char    Temp[50];

    /***********************************************************/
    /* The following portion begin to set running environment. */
    /***********************************************************/

    /* Open and read the informantion of initialization */
    if(ReadIniFile() == FAIL)
        exit(1);

    /* Open the log file which store the message */
    if(OpenLogFile() == FAIL)
        exit(1);

    /* Get the filename of loadfile(s) which will be processed */
    if(OpenDataDir() == FAIL) {
        CloseLogFile();
        exit(1);
    }

    /* Open DATAFILE.DAT file which store the filename of
       loadfile(s) that will be processed */

    if((pFptr = fopen(FILELIST,"r")) == NULL) {
        /* put an error message into log file */
        fprintf(gpLogFp,"Error EDA-XXXXX: Couldn't open FILELIST.EDC\n");
        fprintf(gpLogFp,"   >> %s\n", strerror(errno));

        printf("Opening FILELIST.EDC file.....FAIL\n\n");
        perror("\nCouldn't to open FILELIST.EDC ");
        CloseLogFile();
        exit(1);
    }
    printf("Opening FILELIST.EDC file.....OK\n\n");

    /* Connect database */
    if(Connection() == FAIL) {
        fclose(pFptr);
        CloseLogFile();
        exit(1);
    }
    strcpy((char*)FAB.arr, "2");
    FAB.len = 1;

    /**********************************************************************/
    /* The following portion to extract data from data file process them. */
    /**********************************************************************/
    while (TRUE) {
        /* Get next load filename which will be process */
        gReturnCode = fscanf(pFptr, "%s", gsDataFileName);
        if(!gReturnCode) {
            fprintf(gpLogFp,"Error EDA-XXXXX: Can't read data from FILELIST.EDC file\n");
            fprintf(gpLogFp,"   >> %s\n", strerror(errno));
            iReturnCode=FAIL;
            break;
        } else if(gReturnCode == EOF) {
            fclose(pFptr);
            break;
        }
        /* draw a line, which is used to split defferent log message for each data file,
           in log file */
        fprintf(gpLogFp,"\n-------------------------------------------------\n");

        /*#################################################################################*/
        /*#  The following portion used to get the time, when the data file is processed  #*/
        /*#  and open data file.                                                          #*/
        /*#################################################################################*/
        {
            /* Get date and time when the data file be processed */
            nseconds = (long)time(NULL);
            strcpy(sLogTime,(char *)ctime(&nseconds));
            sLogTime[19] = '\0';

            /* Get path where is used to store data file. */
            sprintf(sPath, "%s%s%s", gsDataDir, SEPS,gsDataFileName);
            /* Open data file for read */
            gpDataFileFp = fopen(sPath, "r");

            /* Check whether exists an error condition. */
            if(gpDataFileFp == NULL) {
                fprintf(gpLogFp,"Error EDA-XXXXX: Couldn't open data file %s   [%s]\n", gsDataFileName, sLogTime);
                fprintf(gpLogFp,"   >> %s\n", strerror(errno));
                return FAIL;
            }

            /* Put a message into log file. */
            fprintf(gpLogFp, "\nOpened data file: %20s  [%s]\n", gsDataFileName, sLogTime);
        }

        /*########################################################################*/
        /*#  The following while loop begin to process each line of lot history  #*/
        /*#  data.                                                               #*/
        /*########################################################################*/
#ifdef _TEST_
        printf("Process file : %s",sPath);
#endif
        /*CleanField(gsDataFileName, strlen((char*)gsDataFileName));*/
        if(strstr(gsDataFileName, "msr_hist") != NULL) {
            /* Reset the counter of processed line */
            giCurLine = 0;
            /* Reset flag */
            iReturnCode = SUCCEED;

            while (TRUE) {
                giCurLine++;
                /* Extract data of measurement history */
                gReturnCode = ExtractMeasHistData();
                /* Flush stream */
                fflush(gpLogFp);
                /*                fflush(gpDataFileFp);*/
                if(gReturnCode == EOF)
                    break;
                else if(gReturnCode == IGNORE) /* Found '*' in value field */
                    continue;
                else if(gReturnCode == FAIL) {
                    iReturnCode=FAIL;
                    CleanOldData();
                    continue;
                } /* End else if(...==FAIL)*/
                continue;
            } /* End while(true)*/
        } /* End if(strstr(...,"msr_hist")...)*/
        else {
            iReturnCode=FAIL;
            fprintf(gpLogFp,"Error file name : %s\n",gsDataFileName);
        }
        /* Flush stream */
        fflush(gpLogFp);
        fclose(gpDataFileFp);
        /* Check the status after extract data, and do some suited process */
        switch(iReturnCode) {
            case FAIL:      /* If an error condition occurs */
                /* Remove this error file to Error Directory */
                RemoveErrorFile(gsDataFileName);
                /* Rollback work */
                EXEC SQL ROLLBACK WORK;
                fprintf(gpLogFp,"Fail\n");
                break;
            case SUCCEED:   /* If no error occurs */
                /* Remove this data file to Backup Directory */
                RemoveDataFile(gsDataFileName);
                /* Commit work */
                EXEC SQL COMMIT WORK;
                fprintf(gpLogFp,"OK\n");
        }/* End switch(iReturnCode) */
    }/* End while(TRUE) */

    /* Remove temporary file */
    sprintf(sCommand, "%s %s", REMOVE, FILELIST);
    system(sCommand);
    ShowTime("File process end!");
    /* Close log file */
    CloseLogFile();
    return 0;
} /* end of main program */

/*****************************************************************************
* TITLE     : Read Initialization File
* DESC.     : To open and read the initial data from .INI file.
* PARAMETER : None
* RETURN    : if error occur then put error message and return FAIL
*             else return SOURCE path, DATA path, BACKUP path, ERROR path, LOG
*             path, USERNAME, PASSWORD and HOST_STRING.
* EXAMPLE   : if(ReadIniFile() == FAIL)
*                exit(0);
*****************************************************************************/
int ReadIniFile()
{
    FILE *pIniFileFp; /* use to store the pointer which points to initial file */
    char sBuffer[LENGTH];
    int  i;
    char *pPtr;

    if((pIniFileFp = fopen(INIFILE,"r")) == NULL) {
        printf("Error EDA-XXXXX: Couldn't open nanyaEdcRtc.ini file\n");
        printf("   >> %s\n", strerror(errno));
        return FAIL;
    }

    while (!feof(pIniFileFp)) {
        fgets(sBuffer, LENGTH, pIniFileFp);
        pPtr = sBuffer;

        if(strchr(sBuffer,'#')!=NULL)
            continue;

        pPtr=strchr(sBuffer,'=');
        pPtr++;

        if(strstr(sBuffer,"SOURCE")!=NULL) {
            for(i=0; *pPtr!='\n'; i++)
                gsSource[i]=*pPtr++;
            gsSource[i]='\0';
            continue;
        }
        if(strstr(sBuffer,"DATADIR")!=NULL) {
            for(i=0; *pPtr!='\n'; i++)
                gsDataDir[i]=*pPtr++;
            gsDataDir[i]='\0';
            continue;
        }
        if(strstr(sBuffer,"LIMITDIR")!=NULL) {
            for(i=0; *pPtr!='\n'; i++)
                gsSpecDir[i]=*pPtr++;
            gsSpecDir[i]='\0';
            continue;
        }
        if(strstr(sBuffer,"BACKUPDIR")!=NULL) {
            for(i=0; *pPtr!='\n'; i++)
                gsBackupDir[i]=*pPtr++;
            gsBackupDir[i]='\0';
            continue;
        }
        if(strstr(sBuffer,"ERRORDIR")!=NULL) {
            for(i=0; *pPtr!='\n'; i++)
                gsErrorDir[i]=*pPtr++;
            gsErrorDir[i]='\0';
            continue;
        }
        if(strstr(sBuffer,"LOG")!=NULL) {
            for(i=0; *pPtr!='\n'; i++)
                gsLogDir[i]=*pPtr++;
            gsLogDir[i]='\0';
            continue;
        }
        if(strstr(sBuffer,"USERNAME")!=NULL) {
            for(i=0; *pPtr!='\n'; i++)
                gsUserName[i]=*pPtr++;
            gsUserName[i]='\0';
            continue;
        }
        if(strstr(sBuffer,"PASSWORD")!=NULL) {
            for(i=0; *pPtr!='\n'; i++)
                gsPassword[i]=*pPtr++;
            gsPassword[i]='\0';
            continue;
        }
        if(strstr(sBuffer,"HOST_STRING")!=NULL) {
            for(i=0; *pPtr!='\n'; i++)
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

/*****************************************************************************
* TITLE     : Open Log File
* DESC.     : To open log file for recording the log messages
* PARAMETER : None
* RETURN    : None
* EXAMPLE   : OpenLogFile() -- just call it
*****************************************************************************/
int OpenLogFile()
{
    char      s_time[30],
              *pPtr,
              temp[30],
              buff[30],
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
    sprintf(logfile,"%s%s%s%s",gsLogDir,SEPS,temp,".edc.log");

    printf("Opening logfile: %10s",logfile);
    if((gpLogFp=fopen(logfile,"a")) == NULL) {
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

/*****************************************************************************
* TITLE     : Open Data Diretory
* DESC.     : To get datafile name list from data-directory.
* PARAMETER : None
* RETURN    : a file(named DATAFILE.DAT) which store the datafile name list
*             will be processed
* EXAMPLE   : OpenDataDir() -- just call it
*****************************************************************************/
int OpenDataDir()
{
    char  sCommand[LENGTH];
    sprintf(sCommand,"%s %s > %s%s%s", LIST, gsDataDir, gsSource, SEPS, FILELIST);
    system(sCommand);
    printf("Getting the list of name of data file(s).....OK\n\n");
    return SUCCEED;
}/* End OpenDataDir() */

/*****************************************************************************
* TITLE     : Connection
* DESC.     : To connect ACME database.
* PARAMETER : None
* RETURN    : if no error occurs, then return SUCCEED, else return FAIL
* EXAMPLE   : Connection();  -- just call it
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


    if(sqlca.sqlcode) {
        CheckSqlError("Connection() CONNECT");
        printf("Error: Couldn't connect Sever[UserName: %s][DB Name: %s]\n", gsUserName,db_string);
        fprintf(gpLogFp,"Error EDA-XXXXX: Couldn't connect Sever[UserName: %s][DB Name: %s]\n\n", gsUserName,db_string);
        fflush(gpLogFp);
        return FAIL;
    } else {
        printf("Connecting to Sever[UserName: %s][DB Name: %s].....OK\n\n", gsUserName,db_string);
        return SUCCEED;
    }
}/* End Connection() */

/****************************************************************************
* TITLE      : Extract Measurement History Data
* DESC.      : To process data, which  are  extracted from WAT spec. files. The
*              loader process relevant data for maintaining tables, which are
*              correlate to WAT. The tables consist of LOT, PRODUCT, TECHNOLOGY,
*              TECH_PROD_MAPPING, ..., etc.
* PARAMETERS : None
* RETURN     : if there is no error encuntered then return SUCCEED, else return
*              FAIL
* EXAMPLE    : ExtractMeasHistData();  -- just call it
*****************************************************************************/
int ExtractMeasHistData()
{
    char    raw_data[9000+1],
            buff[100];
    char    wafer_id_back[20];
    int i,len1;
    if(fgets(raw_data, LENGTH, gpDataFileFp) == NULL) {
        /* Check if exists an 00 condition */
        if(feof(gpDataFileFp)) {
            if(giCurLine == 1)
                fprintf(gpLogFp, "Error EDA-XXXXX: Found an empty data file\n");
            else if(ProcMeasHistData()==FAIL)
                /* { */
                iReturnCode=FAIL;
            /*  EXEC SQL ROLLBACK WORK;
               }
             else
                EXEC SQL COMMIT WORK;*/
            CleanOldData();
            return EOF;
        }
        /* Check if exists an error condition */
        else if (ferror(gpDataFileFp)) {
            fprintf(gpLogFp, "Error EDA-XXXXX: Couldn't access data file\n");
            fprintf(gpLogFp,"   >> %s\n", strerror(errno));
            return FAIL;
        }
    } else

        /* Get p_lot_id */
        if(gotToken(buff,raw_data)==FAIL) {
            fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
            return FAIL;
        }
    if(TrimComa(buff)==0) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): Found a blank string in p_lot_id \n",giCurLine);
        return FAIL;
    }
    strcpy(grMeasHist.p_lot_id,buff);

    /* p_lot_type */
    if(gotToken(buff,raw_data)==FAIL) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
        return FAIL;
    }
    TrimComa(buff);
    strcpy(grMeasHist.p_lot_type,buff);

    /* reprt_time */
    if(gotToken(buff,raw_data)==FAIL) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
        return FAIL;
    }
    if(TrimComa(buff)==0) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): Found a blank string in reprt_time\n",giCurLine);
        return FAIL;
    }
    covChangeDateTimeFormat(grMeasHist.reprt_time,buff);
    /*if(strcmp((char*)gtMeasHist.reprt_time,(char*)grMeasHist.reprt_time)<0)
       strcpy((char*)gtMeasHist.reprt_time,(char*)grMeasHist.reprt_time); */
    /* m_lot_id */
    if(gotToken(buff,raw_data)==FAIL) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
        return FAIL;
    }
    TrimComa(buff);
    strcpy(grMeasHist.m_lot_id,buff);

    /* m_lot_type */
    if(gotToken(buff,raw_data)==FAIL) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
        return FAIL;
    }
    if(TrimComa(buff)==0) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): Found a blank string in m_lot_type\n",giCurLine);
        return FAIL;
    }
    strcpy(grMeasHist.m_lot_type,buff);

    /* m_route_id */
    if(gotToken(buff,raw_data)==FAIL) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
        return FAIL;
    }
    if(TrimComa(buff)==0) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): Found a blank string in m_route_id\n",giCurLine);
        return FAIL;
    }
    strcpy(grMeasHist.m_route_id,buff);

    /* m_oper_no */
    if(gotToken(buff,raw_data)==FAIL) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
        return FAIL;
    }
    if(TrimComa(buff)==0) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): Found a blank string in m_oper_no\n",giCurLine);
        return FAIL;
    }
    strcpy(grMeasHist.m_oper_no,buff);

    /* mfld_id */
    if(gotToken(buff,raw_data)==FAIL) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
        return FAIL;
    }
    TrimComa(buff);
    strcpy(grMeasHist.mfld_id,buff);

    /* item_fld */
    if(gotToken(buff,raw_data)==FAIL) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
        return FAIL;
    }
    TrimComa(buff);
    strcpy(grMeasHist.item_fld,buff);

    /* meas_type */
    if(gotToken(buff,raw_data)==FAIL) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
        return FAIL;
    }
    if(TrimComa(buff)==0) {
        fprintf(gpLogFp, "Wrong EDA-XXXXX(%5d): Found a blank string in meas_type\n",giCurLine);
        return IGNORE; /* remarked by Felix @ 2000/06/27 */
    }
    strcpy(grMeasHist.meas_type,buff);

    /* p_prod_id */
    if(gotToken(buff,raw_data)==FAIL) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
        return FAIL;
    }
    TrimComa(buff);
    strcpy(grMeasHist.p_prod_id,buff);

    /* p_tech_id */
    if(gotToken(buff,raw_data)==FAIL) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
        return FAIL;
    }
    TrimComa(buff);
    strcpy(grMeasHist.p_tech_id,buff);

    /* p_route_id */
    if(gotToken(buff,raw_data)==FAIL) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
        return FAIL;
    }
    TrimComa(buff);
    strcpy(grMeasHist.p_route_id,buff);

    /* p_oper_no */
    if(gotToken(buff,raw_data)==FAIL) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
        return FAIL;
    }
    TrimComa(buff);
    strcpy(grMeasHist.p_oper_no,buff);

    /* p_pd_id */
    if(gotToken(buff,raw_data)==FAIL) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
        return FAIL;
    }
    TrimComa(buff);
    strcpy(grMeasHist.p_pd_id,buff);

    /* p_pd_name */
    if(gotToken(buff,raw_data)==FAIL) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
        return FAIL;
    }
    TrimComa(buff);
    strcpy(grMeasHist.p_pd_name,buff);

    /* p_tool_id */
    if(gotToken(buff,raw_data)==FAIL) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
        return FAIL;
    }
    TrimComa(buff);
    strcpy(grMeasHist.p_tool_id,buff);


    /* p_tool_name */
    if(gotToken(buff,raw_data)==FAIL) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
        return FAIL;
    }
    TrimComa(buff);
    strcpy(grMeasHist.p_tool_name,buff);

    /* p_lc_recipe */
    if(gotToken(buff,raw_data)==FAIL) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
        return FAIL;
    }
    TrimComa(buff);
    strcpy(grMeasHist.p_lc_recipe,buff);

    /* p_recipe */
    if(gotToken(buff,raw_data)==FAIL) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
        return FAIL;
    }
    TrimComa(buff);
    strcpy(grMeasHist.p_recipe,buff);

    /* p_ph_recipe */
    if(gotToken(buff,raw_data)==FAIL) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
        return FAIL;
    }
    TrimComa(buff);
    strcpy(grMeasHist.p_ph_recipe,buff);

    /* prcss_grp */
    if(gotToken(buff,raw_data)==FAIL) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
        return FAIL;
    }
    TrimComa(buff);
    strcpy(grMeasHist.prcss_grp,buff);

    /* empl_id */
    if(gotToken(buff,raw_data)==FAIL) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
        return FAIL;
    }
    TrimComa(buff);
    strcpy(grMeasHist.empl_id,buff);

    /* p_wfhist_time */
    if(gotToken(buff,raw_data)==FAIL) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
        return FAIL;
    }
    if(TrimComa(buff)==0) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): Found a blank string in p_wfhist_time\n",giCurLine);
        return FAIL;
    }
    covChangeDateTimeFormat(grMeasHist.p_wfhist_time,buff);

    /* p_customer_id */
    if(gotToken(buff,raw_data)==FAIL) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
        return FAIL;
    }
    TrimComa(buff);
    strcpy(grMeasHist.p_customer_id,buff);

    /* p_time */
    if(gotToken(buff,raw_data)==FAIL) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
        return FAIL;
    }
    if(TrimComa(buff)==0) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): Found a blank string in p_time\n",giCurLine);
        return FAIL;
    }
    covChangeDateTimeFormat(grMeasHist.p_time,buff);

    /* m_prod_id */
    if(gotToken(buff,raw_data)==FAIL) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
        return FAIL;
    }
    if(TrimComa(buff)==0) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): Found a blank string in m_prod_id\n",giCurLine);
        return FAIL;
    }
    strcpy(grMeasHist.m_prod_id,buff);

    /* m_tech_id */
    if(gotToken(buff,raw_data)==FAIL) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
        return FAIL;
    }
    TrimComa(buff);
    strcpy(grMeasHist.m_tech_id,buff);

    /* m_pd_id */
    if(gotToken(buff,raw_data)==FAIL) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
        return FAIL;
    }
    TrimComa(buff);
    strcpy(grMeasHist.m_pd_id,buff);

    /* m_pd_name */
    if(gotToken(buff,raw_data)==FAIL) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
        return FAIL;
    }
    TrimComa(buff);
    strcpy(grMeasHist.m_pd_name,buff);

    /* m_tool_id */
    if(gotToken(buff,raw_data)==FAIL) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
        return FAIL;
    }
    TrimComa(buff);
    strcpy(grMeasHist.m_tool_id,buff);
    /* added by Felix @ 2000/06/27 */
    if(TruncateRedundantField((char*)grMeasHist.m_tool_id) == 0) {
        if(gtMeasHist.m_lot_type[0]=='A') {
            fprintf(gpLogFp, "Wrong EDA-XXXXX(%05d): Found a blank string in m_tool_id\n", giCurLine-1);
            return IGNORE;
        }
    }
    /* m_tool_name */
    if(gotToken(buff,raw_data)==FAIL) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
        return FAIL;
    }
    TrimComa(buff);
    strcpy(grMeasHist.m_tool_name,buff);

    /* m_lc_recipe */
    if(gotToken(buff,raw_data)==FAIL) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
        return FAIL;
    }
    TrimComa(buff);
    strcpy(grMeasHist.m_lc_recipe,buff);

    /* m_recipe */
    if(gotToken(buff,raw_data)==FAIL) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
        return FAIL;
    }
    TrimComa(buff);
    strcpy(grMeasHist.m_recipe,buff);

    /* m_ph_recipe */
    if(gotToken(buff,raw_data)==FAIL) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
        return FAIL;
    }
    TrimComa(buff);
    strcpy(grMeasHist.m_ph_recipe,buff);

    /* report_time_2 */
    if(gotToken(buff,raw_data)==FAIL) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
        return FAIL;
    }
    if(TrimComa(buff)==0) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): Found a blank string in report_time_2\n",giCurLine);
        return FAIL;
    }
    covChangeDateTimeFormat(grMeasHist.report_time_2,buff);

    /* empl_id_2 */
    if(gotToken(buff,raw_data)==FAIL) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
        return FAIL;
    }
    TrimComa(buff);
    strcpy(grMeasHist.empl_id_2,buff);

    /* m_wfhist_time */
    if(gotToken(buff,raw_data)==FAIL) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
        return FAIL;
    }
    if(TrimComa(buff)==0) {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): Found a blank string in m_wfhist_time\n",giCurLine);
        return FAIL;
    }
    covChangeDateTimeFormat(grMeasHist.m_wfhist_time,buff);
    /*  fprintf(gpLogFp, "grMeasHist.m_wfhist_time:%s\n",grMeasHist.m_wfhist_time); */

    /* wafer_id */
    /*  if(gotToken(buff,raw_data)==FAIL)
         {
          fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
          return FAIL;
         }
      TrimComa(buff);
      strcpy(grMeasHist.wafer_id,buff); */ /* update by hawk 20010615 */

    /* If it's another lot,process measure data */
    if(strcmp(grMeasHist.p_lot_id,stOldData.p_lot_id)!=0 ||
            strcmp(grMeasHist.m_route_id,stOldData.m_route_id)!=0 ||
            strcmp(grMeasHist.m_oper_no,stOldData.m_oper_no)!=0 ||
            strcmp(grMeasHist.item_fld,stOldData.item_fld)!=0 ||
            strcmp(grMeasHist.meas_type,stOldData.meas_type)!=0) {
        /* If we have stored some data */
        if(giCurLine!=1)
            if(ProcMeasHistData()==FAIL)
                /*  {
                   EXEC SQL ROLLBACK WORK;*/
                return FAIL;
        /* }
        else
         EXEC SQL COMMIT WORK;*/

        CleanOldData(); /* Clean All Old Data*/
        StoreOldData();
        StoreMeasHistData(); /* Let this rawdata we just reading to be stored*/
    }

    /* Process Raw Data */
    if(strcmp(grMeasHist.meas_type,"Site")==0) { /* Site level*/
        for(i=0; raw_data[0]!='\0' && raw_data[0]!='\n'; i++) {
            if(i>=MAX_SITE) {
                fprintf(gpLogFp, "Error EDA-XXXXX(%5d): Too many sites,we only can process less 100 sites\n",giCurLine);
                return FAIL;
            }
            /* wafer_id */
            if(gotToken(buff,raw_data)==FAIL)
                if(i==0) {
                    fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Wafer ID Data \n",giCurLine);
                    return FAIL;
                } else
                    break;
            if(TrimComa(buff)==0) {
                /*  fprintf(gpLogFp, "EDA-XXXXX(%5d): Found a blank string in wafer id\n",giCurLine);*/
                /*  return FAIL; */
            }
            strcpy(wafer_id_back,buff);  /* add by hawk 20010615 */
            len1=strlen(wafer_id_back);
            wafer_id_back[len1]='\0';
            /* fprintf(gpLogFp, "Site wafer_id_back:%s\n",wafer_id_back); */
            /* wafer_pos */
            if(gotToken(buff,raw_data)==FAIL)
                if(i==0) {
                    fprintf(gpLogFp, "Error EDA-XXXXX(%5d): Site Level wafer_pos No Data \n",giCurLine);
                    return FAIL;
                } else
                    break;
            if(TrimComa(buff)==0) {
                fprintf(gpLogFp, "Error EDA-XXXXX(%5d): Found a blank string in wafer_pos\n",giCurLine);
                return FAIL;
            }
            grMeasHist.wafer_pos=atoi(buff);

            /* site_pos */
            if(gotToken(buff,raw_data)==FAIL) {
                fprintf(gpLogFp, "Error EDA-XXXXX(%5d): Site Level site_pos No Data \n",giCurLine);
                return FAIL;
            }
            if(TrimComa(buff)==0) {
                fprintf(gpLogFp, "Error EDA-XXXXX(%5d): Found a blank string in wafer_pos\n",giCurLine);
                return FAIL;
            }
            grMeasHist.site_pos=atoi(buff);

            /* item_value */
            if(gotToken(buff,raw_data)==FAIL) {
                fprintf(gpLogFp, "Error EDA-XXXXX(%5d): Site Level item_value No Data \n",giCurLine);
                return FAIL;
            }
            if(TrimComa(buff)==0) {
                continue;  /* item_value = '' means this record is no use */
            }
            if(buff[0]=='*') {
                continue;  /* item_value = '*' means this record is no use */
            }
            grMeasHist.item_value=atof(buff);
            grRawData[grMeasHist.wafer_pos].site_flag[0]=1;
            grRawData[grMeasHist.wafer_pos].site_flag[grMeasHist.site_pos]=1;
            grRawData[grMeasHist.wafer_pos].value[grMeasHist.site_pos]=grMeasHist.item_value;
            strcpy((char*)grRawData[grMeasHist.wafer_pos].wafer_id_back[grMeasHist.site_pos],wafer_id_back);   /* add by hawk 20010615 */
            /*  fprintf(gpLogFp, "Site grRawData[%d].wafer_id_back[%d]:%s\n",grMeasHist.wafer_pos,grMeasHist.site_pos,grRawData[grMeasHist.wafer_pos].wafer_id_back[grMeasHist.site_pos]);
               fprintf(gpLogFp, "Site value:%f grRawData[%d].value[%d]:%f\n",grMeasHist.item_value,grMeasHist.wafer_pos,grMeasHist.site_pos,grRawData[grMeasHist.wafer_pos].value[grMeasHist.site_pos]);*/
        }
    } else if(strcmp(grMeasHist.meas_type,"Wafer")==0) { /* Wafer level */
        for(i=0; raw_data[0]!='\0' && raw_data[0]!='\n'; i++) {

            /* wafer_id */
            if(gotToken(buff,raw_data)==FAIL)
                if(i==0) {
                    fprintf(gpLogFp, "Error EDA-XXXXX(%5d): Wafer Level No Wafer ID Data \n",giCurLine);
                    /*  return FAIL; */
                }
            if(TrimComa(buff)==0) {
                /*  fprintf(gpLogFp, "EDA-XXXXX(%5d): Wafer Level Found a blank string in wafer id\n",giCurLine); */
                /*  return FAIL; */
            }
            wafer_id_back[0]='\0';
            strcpy(wafer_id_back,buff);  /* add by hawk 20010615 */
            len1=strlen(wafer_id_back);
            wafer_id_back[len1]='\0';
            /*   fprintf(gpLogFp, "Wafer wafer_id_back:%s\n",wafer_id_back);    */
            /* wafer_pos */
            if(gotToken(buff,raw_data)==FAIL) {
                fprintf(gpLogFp, "Error EDA-XXXXX(%5d): Wafer Level wafer_pos No Data \n",giCurLine);
                return FAIL;
            }
            if(TrimComa(buff)==0) {
                fprintf(gpLogFp, "Error EDA-XXXXX(%5d): Found a blank string in Wafer Level wafer_pos\n",giCurLine);
                return FAIL;
            }
            grMeasHist.wafer_pos=atoi(buff);

            /* site_pos */
            if(gotToken(buff,raw_data)==FAIL) {
                fprintf(gpLogFp, "Error EDA-XXXXX(%5d):Wafer level site_pos No Data \n",giCurLine);
                return FAIL;
            }
            grMeasHist.site_pos=0;
            /* item_value */
            if(gotToken(buff,raw_data)==FAIL) {
                fprintf(gpLogFp, "Error EDA-XXXXX(%5d): Wafer level item_value No Data \n",giCurLine);
                return FAIL;
            }
            if(TrimComa(buff)==0) {
                continue;  /* item_value = '' means this record is no use */
            }
            if(buff[0]=='*') {
                continue;  /* item_value = '*' means this record is no use */
            }
            grMeasHist.item_value=atof(buff);
            grRawData[grMeasHist.wafer_pos].site_flag[0]=1;
            grRawData[grMeasHist.wafer_pos].value[0]=grMeasHist.item_value;
            /*  grRawData[grMeasHist.wafer_pos].wafer_id_back[1]='\0'; */
            strcpy((char*)grRawData[grMeasHist.wafer_pos].wafer_id_back[1],wafer_id_back);   /* add by hawk 20010615 */
            /*  fprintf(gpLogFp, "Wafer grRawData[%d].wafer_id_back[1]:%s\n",grMeasHist.wafer_pos,grRawData[grMeasHist.wafer_pos].wafer_id_back[1]);
               fprintf(gpLogFp, "Wafer value:%f grRawData[%d].value[0]:%f\n",grMeasHist.item_value,grMeasHist.wafer_pos,grRawData[grMeasHist.wafer_pos].value[grMeasHist.site_pos]);  */
        }
    } else if(strcmp(grMeasHist.meas_type,"Lot")==0) { /* Lot level */
        /* wafer_id */
        if(gotToken(buff,raw_data)==FAIL)
            if(i==0) {
                fprintf(gpLogFp, "Error EDA-XXXXX(%5d):LOT LEVEL No Wafer ID Data \n",giCurLine);
                return FAIL;
            }
        if(TrimComa(buff)==0) {
            /*  fprintf(gpLogFp, "EDA-XXXXX(%5d): LOT LEVEL NO wafer id\n",giCurLine); */
            /*   return FAIL; */
        }
        strcpy(grWaferIdBack[0].wafer_id_back,buff);  /* add by hawk 20010615 */
        /*   fprintf(gpLogFp, "Lot grWaferIdBack[0].wafer_id_back:%s\n",grWaferIdBack[0].wafer_id_back);*/
        /* wafer_pos */
        if(gotToken(buff,raw_data)==FAIL) {
            fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
            return FAIL;
        }
        grMeasHist.wafer_pos=0;
        /* site_pos */
        if(gotToken(buff,raw_data)==FAIL) {
            fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
            return FAIL;
        }
        grMeasHist.site_pos=0;
        /* item_value */
        if(gotToken(buff,raw_data)==FAIL) {
            fprintf(gpLogFp, "Error EDA-XXXXX(%5d): No Data \n",giCurLine);
            return FAIL;
        }
        if(TrimComa(buff)==0) {
            return IGNORE;   /* item_value = '' means this record is no use */
        }
        if(buff[0]=='*') {
            return IGNORE;   /* item_value = '*' means this record is no use */
        }
        grMeasHist.item_value=atof(buff);
        grRawData[0].site_flag[0]=1;
        grRawData[0].value[0]=grMeasHist.item_value;
        /*  fprintf(gpLogFp, "LOT grRawData[0].value[0]:%f\n",grRawData[0].value[0]); */
    } else {
        fprintf(gpLogFp, "Error EDA-XXXXX(%5d): Found a error string in meas_type\n",giCurLine);
        return FAIL;
    }
    return SUCCEED;
}


/****************************************************************************
* TITLE      : Process Measurement History Data
* DESC.      : To process data, which  are  extracted from WAT spec. files. The
*              loader process relevant data for maintaining tables, which are
*              correlate to WAT. The tables consist of LOT, PRODUCT, TECHNOLOGY,
*              TECH_PROD_MAPPING, ..., etc.
* PARAMETERS : None
* RETURN     : if there is no error encuntered then return SUCCEED, else return
*              FAIL
* EXAMPLE    : ProcMeasHistData();  -- just call it
*****************************************************************************/
int ProcMeasHistData()
{
    int i,j,gcReturnCode,wfr_cnt;
    char buff[30];
    int len;
    switch(gtMeasHist.m_lot_type[0]) {
        case 'P':/* EDC rawdata*/
        case 'Q':
        case 'T':
        case 'S':
        case 'C':
        case 'R':
        case 'E':
        case 'M':
        case 'U':
            gcDataType='1';
            break; /* 1 for EDC */
        case 'A':
            gcDataType='2';
            break; /* 2 for RTC */
        default:
            /* remarked by Felix @2000/06/27
              fprintf(gpLogFp,"Warning EDA-XXXXX(%5d): Undefine meas_sub_lot_type[%s]\n",  giCurLine-1, gtMeasHist.m_lot_type); */
            gcDataType='1';
    }

    /* Check exist any data in grRawData */
    if(CheckExistRawData()==UNFOUND)
        return IGNORE;

    /* >>>>> Get lot_id from LOT table <<<<< */
    strcpy((char*)grLot.lotno.arr, (char*)gtMeasHist.p_lot_id);
    grLot.lotno.len = strlen((char*)grLot.lotno.arr);
    /* select lot_id */
    if(GetLotID() == FAIL)
        return FAIL;

    /* >>>>> Get technology_id from TECHNOLOGY table <<<<< */
    strcpy((char*)grTechnology.name.arr, (char*)gtMeasHist.m_route_id);
    grTechnology.name.len = strlen((char*)grTechnology.name.arr);
    /* select technoloty_id */
    if(GetTechID() == FAIL)
        return FAIL;

    /* >>>>> Get product_id from PRODUCT table <<<<< */
    /* prepare name of product */
    /* gtMeasHist.m_prod_id[strlen(gtMeasHist.m_prod_id)-3]='\0';*/ /* mark by hawk*/
    len=strlen(gtMeasHist.m_prod_id); /* update by hawk 20010702 to get FAB1& FAB2 PRODUCT ID*/
    for(i=0; i<=len; i++) {
        if (gtMeasHist.m_prod_id[i]=='.') {
            gtMeasHist.m_prod_id[i]='\0';
            break;
        }
    }

    strcpy((char*)grProduct.name.arr, (char*)gtMeasHist.m_prod_id);
    grProduct.name.len = strlen((char*)grProduct.name.arr);
    grProduct.flag = 0;
    /* select product_id */
    if(GetProdID() == FAIL)
        return FAIL;

    /* >>>>> Process TECH_PROD_MAPPING table <<<<< */
    if(gcDataType == '1') { /* for EDC */
        if(ProcTechProdMap() == FAIL)
            return FAIL;
    }

    /* >>>>> Get move_in_oper_id and move_out_oper_id from OPERATOR table <<<<< */
    if(TruncateRedundantField((char*)gtMeasHist.empl_id) == 0)
        grOperator.flag = -1;
    else {
        strcpy((char*)grOperator.name.arr, (char*)gtMeasHist.empl_id);
        grOperator.name.len = strlen((char*)grOperator.name.arr);
        /* select move_in_oper_id from OPERATOR table */
        if(GetOperatorID() == FAIL )
            return FAIL;
        grOperator.flag = 0;
    }

    /* >>>>> Get equipment_id of measurement and process equipments <<<<< */
    /* extract number of measurement equipment */
    if(TruncateRedundantField((char*)gtMeasHist.m_tool_id) == 0) {
        if(gcDataType == '1' || gcDataType == '3')
            grMeasEquip.flag = -1;
        else {
            fprintf(gpLogFp, "Wrong EDA-XXXXX(%05d): Found a blank string in m_tool_id\n", giCurLine-1);
            return IGNORE;
        }
    } else {
        /* extract name of measurement equipment */
        strcpy((char*)grEquipment.name.arr, (char*)gtMeasHist.m_tool_id);
        grEquipment.name.len = strlen((char*)grEquipment.name.arr);
        /* set module type of measurement equipment */
        strcpy((char*)grEquipment.module.arr, "WIP");
        grEquipment.module.len = 3;

        /* select equipment_id from EQUIPMENT table */
        if(GetEquipID() == FAIL)
            return FAIL;
        strcpy((char*)grMeasEquip.name.arr, (char*)gtMeasHist.m_tool_id);
        grMeasEquip.name.len = strlen((char*)grMeasEquip.name.arr);

        grMeasEquip.equipment_id = grEquipment.equipment_id;
        grMeasEquip.flag = 0;
    }
    /* extract number of process equipment */
    if(TruncateRedundantField((char*)gtMeasHist.p_tool_id) == 0)
        grProcEquip.flag = -1;
    else {
        /* extract name of process equipment */
        strcpy((char*)grEquipment.name.arr, (char*)gtMeasHist.p_tool_id);
        grEquipment.name.len = strlen((char*)grEquipment.name.arr);

        /* set module type of process equipment */
        strcpy((char*)grEquipment.module.arr, "WIP");
        grEquipment.module.len = 3;

        /* select equipment_id from EQUIPMENT table */
        if(GetEquipID() == FAIL)
            return FAIL;

        strcpy((char*)grProcEquip.name.arr, (char*)gtMeasHist.p_tool_id);
        grProcEquip.name.len = strlen((char*)grProcEquip.name.arr);

        grProcEquip.equipment_id = grEquipment.equipment_id;
        grProcEquip.flag = 0;
    }

    strcpy(grHeader.measure_time, gtMeasHist.reprt_time);
    for(i = 0; i < 19; i++) {
        if(i==10)
            if(grHeader.measure_time[i]==' ')
                continue;
            else {
                fprintf(gpLogFp, "Error(%d): Found a illegal date/time format(reprt_time=%s)\n", giCurLine-1, gtMeasHist.reprt_time);
                return FAIL;
            }
        if(!isdigit(grHeader.measure_time[i]) && grHeader.measure_time[i] != ':' && grHeader.measure_time[i] != '/') {
            fprintf(gpLogFp, "Error(%d): Found a illegal date/time format(reprt_time=%s)\n", giCurLine-1, gtMeasHist.reprt_time);
            return FAIL;
        }
    }

    /* >>>>> Get m_recipe from RECIPE table <<<<< */
    if(TruncateRedundantField((char*)gtMeasHist.m_recipe) == 0)
        grMeasRecipe.flag = -1;
    else {
        strcpy((char*)grMeasRecipe.name.arr, (char*)gtMeasHist.m_recipe);
        grMeasRecipe.name.len = strlen((char*)grMeasRecipe.name.arr);
        grMeasRecipe.flag = 0;
    }

    /* >>>>> Get m_lc_recipe from RECIPE table <<<<< */
    if(TruncateRedundantField((char*)gtMeasHist.m_lc_recipe) == 0)
        grMeasRecipe.lc_flag = -1;
    else {
        strcpy((char*)grMeasRecipe.lc_name.arr, (char*)gtMeasHist.m_lc_recipe);
        grMeasRecipe.lc_name.len = strlen((char*)grMeasRecipe.lc_name.arr);
        grMeasRecipe.flag = 0;
    }

    /* >>>>> Get m_recipe from RECIPE table <<<<< */
    if(TruncateRedundantField((char*)gtMeasHist.m_ph_recipe) == 0)
        grMeasRecipe.ph_flag = -1;
    else {
        strcpy((char*)grMeasRecipe.ph_name.arr, (char*)gtMeasHist.m_ph_recipe);
        grMeasRecipe.ph_name.len = strlen((char*)grMeasRecipe.ph_name.arr);
        grMeasRecipe.ph_flag = 0;
    }

    /* >>>>> Get p_recipe_id from RECIPE table <<<<< */
    if(TruncateRedundantField((char*)gtMeasHist.p_recipe) == 0)
        grProcRecipe.flag = -1;
    else {
        strcpy((char*)grProcRecipe.name.arr, (char*)gtMeasHist.p_recipe);
        grProcRecipe.name.len = strlen((char*)grProcRecipe.name.arr);
        grProcRecipe.flag = 0;
    }

    /* >>>>> Get p_lc_recipe from RECIPE table <<<<< */
    if(TruncateRedundantField((char*)gtMeasHist.p_lc_recipe) == 0)
        grProcRecipe.lc_flag = -1;
    else {
        strcpy((char*)grProcRecipe.lc_name.arr, (char*)gtMeasHist.p_lc_recipe);
        grProcRecipe.lc_name.len = strlen((char*)grProcRecipe.lc_name.arr);
        grProcRecipe.lc_flag = 0;
    }

    /* >>>>> Get p_ph_recipe from RECIPE table <<<<< */
    if(TruncateRedundantField((char*)gtMeasHist.p_ph_recipe) == 0)
        grProcRecipe.ph_flag = -1;
    else {
        strcpy((char*)grProcRecipe.ph_name.arr, (char*)gtMeasHist.p_ph_recipe);
        grProcRecipe.ph_name.len = strlen((char*)grProcRecipe.ph_name.arr);
        grProcRecipe.ph_flag = 0;
    }

    /* >>>>> Process ROUTE_STEP tables <<<<< */
    /* extract route and step */
    strcpy((char*)grRoute.route.arr, (char*)grTechnology.name.arr);
    grRoute.route.len = strlen((char *)grRoute.route.arr);

    strcpy((char*)grStep.step.arr, (char*)gtMeasHist.m_oper_no);
    grStep.step.len = strlen((char *)grStep.step.arr);

    switch(GetRouteStepID()) {
        case FAIL:
            return FAIL;
        case UNFOUND:
            if(InsertRouteStepID()==FAIL)
                return FAIL;
    }
    grRoute.desc.arr[grRoute.desc.len] = '\0';

    /* >>>>> Get parameter_id <<<<< */
    sprintf((char*)grParameter.parameter_name.arr, "%s~%s~%s", gtMeasHist.m_route_id, gtMeasHist.m_oper_no, gtMeasHist.item_fld);
    grParameter.parameter_name.len = strlen((char *)grParameter.parameter_name.arr);

    TruncateRedundantField((char*)gtMeasHist.p_oper_no);

    sprintf((char*)grParameter.p_parameter_name.arr, "%s~%s~%s", gtMeasHist.p_route_id, gtMeasHist.p_oper_no, gtMeasHist.item_fld);
    grParameter.p_parameter_name.len = strlen((char *)grParameter.p_parameter_name.arr);
    grParameter.flag = 0;

    /***** process EDC_PARAMETER table and select edc_parameter_id *****/
    if(gcDataType == '1') {
        if(GetEdcParaID() == FAIL)
            return FAIL;
        grCurParameter.parameter_id = grParameter.parameter_id;
        if(ProcProdEdcParaMap() == FAIL)
            return FAIL;
    } else {
        if(GetRtcParaID() == FAIL)
            return FAIL;

        grCurParameter.parameter_id = grParameter.parameter_id;
        if(ProcEquipRtcParaMap() == FAIL)
            return FAIL;
    }
    /* >>>>> Get mfld_id  <<<<< */
    strcpy((char*)grMeasFieldInfo.mfld_id.arr, (char*)gtMeasHist.mfld_id);
    grMeasFieldInfo.mfld_id.len = strlen((char *)grMeasFieldInfo.mfld_id.arr);

    /* >>>>> Get Prcss Grp*/
    strcpy((char*)grLotHist.prcss_grp.arr,(char*)gtMeasHist.prcss_grp);
    grLotHist.prcss_grp.len=strlen((char*)grLotHist.prcss_grp.arr);

    /*============================================================*/
    /*======= Process Edc/Rtc Raw Data ===========================*/
    /*============================================================*/

    switch(gcDataType) {
        case '1': { /* EDC rawdata*/
            switch(gtMeasHist.meas_type[0]) {
                    /* Lot level */
                case 'L':
#ifdef _TEST_
                    ShowStep(" Process EDC LOT");
#endif
                    grEdcLotSum.wafer_count=0;
                    gcReturnCode=CheckEdcLotInfo();
                    switch(gcReturnCode) {
                        case FAIL:
                            return FAIL;
                        case UNFOUND:
                        case FOUND:
                            if(InsertEdcLotInfo()==FAIL)
                                return FAIL;
                        case SKIP:
                            switch(CheckEdcLotSummary()) {
                                case FAIL:
                                    return FAIL;
                                    break;
                                case SKIP:
#ifdef _TEST_
                                    ShowStep(" ======>SKIP");
#endif
                                    return SKIP;
                                    break;
                            }
                    }
                    switch(CheckEdcLotSummary()) {
                        case FAIL:
                            return FAIL;
                            break;
                        case SKIP:
#ifdef _TEST_
                            ShowStep(" ======>SKIP");
#endif
                            return SKIP;
                            break;
                    }

                    if(InsertEdcLotSummary()==FAIL)
                        return FAIL;
#ifdef _TEST_
                    ShowStep(" ===========> OK !!");
#endif
                    break;
                    /* Wafer level */
                case 'W':
#ifdef _TEST_
                    ShowStep(" Process EDC Wafer");
#endif
                    for(i=1,wfr_cnt=0; i<MAX_WAFER; i++) /* Count the wafer number and lot average*/
                        if(grRawData[i].site_flag[0]!=-1) {   /* if flag==-1 ==> non used */
                            wfr_cnt++;
                            grRawData[0].value[0]+=grRawData[i].value[0];
                        }
                    grEdcLotSum.wafer_count=wfr_cnt;    /* wafer number*/
                    grRawData[0].value[0]/=(double)wfr_cnt;  /* lot average */

                    gcReturnCode=CheckEdcLotInfo();
                    switch(gcReturnCode) {
                        case FAIL:
                            return FAIL;
                        case UNFOUND:
                        case FOUND:
                            if(InsertEdcLotInfo()==FAIL)
                                return FAIL;
                        case SKIP:
                            switch(CheckEdcLotSummary()) {
                                case FAIL:
                                    return FAIL;
                                    break;
                                case SKIP:
#ifdef _TEST_
                                    ShowStep(" ======>SKIP");
#endif
                                    return SKIP;
                                    break;
                            }
                    }

                    for(i=1,j=0,wfr_cnt=0; i<MAX_WAFER; i++) /* When flag != -1 (be used) */
                        if(grRawData[i].site_flag[0]!=-1) {    /* Insert RawData into EdcWaferSummary */
                            /* modify 2000/08/21 */
                            gcReturnCode=CheckEdcWaferInfo(i); /* Check if exist the older EdcWaferSummary record */
                            switch(gcReturnCode) {
                                case FAIL:
                                    return FAIL;
                                case UNFOUND:       /* If nonfound,it's meaning that no record in EdcWaferInfo and EdcWaferSummary */
                                case FOUND:         /* If existed the older,mark it to old */
                                    if(InsertEdcWaferInfo(i)==FAIL)    /* So we should insert these rawdata into EdcWaferInfo at first*/
                                        return FAIL;
                                case SKIP:
                                    switch(CheckEdcWaferSummary(i)) {
                                        case FAIL:
                                            return FAIL;
                                            break;
                                        case SKIP:
#ifdef _TEST_
                                            ShowStep(" ======>SKIP");
#endif
                                            return SKIP;
                                            break;
                                    }
                            }

                            if(InsertEdcWaferSummary(i)==FAIL)
                                return FAIL;
                            j++;
                        }
                    if(j==0) {
                        fprintf(gpLogFp,"Wafer count is NULL\n");
                        return FAIL;
                    } else if(InsertEdcLotSummary(j)==FAIL)   /* Insert into EdcLotSummary */
                        return FAIL;
#ifdef _TEST_
                    ShowStep(" ===========> OK !!");
#endif
                    break;
                    /* Site level */
                case 'S':
#ifdef _TEST_
                    ShowStep(" Process EDC Site");
#endif
                    /* Added by FELIX@2001/07/31==>To get wafer_count.*/
                    for(i=1,wfr_cnt=0; i<MAX_WAFER; i++) /* Count the wafer number and lot average*/
                        if(grRawData[i].site_flag[0]!=-1) {   /* if flag==-1 ==> non used */
                            wfr_cnt++;
                            grRawData[0].value[0]+=grRawData[i].value[0];
                        }
                    grEdcLotSum.wafer_count=wfr_cnt;    /* wafer number*/
                    gcReturnCode=CheckEdcLotInfo();
                    switch(gcReturnCode) {
                        case FAIL:
                            return FAIL;
                        case UNFOUND:
                        case FOUND:
                            if(InsertEdcLotInfo()==FAIL)
                                return FAIL;
                        case SKIP:
                            switch(CheckEdcLotSummary()) {
                                case FAIL:
                                    return FAIL;
                                    break;
                                case SKIP:
#ifdef _TEST_
                                    ShowStep(" ======>SKIP");
#endif
                                    return SKIP;
                                    break;
                            }
                    }

                    for(i=1; i<MAX_WAFER; i++) { /* To Get wafer number that we have used */
                        if(grRawData[i].site_flag[0]!=-1) {
                            /* modify 2000/08/21 */
                            /*  fprintf(gpLogFp,"i:%d\n",i);  */
                            gcReturnCode=CheckEdcWaferInfo(i); /* Check if exist the older EdcWaferSummary record */
                            switch(gcReturnCode) {
                                case FAIL:
                                    return FAIL;
                                case UNFOUND:       /* If nonfound,it's meaning that no record in EdcWaferInfo and EdcWaferSummary */
                                case FOUND:         /* If existed the older,mark it to old */
                                    if(InsertEdcWaferInfo(i)==FAIL)    /* So we should insert these rawdata into EdcWaferInfo at first*/
                                        return FAIL;
                                case SKIP:
                                    switch(CheckEdcWaferSummary(i)) {
                                        case FAIL:
                                            return FAIL;
                                            break;
                                        case SKIP:
#ifdef _TEST_
                                            ShowStep(" ======>SKIP");
#endif
                                            return SKIP;
                                            break;
                                    }
                            }

                            CheckEdcSpecValue(i);
#ifdef _ALLSITE_
                            if(ProcessSummaryBySite(i)==FAIL)    /* To calculate Q1,Q3,Max,Min... */
                                return FAIL;
                            if(InsertEdcWaferSummaryBySite()==FAIL) /* Insert rawdata into EdcWaferSummary */
                                return FAIL;
#else
                            if(ProcessSummaryBy9Site(i)==FAIL)    /* To calculate Q1,Q3,Max,Min... */
                                return FAIL;
                            if(InsertEdcWaferSummaryBy9Site()==FAIL) /* Insert rawdata into EdcWaferSummary */
                                return FAIL;
#endif
                        } /* if(grRawData[i].site_flag[0]!=-1) */
                    }  /*for(i=1;i<MAX_WAFER;i++) */
#ifdef _ALLSITE_
                    ProcessLotAvgBySite();
                    if(InsertEdcLotSummaryBySite()==FAIL)
                        return FAIL;
#else
                    ProcessLotAvgBy9Site();
                    if(InsertEdcLotSummaryBy9Site()==FAIL)
                        return FAIL;
#endif
#ifdef _TEST_
                    ShowStep(" ===========> OK !!");
#endif
                    break;
                    /* FAIL Measure Type */
                default:
                    fprintf(gpLogFp,"Error EDA-XXXXX(%5d): Error meas_type in EDC[%s]\n", giCurLine-1, gtMeasHist.meas_type);
                    return FAIL;
            } /* switch(gtMeasHist.meas_type[0]) */
            return SUCCEED;
        } /*case '1': EDC rawdata*/
        case '2': { /* RTC rawdata*/
            gcReturnCode=CheckRtcSummary();
            if(gcReturnCode==FAIL)
                return FAIL;
            else if(gcReturnCode==SKIP) {
#ifdef _TEST_
                ShowStep(" ======>SKIP");
#endif
                return SKIP;
            } else if(gcReturnCode==FOUND)       /* If existed the older,mark it to old */
                if(UpdateRtcSummary()==FAIL)
                    return FAIL;

            for(i=0;; i++) { /* To Get wafer number that we have used */
                if(grRawData[i].site_flag[0]!=-1)
                    break;
                if(i>=MAX_WAFER) {
                    fprintf(gpLogFp, "Error EDA-XXXXX: RTC Wafer number error when process RtcSummaryBySite\n");
                    return FAIL;
                }
            }
            switch(gtMeasHist.meas_type[0]) {
                    /* Wafer level */
                case 'W':
                    sprintf(buff,"%02d",i);
                    strcpy((char*)grWafer.wafer_no.arr,(char*)buff);
                    grWafer.wafer_no.len=strlen((char*)grWafer.wafer_no.arr);
                    if(GetWaferID()==FAIL)
                        return FAIL;
                    /* Lot level */
                case 'L':

#ifdef _TEST_
                    ShowStep(" Process RTC Wafer");
#endif
                    if(InsertRtcSummaryByWafer(i)==FAIL)
                        return FAIL;
#ifdef _TEST_
                    ShowStep(" ===========> OK !!");
#endif
                    break;
                    /* Site level */
                case 'S':
#ifdef _TEST_
                    ShowStep(" Process RTC Site");
#endif
                    CheckRtcSpecValue(i);
#ifdef _ALLSITE_
                    if(ProcessSummaryBySite(i)==FAIL)
                        return FAIL;
                    if(InsertRtcSummaryBySite()==FAIL)
                        return FAIL;
#else
                    if(ProcessSummaryBy9Site(i)==FAIL)
                        return FAIL;
                    if(InsertRtcSummaryBy9Site()==FAIL)
                        return FAIL;
#endif
#ifdef _TEST_
                    ShowStep(" ===========> OK !!");
#endif
                    break;
                    /* FAIL Measure Type */
                default:
                    fprintf(gpLogFp,"Error EDA-XXXXX(%5d): Error meas_type in RTC[%s]\n", giCurLine-1, gtMeasHist.meas_type);
                    return FAIL;
            }
            return SUCCEED;
        }
    }
}

/*****************************************************************************
* TITLE     : Remove Data File
* DESC.     : To remove the file which had processed successfully to backup
*             directory
* PARAMETER : filename -- name which will be remove (with the path which file located)
* RETURN    : None
* EXAMPLE   : RemoveDataFile("edbr1:[acme.incoming]N19998723.LD"); -- just call it
*****************************************************************************/
void RemoveDataFile(sFileName)
char *sFileName;
{
    char    sCommand[512];
    sprintf(sCommand, "%s %s%s%s %s", MOVE, gsDataDir, SEPS, sFileName, gsBackupDir);
    system(sCommand);
}/* End RemoveDataFile() */

/*****************************************************************************
* TITLE     : Remove Error File
* DESC.     : To remove the file which had processed failure to errordata directory
* PARAMETER : filename -- name which will be remove (with the path which file located)
* RETURN    : None
* EXAMPLE   : RemoveErrorFile("edbr1:[acme.incoming]N19998723.LD"); -- just call it
*****************************************************************************/
void RemoveErrorFile(sFileName)
char *sFileName;
{
    char    sCommand[512];
    sprintf(sCommand, "%s %s%s%s %s", MOVE, gsDataDir, SEPS, sFileName, gsErrorDir);
    system(sCommand);
}/* End RemoveErrorFile() */

/*****************************************************************************
* TITLE     : Close Log File
* DESC.     : To close file pointer which point to log file.
* PARAMETER : None
* RETURN    : None
* EXAMPLE   : CloseLogFile(); -- just call it
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
}/* End CloseLogFile() */

/*****************************************************************************
* TITLE     :
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

    if(sqlca.sqlcode != 0) {
        buf_len = sizeof(msg);
        sqlglm(msg, &buf_len, &msg_len);
        msg[msg_len] = '\0';
        fprintf(gpLogFp,"SQL Error(%05d): %s\n", giCurLine, str);
        fprintf(gpLogFp,"   [x] %s\n", msg);
    }
    fflush(gpLogFp);
}/* End CheckSqlError() */

int TrimComa(sTemp)
char *sTemp;
{
    char    *sStr = "";
    unsigned sleng;
    sStr = sTemp;
    sleng=strlen(sStr);
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
    return strlen(sTemp);
}/* End TrimComa() */

/*****************************************************************************
* TITLE     : Change Date Format
* DESC.     : Transfer the format of date from YYYYMMDDHH24.MI.SS to YYYY/MM/DD HH24:MI:SS.
*             For example, transfer "1997071013.22.11" to "1997/07/10 13:22:11".
* PARAMETER : char *fromDate -- the date string which will be transfer
*             char *toDate   -- the date string which has been transfered
* RETURN    : return a transfered date-string which formate is "YYYY/MM/DD HH24:MI:DD"
* EXAMPLE   : char date1,          --  destination string
*                  date2="1997071010.03.55 "; --  source string
*             covChangeDateTimeFormat(date1,date2); --  call covChangeDateTimeFormat()
* RESULT    : date1 = "1997/07/10 10:03:55"
*****************************************************************************/
void covChangeDateTimeFormat(char *toDate, char *fromDate)
{
    char      sYY[4+1] = "",   /* year */
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

/*****************************************************************************
  TITLE     : gotToken
  DESC.     : Parse data line
  PURPOSE   : To cut rawdata by (",).
  PARAMETER :
  RETURN    : SUCCEED/FAIL
  EXAMPLE   : if(goParseLine(Buff,rawDataLine)==FAIL)
******************************************************************************/
int gotToken(gBuff,rline)
char gBuff[100];
char rline[LENGTH];
{
    int j;
    char temp[LENGTH];
    strcpy (temp,rline);
    if(strlen(temp)==0)
        return FAIL;
    for(j=0;; j++) {
        if(temp[j]==',')
            if(temp[j-1]=='\"')
                break;
        if(temp[j]=='\0' || temp[j]=='\n')
            break;
    }
    temp[j]='\0';
    strcpy(gBuff,temp);
    strcpy(rline,(rline+j+1));
    return SUCCEED;
}  /* End gotToken*/

void CleanOldData()
{
    int i,j;
    strcpy(stOldData.p_lot_id,"");
    strcpy(stOldData.m_route_id,"");
    strcpy(stOldData.m_oper_no,"");
    strcpy(stOldData.item_fld,"");
    strcpy(stOldData.meas_type,"");
    for(i=0; i<MAX_WAFER; i++) {
        grRawData[i].wfr_flag=0;
        grRawData[i].site_cnt=0;
        for(j=0; j<MAX_SITE; j++) {
            grRawData[i].site_flag[j]=-1;
            grRawData[i].value[j]=0.0f;
        }
    }
} /* End CleanOldData */

void StoreOldData()
{
    strcpy(stOldData.p_lot_id,grMeasHist.p_lot_id);
    strcpy(stOldData.m_route_id,grMeasHist.m_route_id);
    strcpy(stOldData.m_oper_no,grMeasHist.m_oper_no);
    strcpy(stOldData.item_fld,grMeasHist.item_fld);
    strcpy(stOldData.meas_type,grMeasHist.meas_type);
}

void StoreMeasHistData()
{
    strcpy(gtMeasHist.p_lot_id,grMeasHist.p_lot_id);
    strcpy(gtMeasHist.p_lot_type,grMeasHist.p_lot_type);
    strcpy(gtMeasHist.reprt_time,grMeasHist.reprt_time);
    strcpy(gtMeasHist.m_lot_id,grMeasHist.m_lot_id);
    strcpy(gtMeasHist.m_lot_type,grMeasHist.m_lot_type);
    strcpy(gtMeasHist.m_route_id,grMeasHist.m_route_id);
    strcpy(gtMeasHist.m_oper_no,grMeasHist.m_oper_no);
    strcpy(gtMeasHist.mfld_id,grMeasHist.mfld_id);
    strcpy(gtMeasHist.item_fld,grMeasHist.item_fld);
    strcpy(gtMeasHist.meas_type,grMeasHist.meas_type);
    strcpy(gtMeasHist.p_prod_id,grMeasHist.p_prod_id);
    strcpy(gtMeasHist.p_tech_id,grMeasHist.p_tech_id);
    strcpy(gtMeasHist.p_route_id,grMeasHist.p_route_id);
    strcpy(gtMeasHist.p_oper_no,grMeasHist.p_oper_no);
    strcpy(gtMeasHist.p_pd_id,grMeasHist.p_pd_id);
    strcpy(gtMeasHist.p_pd_name,grMeasHist.p_pd_name);
    strcpy(gtMeasHist.p_tool_id,grMeasHist.p_tool_id);
    strcpy(gtMeasHist.p_tool_name,grMeasHist.p_tool_name);
    strcpy(gtMeasHist.p_lc_recipe,grMeasHist.p_lc_recipe);
    strcpy(gtMeasHist.p_recipe,grMeasHist.p_recipe);
    strcpy(gtMeasHist.p_ph_recipe,grMeasHist.p_ph_recipe);
    strcpy(gtMeasHist.empl_id,grMeasHist.empl_id);
    strcpy(gtMeasHist.p_wfhist_time,grMeasHist.p_wfhist_time);
    strcpy(gtMeasHist.p_customer_id,grMeasHist.p_customer_id);
    strcpy(gtMeasHist.p_time,grMeasHist.p_time);
    strcpy(gtMeasHist.m_prod_id,grMeasHist.m_prod_id);
    strcpy(gtMeasHist.m_tech_id,grMeasHist.m_tech_id);
    strcpy(gtMeasHist.m_pd_id,grMeasHist.m_pd_id);
    strcpy(gtMeasHist.m_pd_name,grMeasHist.m_pd_name);
    strcpy(gtMeasHist.m_tool_id,grMeasHist.m_tool_id);
    strcpy(gtMeasHist.m_tool_name,grMeasHist.m_tool_name);
    strcpy(gtMeasHist.m_lc_recipe,grMeasHist.m_lc_recipe);
    strcpy(gtMeasHist.m_recipe,grMeasHist.m_recipe);
    strcpy(gtMeasHist.m_ph_recipe,grMeasHist.m_ph_recipe);
    strcpy(gtMeasHist.report_time_2,grMeasHist.report_time_2);
    strcpy(gtMeasHist.empl_id_2,grMeasHist.empl_id_2);
    strcpy(gtMeasHist.m_wfhist_time,grMeasHist.m_wfhist_time);
    /*strcpy(gtMeasHist.wafer_id,grMeasHist.wafer_id); */ /*mark by hawk 20010622 */
    strcpy(gtMeasHist.prcss_grp,grMeasHist.prcss_grp);
} /* End StoreOldData */

/*****************************************************************************
* TITLE     : Get Lot'd ID
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
        INTO    :grLot.lot_id
        FROM    LOT
        WHERE   LOTNO = :grLot.lotno;

    /* Check if no data exists */
    if (sqlca.sqlcode == 100 || sqlca.sqlcode==1403) {
        /* Insert data of lot's number into LOT table, and get lot_id */
        if (InsertLotID() == FAIL)
            return FAIL;

        /* If no error condition occurs */
        return SUCCEED;
    }

    /* Check if an error condition occurs */
    else if (sqlca.sqlcode) {
        CheckSqlError("GetLotID() SELECT LOT_ID");
        return FAIL;
    }

    /* If no error contition occurs */
    return SUCCEED;
}/* End of SelectLotID() */

/*****************************************************************************
* TITLE     :
* DESC.     :
* PARAMETER :
* RETURN    :
* EXAMPLE   :
*****************************************************************************/
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
        VALUES  ( :grLot.lotno,
                  lot_id.NEXTVAL,
                  lot_id.NEXTVAL,
                  :grLot.fab);

    /* Check if an error condition occurs */
    if (sqlca.sqlcode) {
        CheckSqlError("InsertLotID() INSERT LOT");
        return FAIL;
    }

    /* Select lot_id */
    EXEC SQL
        SELECT  LOT_ID
        INTO    :grLot.lot_id
        FROM    LOT
        WHERE   LOTNO = :grLot.lotno;

    if (sqlca.sqlcode) {
        CheckSqlError("InsertLotID() SELECT LOT_ID");
        return FAIL;
    }

    /* If no error occurs */
    return SUCCEED;
}/* end InsertLotID() */

/*****************************************************************************
* TITLE     : Get Equipment ID
* DESC.     : To get equipment_id from EQUIPMENT table
* PARAMETER : None
* RETURN    :
* EXAMPLE   : GetEquipID();  -- just call it
*****************************************************************************/
int GetEquipID()
{
    /* Get equipment_id */
    EXEC SQL
        SELECT  EQUIPMENT_ID
        INTO    :grEquipment.equipment_id
        FROM    EQUIPMENT
        WHERE   NAME   = :grEquipment.name  AND
                MODULE = :grEquipment.module;

    /* Check whether equipment's name exists in DB */
    if (sqlca.sqlcode == 100 || sqlca.sqlcode==1403) {
        EXEC SQL
            INSERT
            INTO    EQUIPMENT(
                      NAME,
                      MODULE,
                      EQUIPMENT_ID)
            VALUES  ( :grEquipment.name,
                      :grEquipment.module,
                      equipment_id.NEXTVAL);

        /* Check whether occurs an error condition after insert equipment's name */
        if (sqlca.sqlcode) {
            CheckSqlError("GetEquipID() INSERT EQUIPMENT after INSERT");
            return FAIL;
        }

        /* Get equipment_id after insert equipment's name into DB */
        EXEC SQL
            SELECT  EQUIPMENT_ID
            INTO    :grEquipment.equipment_id
            FROM    EQUIPMENT
            WHERE   NAME   = :grEquipment.name  AND
                    MODULE = :grEquipment.module;

        /* Check whether occurs an error condition after select equipment_id */
        if (sqlca.sqlcode) {
            CheckSqlError("GetEquipID() SELECT EQUIPMENT_ID after INSERT EQUIPMENT");
            return FAIL;
        }
        return SUCCEED;
    }

    /* Check whether occurs an error condition after select equipment_id */
    else if (sqlca.sqlcode) {
        CheckSqlError("GetEquipID() SELECT EQUIPMENT_ID");
        return FAIL;
    }

    /* No error occurs ( sqlca.sqlcode == 0 ) */
    return SUCCEED;
}/* end GettEquipID() */



/*****************************************************************************
* TITLE     : Get Operator Id
* DESC.     : To get operator_id from OPERATOR table
* PARAMETER : None
* RETURN    : If no error occurs, then return SUCCEED, else FAIL
* EXAMPLE   : GetOperatorID();  --  just call it
*****************************************************************************/
int GetOperatorID()
{
    /* Get operator_id */
    EXEC SQL
        SELECT  OPERATOR_ID
        INTO    :grOperator.operator_id
        FROM    OPERATOR
        WHERE   NAME = :grOperator.name;

    /* Check whether operator's name exists in OPERATOR table or not */
    if (sqlca.sqlcode == 100 || sqlca.sqlcode==1403) {
        /* Insert operator's name into DB */
        EXEC SQL
            INSERT
            INTO    OPERATOR(
                      NAME,
                      OPERATOR_ID)
            VALUES  ( :grOperator.name,
                      operator_id.NEXTVAL);

        /* Check whether occurs an error condition */
        if (sqlca.sqlcode) {
            CheckSqlError("GetOperatorID() INSERT OPERATOR");
            return FAIL;
        }

        /* Get operator_id */
        EXEC SQL
            SELECT  OPERATOR_ID
            INTO    :grOperator.operator_id
            FROM    OPERATOR
            WHERE   NAME = :grOperator.name;

        /* Check whether occurs an error condition */
        if (sqlca.sqlcode) {
            CheckSqlError("GetOperatorID() SELECT OPERATOR_ID after INSERT");
            return FAIL;
        }

        /* No error occurs */
        return SUCCEED;
    }

    /* Check whether occurs an error condition */
    else if (sqlca.sqlcode) {
        CheckSqlError("GetOperatorID() SELECT OPERATOR_ID");
        return FAIL;
    }

    /* No error occurs ( sqlca.sqlcode == 0 ) */
    return SUCCEED;
}/* end GetOperatorID() */

/*****************************************************************************
* TITLE     : Truncate Redundant Blank Field
* DESC.     : To truncate redundanct leading and ending blank character(s) from
*             a specific string.
* PARAMETER : sStr -- the string, which will be cleaned
* RETURN    : a pointer, which point to the cleaned string
* EXAMPLE   : char str="  DAVID     Chang   "; --  will be manipulated string
*             TruncateRedundantField(str); --  call CleanBlankField()
* RESULT    : str="DAVID     Chang"
*****************************************************************************/
int TruncateRedundantField(char *sStr)
{
    char    *sTemp = "";    /* store truncated string temporarily */
    int     iLen;           /* store the length of string */


    sTemp = sStr;

    iLen = strlen(sTemp);

    for (; iLen > 0; iLen--) {
        if (isprint(*sTemp) && *sTemp != ' ')
            break;

        sTemp ++;
    }


    iLen = strlen(sTemp);

    for (; iLen > 0; iLen--) {
        if (!isprint(sTemp[iLen - 1]) || sTemp[iLen - 1] == ' ')
            sTemp[iLen - 1] = '\0';
        else
            break;
    }

    sStr = sTemp;

    return (strlen(sTemp));
}/* End TruncateRedundantField() */

/****************************************************************************
* TITLE     : Get Technology ID
* DESC.     : To get technology_id from TECHNOLOGY table.
* PARAMETER : None
* RETRUN    : if occurs an error condition, then return FAIL, else return SUCCEED
* EXAMPLE   : int ret_code;  --  return flag
*             ret_code = GetTechID();  --  just call it
****************************************************************************/
int GetTechID()
{
    /* Get technology_id */
    EXEC SQL
       SELECT   TECHNOLOGY_ID
       INTO     :grTechnology.technology_id
       FROM     TECHNOLOGY
       WHERE    NAME = :grTechnology.name;

    /* Check whether technology id doesn't exist in DB */
    if (sqlca.sqlcode == 100 || sqlca.sqlcode==1403) {
        /* If doesn't exist in DB, then insert technology's name into DB */
        EXEC SQL
           INSERT
           INTO  TECHNOLOGY(
                   NAME,
                   TECHNOLOGY_ID)
           VALUES  ( :grTechnology.name,
                   TECHNOLOGY_ID.NEXTVAL);

        /* Check whether occurs an error condition */
        if (sqlca.sqlcode) {
            CheckSqlError("GetTechID() INSERT TECHNOLOGY");
            return FAIL;
        }

        /* If doesn't occur an error condition, then get technology_id */
        EXEC SQL
           SELECT   TECHNOLOGY_ID
           INTO    :grTechnology.technology_id
           FROM    TECHNOLOGY
           WHERE   NAME = :grTechnology.name;

        /* Check whether occurs an error condition */
        if (sqlca.sqlcode) {
            CheckSqlError("GetTechID() SELECT TECHNOLOGY_ID after INSERT");
            return FAIL;
        }

        /* No error occurs */
        return SUCCEED;
    }

    /* Check whether occurs an error condition */
    else if (sqlca.sqlcode) {
        CheckSqlError("GetTechID() SELECT TECHNOLOGY_ID");
        return FAIL;
    }

    /* No error occurs ( sqlca.sqlcode == 0 ) */
    return SUCCEED;
}/* End of GetTechID() */

/*****************************************************************************
* TITLE     : Get Product ID
* DESC.     : To get product_id from PRODUCT table
* PARAMETER : None
* RETURN    : if an error occurs, then return FAIL, else return SUCCEED
* EXAMPLE   : GetProductID() -- just call it
*****************************************************************************/
int GetProdID()
{
    /* Get product_id */
    EXEC SQL
       SELECT   PRODUCT_ID
       INTO    :grProduct.product_id
       FROM    PRODUCT
       WHERE   NAME = :grProduct.name;

    /* Check whether product's name exists in PRODUCT table or not */
    if (sqlca.sqlcode == 100 || sqlca.sqlcode==1403) {
        EXEC SQL
           INSERT
           INTO    PRODUCT(
                     NAME,
                     PRODUCT_ID,
                     PARENTPRODUCT_ID)
           VALUES   (   :grProduct.name,
                     product_id.NEXTVAL,
                     :grProduct.parentproduct_id);

        /* Check whether occurs an error condition */
        if (sqlca.sqlcode) {
            CheckSqlError("GetProdID() INSERT PRODUCT");
            return FAIL;
        }


        /* Get product_id */
        EXEC SQL
           SELECT  PRODUCT_ID
           INTO    :grProduct.product_id
           FROM    PRODUCT
           WHERE   NAME=:grProduct.name;

        /* Check whether occurs an error condition */
        if (sqlca.sqlcode) {
            CheckSqlError("GetProdID() SELECT PRODUCT_ID after INSERT PRODUCT");
            return FAIL;
        }

        /* No error occurs */
        return SUCCEED;

    }

    /* Check whether occurs an error condition */
    else if (sqlca.sqlcode) {
        CheckSqlError("GetProdID() SELECT PRODUCT_ID");
        return FAIL;
    }

    /* No error occurs ( sqlca.sqlcode == 0 ) */
    return SUCCEED;
}/* End GetProdID() */

/*****************************************************************************
 TITLE     :
 DESC.     :
 PARAMETER :
 RETURN    :
******************************************************************************/
int GetRouteStepID()
{
    EXEC SQL
        SELECT  ROUTE_STEP_ID
        INTO    :grRouteStep.route_step_id
        FROM    ROUTE_STEP
        WHERE   ROUTE = :grRoute.route AND
                STEP = :grStep.step;

    if (sqlca.sqlcode == 100 || sqlca.sqlcode==1403)
        return UNFOUND;

    else if (sqlca.sqlcode) {
        CheckSqlError("GetRouteStepID() SELECT ROUTE_STEP_ID");
        return FAIL;
    }

    /* (sqlca.sqlcode == 0) */
    return FOUND;
}/* end GetRouteStepID() */

/*****************************************************************************
* TITLE        : Process Product and RTC parameter Mapping
* DESC.        : To process PROD_EDC_PARAMETER_MAPPING table.
* AUTHOR       : Erison Liang
* PARAMETER    : None
* RETURN       : If no error occurs, then return SUCCEED, else return FAIL.
*****************************************************************************/
int ProcProdEdcParaMap()
{
    EXEC SQL BEGIN DECLARE SECTION;
        int          product_id;
    EXEC SQL END DECLARE SECTION;

    EXEC SQL
       SELECT   PRODUCT_ID
       INTO  :product_id
       FROM  PROD_EDC_PARAMETER_MAPPING
       WHERE PRODUCT_ID = :grProduct.product_id AND
             EDC_PARAMETER_ID = :grCurParameter.parameter_id;

    if (sqlca.sqlcode == 100 || sqlca.sqlcode==1403) {
        EXEC SQL
           INSERT
           INTO  PROD_EDC_PARAMETER_MAPPING(
                    PRODUCT_ID,
                    EDC_PARAMETER_ID)
           VALUES ( :grProduct.product_id,
                          :grCurParameter.parameter_id);

        /* Check wether occurs an error condition */
        if (sqlca.sqlcode) {
            CheckSqlError("ProcProdEdcParaMap() INSERT PROD_EDC_PARAMETER_MAPPING");
            return FAIL;
        }

        /* No error occurs */
        return SUCCEED;
    }


    /* Check whether occurs an error condition */
    else if (sqlca.sqlcode) {
        CheckSqlError("ProcProdEdcParaMap() SELECT PRODUCT_ID");
        return FAIL;
    }

    /* No error occurs ( sqlca.sqlcode == 0 ) */
    return SUCCEED;

}/* End ProcProdEdcParaMap() */

/*****************************************************************************
* TITLE     : Get EDC Parameter ID
* PURPOSE   :
* PARAMETER :
* RETURN    :
* EXAMPLE   :
*****************************************************************************/
int GetEdcParaID()
{
    EXEC SQL BEGIN DECLARE SECTION;
        VARCHAR     p_name[56];
    EXEC SQL END DECLARE SECTION;

    EXEC SQL
        SELECT  EDC_PARAMETER_ID,
                NVL(P_NAME, 'NULL')
        INTO    :grParameter.parameter_id,
                :p_name
        FROM    EDC_PARAMETER
        WHERE   NAME = :grParameter.parameter_name AND
                ROUTE_STEP_ID = :grRouteStep.route_step_id;

    if (sqlca.sqlcode == 100 || sqlca.sqlcode==1403) {
        if (InsertEdcParaID() == FAIL)
            return FAIL;

        /* If no error occurs (sqlca.sqlcode == 0) */
        return SUCCEED;
    }

    else if (sqlca.sqlcode) {
        CheckSqlError("GetEdcParaID() SELECT EDC_PARAMETER_ID");
        return FAIL;
    }

    /* If no error condition occurs (sqlca.sqlcode == 0) */
    if (strstr((char*)p_name.arr, "NULL") != NULL) {
        EXEC SQL
            UPDATE  EDC_PARAMETER
            SET     P_NAME = :grParameter.p_parameter_name:grParameter.flag
            WHERE   NAME = :grParameter.parameter_name AND
                    ROUTE_STEP_ID = :grRouteStep.route_step_id;

        if (sqlca.sqlcode) {
            CheckSqlError("GetEdcParaID() UPDATE P_NAME");
            return FAIL;
        }
    }

    /* If no error occurs (sqlca.sqlcode == 0) */
    return SUCCEED;

}/* End GetEdcParaID() */

/*****************************************************************************
* TITLE     : Insert EDC Parameter ID
* PURPOSE   :
* PARAMETER :
* RETURN    :
* EXAMPLE   :
*****************************************************************************/
int InsertEdcParaID()
{
    EXEC SQL
        INSERT
        INTO    EDC_PARAMETER(
                    EDC_PARAMETER_ID,
                    NAME,
                    P_NAME,
                    ROUTE_STEP_ID)
        VALUES  (   EDC_PARAMETER_ID.NEXTVAL,
                   :grParameter.parameter_name,
                    :grParameter.p_parameter_name:grParameter.flag,
                    :grRouteStep.route_step_id);

    if (sqlca.sqlcode) {
        CheckSqlError("InsertEdcParaID() INSERT EDC_PARAMETER");
        return FAIL;
    }

    EXEC SQL
        SELECT  EDC_PARAMETER_ID
        INTO    :grParameter.parameter_id
        FROM    EDC_PARAMETER
        WHERE   NAME = :grParameter.parameter_name AND
                ROUTE_STEP_ID = :grRouteStep.route_step_id;


    if (sqlca.sqlcode) {
        CheckSqlError("InsertEdcParaID() SELECT EDC_PARAMETER_ID");
        return FAIL;
    }

    /* If no error condition occurs (sqlca.sqlcode == 0) */
    return SUCCEED;

}/* End InsertEdcParaID() */

/*****************************************************************************
* TITLE     : Get RTC Parameter ID
* PURPOSE   :
* PARAMETER :
* RETURN    :
* EXAMPLE   :
*****************************************************************************/
int GetRtcParaID()
{
    EXEC SQL
        SELECT  RTC_PARAMETER_ID
        INTO    :grParameter.parameter_id
        FROM    RTC_PARAMETER
        WHERE   NAME = :grParameter.parameter_name;

    if (sqlca.sqlcode == 100 || sqlca.sqlcode==1403) {
        if (InsertRtcParaID() == FAIL)
            return FAIL;
    }

    else if (sqlca.sqlcode) {
        CheckSqlError("GetRtcParaID() SELECT RTC_PARAMETER_ID");
        return FAIL;
    }

    /* If no error condition occurs (sqlca.sqlcode == 0) */
    return SUCCEED;

}/* End GetRtcParaID() */

/*****************************************************************************
* TITLE     : Insert RTC Parameter ID
* PURPOSE   :
* PARAMETER :
* RETURN    :
* EXAMPLE   :
*****************************************************************************/
int InsertRtcParaID()
{
    EXEC SQL
        INSERT
        INTO    RTC_PARAMETER(
                    RTC_PARAMETER_ID,
                    NAME)
        VALUES  (   RTC_PARAMETER_ID.NEXTVAL,
                   :grParameter.parameter_name);

    if (sqlca.sqlcode) {
        CheckSqlError("InsertRtcParaID() INSERT RTC_PARAMETER");
        return FAIL;
    }

    EXEC SQL
        SELECT  RTC_PARAMETER_ID
        INTO    :grParameter.parameter_id
        FROM    RTC_PARAMETER
        WHERE   NAME = :grParameter.parameter_name;


    if (sqlca.sqlcode) {
        CheckSqlError("InsertRtcParaID() SELECT RTC_PARAMETER_ID");
        return FAIL;
    }

    /* If no error condition occurs (sqlca.sqlcode == 0) */
    return SUCCEED;

}/* End InsertRtcParaID() */

/*****************************************************************************
* TITLE        : Process Technology and Product Mapping
* DESC.        : To process TECH_PROD_MAPPING table.
* AUTHOR       : Erison Liang
* PARAMETER    : None
* SPECIAL LOGIC:
* RETURN       : If no error occurs, then return SUCCEED, else return FAIL.
* MODIFIED LOG : NAME         DATE       REASON
*                ------------ ---------- --------------------------------------
*                Erison Liang 1998/02/05
*****************************************************************************/
int ProcTechProdMap()
{
    EXEC SQL BEGIN DECLARE SECTION;
        int      tech_id; /* used to store technology_id temporary */
    EXEC SQL END DECLARE SECTION;

    /* Get technology_id */
    EXEC SQL
       SELECT  TECHNOLOGY_ID
       INTO    :tech_id
       FROM    TECH_PROD_MAPPING
       WHERE   TECHNOLOGY_ID = :grTechnology.technology_id  AND
             PRODUCT_ID    = :grProduct.product_id;

    /* Check whether exist in TECH_PROD_MAPPING table */
    if (sqlca.sqlcode == 100 || sqlca.sqlcode==1403) {
        /* Insert technology_id and product_id into TECH_PROD_MAPPING table */
        EXEC SQL
           INSERT
           INTO    TECH_PROD_MAPPING(
                   TECHNOLOGY_ID,
                   PRODUCT_ID)
           VALUES  ( :grTechnology.technology_id,
                   :grProduct.product_id);

        /* Check wether occurs an error condition */
        if (sqlca.sqlcode) {
            CheckSqlError("ProcTechProdMap() INSERT TECH_PROD_MAPPING");
            return FAIL;
        }

        /* No error occurs */
        return SUCCEED;
    }


    /* Check whether occurs an error condition */
    else if (sqlca.sqlcode) {
        CheckSqlError("ProcTechProdMap() SELECT technology_id");
        return FAIL;
    }
    /* No error occurs ( sqlca.sqlcode == 0 ) */
    return SUCCEED;
}/* End ProcTechProdMap() */

/*****************************************************************************
* TITLE        : Process Equipment and RTC parameter Mapping
* DESC.        : To process EQUIP_RTC_PARAMETER_MAPPING table.
* AUTHOR       : Erison Liang
* PARAMETER    : None
* RETURN       : If no error occurs, then return SUCCEED, else return FAIL.
*****************************************************************************/
int ProcEquipRtcParaMap()
{
    EXEC SQL BEGIN DECLARE SECTION;
        int          equipment_id;
        VARCHAR     p_tool_id[50];
        VARCHAR     recipe[50];
        VARCHAR     oper_desc[80+1];
    EXEC SQL END DECLARE SECTION;

    EXEC SQL
       SELECT   EQUIPMENT_ID,
             NVL(P_TOOL_ID, 'NULL'),
             NVL(RECIPE, 'NULL'),
             NVL(OPER_DESC, 'NULL')
       INTO  :equipment_id,
             :p_tool_id,
             :recipe,
             :oper_desc
       FROM  EQUIP_RTC_PARAMETER_MAPPING
       WHERE EQUIPMENT_ID = :grMeasEquip.equipment_id AND
             RTC_PARAMETER_ID = :grCurParameter.parameter_id;

    if (sqlca.sqlcode == 100 || sqlca.sqlcode==1403) {
        strcpy((char*)p_tool_id.arr, (char*)grMeasHist.p_tool_id);
        p_tool_id.len = strlen((char*)p_tool_id.arr);

        EXEC SQL
           INSERT
           INTO  EQUIP_RTC_PARAMETER_MAPPING(
                    EQUIPMENT_ID,
                    RTC_PARAMETER_ID,
                    P_TOOL_ID,
                    RECIPE,
                    OPER_DESC)
           VALUES ( :grMeasEquip.equipment_id,
                    :grCurParameter.parameter_id,
                    :p_tool_id:grProcEquip.flag,
                    :grProcRecipe.name:grProcRecipe.flag,
                    :grRoute.desc);

        /* Check wether occurs an error condition */
        if (sqlca.sqlcode) {
            CheckSqlError("ProcEquipRtcParaMap() INSERT EQUIP_RTC_PARAMETER_MAPPING");
            return FAIL;
        }

        /* No error occurs */
        return SUCCEED;
    }


    /* Check whether occurs an error condition */
    else if (sqlca.sqlcode) {
        CheckSqlError("ProcEquipRtcParaMap() SELECT EQUIPMENT_ID");
        return FAIL;
    }


    /* if recipe or p_tool_id is "NULL", then update it */
    if (strncmp((char*)recipe.arr, "NULL", 4) == 0 ||
            strncmp((char*)p_tool_id.arr, "NULL", 4) == 0 ||
            strncmp((char*)oper_desc.arr, "NULL", 4) == 0) {
        strcpy((char*)p_tool_id.arr, (char*)grMeasHist.p_tool_id);
        p_tool_id.len = strlen((char*)p_tool_id.arr);

        EXEC SQL
           UPDATE   EQUIP_RTC_PARAMETER_MAPPING
           SET      P_TOOL_ID = :p_tool_id,
                 RECIPE = :grProcRecipe.name:grProcRecipe.flag,
                 OPER_DESC = :grRoute.desc
           WHERE EQUIPMENT_ID = :grMeasEquip.equipment_id AND
                 RTC_PARAMETER_ID = :grCurParameter.parameter_id;


        if (sqlca.sqlcode) {
            CheckSqlError("ProcEquipRtcParaMap() UPDATE P_TOOL_ID, RECIPE");
            return FAIL;
        }
    }


    /* No error occurs ( sqlca.sqlcode == 0 ) */
    return SUCCEED;

}/* End ProcEquipRtcParaMap() */

/*****************************************************************************
* TITLE     :
* DESC.     :
* PARAMETER :
* RETURN    :
* EXAMPLE   :
*****************************************************************************/
int InsertRouteStepID()
{
    EXEC SQL
        INSERT
        INTO    ROUTE_STEP(
                  ROUTE,
                  STEP,
                  ROUTE_STEP_ID)
        VALUES  ( :grRoute.route,
                  :grStep.step,
                  ROUTE_STEP_ID.NEXTVAL);

    if (sqlca.sqlcode) {
        CheckSqlError("InsertRouteStepID INSERT ROUTE_STEP");
        return FAIL;
    }

    EXEC SQL
        SELECT  ROUTE_STEP_ID
        INTO    :grRouteStep.route_step_id
        FROM    ROUTE_STEP
        WHERE   ROUTE = :grRoute.route AND
                STEP = :grStep.step;

    if (sqlca.sqlcode) {
        CheckSqlError("InsertRouteStepID() SELECT ROUTE_STEP_ID");
        return FAIL;
    }
    return SUCCEED;
}/* end InsertRouteStepID() */

int CheckEdcLotInfo()
{
    EXEC SQL BEGIN DECLARE SECTION;
        VARCHAR  OLD_TIME[26+1];
    EXEC SQL END DECLARE SECTION;

    EXEC SQL
       SELECT to_char(MEASURE_TIME,'YYYY/MM/DD HH24:MI:SS')
       INTO :OLD_TIME
       FROM  EDC_LOT_INFO
       WHERE
          LOT_ID = :grLot.lot_id AND
          ROUTE_STEP_ID = :grRouteStep.route_step_id AND
          LATEST_FLAG='Y';
#ifdef _TEST_
    ShowStep("   -->CheckEdcLotInfo()");
#endif
    if(sqlca.sqlcode == 100 || sqlca.sqlcode==1403)
        return UNFOUND;
    else if(sqlca.sqlcode) {
        CheckSqlError("CheckEdcLotInfo() CHECK OLDER RECORD");
        return FAIL;
    } else {
        OLD_TIME.arr[19]='\0';
        if(strcmp((char*)OLD_TIME.arr,(char*)grHeader.measure_time)>=0)
            return SKIP;
        else {
            EXEC SQL
               UPDATE EDC_LOT_INFO
               SET LATEST_FLAG='N'
               WHERE
                  LOT_ID = :grLot.lot_id AND
                  ROUTE_STEP_ID = :grRouteStep.route_step_id AND
                  LATEST_FLAG='Y';
            if(sqlca.sqlcode) {
                CheckSqlError("CheckEdcLotInfo() UPDATE EDC_LOT_INFO");
                return FAIL;
            } else
                return FOUND;
        }
    }
}

int CheckEdcLotSummary()
{
    EXEC SQL BEGIN DECLARE SECTION;
        VARCHAR  OLD_TIME[26+1];
    EXEC SQL END DECLARE SECTION;

    EXEC SQL
       SELECT to_char(MEASURE_TIME,'YYYY/MM/DD HH24:MI:SS')
       INTO :OLD_TIME
       FROM  EDC_LOT_SUMMARY
       WHERE
             LOT_ID = :grLot.lot_id AND
             EDC_PARAMETER_ID = :grCurParameter.parameter_id AND
             LATEST_FLAG='Y';
#ifdef _TEST_
    ShowStep("   -->CheckEdcLotSummary()");
#endif
    if(sqlca.sqlcode == 100 || sqlca.sqlcode==1403)
        return UNFOUND;
    else if(sqlca.sqlcode) {
        CheckSqlError("CheckEdcLotSummary() CHECK OLDER RECORD");
        return FAIL;
    } else {
        OLD_TIME.arr[19]='\0';
        if(strcmp((char*)OLD_TIME.arr,(char*)grHeader.measure_time)>=0)
            return SKIP;
        else
            EXEC SQL
               UPDATE EDC_LOT_SUMMARY
               SET LATEST_FLAG='N'
               WHERE
                  LOT_ID = :grLot.lot_id AND
                  EDC_PARAMETER_ID = :grCurParameter.parameter_id;
            if(sqlca.sqlcode) {
                CheckSqlError("CheckEdcLotSummary() UPDATE EDC_LOT_SUMMARY");
                return FAIL;
            } else
                return FOUND;
    }
}

int CheckEdcWaferInfo(i)
int i;
{
    EXEC SQL BEGIN DECLARE SECTION;
        VARCHAR     OLD_TIME[26+1];
        VARCHAR     wafer_id_back1[32];
    EXEC SQL END DECLARE SECTION;

    /*  sprintf((char*)grWafer.wafer_no.arr,"%02d",i);
      grWafer.wafer_no.len=strlen((char*)grWafer.wafer_no.arr);  */
    strcpy((char*)wafer_id_back1.arr, grRawData[i].wafer_id_back[1]);
    wafer_id_back1.len = strlen((char*)wafer_id_back1.arr);

    EXEC SQL
        SELECT WAFER_NO
        INTO  :grWafer.wafer_no
        FROM DEFECT_WAFER_NO_MAPPING
        WHERE WAFER_ID_BACK=:wafer_id_back1;
    /*  fprintf(gpLogFp,"test grRawData[%d].wafer_id_back[1]:%s\n",i,grRawData[i].wafer_id_back[1]);*/

    if(sqlca.sqlcode==100 || sqlca.sqlcode==1403) {
        /*   fprintf(gpLogFp,"CheckEdcWaferInfo Waring can't find grRawData[%d].wafer_id_back[%d]:%s\n",i,grRawData[i].wafer_id_back[1]);  */
        sprintf((char*)grWafer.wafer_no.arr,"%02d",i);
        grWafer.wafer_no.len=strlen((char*)grWafer.wafer_no.arr);
    }

    if(sqlca.sqlcode<0) {
        CheckSqlError("CheckEdcWaferInfo() SELECT WAFER_NO");
        return FAIL;
    }

    EXEC SQL
       SELECT to_char(MEASURE_TIME,'YYYY/MM/DD HH24:MI:SS')
       INTO :OLD_TIME
       FROM EDC_WAFER_INFO
           WHERE LOT_ID = :grLot.lot_id AND
              ROUTE_STEP_ID = :grRouteStep.route_step_id AND
              WAFER_NO=:grWafer.wafer_no AND
              LATEST_FLAG='Y';
#ifdef _TEST_
    ShowStep("   -->CheckEdcWaferInfo() CHECK OLD RECORD");
#endif
    if(sqlca.sqlcode==100 || sqlca.sqlcode==1403) {
        return UNFOUND;
    }

    if(sqlca.sqlcode) {
        CheckSqlError("CheckEdcWaferInfo()");
        /* add by hawk at 20010713*/
        EXEC SQL
           DELETE
           FROM EDC_WAFER_INFO
           WHERE LOT_ID = :grLot.lot_id AND
           ROUTE_STEP_ID = :grRouteStep.route_step_id AND
           WAFER_NO=:grWafer.wafer_no AND
           LATEST_FLAG='Y';
        EXEC SQL COMMIT WORK;
        fprintf(gpLogFp,"FOUND MANY ROWS and DELETE DATA return UNFOUND\n");
        return UNFOUND; /* update by hawk at 20010713*/
        /*   return FAIL; */
    }

    OLD_TIME.arr[19]='\0';
    if(strcmp((char*)OLD_TIME.arr,(char*)grHeader.measure_time)>=0) {
        return SKIP;
    } else {
        EXEC SQL
           UPDATE EDC_WAFER_INFO
           SET LATEST_FLAG='N'
           WHERE LOT_ID = :grLot.lot_id AND
              ROUTE_STEP_ID = :grRouteStep.route_step_id AND
              WAFER_NO=:grWafer.wafer_no;
        if(sqlca.sqlcode) {
            CheckSqlError("CheckEdcWaferInfo() UPDATE EDC_WAFER_INFO");
            return FAIL;
        } else
            return FOUND;
    }

}

int CheckEdcWaferSummary(i)
int i;
{
    EXEC SQL BEGIN DECLARE SECTION;
        VARCHAR     OLD_TIME[26+1];
        VARCHAR     wafer_id_back1[32];
    EXEC SQL END DECLARE SECTION;

    /* sprintf((char*)grWafer.wafer_no.arr,"%02d",i);
     grWafer.wafer_no.len=strlen((char*)grWafer.wafer_no.arr); */
    strcpy((char*)wafer_id_back1.arr, grRawData[i].wafer_id_back[1]);
    wafer_id_back1.len = strlen((char*)wafer_id_back1.arr);


    EXEC SQL
     SELECT WAFER_NO
     INTO  :grWafer.wafer_no
     FROM DEFECT_WAFER_NO_MAPPING
     WHERE WAFER_ID_BACK=:wafer_id_back1;
    if(sqlca.sqlcode==100 || sqlca.sqlcode==1403) {
        /*  fprintf(gpLogFp,"CheckEdcWaferSummary Waring can't find grRawData[%d].wafer_id_back[1]:%s\n",i,grRawData[i].wafer_id_back[1]);  */
        sprintf((char*)grWafer.wafer_no.arr,"%02d",i);
        grWafer.wafer_no.len=strlen((char*)grWafer.wafer_no.arr);
    }

    if(sqlca.sqlcode<0) {
        CheckSqlError("CheckEdcWaferSummary() SELECT  WAFER_NO");
        return FAIL;
    }

    EXEC SQL
       SELECT to_char(MEASURE_TIME,'YYYY/MM/DD HH24:MI:SS')
       INTO :OLD_TIME
       FROM EDC_WAFER_SUMMARY
       WHERE LOT_ID = :grLot.lot_id AND
          WAFER_NO=:grWafer.wafer_no AND
          EDC_PARAMETER_ID = :grCurParameter.parameter_id AND
          LATEST_FLAG='Y';
#ifdef _TEST_
    ShowStep("   -->CheckEdcWaferSummary()");
#endif
    if(sqlca.sqlcode==100 || sqlca.sqlcode==1403)
        return UNFOUND;
    else if(sqlca.sqlcode) {
        /*  fprintf(gpLogFp,"grCurParameter.parameter_id:%d wafer_no:%s \n",grCurParameter.parameter_id,grWafer.wafer_no.arr); */
        CheckSqlError("CheckEdcWaferSummary() test");
        return FAIL;
    } else {
        OLD_TIME.arr[19]='\0';
        if(strcmp((char*)OLD_TIME.arr,(char*)grHeader.measure_time)>=0)
            return SKIP;
        else {
            EXEC SQL
               UPDATE EDC_WAFER_SUMMARY
               SET LATEST_FLAG='N'
            WHERE LOT_ID = :grLot.lot_id AND
               WAFER_NO=:grWafer.wafer_no AND
               EDC_PARAMETER_ID = :grCurParameter.parameter_id AND
               LATEST_FLAG='Y';
            if(sqlca.sqlcode==100 || sqlca.sqlcode==1403)
                return FOUND;
            else if(sqlca.sqlcode) {
                CheckSqlError("CheckEdcWaferSummary() UPDATE EDC_WAFER_SUMMARY");
                return FAIL;
            } else
                return FOUND;
        }
    }
}

int CheckRtcSummary()
{
    EXEC SQL BEGIN DECLARE SECTION;
        int         count;
        VARCHAR     OLD_TIME[26+1];
    EXEC SQL END DECLARE SECTION;

    EXEC SQL
       SELECT COUNT(*)
       INTO  :count
       FROM  RTC_SUMMARY
       WHERE EQUIPMENT_ID = :grMeasEquip.equipment_id AND
             LOT_NO = :grLot.lotno AND
             RTC_PARAMETER_ID = :grCurParameter.parameter_id AND
             LATEST_FLAG = 'Y';
#ifdef _TEST_
    ShowStep("   -->CheckRtcSummary()");
#endif
    if(sqlca.sqlcode) {
        CheckSqlError("CheckRtcLotSummary()");
        return FAIL;
    }
    if(count==0)
        return UNFOUND;
    else {
        EXEC SQL
           SELECT to_char(MEASURE_TIME,'YYYY/MM/DD HH24:MI:SS')
           INTO :OLD_TIME
           FROM RTC_SUMMARY
                WHERE EQUIPMENT_ID = :grMeasEquip.equipment_id AND
                   LOT_NO = :grLot.lotno AND
                   RTC_PARAMETER_ID = :grCurParameter.parameter_id AND
                   LATEST_FLAG = 'Y';
        OLD_TIME.arr[19]='\0';
        if(strcmp((char*)OLD_TIME.arr,(char*)grHeader.measure_time)>=0)
            return SKIP;
        else
            return FOUND;
    }
}

int UpdateRtcSummary()
{
    EXEC SQL
       UPDATE  RTC_SUMMARY
       SET LATEST_FLAG = 'N'
       WHERE EQUIPMENT_ID = :grMeasEquip.equipment_id AND
             LOT_NO = :grLot.lotno AND
             RTC_PARAMETER_ID = :grCurParameter.parameter_id AND
             LATEST_FLAG = 'Y';
#ifdef _TEST_
    ShowStep("   -->UpdateRtcSummary()");
#endif
    if(sqlca.sqlcode) {
        CheckSqlError("UpdateRtcSummary() UPDATE RTC_SUMMARY");
        return FAIL;
    }
    return SUCCEED;
}

/*****************************************************************************
* TITLE     : Insert EDC Lot Information
* PURPOSE   :
* PARAMETER :
* RETURN    :
* EXAMPLE   :
*****************************************************************************/
int InsertEdcLotInfo()
{
    EXEC SQL
        INSERT
        INTO    EDC_LOT_INFO(
                    TECHNOLOGY_ID,
                    PRODUCT_ID,
                    LOT_ID,
                    ROUTE_STEP_ID,
                    MEASURE_RECIPE,
                    PROCESS_RECIPE,
                    MEASURE_EQUIP_ID,
                    MEASURE_EQUIP,
                    PROC_EQUIP_ID,
                    PROC_EQUIP,
                    MEASURE_OPER_ID,
                    MEASURE_OPER,
                    MEASURE_TIME,
                    WEEK,
                    MONTH,
                    WAFER_COUNT,
                    MEASURE_LC_RECIPE,
                    MEASURE_PH_RECIPE,
                    PROCESS_LC_RECIPE,
                    PROCESS_PH_RECIPE,
                    FAB,
                    PRCSS_GRP,
                    LATEST_FLAG)
        VALUES  (   :grTechnology.technology_id,
                    :grProduct.product_id,
                    :grLot.lot_id,
                    :grRouteStep.route_step_id,
                    :grMeasRecipe.name:grMeasRecipe.flag,
                    :grProcRecipe.name:grProcRecipe.flag,
                    :grMeasEquip.equipment_id:grMeasEquip.flag,
                    :grMeasEquip.name:grMeasEquip.flag,
                    :grProcEquip.equipment_id:grProcEquip.flag,
                    :grProcEquip.name:grProcEquip.flag,
                    :grOperator.operator_id:grOperator.flag,
                    :grOperator.name:grOperator.flag,
                    to_date(:grHeader.measure_time,'YYYY/MM/DD HH24:MI:SS'),
                    to_char(to_date(:grHeader.measure_time,'YYYY/MM/DD HH24:MI:SS'),'WW'),
                    to_char(to_date(:grHeader.measure_time,'YYYY/MM/DD HH24:MI:SS'),'MM'),
                    :grEdcLotSum.wafer_count,
                    :grMeasRecipe.lc_name:grMeasRecipe.lc_flag,
                    :grMeasRecipe.ph_name:grMeasRecipe.ph_flag,
                    :grProcRecipe.lc_name:grProcRecipe.lc_flag,
                    :grProcRecipe.ph_name:grProcRecipe.ph_flag,
                    :FAB,
                    :grLotHist.prcss_grp,
                    'Y');
#ifdef _TEST_
    ShowStep("   -->InsertEdcLotInfo()");
#endif
    if (sqlca.sqlcode) {
        CheckSqlError("InsertEdcLotInfo() INSERT EDC_LOT_INFO");
        return FAIL;
    }

    return SUCCEED;
}/* End InsertEdcLotInfo() */

/*****************************************************************************
* TITLE     : Insert EDC Lot Summary
* PURPOSE   :
* PARAMETER :
* RETURN    :
* EXAMPLE   :
*****************************************************************************/
int InsertEdcLotSummary()
{
    EXEC SQL BEGIN DECLARE SECTION;
        double lot_avg;
        short  avg_flag;
    EXEC SQL END DECLARE SECTION;

    avg_flag=0;
    lot_avg=grRawData[0].value[0];

    EXEC SQL
        INSERT
        INTO    EDC_LOT_SUMMARY(
                    PRODUCT_ID,
                    LOT_ID,
                    EDC_PARAMETER_ID,
                    AVERAGE,
                    SITE_COUNT,
                    MEASURE_TIME,
                    LATEST_FLAG)
        VALUES   (  :grProduct.product_id:grProduct.flag,
                    :grLot.lot_id,
                    :grCurParameter.parameter_id,
                    :lot_avg:avg_flag,
                    0,
                    to_date(:grHeader.measure_time,'YYYY/MM/DD HH24:MI:SS'),
                    'Y');
#ifdef _TEST_
    ShowStep("   -->InsertEdcLotSummary()");
#endif
    if (sqlca.sqlcode) {
        CheckSqlError("InsertEdcLotSummary() INSERT EDC_LOT_SUMMARY");
        return FAIL;
    }
    return SUCCEED;
}/* End InsertEdcLotSummary() */

/*****************************************************************************
* TITLE     : Insert Edc Wafer Info(by Wafer/Lot)
* PURPOSE   :
* PARAMETER :
* RETURN    :
* EXAMPLE   :
*****************************************************************************/
int InsertEdcWaferInfo(i)
int i;
{
    EXEC SQL BEGIN DECLARE SECTION;
        VARCHAR     wafer_id_back1[32];
    EXEC SQL END DECLARE SECTION;

    /* sprintf((char*)grWafer.wafer_no.arr,"%02d",i);
     grWafer.wafer_no.len=strlen((char*)grWafer.wafer_no.arr); */
    strcpy((char*)wafer_id_back1.arr, grRawData[i].wafer_id_back[1]);
    wafer_id_back1.len = strlen((char*)wafer_id_back1.arr);

    EXEC SQL
    SELECT WAFER_NO
    INTO  :grWafer.wafer_no
    FROM DEFECT_WAFER_NO_MAPPING
    WHERE WAFER_ID_BACK=:wafer_id_back1;
    if(sqlca.sqlcode==100 || sqlca.sqlcode==1403) {
        /*  fprintf(gpLogFp,"InsertEdcWaferInfo Waring can't find grRawData[%d].wafer_id_back[1]:%s\n",i,grRawData[i].wafer_id_back[1]); */
        sprintf((char*)grWafer.wafer_no.arr,"%02d",i);
        grWafer.wafer_no.len=strlen((char*)grWafer.wafer_no.arr);
    }

    if(sqlca.sqlcode<0) {
        CheckSqlError("InsertEdcWaferInfo() SELECT WAFER_NO");
        return FAIL;
    }
    if(GetWaferID()==FAIL)
        return FAIL;

    /* insert the data into EDC_WAFER_INFO table */
    EXEC SQL
        INSERT
        INTO    EDC_WAFER_INFO(
                  LOT_ID,
                  WAFER_NO,
                  WAFER_ID,
                  ROUTE_STEP_ID,
                  MEASURE_RECIPE,
                  PROCESS_RECIPE,
                  MEASURE_EQUIP_ID,
                  MEASURE_EQUIP,
                  PROC_EQUIP_ID,
                  PROC_EQUIP,
                  MEASURE_OPER_ID,
                  MEASURE_OPER,
                  MEASURE_TIME,
                  WEEK,
                  MONTH,
                  MEASURE_LC_RECIPE,
                  MEASURE_PH_RECIPE,
                  PROCESS_LC_RECIPE,
                  PROCESS_PH_RECIPE,
                  PRCSS_GRP,
                  LATEST_FLAG)
        VALUES  ( :grLot.lot_id,
                  :grWafer.wafer_no,
                  :grWafer.wafer_id,
                  :grRouteStep.route_step_id,
                  :grMeasRecipe.name:grMeasRecipe.flag,
                  :grProcRecipe.name:grProcRecipe.flag,
                  :grMeasEquip.equipment_id:grMeasEquip.flag,
                  :grMeasEquip.name:grMeasEquip.flag,
                  :grProcEquip.equipment_id:grProcEquip.flag,
                  :grProcEquip.name:grProcEquip.flag,
                  :grOperator.operator_id:grOperator.flag,
                  :grOperator.name:grOperator.flag,
                  to_date(:grHeader.measure_time,'YYYY/MM/DD HH24:MI:SS'),
                  to_char(to_date(:grHeader.measure_time,'YYYY/MM/DD HH24:MI:SS'),'WW'),
                  to_char(to_date(:grHeader.measure_time,'YYYY/MM/DD HH24:MI:SS'),'MM'),
                  :grMeasRecipe.lc_name:grMeasRecipe.lc_flag,
                  :grMeasRecipe.ph_name:grMeasRecipe.ph_flag,
                  :grProcRecipe.lc_name:grProcRecipe.lc_flag,
                  :grProcRecipe.ph_name:grProcRecipe.ph_flag,
                  :grLotHist.prcss_grp,
                  'Y');
#ifdef _TEST_
    ShowStep("   -->InsertEdcWaferInfo()");
#endif
    if (sqlca.sqlcode) {
        CheckSqlError("InsertEdcWaferInfo() INSERT EDC_WAFER_INFO");
        return FAIL;
    }

    return SUCCEED;
}/* End InsertEdcWaferInfo() */

/*****************************************************************************
* TITLE   : Insert EDC Wafer Summary
* PURPOSE : Insert the summary of wafer where EDC test into ORACLE database.
* Input   :
* Output  :
* EXAMPLE :
* Version :
* Date    :
*****************************************************************************/
int InsertEdcWaferSummary(wfr_num)
int wfr_num;
{
    EXEC SQL BEGIN DECLARE SECTION;
        double wafer_avg;
        short  avg_flag;
        VARCHAR wafer_id_back1[32];
    EXEC SQL END DECLARE SECTION;

    char buff[20];
    /* >>>>> Get Wafer_id*/
    /* sprintf(buff,"%02d",wfr_num);
     strcpy((char*)grWafer.wafer_no.arr,(char*)buff);
     grWafer.wafer_no.len=strlen((char*)grWafer.wafer_no.arr); */
    strcpy((char*)wafer_id_back1.arr, grRawData[wfr_num].wafer_id_back[1]);
    wafer_id_back1.len = strlen((char*)wafer_id_back1.arr);
    EXEC SQL
       SELECT WAFER_NO
       INTO  :grWafer.wafer_no
       FROM DEFECT_WAFER_NO_MAPPING
       WHERE WAFER_ID_BACK=:wafer_id_back1;
    if(sqlca.sqlcode==100 || sqlca.sqlcode==1403) {
        /*  fprintf(gpLogFp,"InsertEdcWaferSummary Waring can't find grRawData[%d].wafer_id_back[1]:%s\n",wfr_num,grRawData[wfr_num].wafer_id_back[1]);  */
        sprintf((char*)grWafer.wafer_no.arr,"%02d",wfr_num);
        grWafer.wafer_no.len=strlen((char*)grWafer.wafer_no.arr);
    }

    if(sqlca.sqlcode<0) {
        CheckSqlError("InsertEdcWaferSummary() Select WAFER_NO");
        return FAIL;
    }
    if(GetWaferID()==FAIL)
        return FAIL;

    avg_flag=0;
    wafer_avg=grRawData[wfr_num].value[0];
    /* Marked by FELIX@2001/02/14 */
    /* grRawData[0].value[0]+=grRawData[wfr_num].value[0];*/
    EXEC SQL
        INSERT
        INTO    EDC_WAFER_SUMMARY(
                PRODUCT_ID,
                LOT_ID,
                WAFER_ID,
                WAFER_NO,
                MEASURE_TIME,
                ROUTE_STEP_ID,
                EDC_PARAMETER_ID,
                AVERAGE,
                SITE_COUNT,
                LATEST_FLAG)
    VALUES  (   :grProduct.product_id:grProduct.flag,
                :grLot.lot_id,
                :grWafer.wafer_id,
                :grWafer.wafer_no,
                TO_DATE(:grHeader.measure_time, 'YYYY/MM/DD HH24:MI:SS'),
                :grRouteStep.route_step_id,
                :grCurParameter.parameter_id,
                :wafer_avg:avg_flag,
                0,'Y');
#ifdef _TEST_
    ShowStep("   -->InsertEdcWaferSummary()");
#endif
#ifdef _CHECK_
    if(wafer_avg==0)
        fprintf(gpLogFp,"   -->InsertEdcWaferSummary() Average=0 : %5d\n",giCurLine);
#endif
    if (sqlca.sqlcode) {
        CheckSqlError("InsertEdcWaferSummary() INSERT EDC_WAFER_SUMMARY");
        return FAIL;
    }
    return SUCCEED;


}/* End InsertEdcWaferSummary() */
/*****************************************************************************
* TITLE     : Insert RTC Summary By Wafer
* PURPOSE   :
* PARAMETER :
* RETURN    :
* EXAMPLE   :
*****************************************************************************/
int InsertRtcSummaryByWafer(wfr_num)
int wfr_num;
{
    EXEC SQL BEGIN DECLARE SECTION;
        double wafer_avg;
        short  avg_flag;
    EXEC SQL END DECLARE SECTION;

    avg_flag=0;
    wafer_avg=grRawData[wfr_num].value[0];

    EXEC SQL
        INSERT
        INTO    RTC_SUMMARY(
                    LOT_NO,
                    EQUIPMENT_ID,
                    RTC_PARAMETER_ID,
                    MEASURE_TIME,
                    SITE_COUNT,
                    WEEK,
                    MONTH,
                    OPERATOR_ID,
                    MEASURE_RECIPE,
                    PROCESS_RECIPE,
                    P_TOOL_ID,
                    AVERAGE,
                    MEASURE_LC_RECIPE,
                    MEASURE_PH_RECIPE,
                    PROCESS_LC_RECIPE,
                    PROCESS_PH_RECIPE,
                    FAB,
                    LATEST_FLAG,
                    PRCSS_GRP)
      VALUES   (    :grLot.lotno,
                    :grMeasEquip.equipment_id,
                    :grCurParameter.parameter_id,
                    TO_DATE(:grHeader.measure_time, 'YYYY/MM/DD HH24:MI:SS'),
                    0,
                    TO_CHAR(TO_DATE(:grHeader.measure_time, 'YYYY/MM/DD HH24:MI:SS'), 'WW'),
                    TO_CHAR(TO_DATE(:grHeader.measure_time, 'YYYY/MM/DD HH24:MI:SS'), 'MM'),
                    :grOperator.operator_id:grOperator.flag,
                    :grMeasRecipe.name:grMeasRecipe.flag,
                    :grProcRecipe.name:grProcRecipe.flag,
                    :grProcEquip.name:grProcEquip.flag,
                    :wafer_avg:avg_flag,
                    :grMeasRecipe.lc_name:grMeasRecipe.lc_flag,
                    :grMeasRecipe.ph_name:grMeasRecipe.ph_flag,
                    :grProcRecipe.lc_name:grProcRecipe.lc_flag,
                    :grProcRecipe.ph_name:grProcRecipe.ph_flag,
                    :FAB,
                    'Y',
                    :grLotHist.prcss_grp);
#ifdef _TEST_
    ShowStep("   -->InsertRtcSummaryByWafer()");
#endif
#ifdef _CHECK_
    if(wafer_avg==0)
        fprintf(gpLogFp,"   -->InsertRtcSummaryByWafer() Average=0 : %5d\n",giCurLine);
#endif
    if(sqlca.sqlcode) {
        CheckSqlError("InsertRtcSummaryByWafer() INSERT RTC_SUMMARY");
        return FAIL;
    }
    return SUCCEED;
}/* End InsertRtcSummary() */

/*****************************************************************************
* TITLE     : Insert RTC Summary
* PURPOSE   :
* PARAMETER :
* RETURN    :
* EXAMPLE   :
*****************************************************************************/
int InsertRtcSummaryBySite()
{
    EXEC SQL
       INSERT
       INTO    RTC_SUMMARY(
                    LOT_NO,
                    EQUIPMENT_ID,
                    RTC_PARAMETER_ID,
                    MEASURE_TIME,
                    WEEK,
                    MONTH,
                    OPERATOR_ID,
                    MEASURE_RECIPE,
                    PROCESS_RECIPE,
                    P_TOOL_ID,
                    MAX_VAL,
                    MIN_VAL,
                    AVERAGE,
                    MEDIAN,
                    STDDEV,
                    PERCENTILE_25,
                    PERCENTILE_75,
                    SITE_COUNT,
                    SITE_ID_LIST,
                    SITE_VAL_LIST,
                    LATEST_FLAG,
                    FAB,
                    PRCSS_GRP)
      VALUES   (    :grLot.lotno,
                    :grMeasEquip.equipment_id,
                    :grCurParameter.parameter_id,
                    TO_DATE(:grHeader.measure_time, 'YYYY/MM/DD HH24:MI:SS'),
                    TO_CHAR(TO_DATE(:grHeader.measure_time, 'YYYY/MM/DD HH24:MI:SS'), 'WW'),
                    TO_CHAR(TO_DATE(:grHeader.measure_time, 'YYYY/MM/DD HH24:MI:SS'), 'MM'),
                    :grOperator.operator_id:grOperator.flag,
                    :grMeasRecipe.name:grMeasRecipe.flag,
                    :grProcRecipe.name:grProcRecipe.flag,
                    :grProcEquip.name:grProcEquip.flag,
                    :grSum.max_val:grSum.flag,
                    :grSum.min_val:grSum.flag,
                    :grSum.average:grSum.avg_flag,
                    :grSum.median:grSum.flag,
                    :grSum.stddev:grSum.flag,
                    :grSum.percentile_25:grSum.flag,
                    :grSum.percentile_75:grSum.flag,
                    :grSum.site_count,
                    :grSum.site_id_list,
                    :grSum.site_val_list,
                    'Y',:FAB,
                    :grLotHist.prcss_grp);
#ifdef _TEST_
    ShowStep("   -->InsertRtcSummaryBySite()");
#endif
#ifdef _CHECK_
    if(grSum.average==0)
        fprintf(gpLogFp,"   -->InsertRtcSummaryBySite() Average=0 : %5d\n",giCurLine);
#endif
    if (sqlca.sqlcode) {
        CheckSqlError("InsertRtcSummaryBySite() INSERT RTC_SUMMARY");
        fprintf(gpLogFp,"\n-->%d , %d ",grSum.site_id_list.len, grSum.site_val_list.len);
        return FAIL;
    }
    return SUCCEED;
}/* End InsertRtcSummary() */

/*****************************************************************************
* TITLE     : Process EDC/RTC Summary By Site
* PURPOSE   :
* PARAMETER :
* RETURN    :
* EXAMPLE   :
*****************************************************************************/
int ProcessSummaryBySite(wfr_num)
int wfr_num;
{
    EXEC SQL BEGIN DECLARE SECTION;
        VARCHAR wafer_id_back1[32];
    EXEC SQL END DECLARE SECTION;

    int        i,j,site_cnt,site_cnt1;     /* quantity of site which its value is not out-of-valid */
    char       buff[20],temp[2][MAX_LIST_LEN];
    double     work_arr[MAX_SITE];

    temp[0][0]='\0';
    temp[1][0]='\0';
    for(i=1,site_cnt=0,site_cnt1=0; i<MAX_SITE; i++) {
        if(grRawData[wfr_num].site_flag[i]==1) { /* used and not out of spec*/
            grRawData[wfr_num].value[0]+=grRawData[wfr_num].value[i];
            work_arr[site_cnt1++]=grRawData[wfr_num].value[i];
        }
        if(grRawData[wfr_num].site_flag[i]!=-1)
            site_cnt++;
    }

    for(i=1,j=0; j<site_cnt; i++) {
        sprintf(buff,"%15d",i);
        strcat(temp[0],buff);
        if(grRawData[wfr_num].site_flag[i]!=-1) {
            sprintf(buff,"%15f",grRawData[wfr_num].value[i]);
            j++;
        } else
            sprintf(buff,"%15s"," ");
        strcat(temp[1],buff);
    }

    if(site_cnt1==0)
        grRawData[wfr_num].value[0]=0;
    else
        grRawData[wfr_num].value[0]/=(double)site_cnt1;

    /* >>>>> Get Wafer_id*/
    /* sprintf(buff,"%02d",wfr_num);
     strcpy((char*)grWafer.wafer_no.arr,(char*)buff);
     grWafer.wafer_no.len=strlen((char*)grWafer.wafer_no.arr); */
    strcpy((char*)wafer_id_back1.arr, grRawData[wfr_num].wafer_id_back[1]);
    wafer_id_back1.len = strlen((char*)wafer_id_back1.arr);
    EXEC SQL
       SELECT WAFER_NO
       INTO  :grWafer.wafer_no
       FROM DEFECT_WAFER_NO_MAPPING
       WHERE WAFER_ID_BACK=:wafer_id_back1;
    if(sqlca.sqlcode==100 || sqlca.sqlcode==1403) {
        /*  fprintf(gpLogFp,"ProcessSummaryBySite Waring can't find grRawData[%d].wafer_id_back[1]:%s\n",wfr_num,grRawData[wfr_num].wafer_id_back[1]);  */
        sprintf(buff,"%02d",wfr_num);
        strcpy((char*)grWafer.wafer_no.arr,(char*)buff);
        grWafer.wafer_no.len=strlen((char*)grWafer.wafer_no.arr);
        /*  fprintf(gpLogFp,"ProcessSummaryBySite grWafer.wafer_no:%s \n",grWafer.wafer_no.arr);*/
    }

    if(sqlca.sqlcode<0) {
        CheckSqlError("ProcessSummaryBySite() Select WAFER_NO");
        return FAIL;
    }
    if(GetWaferID()==FAIL)
        return FAIL;

    if(!site_cnt) {
        grSum.flag = -1;
        grSum.avg_flag = -1;
    } else if(site_cnt == 1) {
        grSum.flag    = 0;
        grSum.avg_flag = 0;
        grSum.max_val = (double)work_arr[0];
        grSum.min_val = (double)work_arr[0];
        grSum.average = (double)work_arr[0];
        grSum.stddev  = 0.0f;
        grSum.median  = (double)work_arr[0];
        grSum.percentile_25 = 0.0f;
        grSum.percentile_75 = 0.0f;
    } else {
        grSum.flag = 0;
        grSum.avg_flag = 0;
        Sort(work_arr, 0, site_cnt1-1);

        grSum.max_val = Find_max(work_arr, site_cnt1);
        grSum.min_val = Find_min(work_arr);
        grSum.average = grRawData[wfr_num].value[0];
        grSum.stddev  = Find_stddev(work_arr, site_cnt1, grSum.average);
        grSum.median  = Find_medium(work_arr, site_cnt1);
        grSum.percentile_25 = Find_q1(work_arr, site_cnt1);
        grSum.percentile_75 = Find_q3(work_arr, site_cnt1);
    }/* end else section */
    grSum.site_count=site_cnt;
    strcpy((char*)grSum.site_id_list.arr,(char*)temp[0]);
    grSum.site_id_list.len=strlen(temp[0]);
    strcpy((char*)grSum.site_val_list.arr,(char*)temp[1]);
    grSum.site_val_list.len=strlen(temp[1]);
    return SUCCEED;
} /* End ProcessSummaryBySite() */

/*****************************************************************************
* TITLE     : Insert EDC Wafer Summary By Site
* PURPOSE   :
* PARAMETER :
* RETURN    :
* EXAMPLE   :
*****************************************************************************/
int InsertEdcWaferSummaryBySite()
{
    EXEC SQL
       INSERT
       INTO    EDC_WAFER_SUMMARY(
                    PRODUCT_ID,
                    LOT_ID,
                    WAFER_ID,
                    WAFER_NO,
                    ROUTE_STEP_ID,
                    EDC_PARAMETER_ID,
                    SITE_COUNT,
                    MEASURE_TIME,
                    MAX_VAL,
                    MIN_VAL,
                    AVERAGE,
                    STD_DEV,
                    SITE_ID_LIST,
                    SITE_VAL_LIST,
                    LATEST_FLAG)
       VALUES  (   :grProduct.product_id:grProduct.flag,
                   :grLot.lot_id,
                   :grWafer.wafer_id,
                   :grWafer.wafer_no,
                   :grRouteStep.route_step_id,
                   :grCurParameter.parameter_id,
                   :grSum.site_count,
                   TO_DATE(:grHeader.measure_time, 'YYYY/MM/DD HH24:MI:SS'),
                   :grSum.max_val:grSum.flag,
                   :grSum.min_val:grSum.flag,
                   :grSum.average:grSum.avg_flag,
                   :grSum.stddev:grSum.flag,
                   :grSum.site_id_list,
                   :grSum.site_val_list,
                   'Y');
#ifdef _TEST_
    ShowStep("   -->InsertEdcWaferSummaryBySite()");
#endif
#ifdef _CHECK_
    if(grSum.average==0)
        fprintf(gpLogFp,"   -->InsertEdcWaferSummaryBySite() Average=0 : %5d\n",giCurLine);
#endif
    if (sqlca.sqlcode) {
        CheckSqlError("InsertEdcWaferSummaryBySite() INSERT EDC_WAFER_SUMMARY");
        return FAIL;
    }
    return SUCCEED;
} /* End InsertEdcWaferSummaryBySite() */

/*****************************************************************************
* TITLE     : Process EDC Lot Avg By Site
* PURPOSE   :
* PARAMETER :
* RETURN    :
* EXAMPLE   :
*****************************************************************************/
void ProcessLotAvgBySite()
{
    int c,i,j,k,sub_site_cnt[MAX_SITE];
    char       buff[20],temp[2][MAX_LIST_LEN];
    double     work_arr[MAX_SITE];

    temp[0][0]='\0';
    temp[1][0]='\0';
    for(j=0; j<MAX_SITE; j++)
        sub_site_cnt[j]=0;
    for(i=1,k=0,c=0; i<MAX_WAFER; i++) {
        for(j=1; j<MAX_SITE; j++) {
            if(grRawData[i].site_flag[j]==1) {
                grRawData[0].site_flag[j]=1;
                grRawData[0].value[j]+=grRawData[i].value[j];
                work_arr[c]=grRawData[i].value[j];
                c++;
                grRawData[0].value[0]+=grRawData[i].value[j];
                sub_site_cnt[j]++;
            }
            if(grRawData[i].site_flag[j]!=-1)
                k++;
        }
    }

    for(j=i; j<MAX_SITE; j++)
        sub_site_cnt[0]+=sub_site_cnt[j];

    grRawData[0].value[0]/=(double)sub_site_cnt[0];
    grRawData[0].site_flag[0]=1;
    for(i=1; i<MAX_SITE; i++)
        if(grRawData[0].site_flag[i]==1) {
            grRawData[0].value[i]/=(double)sub_site_cnt[i];
            sprintf(buff,"%15d",i);
            strcat(temp[0],buff);
            sprintf(buff,"%15f",grRawData[0].value[i]);
            strcat(temp[1],buff);
        }

    if(!sub_site_cnt[0]) {
        grSum.flag = -1;
        grSum.avg_flag = -1;
    } else if(sub_site_cnt[0] == 1) {
        grSum.flag    = 0;
        grSum.avg_flag = 0;
        grSum.max_val = (double)work_arr[0];
        grSum.min_val = (double)work_arr[0];
        grSum.average = (double)work_arr[0];
        grSum.stddev  = 0.0f;
        grSum.median  = (double)work_arr[0];
        grSum.percentile_25 = 0.0f;
        grSum.percentile_75 = 0.0f;
    } else {
        grSum.flag = 0;
        grSum.avg_flag = 0;
        Sort(work_arr, 0, c-1);

        grSum.max_val = Find_max(work_arr, c);
        grSum.min_val = Find_min(work_arr);
        grSum.average = grRawData[0].value[0];
        grSum.stddev  = Find_stddev(work_arr, c, grSum.average);
        grSum.median  = Find_medium(work_arr, c);
        grSum.percentile_25 = Find_q1(work_arr, c);
        grSum.percentile_75 = Find_q3(work_arr, c);
    }/* end else section */
    grSum.site_count=k;
    strcpy((char*)grSum.site_id_list.arr,(char*)temp[0]);
    grSum.site_id_list.len=strlen(temp[0]);
    strcpy((char*)grSum.site_val_list.arr,(char*)temp[1]);
    grSum.site_val_list.len=strlen(temp[1]);
} /* End ProcessLotAvgBySite() */

/*****************************************************************************
* TITLE     : Insert EDC Lot Summary By Site
* PURPOSE   :
* PARAMETER :
* RETURN    :
* EXAMPLE   :
*****************************************************************************/
int InsertEdcLotSummaryBySite()
{
    EXEC SQL
       INSERT
       INTO    EDC_LOT_SUMMARY(
                   PRODUCT_ID,
                   LOT_ID,
                   EDC_PARAMETER_ID,
                   SITE_COUNT,
                   AVERAGE,
                   MAX_VAL,
                   MEDIAN,
                   MIN_VAL,
                   STD_DEV,
                   PERCENTILE_25,
                   PERCENTILE_75,
                   LATEST_FLAG,
                   MEASURE_TIME,
                   SITE_ID_LIST,
                   SITE_AVG_LIST)
       VALUES  (   :grProduct.product_id:grProduct.flag,
                   :grLot.lot_id,
                   :grCurParameter.parameter_id,
                   :grSum.site_count,
                   :grSum.average:grSum.avg_flag,
                   :grSum.max_val:grSum.flag,
                   :grSum.median:grSum.flag,
                   :grSum.min_val:grSum.flag,
                   :grSum.stddev:grSum.flag,
                   :grSum.percentile_25:grSum.flag,
                   :grSum.percentile_75:grSum.flag,
                   'Y',
                   to_date(:grHeader.measure_time,'YYYY/MM/DD HH24:MI:SS'),
                   :grSum.site_id_list,
                   :grSum.site_val_list);
#ifdef _TEST_
    ShowStep("   -->InsertEdcLotSummaryBySite()");
#endif
    if (sqlca.sqlcode) {
        CheckSqlError("InsertEdcLotSummaryBySite() INSERT EDC_LOT_SUMMARY");
        return FAIL;
    }
    return SUCCEED;
} /* End InsertEdcLotSummaryBySite() */

/*****************************************************************************
* TITLE     : Check EDC SPEC value
* PURPOSE   : if test value out of valid(hi/lo),let this site to untest
* PARAMETER :
* RETURN    :
* EXAMPLE   :
*****************************************************************************/
void CheckEdcSpecValue(wfr_num)
int wfr_num;
{
    int i;
    EXEC SQL BEGIN DECLARE SECTION;
        double valid_hi;
        double  valid_lo;
    EXEC SQL END DECLARE SECTION;

    EXEC SQL
       SELECT VALID_HIGH,
              VALID_LOW
       INTO :valid_hi,:valid_lo
       FROM EDC_SPEC
       WHERE EDC_PARAMETER_ID = :grCurParameter.parameter_id;

    if(sqlca.sqlcode!=100 && sqlca.sqlcode!=1403)    /* if found valid high/low*/
        for(i=1; i<MAX_SITE; i++)
            if(grRawData[wfr_num].site_flag[i]!=-1)
                if(grRawData[wfr_num].value[i]>valid_hi || grRawData[wfr_num].value[i]<valid_lo)
                    grRawData[wfr_num].site_flag[i]=0;

} /* End CheckEdcSpecValue() */

/*****************************************************************************
* TITLE     : Check RTC SPEC Value
* PURPOSE   :
* PARAMETER :
* RETURN    :
* EXAMPLE   :
*****************************************************************************/
void CheckRtcSpecValue(wfr_num)
int wfr_num;
{
    int i;
    EXEC SQL BEGIN DECLARE SECTION;
        double valid_hi;
        double  valid_lo;
    EXEC SQL END DECLARE SECTION;

    EXEC SQL
       SELECT VALID_HIGH,
              VALID_LOW
       INTO :valid_hi,:valid_lo
       FROM RTC_SPEC
       WHERE RTC_PARAMETER_ID = :grCurParameter.parameter_id;


    if(sqlca.sqlcode!=100 && sqlca.sqlcode!=1403)  /* if found valid high/low*/
        for(i=1; i<MAX_SITE; i++)
            if(grRawData[wfr_num].site_flag[i]!=-1)
                if(grRawData[wfr_num].value[i]>valid_hi || grRawData[wfr_num].value[i]<valid_lo)
                    grRawData[wfr_num].site_flag[i]=0;

} /* End CheckRtcSpecValue() */

int CheckExistRawData()
{
    int i,j;
    for(i=0; i<MAX_WAFER; i++)
        for(j=0; j<MAX_SITE; j++)
            if(grRawData[i].site_flag[j]!=-1)
                return FOUND;
#ifdef _TEST_
    ShowStep("==>No Data ");
#endif
    return UNFOUND;
}/*CheckExistRawData() */
/*===========================================================*/
/*===============Math Library================================*/
/*===========================================================*/

/*****************************************************************************
* TITLE     :
* DESC.     :
* PARAMETER :
* RETURN    :
* EXAMPLE   :
*****************************************************************************/
void Exchange(p,q)
double *p,*q;
{
    double  temp;
    temp = *p;
    *p = *q;
    *q = temp;
}

/*****************************************************************************
* TITLE     : Exchange Value
* DESC.     : Quick Sort
* PARAMETER :
* RETURN    :
* EXAMPLE   :
*****************************************************************************/
void Sort(double *sorted_arr, int m, int n)
{
    void  Exchange();

    int   i, j;
    double key;

    key = sorted_arr[m];
    i = j = 0;

    if (m < n) {
        i = m;
        j = n + 1;
        key = sorted_arr[m];
        do {
            for(i++; sorted_arr[i]<key && i<n; i++);
            for(j--; sorted_arr[j] > key && j > m; j--);
            if (i < j)
                Exchange(&sorted_arr[i],&sorted_arr[j]);
        } while (i < j);
        Exchange(&sorted_arr[m], &sorted_arr[j]);
        Sort(sorted_arr, m, j-1);
        Sort(sorted_arr, j+1, n);
    }
}

/*****************************************************************************
* TITLE     : Find Maximun
* DESC.     :
* PARAMETER :
* RETURN    :
* EXAMPLE   :
*****************************************************************************/
double Find_max(double *sorted_arr, int total_num)
{
    return(sorted_arr[total_num-1]);
}/* End Find_max() */

/*****************************************************************************
* TITLE     : Find Minimun
* DESC.     :
* PARAMETER :
* RETURN    :
* EXAMPLE   :
*****************************************************************************/
double Find_min(double *sorted_arr)
{
    return(sorted_arr[0]);
}/* End Find_min() */

/*****************************************************************************
* TITLE     :
* DESC.     :
* PARAMETER :
* RETURN    :
* EXAMPLE   :
*****************************************************************************/
double Find_average(sorted_arr, total_num)
double *sorted_arr;
int    total_num;
{
    double sum = 0.0f;
    int    i ;

    if (total_num == 1)
        return (sorted_arr[0]);
    for(i = 0; i < total_num; i++) {
        sum = sum + sorted_arr[i];
    }
    return (sum / (double)total_num);
}

/*****************************************************************************
* TITLE     :
* DESC.     :
* PARAMETER :
* RETURN    :
* EXAMPLE   :
*****************************************************************************/
double Find_q1(sorted_arr,total_num)
double *sorted_arr;
int   total_num;
{
    double  o_q1 = 0.0f;
    int     integer = 0;
    double  factor = 0.0f;

    if (total_num == 1)
        return (sorted_arr[0]);
    o_q1 = ((double)total_num /(double)4.0) +(double) 0.5;
    integer = (int)floor(o_q1);
    factor = o_q1-(double)integer;
    return ((double)sorted_arr[integer-1]+factor*((double)sorted_arr[integer]-(double)sorted_arr[integer-1]));
}

/*****************************************************************************
* TITLE     :
* DESC.     :
* PARAMETER :
* RETURN    :
* EXAMPLE   :
*****************************************************************************/
double Find_q3(sorted_arr,total_num)
double *sorted_arr;
int    total_num;
{
    double  o_q3=0.0f;
    int     integer;
    double  factor;

    if (total_num==1)
        return (sorted_arr[0]);
    o_q3 = (double)total_num * (double)0.75 +(double) 0.5;
    integer =(int) floor(o_q3);
    factor = o_q3-(double)integer;
    return (sorted_arr[integer-1]+factor*(sorted_arr[integer]-sorted_arr[integer-1]));
}

/*****************************************************************************
* TITLE     :
* DESC.     :
* PARAMETER :
* RETURN    :
* EXAMPLE   :
*****************************************************************************/
double  Find_medium(sorted_arr, total_num)
double  *sorted_arr;
int     total_num;
{
    int middle;

    /* if total number of the array is odd */
    if (total_num % 2 == 1) {
        middle = (total_num + 1) / 2;
        return(sorted_arr[middle - 1]);
    } else /* the total number of the array is even */
        return (sorted_arr[(total_num / 2) - 1] + sorted_arr[(total_num / 2)]) / 2;
}

/*****************************************************************************
* TITLE     :
* DESC.     :
* PARAMETER :
* RETURN    :
* EXAMPLE   :
*****************************************************************************/
double Find_stddev(sorted_arr, total_num, ave)
double *sorted_arr;
int    total_num;
double ave;
{
    double sqr_sum = 0.0f;
    double sum;
    int    i;

    if (total_num == 1)
        return (double)0;

    for (i = 0; i < total_num; i++)
        sqr_sum += (double)sqr((float)sorted_arr[i] - (float)ave);

    sum = (double)((sqr_sum) / (total_num - 1));

    return (double)sqrt(sum);
}

/****************************************************************
****** Belowe only for 9 site************************************
****************************************************************/

/*****************************************************************************
* TITLE     : Process EDC/RTC Summary By 9 Site
* PURPOSE   :
* PARAMETER :
* RETURN    :
* EXAMPLE   :
*****************************************************************************/
int ProcessSummaryBy9Site(wfr_num)
int wfr_num;
{
    EXEC SQL BEGIN DECLARE SECTION;
        VARCHAR wafer_id_back1[32];
    EXEC SQL END DECLARE SECTION;

    int        i,j,site_cnt,site_cnt1;     /* quantity of site which its value is not out-of-valid */
    char       temp[2][MAX_LIST_LEN];
    double     work_arr[MAX_SITE];
    char       buff[30];
    /* Get Wafer_id */
    /*sprintf(buff,"%02d",wfr_num);
    strcpy((char*)grWafer.wafer_no.arr,(char*)buff);
    grWafer.wafer_no.len=strlen((char*)grWafer.wafer_no.arr); */
    strcpy((char*)wafer_id_back1.arr, grRawData[wfr_num].wafer_id_back[1]);
    wafer_id_back1.len = strlen((char*)wafer_id_back1.arr);
    EXEC SQL
     SELECT WAFER_NO
     INTO  :grWafer.wafer_no
     FROM DEFECT_WAFER_NO_MAPPING
     WHERE WAFER_ID_BACK=:wafer_id_back1;
    if(sqlca.sqlcode==100 || sqlca.sqlcode==1403) {
        /*  fprintf(gpLogFp,"ProcessSummaryBy9Site Waring can't find grRawData[%d].wafer_id_back[1]:%s\n",wfr_num,grRawData[wfr_num].wafer_id_back[1]);  */
        sprintf(buff,"%02d",wfr_num);
        strcpy((char*)grWafer.wafer_no.arr,(char*)buff);
        grWafer.wafer_no.len=strlen((char*)grWafer.wafer_no.arr);
        /*  fprintf(gpLogFp,"ProcessSummaryBy9Site grWafer.wafer_no.arr:%s\n",grWafer.wafer_no.arr); */
    }
    if(sqlca.sqlcode < 0 ) {
        fprintf(gpLogFp,"sqlca.sqlcode:%d\n",sqlca.sqlcode);
        CheckSqlError("ProcessSummaryBy9Site() SELECT WAFER_NO");
        return FAIL;
    }
    if(GetWaferID()==FAIL)
        return FAIL;

    temp[0][0]='\0';
    temp[1][0]='\0';
    grSum.site1_flag=-1;
    grSum.site2_flag=-1;
    grSum.site3_flag=-1;
    grSum.site4_flag=-1;
    grSum.site5_flag=-1;
    grSum.site6_flag=-1;
    grSum.site7_flag=-1;
    grSum.site8_flag=-1;
    grSum.site9_flag=-1;

    for(i=1,site_cnt=0,site_cnt1=0; i<MAX_SITE; i++) {
        if(grRawData[wfr_num].site_flag[i]==1) { /* used and not out of spec*/
            grRawData[wfr_num].value[0]+=grRawData[wfr_num].value[i];
            work_arr[site_cnt1++]=grRawData[wfr_num].value[i];
        }
        if(grRawData[wfr_num].site_flag[i]!=-1) {
            switch(i) {
                case 1:
                    grSum.site1_flag=0;
                    break;
                case 2:
                    grSum.site2_flag=0;
                    break;
                case 3:
                    grSum.site3_flag=0;
                    break;
                case 4:
                    grSum.site4_flag=0;
                    break;
                case 5:
                    grSum.site5_flag=0;
                    break;
                case 6:
                    grSum.site6_flag=0;
                    break;
                case 7:
                    grSum.site7_flag=0;
                    break;
                case 8:
                    grSum.site8_flag=0;
                    break;
                case 9:
                    grSum.site9_flag=0;
                    break;
            }
            site_cnt++;
        }
    }

    for(i=1,j=0; j<site_cnt; i++) {
        sprintf(buff,"%15d",i);
        strcat(temp[0],buff);
        if(grRawData[wfr_num].site_flag[i]!=-1) {
            sprintf(buff,"%15f",grRawData[wfr_num].value[i]);
            j++;
        } else
            sprintf(buff,"%15s"," ");
        strcat(temp[1],buff);
    }

    if(site_cnt1==0)
        grRawData[wfr_num].value[0]=0;
    else
        grRawData[wfr_num].value[0]/=(double)site_cnt1;

    grSum.site1_val = (grRawData[wfr_num].site_flag[1]!=-1) ? grRawData[wfr_num].value[1]: 0.0f;
    grSum.site2_val = (grRawData[wfr_num].site_flag[2]!=-1) ? grRawData[wfr_num].value[2]: 0.0f;
    grSum.site3_val = (grRawData[wfr_num].site_flag[3]!=-1) ? grRawData[wfr_num].value[3]: 0.0f;
    grSum.site4_val = (grRawData[wfr_num].site_flag[4]!=-1) ? grRawData[wfr_num].value[4]: 0.0f;
    grSum.site5_val = (grRawData[wfr_num].site_flag[5]!=-1) ? grRawData[wfr_num].value[5]: 0.0f;
    grSum.site6_val = (grRawData[wfr_num].site_flag[6]!=-1) ? grRawData[wfr_num].value[6]: 0.0f;
    grSum.site7_val = (grRawData[wfr_num].site_flag[7]!=-1) ? grRawData[wfr_num].value[7]: 0.0f;
    grSum.site8_val = (grRawData[wfr_num].site_flag[8]!=-1) ? grRawData[wfr_num].value[8]: 0.0f;
    grSum.site9_val = (grRawData[wfr_num].site_flag[9]!=-1) ? grRawData[wfr_num].value[9]: 0.0f;

    if(!site_cnt1) {
        grSum.flag = -1;
        grSum.avg_flag = -1;
    } else if(site_cnt1 == 1) {
        grSum.flag    = 0;
        grSum.avg_flag = 0;
        grSum.max_val = (double)work_arr[0];
        grSum.min_val = (double)work_arr[0];
        grSum.average = (double)work_arr[0];
        grSum.stddev  = 0.0f;
        grSum.median  = (double)work_arr[0];
        grSum.percentile_25 = 0.0f;
        grSum.percentile_75 = 0.0f;
    } else {
        grSum.flag = 0;
        grSum.avg_flag = 0;
        Sort(work_arr, 0, site_cnt1-1);

        grSum.max_val = Find_max(work_arr, site_cnt1);
        grSum.min_val = Find_min(work_arr);
        grSum.average = grRawData[wfr_num].value[0];
        grSum.stddev  = Find_stddev(work_arr, site_cnt1, grSum.average);
        grSum.median  = Find_medium(work_arr, site_cnt1);
        grSum.percentile_25 = Find_q1(work_arr, site_cnt1);
        grSum.percentile_75 = Find_q3(work_arr, site_cnt1);
    }/* end else section */
    grSum.site_count=site_cnt;
    strcpy((char*)grSum.site_id_list.arr,(char*)temp[0]);
    grSum.site_id_list.len=strlen(temp[0]);
    strcpy((char*)grSum.site_val_list.arr,(char*)temp[1]);
    grSum.site_val_list.len=strlen(temp[1]);
    return SUCCEED;
} /* End ProcessSummaryBy9Site() */

/*****************************************************************************
* TITLE     : Process EDC Lot Avg By 9 Site
* PURPOSE   :
* PARAMETER :
* RETURN    :
* EXAMPLE   :
*****************************************************************************/
void ProcessLotAvgBy9Site()
{
    int c,i,j,k,sub_site_cnt[MAX_SITE];
    char       temp[2][MAX_LIST_LEN],buff[100];
    double     work_arr[MAX_SITE*MAX_WAFER];

    temp[0][0]='\0';
    temp[1][0]='\0';
    grSum.site1_flag=-1;
    grSum.site2_flag=-1;
    grSum.site3_flag=-1;
    grSum.site4_flag=-1;
    grSum.site5_flag=-1;
    grSum.site6_flag=-1;
    grSum.site7_flag=-1;
    grSum.site8_flag=-1;
    grSum.site9_flag=-1;
    for(j=0; j<MAX_SITE; j++)
        sub_site_cnt[j]=0;
    for(i=1,k=0,c=0; i<MAX_WAFER; i++) {
        for(j=1; j<MAX_SITE; j++) {
            if(grRawData[i].site_flag[j]==1) {
                grRawData[0].site_flag[j]=1;
                grRawData[0].value[j]+=grRawData[i].value[j];
                work_arr[c++]=grRawData[i].value[j];
                grRawData[0].value[0]+=grRawData[i].value[j];
                sub_site_cnt[j]++;
            }
            if(grRawData[i].site_flag[j]!=-1) {
                switch(j) {
                    case 1:
                        grSum.site1_flag=0;
                        break;
                    case 2:
                        grSum.site2_flag=0;
                        break;
                    case 3:
                        grSum.site3_flag=0;
                        break;
                    case 4:
                        grSum.site4_flag=0;
                        break;
                    case 5:
                        grSum.site5_flag=0;
                        break;
                    case 6:
                        grSum.site6_flag=0;
                        break;
                    case 7:
                        grSum.site7_flag=0;
                        break;
                    case 8:
                        grSum.site8_flag=0;
                        break;
                    case 9:
                        grSum.site9_flag=0;
                        break;
                }
                k++;
            }
        }
    }

    for(j=1; j<MAX_SITE; j++)
        sub_site_cnt[0]+=sub_site_cnt[j];

    grRawData[0].value[0]/=(double)sub_site_cnt[0];
    grRawData[0].site_flag[0]=1;
    for(i=1; i<MAX_SITE; i++)
        if(grRawData[0].site_flag[i]!=-1) {
            grRawData[0].value[i]/=(double)sub_site_cnt[i];
            sprintf(buff,"%15d",i);
            strcat(temp[0],buff);
            sprintf(buff,"%15f",grRawData[0].value[i]);
            strcat(temp[1],buff);
        }

    grSum.site1_val = (grRawData[0].site_flag[1]!=-1) ? grRawData[0].value[1] : 0.0f;
    grSum.site2_val = (grRawData[0].site_flag[2]!=-1) ? grRawData[0].value[2] : 0.0f;
    grSum.site3_val = (grRawData[0].site_flag[3]!=-1) ? grRawData[0].value[3] : 0.0f;
    grSum.site4_val = (grRawData[0].site_flag[4]!=-1) ? grRawData[0].value[4] : 0.0f;
    grSum.site5_val = (grRawData[0].site_flag[5]!=-1) ? grRawData[0].value[5] : 0.0f;
    grSum.site6_val = (grRawData[0].site_flag[6]!=-1) ? grRawData[0].value[6] : 0.0f;
    grSum.site7_val = (grRawData[0].site_flag[7]!=-1) ? grRawData[0].value[7] : 0.0f;
    grSum.site8_val = (grRawData[0].site_flag[8]!=-1) ? grRawData[0].value[8] : 0.0f;
    grSum.site9_val = (grRawData[0].site_flag[9]!=-1) ? grRawData[0].value[9] : 0.0f;

    if(!sub_site_cnt[0]) {
        grSum.flag = -1;
        grSum.avg_flag = -1;
    } else if(sub_site_cnt[0] == 1) {
        grSum.flag    = 0;
        grSum.avg_flag = 0;
        grSum.max_val = (double)work_arr[0];
        grSum.min_val = (double)work_arr[0];
        grSum.average = (double)work_arr[0];
        grSum.stddev  = 0.0f;
        grSum.median  = (double)work_arr[0];
        grSum.percentile_25 = 0.0f;
        grSum.percentile_75 = 0.0f;
    } else {
        grSum.flag = 0;
        grSum.avg_flag = 0;
        Sort(work_arr, 0, c-1);

        grSum.max_val = Find_max(work_arr, c);
        grSum.min_val = Find_min(work_arr);
        grSum.average = grRawData[0].value[0];
        grSum.stddev  = Find_stddev(work_arr, c, grSum.average);
        grSum.median  = Find_medium(work_arr, c);
        grSum.percentile_25 = Find_q1(work_arr, c);
        grSum.percentile_75 = Find_q3(work_arr, c);
    }/* end else section */
    grSum.site_count=k;
    strcpy((char*)grSum.site_id_list.arr,(char*)temp[0]);
    grSum.site_id_list.len=strlen(temp[0]);
    strcpy((char*)grSum.site_val_list.arr,(char*)temp[1]);
    grSum.site_val_list.len=strlen(temp[1]);
} /* End ProcessLotAvgBy9Site() */

/*****************************************************************************
* TITLE     : Insert EDC Lot Summary By 9 Site
* PURPOSE   :
* PARAMETER :
* RETURN    :
* EXAMPLE   :
*****************************************************************************/
int InsertEdcLotSummaryBy9Site()
{
    EXEC SQL
       INSERT
       INTO    EDC_LOT_SUMMARY(
                   PRODUCT_ID,
                   LOT_ID,
                   EDC_PARAMETER_ID,
                   SITE_COUNT,
                   AVERAGE,
                   MAX_VAL,
                   MEDIAN,
                   MIN_VAL,
                   STD_DEV,
                   PERCENTILE_25,
                   PERCENTILE_75,
                   LATEST_FLAG,
                   MEASURE_TIME,
                   SITE_ID_LIST,
                   SITE_AVG_LIST,
                   SITE1_AVG,
                   SITE2_AVG,
                   SITE3_AVG,
                   SITE4_AVG,
                   SITE5_AVG,
                   SITE6_AVG,
                   SITE7_AVG,
                   SITE8_AVG,
                   SITE9_AVG)
       VALUES  (   :grProduct.product_id:grProduct.flag,
                   :grLot.lot_id,
                   :grCurParameter.parameter_id,
                   :grSum.site_count,
                   :grSum.average:grSum.avg_flag,
                   :grSum.max_val:grSum.flag,
                   :grSum.median:grSum.flag,
                   :grSum.min_val:grSum.flag,
                   :grSum.stddev:grSum.flag,
                   :grSum.percentile_25:grSum.flag,
                   :grSum.percentile_75:grSum.flag,
                   'Y',
                   to_date(:grHeader.measure_time,'YYYY/MM/DD HH24:MI:SS'),
                   :grSum.site_id_list,
                   :grSum.site_val_list,
                   :grSum.site1_val:grSum.site1_flag,
                   :grSum.site2_val:grSum.site2_flag,
                   :grSum.site3_val:grSum.site3_flag,
                   :grSum.site4_val:grSum.site4_flag,
                   :grSum.site5_val:grSum.site5_flag,
                   :grSum.site6_val:grSum.site6_flag,
                   :grSum.site7_val:grSum.site7_flag,
                   :grSum.site8_val:grSum.site8_flag,
                   :grSum.site9_val:grSum.site9_flag);
#ifdef _TEST_
    ShowStep("   -->InsertEdcLotSummaryBySite()");
#endif
#ifdef _CHECK_
    if(grSum.average==0)
        fprintf(gpLogFp,"   -->InsertEdcLotSummaryBySite() Average=0 : %5d\n",giCurLine);
#endif

    if (sqlca.sqlcode) {
        CheckSqlError("InsertEdcLotSummaryBySite() INSERT EDC_LOT_SUMMARY");
        return FAIL;
    }
    return SUCCEED;
} /* End InsertEdcLotSummaryBy9Site() */

/*****************************************************************************
* TITLE     : Insert EDC Wafer Summary By Site
* PURPOSE   :
* PARAMETER :
* RETURN    :
* EXAMPLE   :
*****************************************************************************/
int InsertEdcWaferSummaryBy9Site()
{
    EXEC SQL
       INSERT
       INTO    EDC_WAFER_SUMMARY(
                    PRODUCT_ID,
                    LOT_ID,
                    WAFER_ID,
                    WAFER_NO,
                    ROUTE_STEP_ID,
                    EDC_PARAMETER_ID,
                    SITE_COUNT,
                    MEASURE_TIME,
                    MAX_VAL,
                    MIN_VAL,
                    AVERAGE,
                    STD_DEV,
                    SITE1_VAL,
                    SITE2_VAL,
                    SITE3_VAL,
                    SITE4_VAL,
                    SITE5_VAL,
                    SITE6_VAL,
                    SITE7_VAL,
                    SITE8_VAL,
                    SITE9_VAL,
                    LATEST_FLAG,
                    SITE_ID_LIST,
                    SITE_VAL_LIST)
       VALUES  (   :grProduct.product_id:grProduct.flag,
                   :grLot.lot_id,
                   :grWafer.wafer_id,
                   :grWafer.wafer_no,
                   :grRouteStep.route_step_id,
                   :grCurParameter.parameter_id,
                   :grSum.site_count,
                   TO_DATE(:grHeader.measure_time, 'YYYY/MM/DD HH24:MI:SS'),
                   :grSum.max_val:grSum.flag,
                   :grSum.min_val:grSum.flag,
                   :grSum.average:grSum.avg_flag,
                   :grSum.stddev:grSum.flag,
                   :grSum.site1_val:grSum.site1_flag,
                   :grSum.site2_val:grSum.site2_flag,
                   :grSum.site3_val:grSum.site3_flag,
                   :grSum.site4_val:grSum.site4_flag,
                   :grSum.site5_val:grSum.site5_flag,
                   :grSum.site6_val:grSum.site6_flag,
                   :grSum.site7_val:grSum.site7_flag,
                   :grSum.site8_val:grSum.site8_flag,
                   :grSum.site9_val:grSum.site9_flag,
                    'Y',
                   :grSum.site_id_list,
                   :grSum.site_val_list);
#ifdef _TEST_
    ShowStep("   -->InsertEdcWaferSummaryBy9Site()");
#endif
#ifdef _CHECK_
    if(grSum.average==0)
        fprintf(gpLogFp,"   -->InsertEdcWaferSummaryBy9Site() Average=0 : %5d\n",giCurLine);
#endif
    if (sqlca.sqlcode) {
        CheckSqlError("InsertEdcWaferSummaryBy9Site() INSERT EDC_WAFER_SUMMARY");
        return FAIL;
    }
    return SUCCEED;
} /* End InsertEdcWaferSummaryBy9Site() */

/*****************************************************************************
* TITLE     : Insert RTC Summary
* PURPOSE   :
* PARAMETER :
* RETURN    :
* EXAMPLE   :
*****************************************************************************/
int InsertRtcSummaryBy9Site()
{
    EXEC SQL
       INSERT
       INTO    RTC_SUMMARY(
                    LOT_NO,
                    EQUIPMENT_ID,
                    RTC_PARAMETER_ID,
                    MEASURE_TIME,
                    WEEK,
                    MONTH,
                    OPERATOR_ID,
                    MEASURE_RECIPE,
                    PROCESS_RECIPE,
                    P_TOOL_ID,
                    MAX_VAL,
                    MIN_VAL,
                    AVERAGE,
                    MEDIAN,
                    STDDEV,
                    PERCENTILE_25,
                    PERCENTILE_75,
                    SITE_COUNT,
                    SITE1_VAL,
                    SITE2_VAL,
                    SITE3_VAL,
                    SITE4_VAL,
                    SITE5_VAL,
                    SITE6_VAL,
                    SITE7_VAL,
                    SITE8_VAL,
                    SITE9_VAL,
                    LATEST_FLAG,
                    FAB,
                    PRCSS_GRP,
                    SITE_ID_LIST,
                    SITE_VAL_LIST)
      VALUES   (    :grLot.lotno,
                    :grMeasEquip.equipment_id,
                    :grCurParameter.parameter_id,
                    TO_DATE(:grHeader.measure_time, 'YYYY/MM/DD HH24:MI:SS'),
                    TO_CHAR(TO_DATE(:grHeader.measure_time, 'YYYY/MM/DD HH24:MI:SS'), 'WW'),
                    TO_CHAR(TO_DATE(:grHeader.measure_time, 'YYYY/MM/DD HH24:MI:SS'), 'MM'),
                    :grOperator.operator_id:grOperator.flag,
                    :grMeasRecipe.name:grMeasRecipe.flag,
                    :grProcRecipe.name:grProcRecipe.flag,
                    :grProcEquip.name:grProcEquip.flag,
                    :grSum.max_val:grSum.flag,
                    :grSum.min_val:grSum.flag,
                    :grSum.average:grSum.avg_flag,
                    :grSum.median:grSum.flag,
                    :grSum.stddev:grSum.flag,
                    :grSum.percentile_25:grSum.flag,
                    :grSum.percentile_75:grSum.flag,
                    :grSum.site_count,
                    :grSum.site1_val:grSum.site1_flag,
                    :grSum.site2_val:grSum.site2_flag,
                    :grSum.site3_val:grSum.site3_flag,
                    :grSum.site4_val:grSum.site4_flag,
                    :grSum.site5_val:grSum.site5_flag,
                    :grSum.site6_val:grSum.site6_flag,
                    :grSum.site7_val:grSum.site7_flag,
                    :grSum.site8_val:grSum.site8_flag,
                    :grSum.site9_val:grSum.site9_flag,
                    'Y',:FAB,
                    :grLotHist.prcss_grp,
                    :grSum.site_id_list,
                    :grSum.site_val_list);
#ifdef _TEST_
    ShowStep("   -->InsertRtcSummaryBySite()");
#endif
    if (sqlca.sqlcode) {
        CheckSqlError("InsertRtcSummaryBy9Site() INSERT RTC_SUMMARY");
        return FAIL;
    }
    return SUCCEED;
}/* End InsertRtcSummary() */

/*****************************************************************************
* TITLE     : Get Wafer ID
* DESC.     :
* PARAMETER :
* RETURN    :
* EXAMPLE   :
*****************************************************************************/
int GetWaferID()
{
    /* Added by FELIX@2001/05/07*/
    if(strlen((char*)grWafer.wafer_no.arr)==1) {
        grWafer.wafer_no.arr[2]='\0';
        grWafer.wafer_no.arr[1]=grWafer.wafer_no.arr[0];
        grWafer.wafer_no.arr[0]='0';
    }

    EXEC SQL
      SELECT   WAFER_ID
      INTO     :grWafer.wafer_id
      FROM     WAFER
      WHERE    LOT_ID  = :grLot.lot_id  AND
               WAFER_NO = :grWafer.wafer_no;

    if(sqlca.sqlcode == 100 || sqlca.sqlcode==1403) {
        if(InsertWaferID() == FAIL)
            return FAIL;
    }

    else if(sqlca.sqlcode) {
        CheckSqlError("GetWaferID() SELECT WAFER_ID");
        return FAIL;
    }

    /* If found (sqlca.sqlcode == 0) */
    return FOUND;
}/* End of GetWaferID() */

/*****************************************************************************
* TITLE     : Insert Wafer ID
* DESC.     :
* PARAMETER :
* RETURN    :
* EXAMPLE   :
*****************************************************************************/
int InsertWaferID()
{
    EXEC SQL
        INSERT
        INTO    WAFER(
                  LOT_ID,
                  WAFER_NO,
                  WAFER_ID)
        VALUES  ( :grLot.lot_id,
                  :grWafer.wafer_no,
                  WAFER_ID.NEXTVAL);

    if(sqlca.sqlcode) {
        CheckSqlError("InsertWaferID() INSERT WAFER");
        return FAIL;
    }

    EXEC SQL
        SELECT  WAFER_ID
        INTO    :grWafer.wafer_id
        FROM    WAFER
        WHERE   LOT_ID   = :grLot.lot_id   AND
                WAFER_NO = :grWafer.wafer_no;

    if(sqlca.sqlcode) {
        CheckSqlError("InsertWaferID() SELECT WAFER_ID");
        return FAIL;
    }

    /* If no error condition occurs (sqlca.sqlcode == 0) */
    return SUCCEED;
}/* End of InsertWaferID() */

/* show time */
void ShowTime(str)
char *str;
{
#ifdef _SHOWTIME_
    char      s_time[30];
    time_t    nseconds;

    nseconds = (long)time(NULL);
    strcpy(s_time,(char *)ctime(&nseconds));
    s_time[24] = '\0';

    fprintf(gpLogFp,"\n-->%20s  [%s] ",str, s_time);
    fflush(gpLogFp);
#endif
}

/* show step */
void ShowStep(str)
char *str;
{
#ifdef _TEST_
    fprintf(gpLogFp,"\n%s",str);
    fflush(gpLogFp);
#endif
}

