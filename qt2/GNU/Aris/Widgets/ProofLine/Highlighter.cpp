#include "Highlighter.h"
#include <QRegularExpression>
#include <QStack>

const int Highlighter::NPOS = -1;

Highlighter::Highlighter(QObject *parent)
    : QSyntaxHighlighter(parent)
{
    connect(this, &Highlighter::cursorPositionChanged, this, &Highlighter::rehighlight);
    connect(this, &Highlighter::highlightChanged, this, &Highlighter::rehighlight);
}

Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    connect(this, &Highlighter::cursorPositionChanged, this, &Highlighter::rehighlight);
    connect(this, &Highlighter::highlightChanged, this, &Highlighter::rehighlight);
}

void Highlighter::highlightBlock(const QString &text) {
    QTextCharFormat defaultFormat;
    setFormat(0, text.length(), defaultFormat);

    QTextCharFormat highlightFormat;
    QColor glassBlue(0x26,0x75,0xBF,75); //TODO Parameterize
    highlightFormat.setBackground(glassBlue);
    if (m_highlightStart != NPOS)
        setFormat(m_highlightStart, m_highlightCount, highlightFormat);

    QTextCharFormat parenthesisCursorMatchFormat;
    parenthesisCursorMatchFormat.setFontWeight(QFont::Bold);
    parenthesisCursorMatchFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);

    QTextCharFormat excessParenthesesFormat;
    excessParenthesesFormat.setForeground(Qt::red);

    QVector<QPair<int, int>> pairs;
    QStack<int> stack;

    for (int i = 0; i < text.length(); ++i) {
        if (text[i] == '(') {
            stack.push(i);
        } else if (text[i] == ')') {
            if (stack.isEmpty()) {
                setFormat(i, 1, excessParenthesesFormat);
            } else {
                pairs.append(qMakePair(stack.pop(), i));
            }
        }
    }

    while (!stack.isEmpty()) {
        setFormat(stack.pop(), 1, excessParenthesesFormat);
    }

    for (const auto &pair : pairs) {
        if (m_cursorPosition == pair.first || m_cursorPosition == pair.second) {
            setFormat(pair.first, 1, parenthesisCursorMatchFormat);
            setFormat(pair.second, 1, parenthesisCursorMatchFormat);
        }
    }
}

int Highlighter::cursorPosition() const
{
    return m_cursorPosition;
}

void Highlighter::setCursorPosition(int newCursorPosition)
{
    if (m_cursorPosition == newCursorPosition)
        return;
    m_cursorPosition = newCursorPosition;
    emit cursorPositionChanged();
}

void Highlighter::setHighlight(int start, int count)
{
    if (m_highlightStart == start && m_highlightCount == count)
        return;
    m_highlightStart = start;
    m_highlightCount = count;
    emit highlightChanged();
}
