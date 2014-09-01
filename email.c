
// src/tutorial/email.c

//******************************************************************************
//COMP9315 Database Systems Imp 
//Stuart Aitken, Gina Jordanova
//Assignment 1 Adding an Email Data Type to PostgreSQL
//******************************************************************************

#include "postgres.h"
#include "fmgr.h"
#include "libpq/pqformat.h"		// needed for send/recv functions
#include <string.h>
#include <ctype.h>

PG_MODULE_MAGIC;

typedef struct _email {
   char local[128];
   char domain[128];
}  Email;

//* Since we use V1 function calling convention, all these functions have
//* the same signature as far as C is concerned.  We provide these prototypes
//* just to forestall warnings when compiled with gcc -Wmissing-prototypes.

Datum	email_in(PG_FUNCTION_ARGS);
Datum	email_out(PG_FUNCTION_ARGS);
Datum	email_recv(PG_FUNCTION_ARGS);
Datum	email_send(PG_FUNCTION_ARGS);
Datum	email_lt(PG_FUNCTION_ARGS);
Datum	email_le(PG_FUNCTION_ARGS);
Datum	email_eq(PG_FUNCTION_ARGS);
Datum	email_neq(PG_FUNCTION_ARGS);
Datum	email_gt(PG_FUNCTION_ARGS);
Datum	email_ge(PG_FUNCTION_ARGS);
Datum	email_deq(PG_FUNCTION_ARGS);
Datum   email_ndeq(PG_FUNCTION_ARGS);
Datum	email_cmp(PG_FUNCTION_ARGS);
Datum   email_hval(PG_FUNCTION_ARGS);
bool    isValidInput(char *str);

//*****************************************************************************
//* Input/Output functions
//*****************************************************************************

PG_FUNCTION_INFO_V1(email_in);

Datum email_in(PG_FUNCTION_ARGS)
{
    char *str;
    char *token;

    int  i;
    Email *result;
    result = (Email *) palloc(sizeof(Email));
    
    str = PG_GETARG_CSTRING(0);
    
    //convert to connical form
    for (i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }

    if ( isValidInput(str) ) {
        ereport( ERROR, (errcode (ERRCODE_INVALID_TEXT_REPRESENTATION),
                  errmsg ("invalid input syntax for Email: \"%s\"", str)));
    }
    //printf ("tolower( str )= %s\n", str);

    token = strtok(str, "@");
    strcpy(result->local, token);

    token = strtok(NULL, "@");
    strcpy(result->domain, token);
    
    PG_RETURN_POINTER(result);
}

bool isValidInput(char *str) {
    int  i           = 0;
    int  at          = 0;
    int  domainWords = 0;
    char prev        = str[0];
    bool domain      = false;
    bool invalid     = false;
 
    prev = str[0];
    if ( !isalpha(str[0]) ) { invalid = true; }
    for (i = 0; str[i]; i++) {
        //printf("%c ", str[i]);
        //check char is in range [a-z] && [0-9] && '-' && '@' && '.'
        if ( !(isalpha(str[i]) || isdigit(str[i]) || 
               str[i] == '-'   || str[i] == '@'   || str[i] == '.') ) {
            printf ("isalpha = %d, isdigit = %d, is '-' = %d\ninvalid = true\n", 
                    isalpha(str[i]), isdigit(str[i]), (str[i]=='-'));
            invalid = true;
        }

        //check word begins with letter
        //takes care of empty words (e.g. j..shepherd@funny.email.com)
        if (prev == '.' && !isalpha(str[i])) { printf("%c isn't [a-z]\n", str[i]); invalid = true; }

        //check word ends with letter or digit
        if ((str[i] == '.' || str[i] == '@') && 
            !(isalpha(prev) || isdigit(prev))) { printf("last letter of word (%c) isn't [a-z] || [0-9]\n", prev); invalid = true; }
        
        if (str[i] == '@') { 
             domain = true;
             if (++at > 1) { printf ("too many @ (%d)", at); invalid = true; }  
        }
        //check domain contains at least 2 words
        if (domain && (prev == '.' || prev == '@')) {
             domainWords++;
        }
        prev = str[i];
    }

    if (domainWords < 2) { printf("not enough words in domain of %s (%d)\n", str, domainWords); invalid = true; }

  return invalid;
}

PG_FUNCTION_INFO_V1(email_out);

Datum email_out(PG_FUNCTION_ARGS)
{
    Email    *email = (Email *) PG_GETARG_POINTER(0);
    char     *result;

    result = (char *) palloc(sizeof(Email));
    snprintf(result, sizeof(Email), "%s@%s", email->local, email->domain);
    PG_RETURN_CSTRING(result);
}

//*****************************************************************************
//* Binary Input/Output functions
//*****************************************************************************

PG_FUNCTION_INFO_V1(email_recv);

Datum email_recv(PG_FUNCTION_ARGS) {

	StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
	Email *result;

	result = (Email *) palloc(sizeof(Email));
	strcpy(result->local, pq_getmsgstring(buf));
	strcpy(result->domain, pq_getmsgstring(buf));
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(email_send);

Datum email_send(PG_FUNCTION_ARGS) {

	Email *email = (Email *) PG_GETARG_POINTER(0);
	StringInfoData buf;

	pq_begintypsend(&buf);
	pq_sendstring(&buf, email->local);
	pq_sendstring(&buf, email->domain);
	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

//*****************************************************************************
//* Operator class for defining B-tree index
//*****************************************************************************

#define Mag(c)	((c)->local*(c)->local + (c)->domain*(c)->domain)

static int email_cmp_internal(Email *a, Email *b)
{
	//char amag = Mag(a), bmag = Mag(b);
	
	if (strcmp(a->domain, b->domain) != 0 ) {
	    return strcmp(a->domain, b->domain);
	}
	if (strcmp(a->local, b->local) != 0 ) {
	    return strcmp(a->local, b->local);
	}
    return 0;
	//return strcmp(&a, &b);
}

PG_FUNCTION_INFO_V1(email_lt);

Datum email_lt(PG_FUNCTION_ARGS)
{
	Email    *a = (Email *) PG_GETARG_POINTER(0);
	Email    *b = (Email *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(email_cmp_internal(a, b) < 0);
}

PG_FUNCTION_INFO_V1(email_le);

Datum email_le(PG_FUNCTION_ARGS)
{
	Email    *a = (Email *) PG_GETARG_POINTER(0);
	Email    *b = (Email *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(email_cmp_internal(a, b) <= 0);
}

PG_FUNCTION_INFO_V1(email_eq);

Datum email_eq(PG_FUNCTION_ARGS)
{
	Email    *a = (Email *) PG_GETARG_POINTER(0);
	Email    *b = (Email *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(email_cmp_internal(a, b) == 0);
}

PG_FUNCTION_INFO_V1(email_neq);

Datum email_neq(PG_FUNCTION_ARGS)
{
	Email    *a = (Email *) PG_GETARG_POINTER(0);
	Email    *b = (Email *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(email_cmp_internal(a, b) != 0);
}

PG_FUNCTION_INFO_V1(email_ge);

Datum email_ge(PG_FUNCTION_ARGS)
{
	Email    *a = (Email *) PG_GETARG_POINTER(0);
	Email    *b = (Email *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(email_cmp_internal(a, b) >= 0);
}

PG_FUNCTION_INFO_V1(emails_gt);

Datum email_gt(PG_FUNCTION_ARGS)
{
	Email    *a = (Email *) PG_GETARG_POINTER(0);
	Email    *b = (Email *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(email_cmp_internal(a, b) > 0);
}

PG_FUNCTION_INFO_V1(email_deq);

Datum email_deq(PG_FUNCTION_ARGS)
{
	Email    *a = (Email *) PG_GETARG_POINTER(0);
	Email    *b = (Email *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(strcmp(a->domain, b->domain) == 0);
}

PG_FUNCTION_INFO_V1(email_ndeq);

Datum email_ndeq(PG_FUNCTION_ARGS)
{
	Email    *a = (Email *) PG_GETARG_POINTER(0);
	Email    *b = (Email *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(strcmp(a->domain, b->domain) != 0);
}

Datum email_cmp(PG_FUNCTION_ARGS)
{
	Email    *a = (Email *) PG_GETARG_POINTER(0);
	Email    *b = (Email *) PG_GETARG_POINTER(1);

	PG_RETURN_INT32(email_cmp_internal(a, b));
}

Datum email_hval(PG_FUNCTION_ARGS)
{
	//Email    *a = (Email *) PG_GETARG_POINTER(0);

	PG_RETURN_INT32(1);
}
