#include "irc.h"

namespace irc {

    void ReportError(sys::error_code ec, std::string_view where) {
        auto code = ec.value();
        auto message = ec.message();
        std::cout << "[ERROR] While: " << where << " | with code: "
            << code << " | because: " << message << std::endl;
    }

    std::string AuthorizeData::GetAuthMessage() const {
        std::string data;
        data.append(domain::Command::PASS).append(token_);
        data.append("\r\n");
        data.append(domain::Command::NICK).append(nick_);
        data.append("\r\n");
        return data;
    }

    void AuthorizeData::SetNick(std::string_view nick) {
        nick_ = std::string(nick);
    }

    void AuthorizeData::SetToken(std::string_view token) {
        token_ = std::string(token);
    }

    Client::Client(net::io_context& ioc)
        : socket_(tcp::socket(ioc))
    {
    }

    Client::Client(net::io_context& ioc, ssl::context& ctx)
        : socket_(ssl::stream<tcp::socket>(ioc, ctx)) 
    {    
    }

    void Client::SSL_Connect() {
        Connect(true);
    }

    void Client::NOSSL_Connect() {
        Connect(false);
    }

    void Client::Disconnect() {
        ec_.clear();
        sys::error_code ignor;
        if (no_ssl_connected_) {
            socket_.shutdown(net::socket_base::shutdown_send, ignor);
            socket_.close(ec_);
            no_ssl_connected_ = false;
        }
        if (ssl_connected_) {
            ssl_socket_.shutdown(ignor);
            ssl_socket_.lowest_layer().close(ec_);
            ssl_connected_ = false;
        }
        if (ec_) {
            ReportError(ec_, "Disconnecting");
        }
    }

    void Client::Join(const std::string_view chanel_name) {
        ec_.clear();

        if (no_ssl_connected_) {
            net::write(socket_, net::buffer((std::string(domain::Command::JOIN_CHANNEL) + std::string(chanel_name) + "\r\n"s)), ec_);
        }
        else if (ssl_connected_) {
            net::write(ssl_socket_, net::buffer((std::string(domain::Command::JOIN_CHANNEL) + std::string(chanel_name) + "\r\n"s)), ec_);
        }
        else {
            throw std::runtime_error("Authorizing without connection attempt");
        }
        if (ec_) {
            ReportError(ec_, "Join");
        }
    }

    void Client::Authorize(const AuthorizeData& auth_data) {
        ec_.clear();

        if (no_ssl_connected_) {
            net::write(socket_, net::buffer(auth_data.GetAuthMessage()), ec_);
            authorized_ = true;
        }
        else if (ssl_connected_) {
            net::write(ssl_socket_, net::buffer(auth_data.GetAuthMessage()), ec_);
            authorized_ = true;
        }
        else {
            throw std::runtime_error("Authorizing without connection attempt");
        }
        if (ec_) {
            ReportError(ec_, "Authorize"s);
            authorized_ = false;
        }
    }

    void Client::CapRequest() {
        ec_.clear();
        // TODO: Optional capabilityes
        if (no_ssl_connected_) {
            net::write(socket_, net::buffer(std::string(domain::Command::CREQ)
                + std::string(domain::Capabilityes::COMMANDS) + " "
                + std::string(domain::Capabilityes::MEMBERSHIP) + " "
                + std::string(domain::Capabilityes::TAGS) + "\n\r"), ec_);
        }
        else {
            net::write(ssl_socket_, net::buffer(std::string(domain::Command::CREQ)
                + std::string(domain::Capabilityes::COMMANDS) + " "
                + std::string(domain::Capabilityes::MEMBERSHIP) + " "
                + std::string(domain::Capabilityes::TAGS) + "\n\r"), ec_);
        }
        // TODO: Check answer

        if (ec_) {
            ReportError(ec_, "CapRequest");
        }
    }

    std::vector<Message> Client::Read() {
        ec_.clear();

        net::streambuf streambuf;
        if (no_ssl_connected_) {
            net::read_until(socket_, streambuf, "\r\n"s, ec_);
        }
        else if (ssl_connected_) {
            net::read_until(ssl_socket_, streambuf, "\r\n"s, ec_);
        }
        else {
            throw std::runtime_error("Trying read socket without connection");
        }

        if (ec_) {
            ReportError(ec_, "Reading"s);
        }

        std::vector<Message> read_result;
        std::string line;
        std::istream is(&streambuf);
        while (std::getline(is, line)) {
            Message msg = IdentifyMessageType(line);
            if (msg.GetMessageType() == domain::MessageType::PING) {
                Pong(msg.GetContent());
            }
            read_result.push_back(msg);
        }

        return read_result;
    }

    bool Client::Connected() {
        CheckConnect();
        return ssl_connected_ || no_ssl_connected_;
    }

    void Client::Connect(bool secured) {
        ec_.clear();

        tcp::resolver resolver{ ioc_ };
        net::ip::basic_resolver_results<net::ip::tcp> endpoints;

        if (secured) {
            endpoints = resolver.resolve("irc.chat.twitch.tv", "6697", ec_);
        }
        else {
            endpoints = resolver.resolve("irc.chat.twitch.tv", "6667", ec_);
        }
        if (ec_) {
            ReportError(ec_, "Resolving");
        }

        if (secured) {
            net::connect(ssl_socket_.next_layer(), endpoints, ec_);
            ssl_connected_ = true;
        }
        else {
            net::connect(socket_, endpoints, ec_);
            no_ssl_connected_ = true;
        }
        if (ec_) {
            ReportError(ec_, "Connecting");
            no_ssl_connected_ = false;
            ssl_connected_ = false;
        }

        if (secured) {
            ssl_socket_.handshake(ssl::stream_base::client, ec_);
            if (ec_) {
                ReportError(ec_, "Handshake"s);
                ssl_connected_ = false;
            }
        }
    }

    void Client::Pong(std::string_view ball) {
        if (no_ssl_connected_) {
            net::write(socket_, net::buffer("PONG"s.append(ball.substr(4))), ec_);
        }
        else {
            net::write(ssl_socket_, net::buffer("PONG"s.append(ball.substr(4))), ec_);
        }
        if (ec_) {
            ReportError(ec_, "Sending PONG"s);
        }
    }

    void Client::CheckConnect() {
        ec_.clear();

        //TODO;
    }

    Message Client::IdentifyMessageType(std::string_view raw_message) {
        auto splited = domain::Split(raw_message);

        switch (splited.size()) {
        case (0):
            return std::move(Message(domain::MessageType::EMPTY, ""));
        case (2):
            if (splited[0] == domain::Command::PONG) {
                return std::move(Message(domain::MessageType::PING, std::move(std::string(splited[1]))));
            }
            else {
                return std::move(Message(domain::MessageType::UNKNOWN, std::move(std::string(raw_message))));
            }
            break;

        case (3):
            if (splited[1] == domain::Command::JOIN) {
                return std::move(Message(domain::MessageType::JOIN, std::move(std::string(splited[2]))));
            }
            if (splited[1] == domain::Command::PART) {
                return std::move(Message(domain::MessageType::PART, std::move(std::string(splited[2]))));
            }
            break;

        case (4):
            if (splited[2] == domain::Command::ROOMSTATE) {
                return std::move(Message(domain::MessageType::ROOMSTATE, std::move(std::string(splited[0]))));
            }
            if (domain::IsNumber(splited[1])) {
                return std::move(Message(domain::MessageType::STATUSCODE, std::move(std::string(splited[1]))));
            }

            throw std::runtime_error("Unknown message type"s);

        default:
            if (splited[2] == domain::Command::PRIVMSG) {
                std::string content;
                bool is_first = true;
                for (int i = 4; i < splited.size(); ++i) {
                    if (!is_first) {
                        content += ' ';
                    }
                    content += splited[i];
                    is_first = false;
                }
                return std::move(Message(domain::MessageType::PRIVMSG
                    , std::move(content)
                    , std::move(std::string(splited[0]))));
            }
            if (domain::IsNumber(splited[1])) {
                return std::move(Message(domain::MessageType::STATUSCODE, std::move(std::string(splited[1]))));
            }
            if (splited[1] == domain::Command::CRES) {
                return std::move(Message(domain::MessageType::CAPRES, std::move(std::string(raw_message)))); // Dummy
            }

            return std::move(Message(domain::MessageType::ROOMSTATE, std::move(std::string(raw_message)))); // Dummy
        }
        return std::move(Message(domain::MessageType::UNKNOWN, std::move(std::string(raw_message)))); // Dummy

    }


    Message::Message(domain::MessageType message_type, std::string&& content, std::string&& badges)
        : message_type_(message_type)
        , content_(std::move(content))
    {
        std::string badge;
        bool badge_seted = false;
        std::string value;
        for (const char ch : badges) {
            if (!badge_seted && ch != '=') {
                badge += ch;
                continue;
            }
            else {
                badge_seted = true;
            }
            if (ch == ',') {
                badges_[badge].push_back(value);
                value.clear();
            }
            else if (ch == ';') {
                badges_[badge].push_back(value);
                value.clear();
                badge.clear();
                badge_seted = false;
            }
            else if (ch != '=') {
                value += ch;
            }
        }
    }

    domain::MessageType Message::GetMessageType() const {
        return message_type_;
    }

    std::string_view Message::GetContent() const {
        return content_;
    }

    std::string_view Message::GetNick() const {
        if (message_type_ != domain::MessageType::PRIVMSG) {
            throw std::logic_error("Only PRIMSG can have nick");
        }
        std::string badge = "display-name";
        return badges_.at(badge)[0];
    }

    Badges Message::GetBadges() const {
        if (message_type_ != domain::MessageType::PRIVMSG) {
            throw std::logic_error("Only PRIMSG can have badges");
        }
        return badges_;
    }

} // namespace irc

