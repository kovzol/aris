#include "HighlighterBridge.h"
#include <QAbstractTextDocumentLayout>

HighlighterBridge::HighlighterBridge()
{
}

void HighlighterBridge::setDocument(QQuickTextDocument* doc)
{
    if (m_doc == doc)
        return;
    if (m_doc != nullptr)
        disconnect(m_doc, &QQuickTextDocument::textDocumentChanged, this, &HighlighterBridge::handleTextDocumentChanged);
    m_doc = doc;
    if (m_doc == nullptr)
        return;
    m_highlighter.setDocument(m_doc->textDocument());
    connect(m_doc, &QQuickTextDocument::textDocumentChanged, this, &HighlighterBridge::handleTextDocumentChanged);
}

void HighlighterBridge::handleTextDocumentChanged()
{
    m_highlighter.setDocument(m_doc->textDocument());
}

int HighlighterBridge::cursorPosition() const
{
    return m_highlighter.cursorPosition();
}

void HighlighterBridge::resetCursorPosition()
{
    m_highlighter.setCursorPosition(Highlighter::NPOS);
}

int HighlighterBridge::hitTest(qreal x, qreal y)
{
    return m_doc->textDocument()->documentLayout()->hitTest({x, y}, Qt::FuzzyHit) - 1;
}

void HighlighterBridge::highlight(int start, int count)
{
    m_highlighter.setHighlight(start, count);
}

void HighlighterBridge::unhighlight()
{
    m_highlighter.setHighlight(Highlighter::NPOS);
}

void HighlighterBridge::setCursorPosition(int newCursorPosition)
{
    m_highlighter.setCursorPosition(newCursorPosition);
}
