#pragma once
#include "imgui.h"
#include <vector>
#include <string>
#include "../core/Member.h"
#include "../database/DatabaseManager.h"

class MemberManager {
private:
    std::vector<Member> members;
    DatabaseManager& dbManager;
    
    // Стан для UI
    char searchBuffer[256] = "";
    char nameBuffer[256] = "";
    char emailBuffer[256] = "";
    char phoneBuffer[256] = "";
    int memberType = 0; // 0=Student, 1=Faculty, 2=External
    
    bool showAddMemberPopup = false;
    bool showEditMemberPopup = false;
    int selectedMemberIndex = -1;

public:
    MemberManager(DatabaseManager& db);
    
    void render();
    void loadMembers();

private:
    void renderMemberList();
    void renderAddMemberPopup();
    void renderEditMemberPopup();
    void renderSearchBar();
    
    void addMember();
    void editMember();
    void deleteMember(int index);
    void searchMembers(const std::string& query);
};
