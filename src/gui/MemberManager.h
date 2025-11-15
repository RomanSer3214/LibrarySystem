#pragma once
#include <vector>
#include <string>
#include "../core/Member.h"
#include "../database/DatabaseManager.h"

class MemberManager {
private:
    DatabaseManager& dbManager;
    std::vector<Member> members;
    
    // Стан для UI
    char searchBuffer[200] = "";
    char nameBuffer[100] = "";
    char emailBuffer[100] = "";
    char phoneBuffer[30] = "";
    int memberType = 0; // 0=Student, 1=Faculty, 2=External
    
    bool showAddMemberPopup = false;
    bool showEditMemberPopup = false;
    int selectedMemberIndex = -1;

    void renderMemberList();
    void renderAddMemberPopup();
    void renderEditMemberPopup();
    void renderSearchBar();
    
    void addMember();
    void editMember();
    void deleteMember(int index);
    void searchMembers(const std::string& query);

public:
    MemberManager(DatabaseManager& db);
    
    void render();
    void loadMembers();
};
