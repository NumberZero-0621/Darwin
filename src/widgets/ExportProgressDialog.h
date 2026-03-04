#ifndef EXPORTPROGRESSDIALOG_H
#define EXPORTPROGRESSDIALOG_H

#include <QDialog>
#include <QPropertyAnimation>

class ExportProgressDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(double animatedProgress READ animatedProgress WRITE setAnimatedProgress)

public:
    explicit ExportProgressDialog(const QString& infoText, QWidget* parent = nullptr);
    ~ExportProgressDialog() override;

    void setProgress(double progress);   // 0.0 to 1.0
    void setInfoText(const QString& text);

    double animatedProgress() const { return m_animatedProgress; }
    void setAnimatedProgress(double p);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    QPoint m_dragPosition;
    double m_progress;           // target progress
    double m_animatedProgress;   // current animated progress for drawing
    QString m_infoText;
    QPropertyAnimation* m_anim;

    void drawPaperAndLines(QPainter& p, const QRectF& paperRect) const;
    void drawPen(QPainter& p, const QPointF& tipPos) const;
};

#endif // EXPORTPROGRESSDIALOG_H
