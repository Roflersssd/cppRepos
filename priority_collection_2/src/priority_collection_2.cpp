#include <iostream>
#include <iterator>
#include <memory>
#include <set>
#include <utility>
#include <vector>

using namespace std;

template <typename T>
class PriorityCollection {
public:

    using Id = int;

    Id Add(T object) {
        d_objects.push_back({ move(object),0 });
        Id id = d_objects.size() - 1;
        d_pSet.insert({ 0,id });
        return id;
    }

    template <typename ObjInputIt, typename IdOutputIt>
    void Add(ObjInputIt range_begin, ObjInputIt range_end,
        IdOutputIt ids_begin) {
        while (range_begin != range_end) {
            *ids_begin++ = Add(move(*range_begin++));
        }
    }


    bool IsValid(Id id) const {
        return id < d_objects.size() && d_objects[id].d_piority != INVALID_PRIORITY;
    }

    const T& Get(Id id) const {
        return d_objects[id];
    }

    void Promote(Id id) {
        auto temp = d_pSet.extract({ d_objects[id].d_piority, id });
        d_objects[id].d_piority++;
        temp.value().first = d_objects[id].d_piority;
        d_pSet.insert(move(temp));
    }

    pair<const T&, int> GetMax() const {
        auto ptr = d_pSet.crbegin();
        Id id = ptr->second;
        return { d_objects[id].d_data, d_objects[id].d_piority };
    }

    pair<T,int> PopMax() {
        auto ptr = prev(d_pSet.end());
        Id id = ptr->second;
        d_pSet.erase(ptr);
        pair<T, int> maxValue = { move(d_objects[id].d_data), d_objects[id].d_piority };
        d_objects[id].d_piority = INVALID_PRIORITY;
        return maxValue;
    }
private:
    struct Object {
        T d_data;
        int d_piority;

    };

    set<pair<int, Id>> d_pSet;
    vector<Object> d_objects;
    const int INVALID_PRIORITY = -1;
    
};

int main(){
    return 0;
}