/* soapStub.h
   Generated by gSOAP 2.8.23 from vies.h

Copyright(C) 2000-2015, Robert van Engelen, Genivia Inc. All Rights Reserved.
The generated code is released under one of the following licenses:
GPL or Genivia's license for commercial use.
This program is released under the GPL with the additional exemption that
compiling, linking, and/or using OpenSSL is allowed.
*/

#ifndef soapStub_H
#define soapStub_H
#include <vector>
#define SOAP_NAMESPACE_OF_ns2	"urn:ec.europa.eu:taxud:vies:services:checkVat:types"
#include "stdsoap2.h"
#if GSOAP_VERSION != 20823
# error "GSOAP VERSION 20823 MISMATCH IN GENERATED CODE VERSUS LIBRARY CODE: PLEASE REINSTALL PACKAGE"
#endif


/******************************************************************************\
 *                                                                            *
 * Enumerations                                                               *
 *                                                                            *
\******************************************************************************/


#ifndef SOAP_TYPE_ns2__matchCode
#define SOAP_TYPE_ns2__matchCode (15)
/* ns2:matchCode */
enum ns2__matchCode { ns2__matchCode__1 = 1, ns2__matchCode__2 = 2, ns2__matchCode__3 = 3 };
#endif

/******************************************************************************\
 *                                                                            *
 * Types with Custom Serializers                                              *
 *                                                                            *
\******************************************************************************/


/******************************************************************************\
 *                                                                            *
 * Classes and Structs                                                        *
 *                                                                            *
\******************************************************************************/


#if 0 /* volatile type: do not declare here, declared elsewhere */

#endif

#if 0 /* volatile type: do not declare here, declared elsewhere */

#endif

#ifndef SOAP_TYPE__ns2__checkVat
#define SOAP_TYPE__ns2__checkVat (10)
/* ns2:checkVat */
class SOAP_CMAC _ns2__checkVat
{
public:
	std::string countryCode;	/* required element of type xsd:string */
	std::string vatNumber;	/* required element of type xsd:string */
	struct soap *soap;	/* transient */
public:
	virtual int soap_type() const { return 10; } /* = unique type id SOAP_TYPE__ns2__checkVat */
	virtual void soap_default(struct soap*);
	virtual void soap_serialize(struct soap*) const;
	virtual int soap_put(struct soap*, const char*, const char*) const;
	virtual int soap_out(struct soap*, const char*, int, const char*) const;
	virtual void *soap_get(struct soap*, const char*, const char*);
	virtual void *soap_in(struct soap*, const char*, const char*);
	         _ns2__checkVat() { _ns2__checkVat::soap_default(NULL); }
	virtual ~_ns2__checkVat() { }
};
#endif

#ifndef SOAP_TYPE__ns2__checkVatResponse
#define SOAP_TYPE__ns2__checkVatResponse (11)
/* ns2:checkVatResponse */
class SOAP_CMAC _ns2__checkVatResponse
{
public:
	std::string countryCode;	/* SOAP 1.2 RPC return element (when namespace qualified) */	/* required element of type xsd:string */
	std::string vatNumber;	/* required element of type xsd:string */
	std::string requestDate;	/* required element of type xsd:date */
	bool valid;	/* required element of type xsd:boolean */
	std::string *name;	/* optional element of type xsd:string */
	std::string *address;	/* optional element of type xsd:string */
	struct soap *soap;	/* transient */
public:
	virtual int soap_type() const { return 11; } /* = unique type id SOAP_TYPE__ns2__checkVatResponse */
	virtual void soap_default(struct soap*);
	virtual void soap_serialize(struct soap*) const;
	virtual int soap_put(struct soap*, const char*, const char*) const;
	virtual int soap_out(struct soap*, const char*, int, const char*) const;
	virtual void *soap_get(struct soap*, const char*, const char*);
	virtual void *soap_in(struct soap*, const char*, const char*);
	         _ns2__checkVatResponse() { _ns2__checkVatResponse::soap_default(NULL); }
	virtual ~_ns2__checkVatResponse() { }
};
#endif

#ifndef SOAP_TYPE__ns2__checkVatApprox
#define SOAP_TYPE__ns2__checkVatApprox (12)
/* ns2:checkVatApprox */
class SOAP_CMAC _ns2__checkVatApprox
{
public:
	std::string countryCode;	/* required element of type xsd:string */
	std::string vatNumber;	/* required element of type xsd:string */
	std::string *traderName;	/* optional element of type xsd:string */
	std::string *traderCompanyType;	/* optional element of type ns2:companyTypeCode */
	std::string *traderStreet;	/* optional element of type xsd:string */
	std::string *traderPostcode;	/* optional element of type xsd:string */
	std::string *traderCity;	/* optional element of type xsd:string */
	std::string *requesterCountryCode;	/* optional element of type xsd:string */
	std::string *requesterVatNumber;	/* optional element of type xsd:string */
	struct soap *soap;	/* transient */
public:
	virtual int soap_type() const { return 12; } /* = unique type id SOAP_TYPE__ns2__checkVatApprox */
	virtual void soap_default(struct soap*);
	virtual void soap_serialize(struct soap*) const;
	virtual int soap_put(struct soap*, const char*, const char*) const;
	virtual int soap_out(struct soap*, const char*, int, const char*) const;
	virtual void *soap_get(struct soap*, const char*, const char*);
	virtual void *soap_in(struct soap*, const char*, const char*);
	         _ns2__checkVatApprox() { _ns2__checkVatApprox::soap_default(NULL); }
	virtual ~_ns2__checkVatApprox() { }
};
#endif

#ifndef SOAP_TYPE__ns2__checkVatApproxResponse
#define SOAP_TYPE__ns2__checkVatApproxResponse (13)
/* ns2:checkVatApproxResponse */
class SOAP_CMAC _ns2__checkVatApproxResponse
{
public:
	std::string countryCode;	/* SOAP 1.2 RPC return element (when namespace qualified) */	/* required element of type xsd:string */
	std::string vatNumber;	/* required element of type xsd:string */
	std::string requestDate;	/* required element of type xsd:date */
	bool valid;	/* required element of type xsd:boolean */
	std::string *traderName;	/* optional element of type xsd:string */
	std::string *traderCompanyType;	/* optional element of type ns2:companyTypeCode */
	std::string *traderAddress;	/* optional element of type xsd:string */
	std::string *traderStreet;	/* optional element of type xsd:string */
	std::string *traderPostcode;	/* optional element of type xsd:string */
	std::string *traderCity;	/* optional element of type xsd:string */
	enum ns2__matchCode *traderNameMatch;	/* optional element of type ns2:matchCode */
	enum ns2__matchCode *traderCompanyTypeMatch;	/* optional element of type ns2:matchCode */
	enum ns2__matchCode *traderStreetMatch;	/* optional element of type ns2:matchCode */
	enum ns2__matchCode *traderPostcodeMatch;	/* optional element of type ns2:matchCode */
	enum ns2__matchCode *traderCityMatch;	/* optional element of type ns2:matchCode */
	std::string requestIdentifier;	/* required element of type xsd:string */
	struct soap *soap;	/* transient */
public:
	virtual int soap_type() const { return 13; } /* = unique type id SOAP_TYPE__ns2__checkVatApproxResponse */
	virtual void soap_default(struct soap*);
	virtual void soap_serialize(struct soap*) const;
	virtual int soap_put(struct soap*, const char*, const char*) const;
	virtual int soap_out(struct soap*, const char*, int, const char*) const;
	virtual void *soap_get(struct soap*, const char*, const char*);
	virtual void *soap_in(struct soap*, const char*, const char*);
	         _ns2__checkVatApproxResponse() { _ns2__checkVatApproxResponse::soap_default(NULL); }
	virtual ~_ns2__checkVatApproxResponse() { }
};
#endif

#ifndef SOAP_TYPE___ns1__checkVat
#define SOAP_TYPE___ns1__checkVat (24)
/* Operation wrapper: */
struct __ns1__checkVat
{
public:
	_ns2__checkVat *ns2__checkVat;	/* optional element of type ns2:checkVat */
public:
	int soap_type() const { return 24; } /* = unique type id SOAP_TYPE___ns1__checkVat */
};
#endif

#ifndef SOAP_TYPE___ns1__checkVatApprox
#define SOAP_TYPE___ns1__checkVatApprox (28)
/* Operation wrapper: */
struct __ns1__checkVatApprox
{
public:
	_ns2__checkVatApprox *ns2__checkVatApprox;	/* optional element of type ns2:checkVatApprox */
public:
	int soap_type() const { return 28; } /* = unique type id SOAP_TYPE___ns1__checkVatApprox */
};
#endif

#ifndef WITH_NOGLOBAL

#ifndef SOAP_TYPE_SOAP_ENV__Header
#define SOAP_TYPE_SOAP_ENV__Header (29)
/* SOAP Header: */
struct SOAP_ENV__Header
{
public:
	int soap_type() const { return 29; } /* = unique type id SOAP_TYPE_SOAP_ENV__Header */
};
#endif

#endif

#ifndef WITH_NOGLOBAL

#ifndef SOAP_TYPE_SOAP_ENV__Code
#define SOAP_TYPE_SOAP_ENV__Code (30)
/* SOAP Fault Code: */
struct SOAP_ENV__Code
{
public:
	char *SOAP_ENV__Value;	/* optional element of type xsd:QName */
	struct SOAP_ENV__Code *SOAP_ENV__Subcode;	/* optional element of type SOAP-ENV:Code */
public:
	int soap_type() const { return 30; } /* = unique type id SOAP_TYPE_SOAP_ENV__Code */
};
#endif

#endif

#ifndef WITH_NOGLOBAL

#ifndef SOAP_TYPE_SOAP_ENV__Detail
#define SOAP_TYPE_SOAP_ENV__Detail (32)
/* SOAP-ENV:Detail */
struct SOAP_ENV__Detail
{
public:
	char *__any;
	int __type;	/* any type of element <fault> (defined below) */
	void *fault;	/* transient */
public:
	int soap_type() const { return 32; } /* = unique type id SOAP_TYPE_SOAP_ENV__Detail */
};
#endif

#endif

#ifndef WITH_NOGLOBAL

#ifndef SOAP_TYPE_SOAP_ENV__Reason
#define SOAP_TYPE_SOAP_ENV__Reason (35)
/* SOAP-ENV:Reason */
struct SOAP_ENV__Reason
{
public:
	char *SOAP_ENV__Text;	/* optional element of type xsd:string */
public:
	int soap_type() const { return 35; } /* = unique type id SOAP_TYPE_SOAP_ENV__Reason */
};
#endif

#endif

#ifndef WITH_NOGLOBAL

#ifndef SOAP_TYPE_SOAP_ENV__Fault
#define SOAP_TYPE_SOAP_ENV__Fault (36)
/* SOAP Fault: */
struct SOAP_ENV__Fault
{
public:
	char *faultcode;	/* optional element of type xsd:QName */
	char *faultstring;	/* optional element of type xsd:string */
	char *faultactor;	/* optional element of type xsd:string */
	struct SOAP_ENV__Detail *detail;	/* optional element of type SOAP-ENV:Detail */
	struct SOAP_ENV__Code *SOAP_ENV__Code;	/* optional element of type SOAP-ENV:Code */
	struct SOAP_ENV__Reason *SOAP_ENV__Reason;	/* optional element of type SOAP-ENV:Reason */
	char *SOAP_ENV__Node;	/* optional element of type xsd:string */
	char *SOAP_ENV__Role;	/* optional element of type xsd:string */
	struct SOAP_ENV__Detail *SOAP_ENV__Detail;	/* optional element of type SOAP-ENV:Detail */
public:
	int soap_type() const { return 36; } /* = unique type id SOAP_TYPE_SOAP_ENV__Fault */
};
#endif

#endif

/******************************************************************************\
 *                                                                            *
 * Typedefs                                                                   *
 *                                                                            *
\******************************************************************************/

#ifndef SOAP_TYPE__QName
#define SOAP_TYPE__QName (5)
typedef char *_QName;
#endif

#ifndef SOAP_TYPE__XML
#define SOAP_TYPE__XML (6)
typedef char *_XML;
#endif

#ifndef SOAP_TYPE_xsd__date
#define SOAP_TYPE_xsd__date (9)
typedef std::string xsd__date;
#endif

#ifndef SOAP_TYPE_ns2__companyTypeCode
#define SOAP_TYPE_ns2__companyTypeCode (14)
typedef std::string ns2__companyTypeCode;
#endif


/******************************************************************************\
 *                                                                            *
 * Externals                                                                  *
 *                                                                            *
\******************************************************************************/


#endif

/* End of soapStub.h */
