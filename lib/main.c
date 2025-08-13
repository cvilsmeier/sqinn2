#include "utl.h"
#include "io.h"
#include "db.h"
#include "app.h"
#include "sqlite3.h"

#define SQINN_NAME "sqinn2"
#define SQINN_VERSION "2.0.0"

int hasOpt(int argc, char const *argv[], const char *name) {
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], name) == 0) {
            return 1;
        }
    }
    return 0;
}

void getOpt(int argc, char const *argv[], const char *name, char *value, size_t n, const char *defaultValue) {
    for (int i = 0; i < argc - 1; i++) {
        if (strcmp(argv[i], name) == 0) {
            strncpy(value, argv[i + 1], n-1);
            value[n-1] = 0;
            return;
        }
    }
    strncpy(value, defaultValue, n-1);
    value[n-1] = 0;
}

Log* makeLog(int argc, char const *argv[]) {
    // -loglevel <level>
    int level = LOG_LEVEL_OFF;
    char slevel[8];
    getOpt(argc, argv, "-loglevel", slevel, sizeof(slevel), "0");
    level = atoi(slevel);
    level = level < LOG_LEVEL_OFF ? LOG_LEVEL_OFF : level;
    level = level > 2 ? 0 : level;
    // -logfile <file>
    char logfile[512] = {0};
    getOpt(argc, argv, "-logfile", logfile, sizeof(logfile), "");
    // -logstderr
    BOOL stdErr = hasOpt(argc, argv, "-logstderr");
    // build Log
    return newLog(level, logfile, stdErr);
}

Db* makeDb(int argc, char const *argv[]) {
    // -db <dbname>
    char dbname[256] = {0};
    getOpt(argc, argv, "-db", dbname, sizeof(dbname), ":memory:");
    // new Db
    return newDb(dbname, FALSE);
}

void help() {
    printf("%s - SQLite over stdin/stdout.\n", SQINN_NAME);
    printf("\n");
    printf("Usage:\n");
    printf("\t%s [options]\n", SQINN_NAME);
    printf("\n");
    printf("Options:\n");
    printf("\n");
    printf("\t-db <dbname>      Database name. Default is \":memory:\"\n");
    printf("\t-loglevel <level> Log level: 0=off, 1=info, 2=debug. Default is 0 (off).\n");
    printf("\t-logfile  <file>  Log to a file. Default is empty (no file logging).\n");
    printf("\t-logstderr        Log to stderr. Default is off (no stderr logging).\n");
    printf("\n");
    printf("One (and only one) of the following options must be specified:\n");
    printf("\n");
    printf("\t-run             Listen to stdin and write to stdout.\n");
    printf("\t-test            Execute selftest and exit.\n");
    printf("\t-version         Print version and exit.\n");
    printf("\t-sqlite          Print SQLite library version and exit.\n");
    printf("\t-help            Print help page and exit.\n");
    printf("\n");
    printf("When no options are given, %s prints a help page and exits.\n", SQINN_NAME);
}

int main(int argc, char const *argv[]) {
    if (hasOpt(argc, argv, "-run")) {
        theLog = makeLog(argc, argv);
        initMem();
        LOG_INFO2("--- %s v%s start ---", SQINN_NAME, SQINN_VERSION);
        Db *db = makeDb(argc, argv);
        Reader *r = newStdinReader();
        Writer *w = newStdoutWriter();
        App *app = newApp(db, r, w);
        while(App_step(app)) {
            ; // loop until App_step() returns FALSE
        }
        App_free(app);
        Writer_free(w);
        Reader_free(r);
        Db_free(db);
        if (mallocs != frees) {
            LOG_INFO2("found memory leaks: mallocs %d != frees %d", mallocs, frees);
        }
        LOG_INFO2("--- %s v%s exit ---", SQINN_NAME,SQINN_VERSION);
        Log_free(theLog);
        return 0;
    }
    if (hasOpt(argc, argv, "-test")) {
        theLog = makeLog(argc, argv);
        initMem();
        LOG_INFO2("--- %s v%s test start ---", SQINN_NAME, SQINN_VERSION);
        testIo();
        testDb();
        testApp();
        if (mallocs != frees) {
            printMem(stderr);
            ASSERTF(mallocs == frees, "memory leak: %d mallocs, %d frees", mallocs, frees);
        }
        LOG_INFO2("--- %s v%s test ok ---", SQINN_NAME, SQINN_VERSION);
        Log_free(theLog);
        printf("test ok\n");
        return 0;
    }
    if (hasOpt(argc, argv, "-version")) {
        printf("%s v%s\n", SQINN_NAME, SQINN_VERSION);
        return 0;
    }
    if (hasOpt(argc, argv, "-sqlite")) {
        printf("%s\n", SQLITE_VERSION);
        return 0;
    }
    help();
    return 0;
}
