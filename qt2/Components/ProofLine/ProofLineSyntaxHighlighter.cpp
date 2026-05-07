#include "ProofLineSyntaxHighlighter.h"
#include <QRegularExpression>
#include <QStack>

const int ProofLineSyntaxHighlighter::DEFAULT_POSITION = -1;

ProofLineSyntaxHighlighter::ProofLineSyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    connect(this, &ProofLineSyntaxHighlighter::cursorPositionChanged, this, &ProofLineSyntaxHighlighter::rehighlight);
    connect(this, &ProofLineSyntaxHighlighter::mousePositionChanged, this, &ProofLineSyntaxHighlighter::rehighlight);
}

void ProofLineSyntaxHighlighter::highlightBlock(const QString &text) {
    QTextCharFormat defaultFormat;
    setFormat(0, text.length(), defaultFormat);

    QTextCharFormat variableFormat;
    variableFormat.setFontItalic(false); //TODO

    QTextCharFormat highlightFormat;
    QColor glassBlue(0x26,0x75,0xBF,75);
    highlightFormat.setBackground(glassBlue);
    if (m_mousePosition != DEFAULT_POSITION && m_mousePosition < text.length() && text[m_mousePosition] == "→")
        setFormat(m_mousePosition, 1, highlightFormat);

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
        } else if (text[i].isLetter()) {
            setFormat(i, 1, variableFormat);
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

int ProofLineSyntaxHighlighter::cursorPosition() const
{
    return m_cursorPosition;
}

void ProofLineSyntaxHighlighter::setCursorPosition(int newCursorPosition)
{
    if (m_cursorPosition == newCursorPosition)
        return;
    m_cursorPosition = newCursorPosition;
    emit cursorPositionChanged();
}

int ProofLineSyntaxHighlighter::mousePosition() const
{
    return m_mousePosition;
}

void ProofLineSyntaxHighlighter::setMousePosition(int newMousePosition)
{
    if (m_mousePosition == newMousePosition)
        return;
    m_mousePosition = newMousePosition;
    emit mousePositionChanged();
}
