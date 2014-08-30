
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

PG_MODULE_MAGIC;

typedef struct _email {
   char domain[128];
   char local[128];
   
      
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


//*****************************************************************************
//* Input/Output functions
//*****************************************************************************

PG_FUNCTION_INFO_V1(email_in);

Datum email_in(PG_FUNCTION_ARGS)
{
	char  *str;
    char *token;
	Email *result;
	result = (Email *) palloc(sizeof(Email));
	
	str = PG_GETARG_CSTRING(0);
	
    token = strtok(str, "@");
    strcpy(result->local, token);
    
    token = strtok(NULL, "@");
    strcpy(result->domain, token);    
    
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(email_out);

Datum email_out(PG_FUNCTION_ARGS)
{
	Email    *email = (Email *) PG_GETARG_POINTER(0);
	char	 *result;

	result = (char *) palloc(sizeof(Email));
	sprintf(result, "%s@%s", email->local, email->domain);
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



