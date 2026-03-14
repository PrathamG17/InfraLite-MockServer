#pragma once

#include <string>

struct User
{
    int userId;
    std::string username;
    std::string passwordHash;
    std::string role;
    std::string createdAt;
    int isActive;
};
