#ifndef PROOFLINESYNTAXHIGHLIGHTER_H
#define PROOFLINESYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>

class ProofLineSyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
    Q_PROPERTY(int cursorPosition READ cursorPosition WRITE setCursorPosition NOTIFY cursorPositionChanged)
    Q_PROPERTY(int mousePosition READ mousePosition WRITE setMousePosition NOTIFY mousePositionChanged)
public:
    static const int DEFAULT_POSITION;

    ProofLineSyntaxHighlighter(QTextDocument *parent);

    int cursorPosition() const;
    void setCursorPosition(int newCursorPosition);

    int mousePosition() const;
    void setMousePosition(int newMousePosition);

signals:
    void cursorPositionChanged();

    void mousePositionChanged();

protected:
    void highlightBlock(const QString &text) override;

private:
    int m_cursorPosition = DEFAULT_POSITION;
    int m_mousePosition = DEFAULT_POSITION;
};

#endif // PROOFLINESYNTAXHIGHLIGHTER_H
