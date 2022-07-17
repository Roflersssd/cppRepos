#include "../../test_runner.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <queue>
#include <stdexcept>
#include <set>
using namespace std;

template <class T>
class ObjectPool
{
public:
    T *Allocate()
    {
        if (allocatedObjs.empty())
            allocatedObjs.push(new T);
        T *objPtr = allocatedObjs.front();
        allocatedObjs.pop();
        dellocatedObjs.insert(objPtr);
        return objPtr;
    }
    T *TryAllocate()
    {
        if (allocatedObjs.empty())
            return nullptr;
        else
        {
            T *objPtr = allocatedObjs.front();
            allocatedObjs.pop();
            dellocatedObjs.insert(objPtr);
            return objPtr;
        }
    }

    void Deallocate(T *object)
    {
        const auto dellocItr = dellocatedObjs.find(object);
        if (dellocItr == end(dellocatedObjs))
            throw invalid_argument("");
        else
        {
            allocatedObjs.push(*dellocItr);
            dellocatedObjs.erase(dellocItr);
        }
    }

    ~ObjectPool()
    {
        for (auto ptr : dellocatedObjs)
            delete ptr;
        while (!allocatedObjs.empty())
        {
            T *objPtr = allocatedObjs.front();
            delete objPtr;
            allocatedObjs.pop();
        }
    }

private:
    queue<T *> allocatedObjs;
    set<T *> dellocatedObjs;
};

void TestObjectPool()
{
    ObjectPool<string> pool;

    auto p1 = pool.Allocate();
    auto p2 = pool.Allocate();
    auto p3 = pool.Allocate();

    *p1 = "first";
    *p2 = "second";
    *p3 = "third";

    pool.Deallocate(p2);
    ASSERT_EQUAL(*pool.Allocate(), "second");

    pool.Deallocate(p3);
    pool.Deallocate(p1);
    ASSERT_EQUAL(*pool.Allocate(), "third");
    ASSERT_EQUAL(*pool.Allocate(), "first");

    pool.Deallocate(p1);
}

int main()
{
    TestRunner tr;
    RUN_TEST(tr, TestObjectPool);
    return 0;
}