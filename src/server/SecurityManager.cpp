#include "SecurityManager.hpp"

SecurityManager::SecurityManager(const JWTVerifier& verifier)
    : jwtVerifier(verifier) {
}

bool SecurityManager::Authorize(const HttpRequest& request, Role requiredRole)
{
    std::string token = request.GetHeader("Authorization");
    if (token.empty()) return false;

    const std::string bearerPrefix = "Bearer ";
    if (token.rfind(bearerPrefix, 0) == 0)
        token = token.substr(bearerPrefix.size());

    if (!jwtVerifier.Verify(token)) return false;

    Role userRole = ExtractRole(token);
    return userRole == requiredRole;
}

Role SecurityManager::ExtractRole(const std::string& token) {
    auto claims = jwtVerifier.Decode(token);
    auto it = claims.find("role");
    if (it == claims.end()) return Role::Unknown;

    if (it->second == "Admin") return Role::Admin;
    if (it->second == "QA") return Role::QA;
    if (it->second == "Viewer") return Role::Viewer;
    return Role::Unknown;
}
