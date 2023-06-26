#pragma once

#include <map>
#include <string>

class FeedData
{
protected:
    std::map<std::wstring, std::wstring> m_data;

public:
    FeedData() {}
    FeedData(const FeedData& rhs) : m_data{rhs.m_data} {}
    FeedData(FeedData&& rhs) noexcept: m_data{ std::move(rhs.m_data) } {}
    FeedData& operator=(const FeedData& rhs)
    {
        if (this != &rhs)
            m_data = rhs.m_data;

        return *this;
    }
    virtual ~FeedData() {}

    virtual void SetValue(const std::wstring& name, const std::wstring& value)
    {
        m_data[name] = value;
    }

    virtual std::wstring GetValue(const std::wstring& name) const
    {
        auto it = m_data.find(name);
        if (m_data.end() != it)
            return it->second;

        return {};
    }
};
