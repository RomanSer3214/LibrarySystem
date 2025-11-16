#include "LoginWindow.h"
#include "imgui.h"

void renderLoginWindow(DatabaseManager& db, UserSession& session)
{
    // Буфери робимо static, щоб не затиралися кожен кадр
    static char usernameBuf[64] = "";
    static char passwordBuf[64] = "";
    static std::string errorMessage = "";

    ImVec2 displaySize = ImGui::GetIO().DisplaySize;

    // Тінь на фоні (сірування)
    ImGui::GetBackgroundDrawList()->AddRectFilled(
        ImVec2(0,0), displaySize,
        IM_COL32(0,0,0,120) // чорний з прозорістю 120/255
    );
    

    ImGui::SetNextWindowSize(ImVec2(400, 250), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    ImGui::Begin("Авторизація", nullptr,
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoTitleBar);

    ImGui::Text("Вхід в систему бібліотеки");
    ImGui::Spacing();

    ImGui::InputText("Логін", usernameBuf, sizeof(usernameBuf));
    ImGui::InputText("Пароль", passwordBuf, sizeof(passwordBuf),
                     ImGuiInputTextFlags_Password);

    ImGui::Spacing();
    if (ImGui::Button("Увійти", ImVec2(120, 35))) {
        int uid = -1;

        if (db.validateLogin(usernameBuf, passwordBuf, uid)) {
            session.userId = uid;
            session.username = usernameBuf;
            session.isLoggedIn = true;
            errorMessage.clear();
        } else {
            errorMessage = "Невірний логін або пароль!";
            printf("Login failed!\n");
        }
    }

    if (!errorMessage.empty()) {
        ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "%s", errorMessage.c_str());
    }

    ImGui::End();
}

