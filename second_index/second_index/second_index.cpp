#include "d:/git/cppRepos/test_runner.h"

#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <list>

using namespace std;

struct Record {
    string id;
    string title;
    string user;
    int timestamp;
    int karma;
};

class Database {
    unordered_map<string, Record> byId;
    multimap<string, string> byTitle;
    unordered_map< string, multimap<string, string>::iterator> byTitleSec;
    unordered_multimap<string, string> byUser;
    unordered_map< string, unordered_multimap<string, string>::iterator> byUserSec;
    multimap<int, string> byTimestamp;
    unordered_map< string, multimap<int, string>::iterator> byTimestampSec;
    multimap<int, string> byKarma;
    unordered_map< string, multimap<int, string>::iterator> byKarmaSec;

    bool hasId(const string& id) const{
        return byId.find(id) != byId.end();
    }

public:
    bool Put(const Record& record) {
        if (hasId(record.id))
            return false;

        byId[record.id] = record;

        byTitleSec.emplace(record.id, byTitle.emplace(record.title, record.id));

        byUserSec.emplace(record.id, byUser.emplace(record.user, record.id));

        byTimestampSec.emplace(record.id, byTimestamp.emplace(record.timestamp, record.id));

        byKarmaSec.emplace(record.id, byKarma.emplace(record.karma, record.id));

        return true;


    }
    const Record* GetById(const string& id) const {
        if (hasId(id)) {
            return &byId.at(id);
        }
        return nullptr;
    }

    bool Erase(const string& id) {
        if (!hasId(id))
            return false;

        byId.erase(id);

        byTitle.erase(byTitleSec[id]);
        byTitleSec.erase(id);

        byUser.erase(byUserSec[id]);
        byUserSec.erase(id);

        byTimestamp.erase(byTimestampSec[id]);
        byTimestampSec.erase(id);

        byKarma.erase(byKarmaSec[id]);
        byKarmaSec.erase(id);

        return true;
    }

    template <typename Callback>
    void RangeByTimestamp(int low, int high, Callback callback) const {
        for (auto ptr = byTimestamp.lower_bound(low); ptr != byTimestamp.upper_bound(high); ptr++) {
            const std::string id = ptr->second;
            if (!callback(byId.at(id)))
                return;
        }
    }

    template <typename Callback>
    void RangeByKarma(int low, int high, Callback callback) const {
        auto begin = byKarma.lower_bound(low);
        auto end = byKarma.upper_bound(high);
        for (auto ptr = begin; ptr != end; ptr++) {
            const std::string id = ptr->second;
            if (!callback(byId.at(id)))
                return;
        }
    }

    template <typename Callback>
    void AllByUser(const string& user, Callback callback) const {
        auto range = byUser.equal_range(user);
        for (auto ptr = range.first; ptr != range.second; ptr++) {
            const std::string  id = ptr->second;
            if (!callback(byId.at(id)))
                return;
        }
    }
};

void TestRangeBoundaries() {
    const int good_karma = 1000;
    const int bad_karma = -10;

    Database db;
    db.Put({ "id1", "Hello there", "master", 1536107260, good_karma });
    db.Put({ "id2", "O>>-<", "general2", 1536107260, bad_karma });

    int count = 0;
    db.RangeByKarma(bad_karma, good_karma, [&count](const Record&) {
        ++count;
        return true;
        });

    ASSERT_EQUAL(2, count);
}

void TestSameUser() {
    Database db;
    db.Put({ "id1", "Don't sell", "master", 1536107260, 1000 });
    db.Put({ "id2", "Rethink life", "master", 1536107260, 2000 });

    int count = 0;
    db.AllByUser("master", [&count](const Record&) {
        ++count;
        return true;
        });

    ASSERT_EQUAL(2, count);
}

void TestReplacement() {
    const string final_body = "Feeling sad";

    Database db;
    db.Put({ "id", "Have a hand", "not-master", 1536107260, 10 });
    db.Erase("id");
    db.Put({ "id", final_body, "not-master", 1536107260, -10 });

    auto record = db.GetById("id");
    ASSERT(record != nullptr);
    ASSERT_EQUAL(final_body, record->title);
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestRangeBoundaries);
    RUN_TEST(tr, TestSameUser);
    RUN_TEST(tr, TestReplacement);
    return 0;
}