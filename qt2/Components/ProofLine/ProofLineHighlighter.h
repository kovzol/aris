#ifndef DOCUMENTHIGHLIGHTER_H
#define DOCUMENTHIGHLIGHTER_H

#include <QQuickItem>
#include <QQuickTextDocument>
#include "ProofLineSyntaxHighlighter.h"

class ProofLineHighlighter : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QQuickTextDocument* document MEMBER m_doc WRITE setDocument REQUIRED)
    Q_PROPERTY(int cursorPosition READ cursorPosition WRITE setCursorPosition REQUIRED)
    QML_ELEMENT
public:
    ProofLineHighlighter();
    void setDocument(QQuickTextDocument* doc);
    void setCursorPosition(int newCursorPosition);
    int cursorPosition() const;

public slots:
    void resetCursorPosition();
    int setMousePosition(float x, float y);

private:
    void handleTextDocumentChanged();

    QQuickTextDocument* m_doc = nullptr;
    ProofLineSyntaxHighlighter m_highlighter { nullptr };
};

#endif // DOCUMENTHIGHLIGHTER_H
