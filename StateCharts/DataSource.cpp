#include "DataSource.hpp"

namespace StateMachine
{

DataSource::~DataSource()
{

}

const strVector& DataSource::GetStringVector()
{
    return m_strVector;
}

std::optional<bool> DataSource::InsertToStringVec(std::string infoString)
{
    m_strVector.push_back(infoString);
    return true;
}

std::optional<bool> DataSource::ClearStringVector()
{
    if (m_strVector.empty())
    {
        return std::nullopt;
    }
    else
    {
        m_strVector.clear();
        return true;
    }
}

}