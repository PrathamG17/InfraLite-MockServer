#pragma once

#include <vector>
#include "database_handler.hpp"
#include "db/user.hpp"

class UserRepository
{
    private :
        DatabaseHandler* m_pDB;

    public :
        UserRepository(DatabaseHandler* db);

        bool CreateTable();

        int AddUser(const std::string& username,
                    const std::string& passwordHash,
                    const std::string& role);

        std::vector<User> GetUsers();
};
