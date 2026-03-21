#pragma once
#include <string>
#include <map>
#include <jwt-cpp/jwt.h>
#include <picojson/picojson.h>

class JWTVerifier
{
private:
    std::string secretKey;
    std::string algorithm;

public:
    JWTVerifier(const std::string& secret, const std::string& algo);

    // Generate a signed JWT token with given claims
    std::string Generate(const std::map<std::string, std::string>& claims);

    // Verify the token signature and issuer
    bool Verify(const std::string& token);

    // Decode payload claims into a map
    std::map<std::string, std::string> Decode(const std::string& token);
};
