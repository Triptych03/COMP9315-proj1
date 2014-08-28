
// src/tutorial/email.c

//******************************************************************************
//COMP9315 Database Systems Imp 
//Stuart Aitken, Gina Jordanova
//Assignment 1 Adding an Email Data Type to PostgreSQL
//******************************************************************************

#include "postgres.h"
#include "fmgr.h"
#include "libpq/pqformat.h"		// needed for send/recv functions


PG_MODULE_MAGIC;

typedef struct Email
{
   char local;
   char domain;   
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


//*****************************************************************************
//* Input/Output functions
//*****************************************************************************

PG_FUNCTION_INFO_V1(email_in);

Datum email_in(PG_FUNCTION_ARGS)
{
	char	   *str = PG_GETARG_CSTRING(0);
	char		local, domain;
	Email      *result;

	if (sscanf(str, "%s @ %s", &local, &domain) != 2)
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
				 errmsg("invalid input syntax for Email Address: \"%s\"",
						str)));

	result = (Email *) palloc(sizeof(Email));
	result->local = local;
	result->domain = domain;
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(email_out);

Datum email_out(PG_FUNCTION_ARGS)
{
	Email    *email = (Email *) PG_GETARG_POINTER(0);
	char	 *result;

	result = (char *) palloc(100);
	snprintf(result, 100, "%s @ %s", email->local, email->domain);
	PG_RETURN_CSTRING(result);
}

//*****************************************************************************
//* Binary Input/Output functions
//*****************************************************************************

PG_FUNCTION_INFO_V1(email_recv);

Datum email_recv(PG_FUNCTION_ARGS)
{
	StringInfo	buf = (StringInfo) PG_GETARG_POINTER(0);
	Email    *result;

	result = (Email *) palloc(sizeof(Email));
	result->local = pq_getmsgstring(buf);
	result->domain = pq_getmsgstring(buf);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(email_send);

Datum email_send(PG_FUNCTION_ARGS)
{
	Email    *email = (Email *) PG_GETARG_POINTER(0);
	StringInfoData buf;

	pq_begintypsend(&buf);
	pq_sendstring(&buf, email->local);
	pq_sendstring(&buf, email->domain);
	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

//*****************************************************************************
//* Operator class for defining B-tree index
//*****************************************************************************

PG_FUNCTION_INFO_V1(complex_abs_lt);

Datum email_lt(PG_FUNCTION_ARGS)
{
	Email    *a = (Email *) PG_GETARG_POINTER(0);
	Email    *b = (Email *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(strcmp(a, b) < 0);
}

PG_FUNCTION_INFO_V1(email_le);

Datum email_le(PG_FUNCTION_ARGS)
{
	Email    *a = (Email *) PG_GETARG_POINTER(0);
	Email    *b = (Email *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(strcmp(a, b) <= 0);
}

PG_FUNCTION_INFO_V1(email_eq);

Datum email_eq(PG_FUNCTION_ARGS)
{
	Email    *a = (Email *) PG_GETARG_POINTER(0);
	Email    *b = (Email *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(strcmp(a, b) == 0);
}

PG_FUNCTION_INFO_V1(email_neq);

Datum email_eq(PG_FUNCTION_ARGS)
{
	Email    *a = (Email *) PG_GETARG_POINTER(0);
	Email    *b = (Email *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(strcmp(a, b) != 0);
}

PG_FUNCTION_INFO_V1(email_ge);

Datum email_ge(PG_FUNCTION_ARGS)
{
	Email    *a = (Email *) PG_GETARG_POINTER(0);
	Email    *b = (Email *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(strcmp(a, b) >= 0);
}

PG_FUNCTION_INFO_V1(emails_gt);

Datum email_gt(PG_FUNCTION_ARGS)
{
	Email    *a = (Email *) PG_GETARG_POINTER(0);
	Email    *b = (Email *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(strcmp(a, b) > 0);
}

PG_FUNCTION_INFO_V1(email_cmp);

Datum email_deq(PG_FUNCTION_ARGS)
{
	Email    *a = (Email *) PG_GETARG_POINTER(0);
	Email    *b = (Email *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(strcmp(a->domain, b->domain) == 0);
}

PG_FUNCTION_INFO_V1(email_cmp);

Datum email_ndeq(PG_FUNCTION_ARGS)
{
	Email    *a = (Email *) PG_GETARG_POINTER(0);
	Email    *b = (Email *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(strcmp(a->domain, b->domain) != 0);
}


