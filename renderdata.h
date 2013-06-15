#ifndef RENDERDATA_H
#define RENDERDATA_H

#include <QString>
#include <QVector>

/** Object containing the data needed to render a History buffer's visible data
 *
 *  Used as a temporary object handed off from History::renderData() to 
 *  TerminalWidget::paintEvent()
 *
 *  Usage:
 *
 *      RenderData rd = my_getHistoryBuffer().renderData();
 *
 *      rd.begin();
 *      while (rd.nextLine())
 *      {
 *          RenderData::Section section;
 *
 *          while (rd.next(&section))
 *          {
 *              my_renderSection(section);
 *          }
 *
 *          my_moveToNextLine();
 *      }
 *      rd.end();
 */
class RenderData
{
public:
    /** A section of a line.
     *
     *  Each line has one or more sections. A single line is split into
     *  multiple sections when the colors change
     */
    struct Section
    {
        int     line;       // The row index of the line this sections occurs
        QString data;       // The textual contents of this section

        int     foreground; // Color palette index of the foreground color
        int     background; // Color palette index of the background color
    };

    RenderData(const QVector<Section> &sections);
    ~RenderData();

    /** Methods for traversing the render data.
     *  See the usage section of this class's doc comment
     */

    void begin();
    bool nextLine();
    bool next(Section *out);
    void end();

private:
    QVector<Section>    m_sections;
    int                 m_currentLine;
    int                 m_currentSection;
};

#endif
