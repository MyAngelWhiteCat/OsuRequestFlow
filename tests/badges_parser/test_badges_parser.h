#include <string>

#include "../../src/message.h"
#include "../../src/domain.h"

namespace test_badge_parser {

    void TestParsingPRIVMSG(std::string PRIVMSG) {
        auto split = irc::domain::Split(PRIVMSG);
        irc::domain::Message msg(irc::domain::MessageType::PRIVMSG, std::string(split[4]), std::string(split[0]));
        auto badges = msg.GetBadges();
        /*for (const auto& [badge, values] : badges) {
            std::cout << "Badge: " << badge << "\n";
            for (const auto& value : values) {
                std::cout << value << "\n";
            }
            std::cout << "\n";
        }*/
        std::cout << "Main badge: " << static_cast<int>(msg.GetRole()) << std::endl;
    }

    void RunTests() {
        std::string user = "@badge-info=;badges=;client-nonce=a23479049666f973233e19c1f6b98175;color=;display-name=EblAde1_;emotes=;first-msg=0;flags=;id=bbf84ad6-2d6b-4315-9407-9e3ce3a0ff93;mod=0;returning-chatter=0;room-id=124613167;subscriber=0;tmi-sent-ts=1764437356751;turbo=0;user-id=799789864;user-type= :eblade1_!eblade1_@eblade1_.tmi.twitch.tv PRIVMSG #myangelwhitecat :as_user";
        std::string vip = "@badge-info=;badges=vip/1;client-nonce=f0ed63a2c802825d8f6965049c2b089b;color=;display-name=EblAde1_;emotes=;first-msg=0;flags=;id=9819d1f5-2c64-4dea-b7be-1f5031f2eb75;mod=0;returning-chatter=0;room-id=124613167;subscriber=0;tmi-sent-ts=1764437369605;turbo=0;user-id=799789864;user-type=;vip=1 :eblade1_!eblade1_@eblade1_.tmi.twitch.tv PRIVMSG #myangelwhitecat :as_vip";
        std::string mode = "@badge-info=;badges=moderator/1;client-nonce=54ac998612e0f2378e6d820eb6a943fe;color=;display-name=EblAde1_;emotes=;first-msg=0;flags=;id=b0198b24-6794-4133-98d9-933f4f703f19;mod=1;returning-chatter=0;room-id=124613167;subscriber=0;tmi-sent-ts=1764437383564;turbo=0;user-id=799789864;user-type=mod :eblade1_!eblade1_@eblade1_.tmi.twitch.tv PRIVMSG #myangelwhitecat :as_moderator";
        std::string broadcaster = "@badge-info=subscriber/54;badges=broadcaster/1,subscriber/0;client-nonce=ddca48da514181768574e969c51295a2;color=#0000FF;display-name=MyAngeIWhiteCat;emotes=;first-msg=0;flags=;id=6ece678a-b92a-44ff-a805-193db5dcc8ed;mod=0;returning-chatter=0;room-id=124613167;subscriber=1;tmi-sent-ts=1764437336631;turbo=0;user-id=124613167;user-type= :myangelwhitecat!myangelwhitecat@myangelwhitecat.tmi.twitch.tv PRIVMSG #myangelwhitecat :as_broadcaster";
        std::cout << "-----------------------USER---------------------------" << std::endl;
        TestParsingPRIVMSG(user);
        std::cout << "-----------------------VIP---------------------------" << std::endl;
        TestParsingPRIVMSG(vip);
        std::cout << "-----------------------MODERATOR---------------------------" << std::endl;
        TestParsingPRIVMSG(mode);
        std::cout << "-----------------------BROADCASTER---------------------------" << std::endl;
        TestParsingPRIVMSG(broadcaster);
    }

}