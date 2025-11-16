#pragma once
#include "../database/DatabaseManager.h"

struct UserSession {
    int userId = -1;
    std::string username;
    bool isLoggedIn = false;
};

void renderLoginWindow(DatabaseManager& db, UserSession& session);

