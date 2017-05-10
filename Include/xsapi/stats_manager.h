// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once
#include "xsapi/leaderboard.h"
#include "xsapi/social.h"
#include "xsapi/user_statistics.h"
#include "xsapi/achievements.h"

namespace xbox { namespace services { namespace stats { namespace manager { 

#if TV_API | XBOX_UWP
    typedef  Windows::Xbox::System::User^ xbox_live_user_t;
#else
    typedef std::shared_ptr<xbox::services::system::xbox_live_user> xbox_live_user_t;
#endif

class stats_manager_impl;
static const uint8_t STAT_PRESENCE_CHARS_NUM = 64;

/// <summary> 
/// Represents data for stat
/// internal representation
/// </summary>
union stat_data
{
    double numberType;
    char_t stringType[STAT_PRESENCE_CHARS_NUM];
};

/// <summary> 
/// Type of data the stat is
/// </summary>
enum class stat_data_type
{
    undefined,
    number,
    string
};

/// <summary> 
/// Type of stat change event
/// </summary>
enum class stat_event_type
{
    /// <summary> 
    /// A local user has been added
    /// </summary>
    local_user_added,

    /// <summary> 
    /// A local user has been removed
    /// </summary>
    local_user_removed,

    /// <summary> 
    /// stat has been updated
    /// </summary>
    stat_update_complete,

    /// <summary> 
    /// cast event args to leaderboard_result_event_args
    /// </summary>
    get_leaderboard_complete 
};

/// <summary> 
/// Represents a simplified stat
/// </summary>
class stat_value
{
public:
    /// <summary> 
    /// Name of the statistic
    /// </summary>
    /// <returns>A stat container that has the stat name string</returns>
    _XSAPIIMP const string_t name() const;

    /// <summary> 
    /// Return data as numerical type
    /// </summary>
    /// <returns>Float data for statistic</returns>
    /// <remarks>Will debug assert if data is not type requested</remarks>
    _XSAPIIMP double as_number() const;

    /// <summary> 
    /// Return data as integer type
    /// </summary>
    /// <returns>Float data for statistic</returns>
    /// <remarks>Will debug assert if data is not type requested</remarks>
    _XSAPIIMP int64_t as_integer() const;

    /// <summary> 
    /// Return data as string type
    /// </summary>
    /// <returns>data as char_t*</returns>
    /// <remarks>Will debug assert if data is not type requested</remarks>
    _XSAPIIMP const string_t as_string() const;

    /// <summary> 
    /// Return type of data the data object is
    /// </summary>
    /// <returns>Stat data type</returns>
    _XSAPIIMP stat_data_type data_type() const;

    /// Internal function
    static xbox_live_result<stat_value> _Deserialize(_In_ const web::json::value& data);

    /// Internal function
    stat_value();

private:
    void set_stat(
        _In_ double value
        );

    void set_stat(
        _In_ const char_t* value
        );

    void set_name(
    _In_ const string_t& name
    );

    web::json::value serialize() const;

    stat_data_type m_dataType;
    xbox_live_user_t m_localUserOwner;
    char_t m_name[STAT_PRESENCE_CHARS_NUM];
    stat_data m_statData;
    std::function<void(stat_value)> m_callbackFunc;
    friend class stats_value_document;
};

struct stat_event_args 
{
    virtual ~stat_event_args() {}
};

class leaderboard_result_event_args : public stat_event_args
{
public:
    /// <summary> 
    /// Gets the leaderboard result from a leaderboard request
    /// </summary>
    _XSAPIIMP const xbox_live_result<leaderboard::leaderboard_result>& result();

    /// <summary> 
    /// Internal function
    /// </summary>
    leaderboard_result_event_args(const xbox_live_result<leaderboard::leaderboard_result>& result);

private:
    xbox_live_result<leaderboard::leaderboard_result> m_result;
};

class stat_event
{
public:
    /// <summary> 
    /// Represents error code and error message
    /// </summary>
    /// <return>The returned xbox live result</return>
    _XSAPIIMP xbox_live_result<void>& error_info();

    /// <summary> 
    /// The type of event the statistic is
    /// </summary>
    /// <return>The event type</return>
    _XSAPIIMP stat_event_type event_type() const;

    /// <summary> 
    /// The data of event from stats manager
    /// You need to cast this to one of the event arg classes to retrieve the data for that particular event
    /// </summary>
    _XSAPIIMP std::shared_ptr<stat_event_args> event_args() const;

    /// <summary> 
    /// Local user the event is for
    /// </summary>
    /// <return>The returned user</return>
    _XSAPIIMP const xbox_live_user_t& local_user() const;

    /// Internal function
    stat_event(
        stat_event_type eventType,
        xbox_live_user_t user,
        xbox_live_result<void> errorInfo,
        std::shared_ptr<stat_event_args> args = nullptr
        );

private:
    stat_event_type m_eventType;
    std::shared_ptr<stat_event_args> m_eventArgs;
    xbox_live_user_t m_localUser;
    xbox_live_result<void> m_errorInfo;
};

/// <summary> 
/// Stats Manager handles and writes to the service a local users stats
/// </summary>
class stats_manager
{
public:
    /// <summary> 
    /// Instantiates and returns an instance of stats manager
    /// </summary>
    _XSAPIIMP static std::shared_ptr<stats_manager> get_singleton_instance();

    /// <summary> 
    /// Adds a local user to the stats manager
    /// Returns a local_user_added event from do_work
    /// </summary>
    /// <param name="user">The user to add to the statistic manager</param>
    _XSAPIIMP xbox_live_result<void> add_local_user(
        _In_ const xbox_live_user_t& user
        );

    /// <summary> 
    /// Removes a local user from the stats manager
    /// Returns a local_user_removed event from do_work
    /// </summary>
    /// <param name="user">The user to be removed from the statistic manager</param>
    _XSAPIIMP xbox_live_result<void> remove_local_user(
        _In_ const xbox_live_user_t& user
        );

    /// <summary> 
    /// Requests the current stat values to be uploaded to the service
    /// This will send immediately instead of automatically during a 30 second window
    /// </summary>
    /// <remarks>This will be throttled if called too often</remarks>
    _XSAPIIMP xbox_live_result<void> request_flush_to_service(
        _In_ const xbox_live_user_t& user,
        _In_ bool isHighPriority = false
        );

    /// <summary> 
    /// Returns any events that have been processed
    /// </summary>
    /// <return>A list of events that have happened since previous do_work</return>
    _XSAPIIMP std::vector<stat_event> do_work();

    /// <summary> 
    /// Replaces the numerical stat by the value. Can be positive or negative
    /// </summary>
    /// <param name="user">The local user whose stats to access</param>
    /// <param name="statName">The name of the statistic to modify</param>
    /// <param name="statValue">Value to replace the stat by</param>
    /// <return>Whether or not the setting was successful. Can fail if stat is not of numerical type. Will return updated stat</return>
    _XSAPIIMP xbox_live_result<void> set_stat_as_number(
        _In_ const xbox_live_user_t& user,
        _In_ const string_t& statName,
        _In_ double statValue
        );

    /// <summary> 
    /// Replaces the numerical stat by the value. Can be positive or negative
    /// </summary>
    /// <param name="user">The local user whose stats to access</param>
    /// <param name="statName">The name of the statistic to modify</param>
    /// <param name="statValue">Value to replace the stat by</param>
    /// <return>Whether or not the setting was successful. Can fail if stat is not of numerical type. Will return updated stat</return>
    _XSAPIIMP xbox_live_result<void> set_stat_as_integer(
        _In_ const xbox_live_user_t& user,
        _In_ const string_t& statName,
        _In_ int64_t statValue
        );

    /// <summary> 
    /// Replaces a string stat with the given value.
    /// </summary>
    /// <param name="user">The local user whose stats to access</param>
    /// <param name="statName">The name of the statistic to modify</param>
    /// <param name="statValue">Value to replace the stat by</param>
    /// <return>Whether or not the setting was successful. Can fail if stat is not of string type. Will return updated stat</return>
    _XSAPIIMP xbox_live_result<void> set_stat_as_string(
        _In_ const xbox_live_user_t& user,
        _In_ const string_t& statName,
        _In_ const string_t& statValue
        );

    /// <summary> 
    /// Gets all stat names in the stat document.
    /// </summary>
    /// <param name="user">The local user whose stats to access</param>
    /// <param name="statNameList">The list to fill with stat names</param>
    /// <return>Whether or not the setting was successful.</return>
    _XSAPIIMP xbox_live_result<void> get_stat_names(
        _In_ const xbox_live_user_t& user,
        _Inout_ std::vector<string_t>& statNameList
        );

    /// <summary> 
    /// Gets a stat value
    /// </summary>
    /// <param name="user">The local user whose stats to access</param>
    /// <param name="statName">The name of the statistic to modify</param>
    /// <return>Whether or not the setting was successful along with updated stat</return>
    _XSAPIIMP xbox_live_result<stat_value> get_stat(
        _In_ const xbox_live_user_t& user,
        _In_ const string_t& statName
        );

    /// <summary> 
    /// Deletes a stat. Will clear stat from service and social leaderboard information
    /// </summary>
    /// <param name="user">The local user whose stats to access</param>
    /// <param name="statName">The name of the statistic to delete</param>
    /// <return>Whether or not the stat deletion was successful</return>
    _XSAPIIMP xbox_live_result<void> delete_stat(
        _In_ const xbox_live_user_t& user,
        _In_ const string_t& statName
        );

    _XSAPIIMP stats_manager();

    /// <summary> 
    /// Starts a request for a global leaderboard. You can retrieve the resulting data by checking
    /// the events returned from do_work for an event of type get_leaderboard_complete
    /// Use leaderboard_query::get_next_query() to retrieve more data about this leaderboard.
    /// </summary>
    /// <param name="user">The local user whose stats to access</param>
    /// <param name="statName">The name of the statistic to get the leaderboard of</param>
    /// <param name="query">The query parameters of the leaderboard request</param>
    /// <return>Whether or not the leaderboard request was started correctly</return>
    _XSAPIIMP xbox_live_result<void> get_leaderboard(
        _In_ const xbox_live_user_t& user, 
        _In_ const string_t& statName, 
        _In_ leaderboard::leaderboard_query query
        );

    /// <summary> 
    /// Starts a request for a social leaderboard. You can retrieve the resulting data by checking
    /// the events returned from do_work for an event of type get_leaderboard_complete
    /// Use leaderboard_query::get_next_query() to retrieve more data about this leaderboard.
    /// </summary>
    /// <param name="user">The local user whose stats to access</param>
    /// <param name="statName">The name of the statistic to get the leaderboard of</param>
    /// <param name="socialGroup">The name of the social group</param>
    /// <param name="query">The query parameters of the leaderboard request</param>
    /// <return>Whether or not the leaderboard request was started correctly</return>
    _XSAPIIMP xbox_live_result<void> get_social_leaderboard(
        _In_ const xbox_live_user_t& user, 
        _In_ const string_t& statName, 
        _In_ const string_t& socialGroup, 
        _In_ leaderboard::leaderboard_query query
        );

private:
    std::shared_ptr<stats_manager_impl> m_statsManagerImpl;
};

} } } }