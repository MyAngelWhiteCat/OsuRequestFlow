#include "../../src/command_parser.h"
#include <iostream>

namespace test_command_parser {

    void TestOsuLink(commands::CommandParser& parser) {
        std::string link = "https://osu.ppy.sh/beatmapsets/2440776#osu/5324309";
        std::string protocol = parser.GetProtocol(link);
        std::string domain = parser.GetDomain(link, protocol);
        std::string catalogue = parser.GetCatalogue(link, protocol, domain);
        std::string game_mode = parser.GetGameMode(link, protocol, domain, catalogue);
        std::string map_id = parser.GetOsuMapID(link);
        std::cout << protocol << "\n" << domain << "\n" << catalogue << "\n" << game_mode << "\n" << map_id << std::endl;
    }

    void RunTest() {
        commands::CommandParser parser;
        TestOsuLink(parser);
    }

}