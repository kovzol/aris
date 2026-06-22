#pragma once

#include <QSyntaxHighlighter>

class Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT
    Q_PROPERTY(int cursorPosition READ cursorPosition WRITE setCursorPosition NOTIFY cursorPositionChanged)

public:
    static const int NPOS;

    explicit Highlighter(QObject *parent);
    explicit Highlighter(QTextDocument *parent);

    int cursorPosition() const;
    void setCursorPosition(int newCursorPosition);
    void setHighlight(int start, int count = 1);

signals:
    void cursorPositionChanged();
    void highlightChanged();

protected:
    void highlightBlock(const QString &text) override;

private:
    int m_cursorPosition = NPOS;
    int m_highlightStart = NPOS;
    int m_highlightCount = 0;
};
