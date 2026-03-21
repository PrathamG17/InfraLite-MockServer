#pragma once
#include "request.hpp"
#include "JwtHandler.hpp"
#include <string>

enum class Role { Admin, QA, Viewer, Unknown };

class SecurityManager
{
private:
    JWTVerifier jwtVerifier;

public:
    SecurityManager(const JWTVerifier& verifier);

    bool Authorize(const HttpRequest& request, Role requiredRole);
    Role ExtractRole(const std::string& token);
};
