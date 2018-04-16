#ifndef __HTTPREQUEST_HPP__
#define __HTTPREQUEST_HPP__    1

struct HttpRequest {
    const char *Host;
    const char *Url;
    const char *Type;
    unsigned int Length;
};

extern HttpRequest request[];

extern unsigned int MAXREQUEST;

#endif // __HTTPREQUEST_HPP__
