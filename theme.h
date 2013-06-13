#ifndef THEME_H
#define THEME_H

#include <QColor>
#include <QVector>

/** A serializable color palette */
class Theme
{
public:
    Theme();
    ~Theme();

    /** Gets or sets the color at the given index */
    QColor color(int index) const;
    void setColor(int index, const QColor &c);

    // TODO other things like theme names and file paths
    //      will be added when we support theme options

private:
    /** Gives this theme the default colors, by loading the xterm-256 palette.
     *  See http://commons.wikimedia.org/wiki/File:Xterm_color_chart.png
     *
     *  Used as a fallback if no theme file is available or there was a parse
     *  error
     */
    void setDefault();

    /** Contains the actual color data */
    QVector<QColor> m_palette;
};

#endif
