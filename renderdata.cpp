
#include "renderdata.h"

RenderData::RenderData(const QVector<Section> &sections)
    : m_sections(sections),
      m_currentLine(-1),
      m_currentSection(-1)
{ }

RenderData::~RenderData() { }

void RenderData::begin()
{
    // We defer actual initialization to the nextLine() function, so that the
    // caller can use nextLine() as a while loop condition

    m_currentLine = -1;
    m_currentSection = -1;
}

bool RenderData::nextLine()
{
    if (m_sections.size() == 0)
        return false;

    if (m_currentLine == -1)
    {
        m_currentLine = m_sections[0].line;
        m_currentSection = 0;

        return true;
    }

    while (m_currentSection < m_sections.size() &&
           m_sections[m_currentSection].line == m_currentLine)
    {
        ++m_currentSection;
    }

    if (m_currentSection >= m_sections.size())
        return false;

    m_currentLine = m_sections[m_currentSection].line;
    return true;
}

bool RenderData::next(Section *out)
{
    Q_ASSERT(m_sections.size() > 0);

    if (m_currentSection >= m_sections.size())
        return false;

    if (m_sections[m_currentSection].line == m_currentLine)
    {
        *out = m_sections[m_currentSection];
        ++m_currentSection;

        return true;
    }

    return false;
}

void RenderData::end()
{
    m_currentLine = -1;
    m_currentSection = -1;
}

