#include "MemberManager.h"
#include "imgui.h"

MemberManager::MemberManager(DatabaseManager& db) : dbManager(db) {
    loadMembers();
}

void MemberManager::render() {
    renderSearchBar();
    
    ImGui::Separator();
    
    if (ImGui::Button("Додати читача")) {
        showAddMemberPopup = true;
        memset(nameBuffer, 0, sizeof(nameBuffer));
        memset(emailBuffer, 0, sizeof(emailBuffer));
        memset(phoneBuffer, 0, sizeof(phoneBuffer));
        memberType = 0;
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Редагувати") && selectedMemberIndex >= 0) {
        showEditMemberPopup = true;
        const auto& member = members[selectedMemberIndex];
        
        strncpy(nameBuffer, member.getName().c_str(), sizeof(nameBuffer) - 1);
        strncpy(emailBuffer, member.getEmail().c_str(), sizeof(emailBuffer) - 1);
        strncpy(phoneBuffer, member.getPhone().c_str(), sizeof(phoneBuffer) - 1);
        
        // null-termination
        nameBuffer[sizeof(nameBuffer) - 1] = '\0';
        emailBuffer[sizeof(emailBuffer) - 1] = '\0';
        phoneBuffer[sizeof(phoneBuffer) - 1] = '\0';
        
        memberType = static_cast<int>(member.getType());
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Видалити") && selectedMemberIndex >= 0) {
        deleteMember(selectedMemberIndex);
    }
    
    ImGui::Spacing();
    
    renderMemberList();
    
    if (showAddMemberPopup) {
        renderAddMemberPopup();
    }
    
    if (showEditMemberPopup) {
        renderEditMemberPopup();
    }
}

void MemberManager::renderSearchBar() {
    ImGui::Text("Пошук:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(400);  
    ImGui::InputText("##search", searchBuffer, sizeof(searchBuffer));
    if (strlen(searchBuffer) > 0) {
        searchMembers(searchBuffer);
    }
}

void MemberManager::renderMemberList() {
    if (ImGui::BeginTable("MembersTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Ім'я");
        ImGui::TableSetupColumn("Email");
        ImGui::TableSetupColumn("Телефон");
        ImGui::TableSetupColumn("Тип");
        ImGui::TableHeadersRow();

        for (int i = 0; i < static_cast<int>(members.size()); i++) {
            const auto& member = members[i];

            ImGui::TableNextRow();

            // Рядок клікабельний через Selectable
            ImGui::TableSetColumnIndex(0);
            if (ImGui::Selectable(std::to_string(member.getId()).c_str(), selectedMemberIndex == i, ImGuiSelectableFlags_SpanAllColumns)) {
                selectedMemberIndex = i;
            }

            ImGui::TableSetColumnIndex(1); ImGui::Text("%s", member.getName().c_str());
            ImGui::TableSetColumnIndex(2); ImGui::Text("%s", member.getEmail().c_str());
            ImGui::TableSetColumnIndex(3); ImGui::Text("%s", member.getPhone().c_str());
            ImGui::TableSetColumnIndex(4); ImGui::Text("%s", member.getTypeString().c_str());
        }

        ImGui::EndTable();
    }
}


void MemberManager::renderAddMemberPopup() {
    static std::string addMemberError;
    static bool showAddMemberError = false;

    ImGui::OpenPopup("Додати читача");
    if (ImGui::BeginPopupModal("Додати читача", &showAddMemberPopup)) {
        ImGui::InputText("Ім'я", nameBuffer, sizeof(nameBuffer));
        ImGui::InputText("Email", emailBuffer, sizeof(emailBuffer));
        ImGui::InputText("Телефон", phoneBuffer, sizeof(phoneBuffer));
        
        const char* memberTypes[] = { "Студент", "Викладач", "Зовнішній" };
        ImGui::Combo("Тип читача", &memberType, memberTypes, IM_ARRAYSIZE(memberTypes));
        
        if (ImGui::Button("Зберегти")) {
            // validation: name required
            std::string name = nameBuffer;
            if (name.empty()) {
                addMemberError = "Ім'я обов'язкове.";
                showAddMemberError = true;
                ImGui::OpenPopup("Помилка додавання читача");
            } else {
                addMember();
                showAddMemberPopup = false;
                showAddMemberError = false;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Скасувати")) {
            showAddMemberPopup = false;
        }

        if (ImGui::BeginPopupModal("Помилка додавання читача", &showAddMemberError, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("%s", addMemberError.c_str());
            if (ImGui::Button("OK")) {
                ImGui::CloseCurrentPopup();
                showAddMemberError = false;
            }
            ImGui::EndPopup();
        }
        
        ImGui::EndPopup();
    }
}

void MemberManager::renderEditMemberPopup() {
    static std::string editMemberError;
    static bool showEditMemberError = false;

    ImGui::OpenPopup("Редагувати читача");
    if (ImGui::BeginPopupModal("Редагувати читача", &showEditMemberPopup)) {
        ImGui::InputText("Ім'я", nameBuffer, sizeof(nameBuffer));
        ImGui::InputText("Email", emailBuffer, sizeof(emailBuffer));
        ImGui::InputText("Телефон", phoneBuffer, sizeof(phoneBuffer));
        
        const char* memberTypes[] = { "Студент", "Викладач", "Зовнішній" };
        ImGui::Combo("Тип читача", &memberType, memberTypes, IM_ARRAYSIZE(memberTypes));
        
        if (ImGui::Button("Зберегти")) {
            std::string name = nameBuffer;
            if (name.empty()) {
                editMemberError = "Ім'я обов'язкове.";
                showEditMemberError = true;
                ImGui::OpenPopup("Помилка редагування читача");
            } else {
                editMember();
                showEditMemberPopup = false;
                showEditMemberError = false;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Скасувати")) {
            showEditMemberPopup = false;
        }

        if (ImGui::BeginPopupModal("Помилка редагування читача", &showEditMemberError, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("%s", editMemberError.c_str());
            if (ImGui::Button("OK")) {
                ImGui::CloseCurrentPopup();
                showEditMemberError = false;
            }
            ImGui::EndPopup();
        }
        
        ImGui::EndPopup();
    }
}

void MemberManager::addMember() {
    Member::Type type = static_cast<Member::Type>(memberType);
    Member newMember(0, nameBuffer, emailBuffer, phoneBuffer, type);
    
    if (dbManager.addMember(newMember)) {
        loadMembers(); // Перезавантажити з БД для отримання правильного ID
    }
}

void MemberManager::editMember() {
    if (selectedMemberIndex >= 0 && static_cast<size_t>(selectedMemberIndex) < members.size()) {
        const Member& old = members[selectedMemberIndex];
        int id = old.getId();
        Member::Type type = static_cast<Member::Type>(memberType);
        // preserve maxBooksAllowed from existing record
        int maxAllowed = old.getMaxBooksAllowed();
        Member updated(id, std::string(nameBuffer), std::string(emailBuffer), std::string(phoneBuffer), type, maxAllowed);

        if (dbManager.updateMember(updated)) {
            loadMembers();
        } else {
            // reload to reflect DB state if update failed
            loadMembers();
        }
    }
}

void MemberManager::deleteMember(int index) {
    if (index >= 0 && static_cast<size_t>(index) < members.size()) {
        int id = members[index].getId();
        members.erase(members.begin() + index);
        dbManager.deleteMember(id);
        selectedMemberIndex = -1;
    }
}

void MemberManager::loadMembers() {
    members = dbManager.getAllMembers();
}

void MemberManager::searchMembers(const std::string& query) {
    if (query.empty()) {
        loadMembers();
        return;
    }
    
    std::vector<Member> allMembers = dbManager.getAllMembers();
    members.clear();
    
    for (const auto& member : allMembers) {
        if (member.getName().find(query) != std::string::npos ||
            member.getEmail().find(query) != std::string::npos ||
            member.getPhone().find(query) != std::string::npos) {
            members.push_back(member);
        }
    }
}
