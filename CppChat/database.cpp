#include "database.h"

#include <QDir>
#include <QStandardPaths>
#include <QSqlError>
#include <QSqlQuery>

DataBase &DataBase::GetInstance()
{
    static DataBase db;
    return db;
}

bool DataBase::initialization(const QString &db_path)
{

    if (_db.isOpen()){
        return true;
    }
    _db_path = db_path.isEmpty() ?
    QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/chat_data.db":
                   db_path;

    // qDebug() <<_db_path;

    // 确保目录存在
    QDir().mkpath(QFileInfo(_db_path).absolutePath());
    _db = QSqlDatabase::addDatabase("QSQLITE","chat_connection");
    _db.setDatabaseName(_db_path);

    if (!_db.open()) {
        return false;
    }
    return createMessagesTables() && createConversationTable() && createFriendsTable();
}

bool DataBase::createMessagesTables()
{
    QSqlQuery query(_db);

    QString query_str =
        "CREATE TABLE IF NOT EXISTS messages ("
        "id                INTEGER PRIMARY KEY AUTOINCREMENT,"
        "uid               TEXT    NOT NULL,"
        "owner             INTEGER NOT NULL,"
        "from_uid          INTEGER NOT NULL,"
        "to_uid            INTEGER NOT NULL,"
        "timestamp         TEXT,"  // 毫秒时间戳
        "env               INTEGER NOT NULL,"  // 0=Private 1=Group
        "content_type      INTEGER NOT NULL,"  // MessageType 枚举
        "content_data      TEXT    NOT NULL,"  // 文本或缩略图 base64
        "content_mime_type TEXT,"              // 可为空
        "content_fid       TEXT,"              // 可为空
        "status            INTEGER NOT NULL DEFAULT 0" // 0=正常 1=撤回 ...
        ")";

    if (!query.exec(query_str)){
        // qDebug() << "Failed to create table:" << query.lastError().text();
        return false;
    }

    // 创建索引
    QStringList indexes = {
        "CREATE INDEX IF NOT EXISTS idx_from_uid ON messages(from_uid)",
        "CREATE INDEX IF NOT EXISTS idx_to_uid ON messages(to_uid)",
        "CREATE INDEX IF NOT EXISTS idx_timestamp ON messages(timestamp)",
        "CREATE INDEX IF NOT EXISTS idx_from_timestamp ON messages(from_uid, timestamp)",
        "CREATE INDEX IF NOT EXISTS idx_to_timestamp ON messages(to_uid, timestamp)"
    };

    for (const QString& sql : indexes) {
        if (!QSqlQuery(_db).exec(sql)) {
            // qDebug() << "Failed to create index:" << _db.lastError().text();
        }
    }
    return true;
}

bool DataBase::storeMessage(const MessageItem &message)
{
    QSqlQuery query(_db);
    query.prepare(R"(
        INSERT INTO messages
        (uid,from_uid,to_uid,timestamp,env,content_type,content_data,content_mime_type,content_fid,status,owner)
        values(?,?,?,?,?,?,?,?,?,?,?)
    )");

    query.addBindValue(message.id);
    query.addBindValue(message.from_id);
    query.addBindValue(message.to_id);
    query.addBindValue(message.timestamp.toString("yyyy-MM-dd HH:mm:ss"));
    query.addBindValue(static_cast<int>(message.env));
    query.addBindValue(static_cast<int>(message.content.type));
    query.addBindValue(message.content.data);
    query.addBindValue(message.content.mimeType);
    query.addBindValue(message.content.fid);
    query.addBindValue(0);
    query.addBindValue(UserManager::GetInstance()->GetUid());

    if (!query.exec()){
        // qDebug() << "Failed to store message:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DataBase::storeMessages(const std::vector<MessageItem> &messages)
{

    if (messages.empty()){
        return true;
    }
    if (!_db.transaction()){
        return false;
    }


    QSqlQuery query(_db);
    query.prepare(R"(
        INSERT INTO messages
        (uid,from_uid,to_uid,timestamp,env,content_type,content_data,content_mime_type,content_fid,status,owner)
        values(?,?,?,?,?,?,?,?,?,?,?)
    )");

    for (const MessageItem&message:messages){
        query.addBindValue(message.id);
        query.addBindValue(message.from_id);
        query.addBindValue(message.to_id);
        query.addBindValue(message.timestamp.toString("yyyy-MM-dd HH:mm:ss"));
        query.addBindValue(static_cast<int>(message.env));
        query.addBindValue(static_cast<int>(message.content.type));
        query.addBindValue(message.content.data);
        query.addBindValue(message.content.mimeType);
        query.addBindValue(message.content.fid);
        query.addBindValue(message.status);
        query.addBindValue(UserManager::GetInstance()->GetUid());
    }
    if (!query.execBatch()){
        // qDebug() << "ExecBatch Error:" << query.lastError().text();
        _db.rollback();
        return false;
    }
    if (!_db.commit()){
        // qDebug() << "Commit Error:" << _db.lastError().text();
        _db.rollback();
        return false;
    }

    return true;
}

bool DataBase::storeMessages(const std::vector<std::shared_ptr<MessageItem>> &messages)
{
    if (messages.empty()){
        return true;
    }
    if (!_db.transaction()){
        // qDebug() << "Transaction Start Error:" << _db.lastError().text();
        return false;
    }

    QSqlQuery query(_db);
    query.prepare(R"(
        INSERT INTO messages
        (uid, from_uid, to_uid, timestamp, env, content_type, content_data, content_mime_type, content_fid, status,owner)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");

    for (const auto& message_ptr : messages){
        const MessageItem& message = *message_ptr;  // 解引用 shared_ptr
        query.addBindValue(message.id);
        query.addBindValue(message.from_id);
        query.addBindValue(message.to_id);
        query.addBindValue(message.timestamp.toString("yyyy-MM-dd HH:mm:ss"));
        query.addBindValue(static_cast<int>(message.env));
        query.addBindValue(static_cast<int>(message.content.type));
        query.addBindValue(message.content.data);
        query.addBindValue(message.content.mimeType);
        query.addBindValue(message.content.fid);
        query.addBindValue(message.status);
        query.addBindValue(UserManager::GetInstance()->GetUid());
    }

    if (!query.execBatch()){
        qDebug() << "ExecBatch Error (shared_ptr version):" << query.lastError().text();
        _db.rollback();
        return false;
    }
    if (!_db.commit()){
        qDebug() << "Commit Error:" << _db.lastError().text();
        _db.rollback();
        return false;
    }

    qDebug() << "Successfully stored" << messages.size() << "messages (shared_ptr version)";
    return true;
}
std::vector<MessageItem> DataBase::getMessages(int peerUid,  QString sinceTimestamp,int limit)
{
    std::vector<MessageItem>messages;
    QSqlQuery query(_db);

    QString sql = R"(
        SELECT * FROM messages
        WHERE ((from_uid = ? AND to_uid = ?) OR (from_uid = ? AND to_uid = ?)) AND owner = ?
    )";

    QVariantList params;
    params << peerUid << UserManager::GetInstance()->GetUid() << UserManager::GetInstance()->GetUid() << peerUid << UserManager::GetInstance()->GetUid();

    if (!sinceTimestamp.isEmpty()){
        sql += "AND timestamp < ? ";
        params << sinceTimestamp;
    }

    sql += "ORDER BY timestamp desc ";
    if (limit > 0){
        sql+="LIMIT ?";
        params << limit;
    }

    query.prepare(sql);
    for(int i = 0;i<params.size();++i){
        query.addBindValue(params[i]);
    }
    if (!query.exec()){
        qDebug() << "Failed To Get Messages:" << query.lastError().text();
        return messages;
    }

    int count = 0;
    QDateTime last_time;
    while(query.next()){
        messages.push_back(createMessageFromQuery(query));
        count++;
        last_time = query.value("timestamp").toDateTime();
    }
    if (count > 0){
        emit SignalRouter::GetInstance().on_change_last_time(peerUid,last_time);
    }

    if (count<limit){
        UserManager::GetInstance()->setMessagesFinished(peerUid);
    }
    return messages;
}

std::optional<MessageItem> DataBase::getMessage(int peerUid, const QString &message_id)
{
    MessageItem message;
    QSqlQuery query(_db);

    QString sql = R"(
        SELECT * FROM messages
        WHERE to_uid = ? AND uid = ? AND from_uid = ? AND owner = ?
    )";
    qDebug() << "myuid:" << UserManager::GetInstance()->GetUid();
    query.prepare(sql);
    query.addBindValue(peerUid);
    query.addBindValue(message_id);
    query.addBindValue(UserManager::GetInstance()->GetUid());
    query.addBindValue(UserManager::GetInstance()->GetUid());
    if (!query.exec()){
        qDebug() << "Failed To Get Message:" << query.lastError().text();
        return std::nullopt;
    }
    if (query.next()){
        message = createMessageFromQuery(query);
    }
    return message;
}


bool DataBase::updateMessageStatus(int messageId, int status)
{
    QSqlQuery query(_db);
    query.prepare("UPDATE messages SET status = ? WHERE to_uid = ?");

    query.addBindValue(status);
    query.addBindValue(messageId);

    if (!query.exec()) {
        qDebug() << "Failed to update message status:" << query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        qDebug() << "updateMessageStatus::No message found with id:" << messageId;
        return false;
    }

    return true;
}

bool DataBase::updateMessagesStatus(int peerUid, int status)
{
    QSqlQuery query(_db);
    query.prepare("UPDATE messages SET status = ? WHERE to_uid = ?");

    query.addBindValue(status);
    query.addBindValue(peerUid);

    if (!query.exec()) {
        qDebug() << "Failed to update message status:" << query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        qDebug() << "updateMessagesStatus::No message found with id:" << peerUid;
        return false;
    }
    return true;
}

bool DataBase::deleteMessage(int messageId)
{
    QSqlQuery query(_db);
    query.prepare("DELETE FROM messages WHERE uid = ?");

    query.addBindValue(messageId);

    if (!query.exec()) {
        qDebug() << "Failed to delete message:" << query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        qDebug() << "deleteMessage::No message found with id:" << messageId;
        return false;
    }

    qDebug() << "Deleted message:" << messageId;
    return true;
}

MessageItem DataBase::createMessageFromQuery(const QSqlQuery &query)
{   
    MessageItem msg;
    msg.id = query.value("uid").toString();
    msg.from_id = query.value("from_uid").toInt();
    msg.to_id = query.value("to_uid").toInt();
    msg.timestamp = query.value("timestamp").toDateTime();
    msg.env = MessageEnv(query.value("env").toInt());
    msg.content.type = MessageType(query.value("content_type").toInt());
    msg.content.data = query.value("content_data").toString();
    msg.content.mimeType = query.value("content_mime_type").toString();
    msg.content.fid = query.value("content_fid").toString();
    qDebug() << msg.content.data;
    return msg;
}

bool DataBase::createConversationTable()
{
    QSqlQuery query (_db);
    QString sql_str = R"(
    CREATE TABLE IF NOT EXISTS conversations(
        id          INTEGER PRIMARY KEY AUTOINCREMENT,
        uid         TEXT    NOT NULL UNIQUE,
        to_uid      INTEGER NOT NULL,
        from_uid    INTEGER NOT NULL,
        create_time TEXT ,
        update_time TEXT ,
        name        TEXT ,
        icon        TEXT ,
        status      INTEGER DEFAULT 0,
        deleted     INTEGER DEFAULT 0,
        pined       INTEGER DEFAULT 0,
        processed   INTEGER DEFAULT 0,
        env         INTEGER DEFAULT 0
    )
    )";

    if (!query.exec(sql_str)){
        qDebug() << "Failed to create table:" << query.lastError().text();
        return false;
    }

    // 创建索引
    QStringList indexes = {
        "CREATE INDEX IF NOT EXISTS idx_from_uid ON conversations(from_uid)",
        "CREATE INDEX IF NOT EXISTS idx_to_uid ON conversations(to_uid)",
        "CREATE INDEX IF NOT EXISTS idx_deleted ON conversations(deleted)",
        "CREATE INDEX IF NOT EXISTS idx_pined ON conversations(pined)",
        "CREATE INDEX IF NOT EXISTS idx_processed ON conversations(processed)",
    };

    for (const QString& sql : indexes) {
        if (!QSqlQuery(_db).exec(sql)) {
            qDebug() << "Failed to create index:" << _db.lastError().text();
        }
    }
    return true;
}

bool DataBase::createOrUpdateConversation(const ConversationItem& conv)
{
    QSqlQuery query(_db);
    query.prepare(R"(
        INSERT OR REPLACE INTO conversations
        (uid,to_uid, from_uid, create_time, update_time, name, icon,status,deleted,pined,processed,env)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");

    qint64 now = QDateTime::currentSecsSinceEpoch();

    query.addBindValue(conv.id);
    query.addBindValue(conv.to_uid);
    query.addBindValue(conv.from_uid);
    query.addBindValue(conv.create_time.toString("yyyy-MM-dd HH:mm:ss"));
    query.addBindValue(!conv.update_time.isNull() ? conv.update_time.toString("yyyy-MM-dd HH:mm:ss") : QString::number(now));
    query.addBindValue(conv.name);
    query.addBindValue(conv.icon);
    query.addBindValue(conv.status);
    query.addBindValue(conv.deleted);
    query.addBindValue(conv.pined);
    query.addBindValue(conv.processed?1:0);
    query.addBindValue(conv.env);

    if (!query.exec()) {
        qDebug() << "Failed to create/update conversation:" << query.lastError().text();
        return false;
    }

    return true;
}

bool DataBase::existConversation(int peerUid)
{
    QSqlQuery query(_db);
    query.prepare(R"(
        SELECT COUNT(*) FROM conversations
        WHERE to_uid = ?
        AND deleted = 0
    )");

    query.addBindValue(peerUid);
    if (!query.exec()){
        qDebug() << "Failed to check conversation existence:" << query.lastError().text();
        return false;
    }
    if (query.next()){
        int count = query.value(0).toInt();
        return count > 0;
    }
    return false;
}

bool DataBase::createOrUpdateConversations(const std::vector<ConversationItem> &conversations)
{
    if (conversations.empty()) {
        return true;
    }

    _db.transaction();

    QSqlQuery query(_db);
    query.prepare(R"(
        INSERT OR REPLACE INTO conversations
<<<<<<< HEAD
        (uid, to_uid, from_uid, create_time, update_time, name, icon, status, deleted, pined, processed)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
=======
        (uid, to_uid, from_uid, create_time, update_time, name, icon, status, delted, pined,processed)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
>>>>>>> origin/main
    )");

    QDateTime now = QDateTime::currentDateTime();

    // 预先绑定所有参数
    QVariantList ids, to_uids, from_uids, create_times, update_times;
    QVariantList names, icons, statuses, deleteds, pineds,processeds,envs;

    for (const auto& conv : conversations) {
        ids                 << conv.id;
        to_uids             << conv.to_uid;
        from_uids           << conv.from_uid;
        create_times        << conv.create_time.toString("yyyy-MM-dd HH:mm:ss");
        update_times        << (!conv.update_time.isNull() ? conv.update_time.toString("yyyy-MM-dd HH:mm:ss") : now.toString("yyyy-MM-dd HH:mm:ss"));
        names               << conv.name;
        icons               << conv.icon;
        statuses            << conv.status;
        deleteds            << conv.deleted;
        pineds              << conv.pined;
        processeds          << (conv.processed ? 1 : 0);
        envs                << conv.env;
    }

    qDebug() << "conversations size : " << conversations.size();
    query.addBindValue(ids);
    query.addBindValue(to_uids);
    query.addBindValue(from_uids);
    query.addBindValue(create_times);
    query.addBindValue(update_times);
    query.addBindValue(names);
    query.addBindValue(icons);
    query.addBindValue(statuses);
    query.addBindValue(deleteds);
    query.addBindValue(pineds);
    query.addBindValue(processeds);
    query.addBindValue(envs);

    if (!query.execBatch()) {
        qDebug() << "Failed to batch create/update conversations:" << query.lastError().text();
        _db.rollback();
        return false;
    }

    if (!_db.commit()) {
        qDebug() << "Failed to commit transaction:" << _db.lastError().text();
        _db.rollback();
        return false;
    }

    qDebug() << "Successfully batch created/updated" << conversations.size() << "conversations";
    return true;
}

bool DataBase::createOrUpdateConversations(const std::vector<std::shared_ptr<ConversationItem>> &conversations)
{
    if (conversations.empty()) {
        return true;
    }

    qDebug() << "createOrUpdateConversations";

    _db.transaction();

    QSqlQuery query(_db);
    query.prepare(R"(
        INSERT OR REPLACE INTO conversations
<<<<<<< HEAD
        (uid, to_uid, from_uid, create_time, update_time, name, icon, status, deleted, pined, processed, env)
=======
        (uid, to_uid, from_uid, create_time, update_time, name, icon, status, delted, pined,processed,env)
>>>>>>> origin/main
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");

    QDateTime now = QDateTime::currentDateTime();

    // 预先绑定所有参数
    QVariantList ids, to_uids, from_uids, create_times, update_times;
    QVariantList names, icons, statuses, deleteds, pineds,processeds,envs;

    for (const auto& conv_ptr : conversations) {
        const ConversationItem& conv = *conv_ptr;  // 解引用 shared_ptr
        ids                 << conv.id;
        to_uids             << conv.to_uid;
        from_uids           << conv.from_uid;
        create_times        << conv.create_time;
        update_times        << (!conv.update_time.isNull() ? conv.update_time.toString("yyyy-MM-dd HH:mm:ss") : now.toString("yyyy-MM-dd HH:mm:ss"));
        names               << conv.name;
        icons               << conv.icon;
        statuses            << conv.status;
        deleteds            << conv.deleted;
        pineds              << conv.pined;
        processeds          << (conv.processed?1:0);
        envs                << conv.env;
    }

    query.addBindValue(ids);
    query.addBindValue(to_uids);
    query.addBindValue(from_uids);
    query.addBindValue(create_times);
    query.addBindValue(update_times);
    query.addBindValue(names);
    query.addBindValue(icons);
    query.addBindValue(statuses);
    query.addBindValue(deleteds);
    query.addBindValue(pineds);
    query.addBindValue(processeds);
    query.addBindValue(envs);

    if (!query.execBatch()) {
        qDebug() << "Failed to batch create/update conversations (shared_ptr version):" << query.lastError().text();
        _db.rollback();
        return false;
    }

    if (!_db.commit()) {
        qDebug() << "Failed to commit transaction:" << _db.lastError().text();
        _db.rollback();
        return false;
    }

    qDebug() << "Successfully batch created/updated" << conversations.size() << "conversations (shared_ptr version)";
    return true;
}
std::vector<ConversationItem> DataBase::getConversationList()
{
    std::vector<ConversationItem>conversations;

    QSqlQuery query(_db);
    query.prepare(R"(
        SELECT * FROM conversations
        WHERE deleted = 0 AND from_uid = ?
        ORDER BY pined desc , update_time desc
    )");
    query.addBindValue(UserManager::GetInstance()->GetUid());

    if (!query.exec()){
        qDebug() << "Failed to get conversations list:" << query.lastError().text();
        return conversations;
    }
    while (query.next()) {
        ConversationItem conv = createConversationFromQuery(query);
        // 补充动态数据
        conv.message = getLastMessage(conv.to_uid);
        conversations.push_back(std::move(conv));
    }
    return conversations;
}

std::vector<std::shared_ptr<ConversationItem> > DataBase::getConversationListPtr()
{
    std::vector<std::shared_ptr<ConversationItem> >conversations;

    QSqlQuery query(_db);
    query.prepare(R"(
        SELECT * FROM conversations
        WHERE deleted = 0 AND from_uid = ?
        ORDER BY pined desc , update_time desc
    )");
    qDebug() << "getUid:"<<UserManager::GetInstance()->GetUid();
    query.addBindValue(UserManager::GetInstance()->GetUid());

    if (!query.exec()){
        qDebug() << "Failed to get conversations list:" << query.lastError().text();
        return conversations;
    }
    while (query.next()) {
        ConversationItem conv = createConversationFromQuery(query);
        // 补充动态数据
        conv.message = getLastMessage(conv.to_uid);
        conversations.push_back(std::make_shared<ConversationItem>(std::move(conv)));
    }
    return conversations;
}

ConversationItem DataBase::getConversation(int peerUid)
{
    ConversationItem conv;
    QSqlQuery query(_db);
    query.prepare(R"(
        SELECT * FROM conversations
        WHERE to_uid = ?
        AND from_uid = ?
    )");

    query.addBindValue(peerUid);
    query.addBindValue(UserManager::GetInstance()->GetUid());
    if (!query.exec() ){
        qDebug() << "Failed to get conversation :" << query.lastError().text();
        return conv;
    }
    if (query.next()){
        conv = createConversationFromQuery(query);
    }
    return conv;
}

ConversationItem DataBase::createConversationFromQuery(const QSqlQuery &query)
{

    // 添加有效性检查
    if (!query.isValid()) {
        qDebug() << "Warning: createConversationFromQuery called with invalid query";
        return ConversationItem();
    }

    ConversationItem conv;
    conv.id = query.value("uid").toString();
    conv.from_uid       =     query.value("from_uid").toInt();
    conv.to_uid         =     query.value("to_uid").toInt();
    conv.create_time    =     query.value("create_time").toDateTime();
    conv.update_time    =     query.value("update_time").toDateTime();
    conv.name           =     query.value("name").toString();
    conv.icon           =     query.value("icon").toString();
    conv.status         =     query.value("status").toInt();
    conv.deleted        =     query.value("deleted").toInt();
    conv.pined          =     query.value("pined").toInt();
    conv.processed      =     query.value("processed").toInt() == 1 ? true: false;
    conv.env            =     query.value("env").toInt();

    return conv;
}

QString DataBase::getLastMessage(int peerUid)
{
    QString text;;
    int myUid = UserManager::GetInstance()->GetUid();

    QSqlQuery query(_db);
    query.prepare(R"(
        SELECT * FROM messages
        WHERE (from_uid = ? AND to_uid = ?) OR (from_uid = ? AND to_uid = ?)
        ORDER BY timestamp desc
        LIMIT 1
    )");

    query.addBindValue(myUid);
    query.addBindValue(peerUid);
    query.addBindValue(peerUid);
    query.addBindValue(myUid);

    if (query.exec() && query.next()) {

        // 在访问任何值之前，先检查查询是否在有效记录上
        if (!query.isValid()) {
            qDebug() << "Query is not valid in getLastMessage";
            return "";
        }

        switch(query.value("content_type").toInt())
        {
        case static_cast<int>(MessageType::AudioMessage):
            text = "[Audio]";
            break;
        case static_cast<int>(MessageType::TextMessage):
            text = query.value("content_data").toString();
            break;
        case static_cast<int>(MessageType::ImageMessage):
            text = "[Image]";
            break;
        case static_cast<int>(MessageType::VideoMessage):
            text = "[Video]";
            break;
        case static_cast<int>(MessageType::OtherFileMessage):
            text = "[File]";
            break;
        default:
            text = "";
            break;
        };
    }
    return text;
}

bool DataBase::createFriendsTable()
{
    QSqlQuery query(_db);
    QString sql_str =
        "CREATE TABLE IF NOT EXISTS friends ("
        "id          INTEGER PRIMARY KEY,"
        "from_uid    INTEGER NOT NULL,"
        "to_uid      INTEGER NOT NULL,"
        "sex         INTEGER NOT NULL DEFAULT 0,"
        "status      INTEGER NOT NULL DEFAULT 0,"
        "email       TEXT,"
        "name        TEXT    NOT NULL,"
        "avatar      TEXT,"
        "desc        TEXT,"
        "back        TEXT"   // 备用字段
        ")";
    if (!query.exec(sql_str)){
        qDebug() << "Failed to create friends table:" << query.lastError().text();
        return false;
    }

    QStringList indexes = {
       "CREATE INDEX IF NOT EXISTS idx_friends_from_uid ON friends(from_uid)",
       "CREATE INDEX IF NOT EXISTS idx_friends_to_uid ON friends(to_uid)",
       "CREATE INDEX IF NOT EXISTS idx_friends_status ON friends(status)",
       "CREATE INDEX IF NOT EXISTS idx_friends_name ON friends(name)"
    };
    for (const QString& sql : indexes) {
        if (!QSqlQuery(_db).exec(sql)) {
            qDebug() << "Failed to create friends index:" << _db.lastError().text();
        }
    }
    return true;
}

std::shared_ptr<UserInfo> DataBase::getFriendInfoPtr(int peerUid)
{
    qDebug() << "peerUid:" << peerUid;
    QSqlQuery query(_db);
    query.prepare(R"(
        SELECT * FROM friends
        WHERE to_uid = ?
        AND from_uid = ?
    )");
    query.addBindValue(peerUid);
    query.addBindValue(UserManager::GetInstance()->GetUid());
    qDebug() << "myUid :" << UserManager::GetInstance()->GetUid() << "\tPeerUid:" << peerUid;

    if (!query.exec()){
        qDebug()<< "Failed to get FriendInfo" << query.lastError().text();
        return std::shared_ptr<UserInfo>();
    }
    // query.next();
    std::shared_ptr<UserInfo>info = nullptr;
    if (query.next()){
        info = std::make_shared<UserInfo>(createFriendInfoFromQuery(query));
    }
    return info;
}

UserInfo DataBase::getFriendInfo(int peerUid)
{
    QSqlQuery query(_db);
    query.prepare(R"(
        SELECT * FROM friends
        WHERE to_uid = ?
    )");
    query.addBindValue(peerUid);
    if (!query.exec()){
        qDebug()<< "Failed to get FriendInfo" << query.lastError().text();
        return UserInfo{};
    }
    UserInfo info = createFriendInfoFromQuery(query);
    return info;
}

std::vector<UserInfo> DataBase::getFriends()
{
    std::vector<UserInfo>friends;
    QSqlQuery query(_db);
    query.prepare(R"(
        SELECT * FROM friends
        WHERE from_uid = ?
    )");
    query.addBindValue(UserManager::GetInstance()->GetUid());
    if (!query.exec()){
        qDebug() << "Failed to get Friends list:" << query.lastError().text();
        return friends;
    }
    while (query.next()) {
        UserInfo info = createFriendInfoFromQuery(query);
        friends.push_back(info);
    }
    return friends;
}

std::vector<std::shared_ptr<UserInfo>> DataBase::getFriendsPtr()
{
    std::vector<std::shared_ptr<UserInfo> >friends;
    QSqlQuery query(_db);
    query.prepare(R"(
        SELECT * FROM friends
        WHERE from_uid = ?
    )");

    query.addBindValue(UserManager::GetInstance()->GetUid());
    if (!query.exec()){
        qDebug() << "Failed to get Friends list:" << query.lastError().text();
        return friends;
    }
    while (query.next()) {
        UserInfo info = createFriendInfoFromQuery(query);
        friends.push_back(std::make_shared<UserInfo>(std::move(info)));
    }
    return friends;
}


bool DataBase::storeFriends(const std::vector<std::shared_ptr<UserInfo>> friends)
{
    if (friends.empty()){
        return false;
    }
    if (!_db.transaction()){
        qDebug() << "Transaction Start Error : " << _db.lastError().text();
        return false;
    }

    QSqlQuery query(_db);
    query.prepare(R"(
        INSERT OR REPLACE INTO friends
        (from_uid, to_uid, sex, status, email, name, avatar, desc, back)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");

    // 使用 QVariantList 来批量绑定
    QVariantList from_uids, to_uids, sexes, statuses, emails, names, avatars, descs, backs;

    for (const auto& friend_ptr : friends){
        const UserInfo& info = *friend_ptr;
        from_uids << UserManager::GetInstance()->GetUid();
        to_uids << info.id;
        sexes << info.sex;
        statuses << info.status;
        emails << info.email;
        names << info.name;
        avatars << info.avatar;
        descs << info.desc;
        backs << info.back;
    }

    query.addBindValue(from_uids);
    query.addBindValue(to_uids);
    query.addBindValue(sexes);
    query.addBindValue(statuses);
    query.addBindValue(emails);
    query.addBindValue(names);
    query.addBindValue(avatars);
    query.addBindValue(descs);
    query.addBindValue(backs);

    if (!query.execBatch()){
        qDebug() << "ExecBatch Error (shared_ptr friends):" << query.lastError().text();
        _db.rollback();
        return false;
    }

    if (!_db.commit()){
        qDebug() << "Commit Error:" << _db.lastError().text();
        _db.rollback();
        return false;
    }

    qDebug() << "Successfully stored" << friends.size() << "friends (shared_ptr version)";
    return true;
}


// bool DataBase::storeFriends(const std::vector<std::shared_ptr<UserInfo> > friends)
// {
//     if (friends.empty()){
//         return false;
//     }
//     if (!_db.transaction()){
//         qDebug() << "Transaction Start Error : " << _db.lastError().text();
//         return false;
//     }

//     QSqlQuery query(_db);
//     query.prepare(R"(
//         INSERT OR REPLACE INTO friends
//         (from_uid, to_uid, sex, status, email, name, avatar, desc, back)
//         VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
//     )");

//     for (const auto& friend_ptr : friends){
//         const UserInfo& info = *friend_ptr;
//         query.addBindValue(UserManager::GetInstance()->GetUid());
//         query.addBindValue(info.id);
//         query.addBindValue(info.sex);
//         query.addBindValue(info.status);
//         query.addBindValue(info.email);
//         query.addBindValue(info.name);
//         query.addBindValue(info.avatar);
//         query.addBindValue(info.desc);  // 映射到 description 字段
//         query.addBindValue(info.back);
//     }

//     if (!query.execBatch()){
//         qDebug() << "ExecBatch Error (shared_ptr friends):" << query.lastError().text();
//         _db.rollback();
//         return false;
//     }

//     if (!_db.commit()){
//         qDebug() << "Commit Error:" << _db.lastError().text();
//         _db.rollback();
//         return false;
//     }

//     qDebug() << "Successfully stored" << friends.size() << "friends (shared_ptr version)";
//     return true;
// }


bool DataBase::storeFriends(const std::vector<UserInfo> friends)
{
    if (friends.empty()){
        return false;
    }
    if (!_db.transaction()){
        qDebug() << "Transaction Start Error : " << _db.lastError().text();
        return false;
    }

    QSqlQuery query(_db);
    query.prepare(R"(
        INSERT OR REPLACE INTO friends
        (from_uid, to_uid, sex, status, email, name, avatar, desc, back)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");

    for (const auto& friend_ptr : friends){
        const UserInfo& info = friend_ptr;
        query.addBindValue(UserManager::GetInstance()->GetUid());
        query.addBindValue(info.id);
        query.addBindValue(info.sex);
        query.addBindValue(info.status);
        query.addBindValue(info.email);
        query.addBindValue(info.name);
        query.addBindValue(info.avatar);
        query.addBindValue(info.desc);  // 映射到 description 字段
        query.addBindValue(info.back);

    }
    if (!query.execBatch()){
        qDebug() << "ExecBatch Error (shared_ptr friends):" << query.lastError().text();
        _db.rollback();
        return false;
    }

    if (!_db.commit()){
        qDebug() << "Commit Error:" << _db.lastError().text();
        _db.rollback();
        return false;
    }

    qDebug() << "Successfully stored" << friends.size() << "friends (shared_ptr version)";
    return true;
}

bool DataBase::storeFriend(const UserInfo &info)
{
    QSqlQuery query(_db);
    query.prepare(R"(
        INSERT OR REPLACE INTO friends
        (from_uid, to_uid, sex, status, email, name, avatar, desc, back)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");

    query.addBindValue(UserManager::GetInstance()->GetUid());
    query.addBindValue(info.id);
    query.addBindValue(info.sex);
    query.addBindValue(info.status);
    query.addBindValue(info.email);
    query.addBindValue(info.name);
    query.addBindValue(info.avatar);
    query.addBindValue(info.desc);  // 映射到 description 字段
    query.addBindValue(info.back);

    if (!query.exec()){
        qDebug() << "Failed to store friend:" << query.lastError().text();
        return false;
    }

    qDebug() << "Successfully stored friend:" << info.name;
    return true;
}

bool DataBase::storeFriend(const std::shared_ptr<UserInfo> &info)
{
    QSqlQuery query(_db);
    query.prepare(R"(
        INSERT OR REPLACE INTO friends
        (from_uid, to_uid, sex, status, email, name, avatar, desc, back)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");

    query.addBindValue(UserManager::GetInstance()->GetUid());
    query.addBindValue(info->id);
    query.addBindValue(info->sex);
    query.addBindValue(info->status);
    query.addBindValue(info->email);
    query.addBindValue(info->name);
    query.addBindValue(info->avatar);
    query.addBindValue(info->desc);  // 映射到 description 字段
    query.addBindValue(info->back);

    if (!query.exec()){
        qDebug() << "Failed to store friend:" << query.lastError().text();
        return false;
    }
    qDebug() << "Successfully stored friend:" << info->name;
    return true;
}

UserInfo DataBase::createFriendInfoFromQuery(const QSqlQuery &query)
{
    UserInfo info;

    info.id = query.value("to_uid").toInt();
    qDebug() <<"to_id:"<<info.id;
    info.name = query.value("name").toString();
    info.avatar = query.value("avatar").toString();
    info.email = query.value("email").toString();
    info.sex = query.value("sex").toInt();
    info.desc = query.value("desc").toString();
    info.back = query.value("back").toString();
    return info;
}
