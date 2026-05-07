#include "ProofLineHighlighter.h"
#include <QAbstractTextDocumentLayout>

ProofLineHighlighter::ProofLineHighlighter()
{
}

void ProofLineHighlighter::setDocument(QQuickTextDocument* doc)
{
    if (m_doc == doc)
        return;
    if (m_doc != nullptr)
        disconnect(m_doc, &QQuickTextDocument::textDocumentChanged, this, &ProofLineHighlighter::handleTextDocumentChanged);
    m_doc = doc;
    if (m_doc == nullptr)
        return;
    m_highlighter.setDocument(m_doc->textDocument());
    connect(m_doc, &QQuickTextDocument::textDocumentChanged, this, &ProofLineHighlighter::handleTextDocumentChanged);
}

void ProofLineHighlighter::handleTextDocumentChanged()
{
    m_highlighter.setDocument(m_doc->textDocument());
}

int ProofLineHighlighter::cursorPosition() const
{
    return m_highlighter.cursorPosition();
}

void ProofLineHighlighter::resetCursorPosition()
{
    m_highlighter.setCursorPosition(ProofLineSyntaxHighlighter::DEFAULT_POSITION);
}

int ProofLineHighlighter::setMousePosition(float x, float y)
{
    int pos = m_doc->textDocument()->documentLayout()->hitTest({x, y}, Qt::FuzzyHit) - 1;
    m_highlighter.setMousePosition(pos);
    return pos;
}

void ProofLineHighlighter::setCursorPosition(int newCursorPosition)
{
    m_highlighter.setCursorPosition(newCursorPosition);
}
