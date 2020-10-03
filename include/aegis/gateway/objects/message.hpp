//
// message.hpp
// ***********
//
// Copyright (c) 2020 Sharon Fox (sharon at xandium dot io)
//
// Distributed under the MIT License. (See accompanying file LICENSE)
//
// Revision by LSW
//

#pragma once

#include "aegis/config.hpp"
#include "aegis/snowflake.hpp"
#include "aegis/shards/shard.hpp"
#include "aegis/rest/rest_reply.hpp"
#include "aegis/gateway/objects/attachment.hpp"
#include "aegis/gateway/objects/embed.hpp"
#include "aegis/gateway/objects/reaction.hpp"
#include "aegis/gateway/objects/user.hpp"
#include <nlohmann/json.hpp>
#include "lsw/future_mod.h"
//#include "aegis/futures.hpp"

namespace aegis
{

    struct edit_message_t;

    namespace gateway
    {

        namespace objects
        {

            class message;

            /// \cond TEMPLATES
            AEGIS_DECL void from_json(const nlohmann::json& j, objects::message& m);

            AEGIS_DECL void to_json(nlohmann::json& j, const objects::message& m);
            /// \endcond

            /// Type of message
            enum message_type
            {
                Default = 0,
                RecipientAdd = 1,
                RecipientRemove = 2,
                Call = 3,
                ChannelNameChange = 4,
                ChannelIconChange = 5,
                ChannelPinnedMessage = 6,
                GuildMemberJoin = 7
            };

            class message
            {
            public:
                /// Constructor for the message object
                /**
                 * @param content String of the message sent
                 * @param _channel Pointer of channel object
                 * @param _guild Pointer of guild object
                 */
                explicit message(const std::string& content, aegis::channel* _channel, aegis::guild* _guild) noexcept
                    : _content(content)
                    , _channel(_channel)
                    , _guild(_guild)
                {
                }

                /// Constructor for the message object
                /**
                 * @param _json JSON string of the message object
                 * @param bot Pointer of core object
                 */
                message(const std::string& _json, aegis::core* _core) noexcept
                    : _core(_core)
                {
                    from_json(nlohmann::json::parse(_json), *this);
                    populate_self();
                }

                /// Constructor for the message object
                /**
                 * @param _json JSON object of the message object
                 * @param _core Pointer of core object
                 */
                message(const nlohmann::json& _json, aegis::core* _core) noexcept
                    : _core(_core)
                {
                    from_json(_json, *this);
                    populate_self();
                }

                /// Constructor for the message object
                /**
                 * @param _core Pointer of core object
                 */
                message(aegis::core* _core) noexcept
                    : _core(_core)
                {
                    populate_self();
                }

                /// Set the channel of the message object. This is mostly an internal function
                /// though is left public for lower level use
                /**
                 * @param _channel Pointer of the channel to assign the message to
                 */
                AEGIS_DECL void set_channel(aegis::channel* _channel)
                {
                    this->_channel = _channel;
                }

                /// Set the guild of the message object. This is mostly an internal function
                /// though is left public for lower level use
                /**
                 * @param _guild Pointer of the guild to assign the message to
                 */
                AEGIS_DECL void set_guild(aegis::guild* _guild)
                {
                    this->_guild = _guild;
                }

                message() = default;
                message& operator=(const message&) = default;
                message(const message&) = default;
                message(message&& msg) = default;

                /// Comparison of message content
                inline bool operator==(const std::string& rhs)
                {
                    return _content == rhs;
                }

                /// Comparison of message content
                inline bool operator!=(const std::string& rhs)
                {
                    return !(*this == rhs);
                }

                /// Comparison of message content
                inline bool operator==(const char* rhs)
                {
                    return _content == rhs;
                }

                /// Comparison of message content
                inline bool operator!=(const char* rhs)
                {
                    return !(*this == rhs);
                }

                std::string timestamp; /**< ISO8601 timestamp of when this message was sent */
                std::string edited_timestamp; /**< ISO8601 timestamp of when this message was last edited */
                bool tts = false; /**< Whether this is a TTS message */
                bool mention_everyone = false; /**< Whether this message mentions everyone */
                std::vector<snowflake> mentions; /**< Users specifically mentioned in the message */
                std::vector<snowflake> mention_roles; /**< Roles specifically mentioned in the message */
                std::vector<objects::attachment> attachments; /**< Any attached files */
                std::vector<objects::embed> embeds; /**< Any embedded content */
                bool pinned = false; /**< Whether this message is pinned */
                std::vector<objects::reaction> reactions; /**< Reactions to the message */
                snowflake nonce; /**< Used for validating that a message was sent */
                std::string webhook_id; /**< If this message was generated by the webhook, this is the webhook's ID */
                objects::message_type type = Default; /**< The type of this message */
                user author; /**< Author of this message */

                /// Whether message is a Direct Message
                /**
                 * @returns bool
                 */
                bool is_dm() const noexcept
                {
                    return !_guild_id;
                }

                /// Whether message is from a bot
                /**
                 * @returns bool
                 */
                bool is_bot() const noexcept
                {
                    return author.is_bot();
                }

                /// Whether message is from a webhook
                /**
                 * @returns bool
                 */
                bool is_webhook() const noexcept
                {
                    return !webhook_id.empty();
                }

                /// Get a reference to the content of the message
                /**
                 * @returns const std::string &
                 */
                const std::string& get_content() const noexcept
                {
                    return _content;
                }

                /// Set the message's content
                /**
                 * @param content String of content to set
                 */
                void set_content(const std::string& content) noexcept
                {
                    _content = content;
                }

                /// Get the ID of the message
                /**
                 * @returns aegis::snowflake
                 */
                snowflake get_id() const noexcept
                {
                    return _message_id;
                }

                /// Get the Channel ID of the message
                /**
                 * @returns aegis::snowflake
                 */
                snowflake get_channel_id() const noexcept
                {
                    return _channel_id;
                }

                /// Get the Guild ID of the message
                /**
                 * @returns aegis::snowflake
                 */
                snowflake get_guild_id() const noexcept
                {
                    return _guild_id;
                }

                /// Get the Member ID of the author of the message
                /**
                 * @returns aegis::snowflake
                 */
                snowflake get_author_id() const noexcept
                {
                    return _author_id;
                }

                /// Whether the message has a valid guild set. The result of this function
                /// does not determine whether the object was a DM or not
                /// @see is_dm
                /**
                 * @returns bool
                 */
                bool has_guild() const noexcept
                {
                    return _guild != nullptr || _guild_id != 0;
                }

                /// Whether the message has a valid channel set
                /**
                 * @returns bool
                 */
                bool has_channel() const noexcept
                {
                    return _channel != nullptr || _channel_id != 0;
                }

#if !defined(AEGIS_DISABLE_ALL_CACHE)
                /// Whether the message has a valid member set
                /**
                 * @returns bool
                 */
                bool has_member() const noexcept
                {
                    return _user != nullptr || _author_id != 0;
                }
#endif

                /// Get a reference to the guild object this message was sent in
                /**
                 * @returns aegis::guild
                 */
                AEGIS_DECL aegis::guild& get_guild();

                /// Get a reference to the channel object this message was sent in
                /**
                 * @returns aegis::channel
                 */
                AEGIS_DECL aegis::channel& get_channel();

#if !defined(AEGIS_DISABLE_ALL_CACHE)
                /// Get a reference to the user object this message was sent by
                /**
                 * @returns aegis::user
                 */
                AEGIS_DECL aegis::user& get_user();
#endif

                /// Delete this message
                /**
                 * @returns LSW::v5::Tools::Future<rest::rest_reply>
                 */
                AEGIS_DECL LSW::v5::Tools::Future<rest::rest_reply> delete_message();

                /// Edit this message
                /**
                 * @param content String to set the new content to
                 * @returns LSW::v5::Tools::Future<message>
                 */
                AEGIS_DECL LSW::v5::Tools::Future<message> edit(const std::string& content);

                /// Edit this message
                /**
                 * @see edit_message_t
                 * @param obj Struct of the edit message request
                 * @returns LSW::v5::Tools::Future<message>
                 */
                AEGIS_DECL LSW::v5::Tools::Future<message> edit(edit_message_t& obj);

                /// Add a reaction to this message
                /**
                 * @param content String of the emoji to add. Unicode emoji or `emojiname:emoji_id`
                 * @returns LSW::v5::Tools::Future<rest::rest_reply>
                 */
                AEGIS_DECL LSW::v5::Tools::Future<rest::rest_reply> create_reaction(const std::string& content);

                /// Delete your reaction to this message
                /**
                 * @param content String of the emoji to set. Unicode emoji or `emojiname:emoji_id`
                 * @returns LSW::v5::Tools::Future<rest::rest_reply>
                 */
                AEGIS_DECL LSW::v5::Tools::Future<rest::rest_reply> delete_own_reaction(const std::string& content);

                /// Delete other user reaction to this message (not available for DMs)
                /**
                 * @param content String of the emoji to set. Unicode emoji or `emojiname:emoji_id`
                 * @param member_id Snowflake of the member to delete the reaction for
                 * @returns LSW::v5::Tools::Future<rest::rest_reply>
                 */
                AEGIS_DECL LSW::v5::Tools::Future<rest::rest_reply> delete_user_reaction(const std::string& content, const snowflake member_id);

                /// Delete all reactions on this message (not available for DMs)
                /**
                 * @returns LSW::v5::Tools::Future<rest::rest_reply>
                 */
                AEGIS_DECL LSW::v5::Tools::Future<rest::rest_reply> delete_all_reactions();

                /// Obtain the relevant snowflakes related to this message
                /**
                 * Returns { channel_id, guild_id, message_id, author_id }
                 * Some may be 0 such as guild for a DM or author for a webhook
                 * @returns std::tuple<aegis::snowflake, aegis::snowflake, aegis::snowflake, aegis::snowflake>
                 */
                std::tuple<snowflake, snowflake, snowflake, snowflake> get_related_ids() const noexcept
                {
                    return std::tuple<snowflake, snowflake, snowflake, snowflake>{ _channel_id, _guild_id, _message_id, _author_id };
                };

            private:
                friend AEGIS_DECL void from_json(const nlohmann::json& j, objects::message& m);
                friend AEGIS_DECL void to_json(nlohmann::json& j, const objects::message& m);
                friend class aegis::core;

                AEGIS_DECL void populate_self();

                std::string _content;/**< String of the message contents */
                aegis::channel* _channel = nullptr;/**< Pointer to the channel this message belongs to */
                aegis::guild* _guild = nullptr;/**< Pointer to the guild this message belongs to */
#if !defined(AEGIS_DISABLE_ALL_CACHE)
                aegis::user* _user = nullptr;/**< Pointer to the author of this message */
#endif
                aegis::core* _core = nullptr;
                snowflake _message_id = 0; /**< snowflake of the message */
                snowflake _channel_id = 0; /**< snowflake of the channel this message belongs to */
                snowflake _guild_id = 0; /**< snowflake of the guild this message belongs to */
                snowflake _author_id = 0; /**< snowflake of the author of this message */
            };

        }

    }

}

#if defined(AEGIS_HEADER_ONLY)
#include "aegis/gateway/objects/impl/message.cpp"
#endif
