#pragma once
#include <string>
#include <openssl/ssl.h>
#include <openssl/err.h>

class TLSContext
{
private:
    std::string certFile;
    std::string keyFile;
    SSL_CTX* ctx;

public:
    TLSContext(const std::string& cert, const std::string& key);
    ~TLSContext();

    bool Init();
    SSL_CTX* GetCTX() const;
};
