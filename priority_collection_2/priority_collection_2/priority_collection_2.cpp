#include "D:/git/cppRepos/test_runner.h"
#include <iostream>
#include <iterator>
#include <memory>
#include <list>
#include <utility>
#include <vector>

using namespace std;

template <typename T>
class PriorityCollection {
    struct Node {
        int d_priority;
        T d_data;
    };

    list<Node> d_priorityList;

public:
    using Id = typename list<Node>::iterator;
    Id Add(T object) {
        Node newObject{ 0, move(object) };
        d_priorityList.push_back(newObject);
        return --d_priorityList.end();
    }

    template <typename ObjInputIt, typename IdOutputIt>
    void Add(ObjInputIt range_begin, ObjInputIt range_end,
        IdOutputIt ids_begin);


    bool IsValid(Id id) const;

    const T& Get(Id id) const;

    void Promote(Id id);

    pair<const T&, int> GetMax() const;
    pair<T, int> PopMax();


    
};


class StringNonCopyable : public string {
public:
    using string::string;  
    StringNonCopyable(const StringNonCopyable&) = delete;
    StringNonCopyable(StringNonCopyable&&) = default;
    StringNonCopyable& operator=(const StringNonCopyable&) = delete;
    StringNonCopyable& operator=(StringNonCopyable&&) = default;
};

void TestNoCopy() {
    PriorityCollection<StringNonCopyable> strings;
    const auto white_id = strings.Add("white");
    const auto yellow_id = strings.Add("yellow");
    const auto red_id = strings.Add("red");

    strings.Promote(yellow_id);
    for (int i = 0; i < 2; ++i) {
        strings.Promote(red_id);
    }
    strings.Promote(yellow_id);
    {
        const auto item = strings.PopMax();
        ASSERT_EQUAL(item.first, "red");
        ASSERT_EQUAL(item.second, 2);
    }
    {
        const auto item = strings.PopMax();
        ASSERT_EQUAL(item.first, "yellow");
        ASSERT_EQUAL(item.second, 2);
    }
    {
        const auto item = strings.PopMax();
        ASSERT_EQUAL(item.first, "white");
        ASSERT_EQUAL(item.second, 0);
    }
}

int main() {
    //TestRunner tr;
    //RUN_TEST(tr, TestNoCopy);
    PriorityCollection<int> pc;
    auto ptr = pc.Add(2);

    return 0;
}