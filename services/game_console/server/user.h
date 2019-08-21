#pragma once
#include <mutex>
#include <list>
#include "misc.h"
#include "notification.h"
#include "team.h"

struct User
{
    static const uint32_t kNotificationQueueSize = 32;

    User() = delete;
    User(const std::string& name, const std::string& password, Team* team);

    Team* GetTeam();

    void SetAuthKey(uint32_t authKey);
    uint32_t GetAuthKey() const;
    uint32_t GenerateAuthKey();

    const std::string& GetName() const;

    const std::string& GetPassword() const;
    void ChangePassword(const std::string& newPassword);

    void SetIPAddr(IPAddr ipAddr);
    IPAddr GetIPAddr() const;

    bool AddNotification(Notification* n);
    Notification* GetNotification();
    uint32_t GetNotificationsInQueue();
    void Update();
    void SetNotifySocket(int sock);

    void DumpStats(std::string& out, IPAddr hwConsoleIp) const;

private:
    std::string m_name;
    std::string m_password;
    Team* m_team = nullptr;
    IPAddr m_ipAddr = ~0u;
    AuthKey m_authKey = kInvalidAuthKey;
    std::list<Notification*> m_notifications;
    int m_notifySocket = -1;
    float m_lastUserNotifyTime = 0.0f;

    void NotifyUser();
};