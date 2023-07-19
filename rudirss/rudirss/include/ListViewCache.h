#pragma once

#include <map>

template<typename ELE>
class ListViewCache: public std::map<long long, ELE>
{
public:
    ListViewCache() {}
    virtual ~ListViewCache() {}

    enum class InsertionDirection
    {
        FRONT,
        BACK,
        NONE,
    };

    InsertionDirection GetInsertionDirection(long long from, long long to)
    {
        if (this->empty())
            return InsertionDirection::NONE;

        InsertionDirection insertionDirection = InsertionDirection::NONE;
        if (to > this->rbegin()->first)
        {
            insertionDirection = InsertionDirection::BACK;
        }
        else if (from < this->begin()->first)
        {
            insertionDirection = InsertionDirection::FRONT;
        }

        return insertionDirection;
    }

    void DeleteFrontElements(size_t cnt)
    {
        size_t iterationCnt = this->size() > cnt ? cnt : this->size();
        for (size_t c = 0; c < iterationCnt; c++)
            this->erase(this->begin());
    }

    void DeleteBackElements(size_t cnt)
    {
        size_t iterationCnt = this->size() > cnt ? cnt : this->size();
        for (size_t c = 0; c < iterationCnt; c++)
            this->erase(this->rbegin()->first);
    }
};
