#pragma once

#include <QQuickItem>
#include <QQuickTextDocument>
#include <QtQml/qqmlregistration.h>
#include "Highlighter.h"

class HighlighterBridge : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QQuickTextDocument* document MEMBER m_doc WRITE setDocument REQUIRED)
    Q_PROPERTY(int cursorPosition READ cursorPosition WRITE setCursorPosition REQUIRED)
    QML_ELEMENT

public:
    HighlighterBridge();

    void setDocument(QQuickTextDocument* doc);

    void setCursorPosition(int newCursorPosition);
    int cursorPosition() const;

public slots:
    void resetCursorPosition();
    int hitTest(qreal x, qreal y);
    void highlight(int start, int count = 1);
    void unhighlight();

private:
    void handleTextDocumentChanged();

    QQuickTextDocument* m_doc = nullptr;
    Highlighter m_highlighter { this };
    int m_cursorPosition;
};
