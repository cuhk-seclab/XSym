/*
Created from the snmputil sample in the Microsoft SDK for NT

$Id: winsnmp.c,v 1.9 1998/12/28 09:34:51 sas Exp $ 
*/

#include "php.h"
#include "internal_functions.h"
#if COMPILE_DL
#include "../phpdl.h"
#include "functions/dl.h"
#endif
#include "php3_snmp.h"
#include <sys/types.h>

#include <windows.h>

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#if HAVE_SNMP

#include <snmp.h>
#include <mgmtapi.h>


function_entry snmp_functions[] = {
    {"snmpget", php3_snmpget, NULL},
    {"snmpwalk", php3_snmpwalk, NULL},
    {NULL,NULL,NULL}
};

php3_module_entry snmp_module_entry = {
	"SNMP",snmp_functions,NULL,NULL,NULL,NULL,NULL,0,0,0,NULL
};

#if COMPILE_DL
DLEXPORT php3_module_entry *get_module() { return &snmp_module_entry; }
#endif

#define GET     1
#define WALK    2
#define GETNEXT 3

#define TIMEOUT 6000 /* milliseconds */
#define RETRIES 3


void _php3_snmp(INTERNAL_FUNCTION_PARAMETERS, int st) {
	pval *a1, *a2, *a3;
	INT	operation;
    LPSTR              agent;
    LPSTR              community;
    RFC1157VarBindList variableBindings;
    LPSNMP_MGR_SESSION session;

    INT        timeout = TIMEOUT;
    INT        retries = RETRIES;

    BYTE       requestType;
    AsnInteger errorStatus;
    AsnInteger errorIndex;
    AsnObjectIdentifier oid;
   char        *chkPtr = NULL;

	if (getParameters(ht, 3, &a1, &a2, &a3) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_string(a1);
	convert_to_string(a2);
	convert_to_string(a3);

	agent=a1->value.str.val;
	community=a2->value.str.val;
	operation=st;
	SnmpMgrStrToOid(a3->value.str.val, &oid);

/* 
   I've limited this to only one oid, but we can create a
   list of oid's here, and expand the function to take multiple
   oid's
*/
    variableBindings.list->name = oid; 
    variableBindings.list->value.asnType = ASN_NULL;
    variableBindings.len = 1;

/* Establish a SNMP session to communicate with the remote agent.  The
   community, communications timeout, and communications retry count
   for the session are also required.
*/
	if ((session = SnmpMgrOpen(agent, community, timeout, retries)) == NULL){
		php3_error(E_WARNING,"error on SnmpMgrOpen %d\n", GetLastError());
	}

    /* Determine and perform the requested operation.*/
    if (operation == GET || operation == GETNEXT){
        /* Get and GetNext are relatively simple operations to perform.
           Simply initiate the request and process the result and/or
           possible error conditions. */

        if (operation == GET){
            requestType = ASN_RFC1157_GETREQUEST;
		} else {
            requestType = ASN_RFC1157_GETNEXTREQUEST;
		}

        /* Request that the API carry out the desired operation.*/
		if (!SnmpMgrRequest(session, requestType, &variableBindings,
                            &errorStatus, &errorIndex)){
            /* The API is indicating an error. */
            php3_error(E_WARNING,"error on SnmpMgrRequest %d\n", GetLastError());
        } else {
            /* The API succeeded, errors may be indicated from the remote
               agent. */
            if (errorStatus > 0){
                php3_error(E_WARNING,"Error: errorStatus=%d, errorIndex=%d\n",
                       errorStatus, errorIndex);
            } else {
                /* Display the resulting variable bindings.*/
				UINT i;
                char *string = NULL;

                for(i=0; i < variableBindings.len; i++)
                    {
                    SnmpMgrOidToStr(&variableBindings.list[i].name, &string);
                    php3_printf("Variable = %s\n", string);
                    if (string) SNMP_free(string);

                    php3_printf("Value    = ");
                    SnmpUtilPrintAsnAny(&variableBindings.list[i].value);

                    php3_printf("\n");
                    } /* end for() */
                }
            }

        /* Free the variable bindings that have been allocated.*/
        SnmpUtilVarBindListFree(&variableBindings);
        }
    else if (operation == WALK)
        {
        /* Walk is a common term used to indicate that all MIB variables
           under a given OID are to be traversed and displayed.  This is
           a more complex operation requiring tests and looping in addition
           to the steps for get/getnext above. */
        AsnObjectIdentifier root;
        AsnObjectIdentifier tempOid;

        SnmpUtilOidCpy(&root, &variableBindings.list[0].name);
        requestType = ASN_RFC1157_GETNEXTREQUEST;

        while(1)
            {
            if (!SnmpMgrRequest(session, requestType, &variableBindings,
                                &errorStatus, &errorIndex)){
                /* The API is indicating an error.*/
                php3_error(E_WARNING,"error on SnmpMgrRequest %d\n", GetLastError());
                break;
                }
            else
                {
                /* The API succeeded, errors may be indicated from the remote
                   agent.
                   Test for end of subtree or end of MIB. */
                if (errorStatus == SNMP_ERRORSTATUS_NOSUCHNAME ||
                    SnmpUtilOidNCmp(&variableBindings.list[0].name,
                                    &root, root.idLength))
                    {
                    PUTS("End of MIB subtree.\n\n");
                    break;
                    }

                /* Test for general error conditions or sucesss. */
                if (errorStatus > 0){
                    php3_error(E_ERROR,"Error: errorStatus=%d, errorIndex=%d \n",
                           errorStatus, errorIndex);
                    break;
                    }
                else
                    {
                    /* Display resulting variable binding for this iteration. */
                    char *string = NULL;

                    SnmpMgrOidToStr(&variableBindings.list[0].name, &string);
                    php3_printf("Variable = %s\n", string);
                    if (string) SNMP_free(string);

                    php3_printf("Value    = ");
                    SnmpUtilPrintAsnAny(&variableBindings.list[0].value);

                    php3_printf("\n");
                    }
                } /* end if () */
            /* Prepare for the next iteration.  Make sure returned oid is
               preserved and the returned value is freed.
			*/
            SnmpUtilOidCpy(&tempOid, &variableBindings.list[0].name);
            SnmpUtilVarBindFree(&variableBindings.list[0]);
            SnmpUtilOidCpy(&variableBindings.list[0].name, &tempOid);
            variableBindings.list[0].value.asnType = ASN_NULL;
            SnmpUtilOidFree(&tempOid);
            } /* end while() */
        /* Free the variable bindings that have been allocated.*/
        SnmpUtilVarBindListFree(&variableBindings);
        SnmpUtilOidFree(&root);
	} // end if (operation)


	/* Close SNMP session with the remote agent.*/
	if (!SnmpMgrClose(session)){
		php3_error(E_WARNING,"error on SnmpMgrClose %d\n", GetLastError());
	}
}

DLEXPORT void php3_snmpget(INTERNAL_FUNCTION_PARAMETERS) {
	_php3_snmp(INTERNAL_FUNCTION_PARAM_PASSTHRU,1);
}

DLEXPORT void php3_snmpwalk(INTERNAL_FUNCTION_PARAMETERS) {
	_php3_snmp(INTERNAL_FUNCTION_PARAM_PASSTHRU,2);
}


#endif
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
