#include "MemberManager.h"
#include <algorithm>
#include <cstring>

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
        
        // Гарантуємо нуль-термінацію
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
    ImGui::InputText("Пошук", searchBuffer, sizeof(searchBuffer));
    if (ImGui::Button("Шукати")) {
        searchMembers(searchBuffer);
    }
    ImGui::SameLine();
    if (ImGui::Button("Очистити")) {
        memset(searchBuffer, 0, sizeof(searchBuffer));
        loadMembers();
    }
}

void MemberManager::renderMemberList() {
    if (ImGui::BeginTable("MembersTable", 5, 
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | 
        ImGuiTableFlags_ScrollY | ImGuiTableFlags_Sortable)) {
        
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Ім'я");
        ImGui::TableSetupColumn("Email");
        ImGui::TableSetupColumn("Телефон");
        ImGui::TableSetupColumn("Тип");
        ImGui::TableHeadersRow();
        
        // ВИПРАВЛЕНО: використання size_t замість int
        for (size_t i = 0; i < members.size(); i++) {
            const auto& member = members[i];
            ImGui::TableNextRow();
            
            if (static_cast<int>(i) == selectedMemberIndex) {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, 
                    ImGui::GetColorU32(ImGuiCol_Header));
            }
            
            if (ImGui::IsItemClicked()) {
                selectedMemberIndex = static_cast<int>(i);
            }
            
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", member.getId());
            
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", member.getName().c_str());
            
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", member.getEmail().c_str());
            
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%s", member.getPhone().c_str());
            
            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%s", member.getTypeString().c_str());
        }
        
        ImGui::EndTable();
    }
}

void MemberManager::renderAddMemberPopup() {
    ImGui::OpenPopup("Додати читача");
    if (ImGui::BeginPopupModal("Додати читача", &showAddMemberPopup)) {
        ImGui::InputText("Ім'я", nameBuffer, sizeof(nameBuffer));
        ImGui::InputText("Email", emailBuffer, sizeof(emailBuffer));
        ImGui::InputText("Телефон", phoneBuffer, sizeof(phoneBuffer));
        
        const char* memberTypes[] = { "Студент", "Викладач", "Зовнішній" };
        ImGui::Combo("Тип читача", &memberType, memberTypes, IM_ARRAYSIZE(memberTypes));
        
        if (ImGui::Button("Зберегти")) {
            addMember();
            showAddMemberPopup = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Скасувати")) {
            showAddMemberPopup = false;
        }
        
        ImGui::EndPopup();
    }
}

void MemberManager::renderEditMemberPopup() {
    ImGui::OpenPopup("Редагувати читача");
    if (ImGui::BeginPopupModal("Редагувати читача", &showEditMemberPopup)) {
        ImGui::InputText("Ім'я", nameBuffer, sizeof(nameBuffer));
        ImGui::InputText("Email", emailBuffer, sizeof(emailBuffer));
        ImGui::InputText("Телефон", phoneBuffer, sizeof(phoneBuffer));
        
        const char* memberTypes[] = { "Студент", "Викладач", "Зовнішній" };
        ImGui::Combo("Тип читача", &memberType, memberTypes, IM_ARRAYSIZE(memberTypes));
        
        if (ImGui::Button("Зберегти")) {
            editMember();
            showEditMemberPopup = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Скасувати")) {
            showEditMemberPopup = false;
        }
        
        ImGui::EndPopup();
    }
}

void MemberManager::addMember() {
    Member::Type type = static_cast<Member::Type>(memberType);
    Member newMember(0, nameBuffer, emailBuffer, type);
    newMember.setPhone(phoneBuffer);
    
    if (dbManager.addMember(newMember)) {
        loadMembers(); // Перезавантажити з БД для отримання правильного ID
    }
}

void MemberManager::editMember() {
    // ВИПРАВЛЕНО: використання size_t та видалення невикористовуваної змінної
    if (selectedMemberIndex >= 0 && static_cast<size_t>(selectedMemberIndex) < members.size()) {
        // Оновлення читача
        // У реальному проекті треба реалізувати updateMember в DatabaseManager
        // auto& member = members[selectedMemberIndex]; // Видалено невикористовувану змінну
    }
}

void MemberManager::deleteMember(int index) {
    // ВИПРАВЛЕНО: використання size_t
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
