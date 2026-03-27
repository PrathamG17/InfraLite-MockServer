#include "JwtHandler.hpp"

JWTVerifier::JWTVerifier(const std::string& secret, const std::string& algo)
    : secretKey(secret), algorithm(algo) {
}

std::string JWTVerifier::Generate(const std::map<std::string, std::string>& claims)
{
    auto token = jwt::create()
        .set_issuer("your-issuer")
        .set_type("JWS")
        .set_algorithm(algorithm);

    for (const auto& kv : claims) {
        token.set_payload_claim(kv.first, jwt::claim(kv.second));
    }

    return token.sign(jwt::algorithm::hs256{ secretKey });
}

bool JWTVerifier::Verify(const std::string& token)
{
    try
    {
        auto decoded = jwt::decode(token);

        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{ secretKey })
            .with_issuer("your-issuer");

        verifier.verify(decoded);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

std::map<std::string, std::string> JWTVerifier::Decode(const std::string& token)
{
    std::map<std::string, std::string> claims;

    auto decoded = jwt::decode(token);
    const auto& obj = decoded.get_payload_json();

    for (const auto& kv : obj)
    {
        claims[kv.first] = kv.second.to_str();
    }

    return claims;
}
