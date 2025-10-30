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

    void Client::Connect() {
        ec_.clear();

        ConnectionVisitor visitor(*this);
        std::visit(visitor, socket_);
        if (ec_) {
            ReportError(ec_, "Connection");
        }
    }

    void Client::Disconnect() {
        ec_.clear();

        DisconnectVisitor visitor(*this);
        std::visit(visitor, socket_);
        if (ec_) {
            ReportError(ec_, "Disconnecting");
        }
    }

    void Client::Join(const std::string_view chanel_name) {
        ec_.clear();

        JoinVisitor visitor(*this, chanel_name);
        std::visit(visitor, socket_);
        if (ec_) {
            ReportError(ec_, "Join");
        }
    }

    void Client::Authorize(const AuthorizeData& auth_data) {
        ec_.clear();

        AuthorizeVisitor visitor(*this, auth_data);
        std::visit(visitor, socket_);
        if (ec_) {
            ReportError(ec_, "Authorize"s);
            authorized_ = false;
        }
    }

    void Client::CapRequest() {
        ec_.clear();

        CapRequestVisitor visitor(*this);
        std::visit(visitor, socket_);
        if (ec_) {
            ReportError(ec_, "CapRequest");
        }
    }

    std::vector<Message> Client::Read() {
        ec_.clear();

        ReadMessageVisitor visitor(*this);
        return std::visit(visitor, socket_);
        if (ec_) {
            ReportError(ec_, "Reading");
            return {};
        }
    }

    bool Client::Connected() {
        CheckConnect();
        return ssl_connected_ || no_ssl_connected_;
    }

    void Client::Pong(std::string_view ball) {
        ec_.clear();

        PingPongVisitor visitor(*this, ball);
        std::visit(visitor, socket_);
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

