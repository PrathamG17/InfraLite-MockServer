#include "TLSContext.hpp"

TLSContext::TLSContext(const std::string& cert, const std::string& key): certFile(cert), keyFile(key), ctx(nullptr) {}

TLSContext::~TLSContext() 
{
    if (ctx) 
    {
        SSL_CTX_free(ctx);
        ctx = nullptr;
    }
}

bool TLSContext::Init() 
{
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();

    ctx = SSL_CTX_new(TLS_server_method());
    if (!ctx) 
    {
        return false;
    }

    //if (SSL_CTX_use_certificate_file(ctx, certFile.c_str(), SSL_FILETYPE_PEM) <= 0) 
    //{
    //    return false;
    //}
    //if (SSL_CTX_use_PrivateKey_file(ctx, keyFile.c_str(), SSL_FILETYPE_PEM) <= 0) 
    //{
    //    return false;
    //}

    if (SSL_CTX_use_certificate_file(ctx, certFile.c_str(), SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        return false;
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, keyFile.c_str(), SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        return false;
    }

    return true;
}

SSL_CTX* TLSContext::GetCTX() const 
{
    return ctx;
}
