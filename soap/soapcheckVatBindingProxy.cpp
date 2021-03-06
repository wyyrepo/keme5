/* soapcheckVatBindingProxy.cpp
   Generated by gSOAP 2.8.23 from vies.h

Copyright(C) 2000-2015, Robert van Engelen, Genivia Inc. All Rights Reserved.
The generated code is released under one of the following licenses:
GPL or Genivia's license for commercial use.
This program is released under the GPL with the additional exemption that
compiling, linking, and/or using OpenSSL is allowed.
*/

#include "soapcheckVatBindingProxy.h"

checkVatBindingProxy::checkVatBindingProxy()
{	this->soap = soap_new();
	this->soap_own = true;
	checkVatBindingProxy_init(SOAP_IO_DEFAULT, SOAP_IO_DEFAULT);
}

checkVatBindingProxy::checkVatBindingProxy(struct soap *_soap)
{	this->soap = _soap;
	this->soap_own = false;
	checkVatBindingProxy_init(_soap->imode, _soap->omode);
}

checkVatBindingProxy::checkVatBindingProxy(const char *url)
{	this->soap = soap_new();
	this->soap_own = true;
	checkVatBindingProxy_init(SOAP_IO_DEFAULT, SOAP_IO_DEFAULT);
	soap_endpoint = url;
}

checkVatBindingProxy::checkVatBindingProxy(soap_mode iomode)
{	this->soap = soap_new();
	this->soap_own = true;
	checkVatBindingProxy_init(iomode, iomode);
}

checkVatBindingProxy::checkVatBindingProxy(const char *url, soap_mode iomode)
{	this->soap = soap_new();
	this->soap_own = true;
	checkVatBindingProxy_init(iomode, iomode);
	soap_endpoint = url;
}

checkVatBindingProxy::checkVatBindingProxy(soap_mode imode, soap_mode omode)
{	this->soap = soap_new();
	this->soap_own = true;
	checkVatBindingProxy_init(imode, omode);
}

checkVatBindingProxy::~checkVatBindingProxy()
{	if (this->soap_own)
		soap_free(this->soap);
}

void checkVatBindingProxy::checkVatBindingProxy_init(soap_mode imode, soap_mode omode)
{	soap_imode(this->soap, imode);
	soap_omode(this->soap, omode);
	soap_endpoint = NULL;
	static const struct Namespace namespaces[] =
{
	{"SOAP-ENV", "http://schemas.xmlsoap.org/soap/envelope/", "http://www.w3.org/*/soap-envelope", NULL},
	{"SOAP-ENC", "http://schemas.xmlsoap.org/soap/encoding/", "http://www.w3.org/*/soap-encoding", NULL},
	{"xsi", "http://www.w3.org/2001/XMLSchema-instance", "http://www.w3.org/*/XMLSchema-instance", NULL},
	{"xsd", "http://www.w3.org/2001/XMLSchema", "http://www.w3.org/*/XMLSchema", NULL},
	{"ns1", "urn:ec.europa.eu:taxud:vies:services:checkVat", NULL, NULL},
	{"ns2", "urn:ec.europa.eu:taxud:vies:services:checkVat:types", NULL, NULL},
	{NULL, NULL, NULL, NULL}
};
	soap_set_namespaces(this->soap, namespaces);
}

void checkVatBindingProxy::destroy()
{	soap_destroy(this->soap);
	soap_end(this->soap);
}

void checkVatBindingProxy::reset()
{	this->destroy();
	soap_done(this->soap);
	soap_initialize(this->soap);
	checkVatBindingProxy_init(SOAP_IO_DEFAULT, SOAP_IO_DEFAULT);
}

void checkVatBindingProxy::soap_noheader()
{	this->soap->header = NULL;
}

const SOAP_ENV__Header *checkVatBindingProxy::soap_header()
{	return this->soap->header;
}

const SOAP_ENV__Fault *checkVatBindingProxy::soap_fault()
{	return this->soap->fault;
}

const char *checkVatBindingProxy::soap_fault_string()
{	return *soap_faultstring(this->soap);
}

const char *checkVatBindingProxy::soap_fault_detail()
{	return *soap_faultdetail(this->soap);
}

int checkVatBindingProxy::soap_close_socket()
{	return soap_closesock(this->soap);
}

int checkVatBindingProxy::soap_force_close_socket()
{	return soap_force_closesock(this->soap);
}

void checkVatBindingProxy::soap_print_fault(FILE *fd)
{	::soap_print_fault(this->soap, fd);
}

#ifndef WITH_LEAN
#ifndef WITH_COMPAT
void checkVatBindingProxy::soap_stream_fault(std::ostream& os)
{	::soap_stream_fault(this->soap, os);
}
#endif

char *checkVatBindingProxy::soap_sprint_fault(char *buf, size_t len)
{	return ::soap_sprint_fault(this->soap, buf, len);
}
#endif

int checkVatBindingProxy::checkVat(const char *endpoint, const char *soap_action, _ns2__checkVat *ns2__checkVat, _ns2__checkVatResponse &ns2__checkVatResponse)
{	struct soap *soap = this->soap;
	struct __ns1__checkVat soap_tmp___ns1__checkVat;
	if (endpoint)
		soap_endpoint = endpoint;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://ec.europa.eu/taxation_customs/vies/services/checkVatService";
	if (soap_action == NULL)
		soap_action = "";
	soap_begin(soap);
	soap->encodingStyle = NULL;
	soap_tmp___ns1__checkVat.ns2__checkVat = ns2__checkVat;
	soap_serializeheader(soap);
	soap_serialize___ns1__checkVat(soap, &soap_tmp___ns1__checkVat);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___ns1__checkVat(soap, &soap_tmp___ns1__checkVat, "-ns1:checkVat", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_url(soap, soap_endpoint, NULL), soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___ns1__checkVat(soap, &soap_tmp___ns1__checkVat, "-ns1:checkVat", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!&ns2__checkVatResponse)
		return soap_closesock(soap);
	ns2__checkVatResponse.soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	ns2__checkVatResponse.soap_get(soap, "ns2:checkVatResponse", NULL);
	if (soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

int checkVatBindingProxy::checkVatApprox(const char *endpoint, const char *soap_action, _ns2__checkVatApprox *ns2__checkVatApprox, _ns2__checkVatApproxResponse &ns2__checkVatApproxResponse)
{	struct soap *soap = this->soap;
	struct __ns1__checkVatApprox soap_tmp___ns1__checkVatApprox;
	if (endpoint)
		soap_endpoint = endpoint;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://ec.europa.eu/taxation_customs/vies/services/checkVatService";
	if (soap_action == NULL)
		soap_action = "";
	soap_begin(soap);
	soap->encodingStyle = NULL;
	soap_tmp___ns1__checkVatApprox.ns2__checkVatApprox = ns2__checkVatApprox;
	soap_serializeheader(soap);
	soap_serialize___ns1__checkVatApprox(soap, &soap_tmp___ns1__checkVatApprox);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___ns1__checkVatApprox(soap, &soap_tmp___ns1__checkVatApprox, "-ns1:checkVatApprox", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_url(soap, soap_endpoint, NULL), soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___ns1__checkVatApprox(soap, &soap_tmp___ns1__checkVatApprox, "-ns1:checkVatApprox", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!&ns2__checkVatApproxResponse)
		return soap_closesock(soap);
	ns2__checkVatApproxResponse.soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	ns2__checkVatApproxResponse.soap_get(soap, "ns2:checkVatApproxResponse", NULL);
	if (soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}
/* End of client proxy code */
