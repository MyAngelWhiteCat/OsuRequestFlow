#include <string>

#include "../../src/message.h"
#include "../../src/domain.h"

namespace test_badge_parser {

    void TestParsingReply() {
        std::string PRIVMSG_REPLY = "@badge-info=subscriber/50;badges=subscriber/48,premium/1;client-nonce=0a3cd65c0adb1d127e10f47f15d70119;color=#00D95D;display-name=TheNewVirus_;emotes=;first-msg=0;flags=;id=4081bc1a-db9a-4ef6-a664-b15e3c127c38;mod=0;reply-parent-display-name=KreyGasmed;reply-parent-msg-body=8%!!\sIT\sCOMING\sOUT\sTODAY;reply-parent-msg-id=6fa5de3e-9242-4962-a8bd-bc43b764dc05;reply-parent-user-id=87447781;reply-parent-user-login=kreygasmed;reply-thread-parent-display-name=KreyGasmed;reply-thread-parent-msg-id=6fa5de3e-9242-4962-a8bd-bc43b764dc05;reply-thread-parent-user-id=87447781;reply-thread-parent-user-login=kreygasmed;returning-chatter=0;room-id=26490481;subscriber=1;tmi-sent-ts=1763029658409;turbo=0;user-id=59203031;user-type= :thenewvirus_!thenewvirus_@thenewvirus_.tmi.twitch.tv PRIVMSG #summit1g :@KreyGasmed Pffttt how we tell him";
        auto split = irc::domain::Split(PRIVMSG_REPLY);
        //...
    }

    void TestParsingPRIVMSG() {
        std::string PRIVMSG = "@badge-info=;badges=broadcaster/1;client-nonce=459e3142897c7a22b7d275178f2259e0;color=#0000FF;display-name=lovingt3s;emote-only=1;emotes=62835:0-10;first-msg=0;flags=;id=885196de-cb67-427a-baa8-82f9b0fcd05f;mod=0;room-id=713936733;subscriber=0;tmi-sent-ts=1643904084794;turbo=0;user-id=713936733;user-type= :lovingt3s!lovingt3s@lovingt3s.tmi.twitch.tv PRIVMSG #lovingt3s :bleedPurple";
        auto split = irc::domain::Split(PRIVMSG);
        std::cout << split.size() << std::endl;
        irc::domain::Message msg(irc::domain::MessageType::PRIVMSG, std::string(split[4]), std::string(split[0]));
        auto badges = msg.GetBadges();
        std::cout << badges.at("color")[0].substr(1) << std::endl;
        //...
    }

    void RunTests() {
        TestParsingPRIVMSG();
    }

}